#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <climits>

#include <omp.h>
#include <hdf5.h>
 
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "LineReader.h"
#include "ParamReader.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"
#include "CellSampler.h"
#include "AgentYMTCollector.h"
#include "YMTHDFWriter.h"

#define MAX_NAME  256

const char DEF_SPC[] = "sapiens";
#define DEF_STEP     65000
#define DEF_QDF_PAT  "ooa_pop-sapiens_SG_%06d.qdf"

typedef std::map<std::string, std::vector<float>>    locvalvec;
typedef std::map<std::string, std::vector<uint>>     locselidx;
typedef std::pair<uint, std::vector<float>>          statitems;
typedef std::map<std::string, statitems>             statmap;

//----------------------------------------------------------------------------
//  usage
//
void usage(const char *pApp) {
    stdfprintf(stderr, "%s - collect agent-id, cell-id and item value from a population's AgentDataSet\n", pApp);
    stdfprintf(stderr, "Creates an HDF file containing the list, as well as a\n"); 
    stdfprintf(stderr, "CSV file containing number, min-val, max-val and median for each node.\n");
    stdfprintf(stderr, "usage:\n");
    stdfprintf(stderr, "  %s  -p '<pop-dir-pattern>:<species>'  -g <geo-qdf>\n", pApp);
    stdfprintf(stderr, "      -l '<location-file> -o <out-body>] [-h] [-v]\n");
    stdfprintf(stderr, "where\n");
    stdfprintf(stderr, "  geo-qdf          a QDF with Geography and Grid\n");
    stdfprintf(stderr, "  pop-dir-pattern  a pattern for directories containing qdf (only one '*' allowed)\n");
    stdfprintf(stderr, "  species          the species name for which to do the analysis (if omitted, 'sapiens' is used\n");
    stdfprintf(stderr, "  item-name        the name of the agents hybrdidization value\n");
    stdfprintf(stderr, "  item-type        the data type of the item ('byte', 'int', 'long', 'float', 'double')\n");
    stdfprintf(stderr, "  location-file    a text wile containing sampling information. Consists of liens of the form\n");
    stdfprintf(stderr, "                     <name> <lon> <lat> <radius> <num>\n");
    stdfprintf(stderr, "  -h               prepend CSV style headerline to output\n");
    stdfprintf(stderr, "Example:\n");
    stdfprintf(stderr, "  %s -p '/data/batchA_00*:sapiens' -n PheneticHyb:float -o testing\n", pApp);
    stdfprintf(stderr, "Will create output files testing.hdf and testing.csv.\n");
    stdfprintf(stderr, "Assumes the existence of files ooa_pop-<species>_SG_00[01]000.qdf in /data/batchA_00*\n");
    stdfprintf(stderr, "\n");
}

//----------------------------------------------------------------------------
//  calcLandWater
//    assumes: pPopQDF has geo group; cell ids == cell indexes
//
int calcLandWater(const char *pPopQDF, loc_cells &mSelected, loc_landwater &mLandWater) {
    int iResult = 0;

    QDFArray *pQA = QDFArray::create(pPopQDF);
    if (pQA != NULL) {
        iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_ALTITUDE);
        double *pdAlt = NULL;
        if (iResult == 0) {
            uint iNumCells = pQA->getSize();
            pdAlt = new double[iNumCells];
            uint iCount = pQA->getFirstSlab(pdAlt, iNumCells);
            if (iCount == iNumCells) {
                stdfprintf(stderr, "[calcLandWater] Read %d Altitudes\n", iCount);
                iResult = 0;
            } else {
                iResult = -1;
                stdfprintf(stderr, "[calcLandWater] Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", pPopQDF, GEOGROUP_NAME, GEO_DS_ALTITUDE, iCount, iNumCells);
            }
        } else {
            iResult = -1;
            stdfprintf(stderr, "[calcLandWater] Couldn't open data set [%s]\n", GEO_DS_ALTITUDE);
        }
        pQA->closeArray();
    
        if (iResult == 0) {
            mLandWater.clear();

            loc_cells::const_iterator it;
            for (it = mSelected.begin(); it != mSelected.end(); ++it)  {
                int iLand  = 0;
                int iWater = 0;
                for (uint k = 0; k < it->second.size(); k++) {
                    if (pdAlt[it->second[k]] < 0) {
                        iWater ++;
                    } else {
                        iLand++;
                    } 
                }
                mLandWater[it->first] = std::pair<int,int>(iLand, iWater);
            }
        }
    
        if (pdAlt != NULL) {;
            delete[] pdAlt;
        }
        delete pQA;;
    } else {
        iResult = -1;
        stdfprintf(stderr, "[calcLandWater] Couldn't create QDFArray\n");
  
    }
    return iResult;
}


//----------------------------------------------------------------------------
//  getDirName
//
int getDirName(const char *pPat, char *pMatch, char *pName) {
    int iResult = -1;
    char *sPat = new char[strlen(pPat)+1];
    strcpy(sPat, pPat);
    char *sMatch = new char[strlen(pMatch)+1];
    strcpy(sMatch, pMatch);
    stringvec vPatParts;
    stringvec vMatchParts;
    char *p = strtok(sPat, "/");
    while (p!= NULL) {
        vPatParts.push_back(p);
        p = strtok(NULL, "/");
    }

    p = strtok(sMatch, "/");
    while (p!= NULL) {
        vMatchParts.push_back(p);
        p = strtok(NULL, "/");
    }
        
    if (vPatParts.size() == vMatchParts.size()) { 
        bool bEqual = true;
        for (uint i = 0; (iResult <0) && (i < vPatParts.size()); i++) {
            if (vPatParts[i] == vMatchParts[i]) {
                // that's ok
            } else if (strchr(vPatParts[i].c_str(), '*') != NULL) {
                strcpy(pName, vMatchParts[i].c_str());
                iResult = 0;
                bEqual = false;
            } else {
                stdfprintf(stderr, "[getDirName] Part mismatch   [%s] != [%s]\n", vPatParts[i].c_str(), vMatchParts[i].c_str());
                bEqual = false;
            }
        }
        if (bEqual) {
            strcpy(pName, vMatchParts.back().c_str());
            iResult = 0;
        }
    } else {
        stdfprintf(stderr, "[getDirName] number of parts don't match: %zd != %zd\n",vPatParts.size(), vMatchParts.size());
    }

    delete[] sPat;
    delete[] sMatch;
    return iResult;
}


//----------------------------------------------------------------------------
//  main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        char *sLocationsFile = NULL;
        char *sGeoQDF        = NULL;
        char *sPopQDF        = NULL;
        char *sOutbody       = NULL;
        const char *pSpecies       = NULL;
        bool bVerbose   = false;

        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(5,
                                   "-p:S!",     &sPopQDF,
                                   "-g:S",      &sGeoQDF,
                                   "-l:S!",     &sLocationsFile,
                                   "-o:S",      &sOutbody,
                                   "-v:0",      &bVerbose);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
                
            if (iResult >= 0) {

                if (iResult == 0) {
                    stdfprintf(stderr, "popqdf: [%s]\n", sPopQDF);
                    // separate species from pattern
                    char *pSpc = strchr(sPopQDF, ':');
                    if (pSpc != NULL) {
                        *pSpc++ = '\0';
                        pSpecies = pSpc;
                    } else {
                        stdfprintf(stderr, "No Species given - %s is used\n", DEF_SPC);
                        pSpecies = DEF_SPC;
                    }

                }

                
                //@@glob_t gt;
                //@@iResult = glob(sPopQDFPat, 0, NULL, &gt);
                //int iNumDirs = gt.gl_pathc
                //int iNumDirs = 1;

                if (iResult == 0) {

                    // create output names
                    char *sOutNameHDF = new char[strlen(sOutbody)+5]; // ".hdf"+NUL
                    //                    char *sOutNameCSV = new char[strlen(sOutbody)+5]; // ".csv"+NUL
                    sprintf(sOutNameHDF, "%s.hdf", sOutbody);
                    //                    sprintf(sOutNameCSV, "%s.csv", sOutbody);

                    stringvec vNames;
                    loc_cells mSelected;
                    CellSampler *pCS = CellSampler::createInstance(sGeoQDF, sLocationsFile, 0);
                    if (pCS != NULL) {
                        
                        mSelected = pCS->getSelected();
                        vNames = pCS->getNames();
                        
                        stdfprintf(stderr, "CellSampler found %zd regions\n", mSelected.size());
                        if (bVerbose) {
                            pCS->showSelected(stderr, 10);
                        }
                        delete pCS;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "Couldn't create CellSampler\n");
                    }

                    //if (fOutCSV != NULL) {
                        YMTHDFWriter *pYHW = YMTHDFWriter::createInstance(sOutNameHDF, vNames.size());
                        if (pYHW != NULL)  {
                            // loop through directories
                            // get qdf file or determine if we have a die out
                            bool bDiedOut = false;

                            if (iResult == 0) {

                                stringvec vParts;
                                splitString(sPopQDF, vParts, "/");
                                const char *pSimName = vParts[vParts.size()-2].c_str();

                                stdfprintf(stderr, "sPopDir [%s], pSimName [%s]\n", sPopQDF, pSimName);
                                //                                locvalvec mLocHybs;
                                loc_landwater mLandWater;
                                if (bDiedOut) {
                                    stdfprintf(stderr, "doing diedout outputs\n"); fflush(stderr);
                                    pYHW->writeDataRegionized(pSimName, -1, -1, NULL, 0, mSelected, mLandWater);
                                    //iResult = addCSVLine(fOutCSV, pSimName, -1, 0, mLocHybs);
                                } else {
                                    stdfprintf(stderr, "doing normal outputs\n"); fflush(stderr);
                                    if (sGeoQDF == NULL) {
                                        sGeoQDF = sPopQDF;
                                    }
                                    
                                    AgentYMTCollector *pAYC = AgentYMTCollector::createInstance(sPopQDF, pSpecies);
                                    if (pAYC != NULL) {
                                            
                                        uchar *pData       = pAYC->getArray();
                                        uint   iNumAgs     = pAYC->getNumValues();
                                        int    iCurStep    = pAYC->getCurStep();
                                        float  fStartTime  = pAYC->getStartTime();

                                        iResult = calcLandWater(sPopQDF, mSelected, mLandWater);
                                        stdfprintf(stderr, "AgentYMTCollector found %u values\n", iNumAgs);

                                        std::vector<uint> vSelIndexes;
                                        //                                        structureData(pData, iNumAgs, mSelected, mLocHybs);
                                        iResult = pYHW->writeDataRegionized(pSimName, iCurStep, fStartTime, pData, iNumAgs, mSelected, mLandWater);
                                        if (iResult == 0) {
                                            //iResult = addCSVLine(fOutCSV, sSimName, pData, iNumAgs, mSelected);
                                            //                                            iResult = addCSVLine(fOutCSV, pSimName,  iCurStep, iNumAgs, mLocHybs);
                                            if (iResult == 0) {
                                                stdfprintf(stderr, "+++ success for [%s] +++\n", sPopQDF);
                                            } else {
                                                stdfprintf(stderr, "couldn't add csv line\n");
                                            }
                                        } else {
                                            iResult = -1;
                                            stdfprintf(stderr, "couldn't write data to HDF file\n");
                                        }
                                        delete pAYC;
                                    } else {
                                        iResult = -1;
                                        stdfprintf(stderr, "Couldn't create AgentItemCollector\n");
                                    }
                                }
                            }

                            delete pYHW;
                        } else {
                            iResult = -1;
                            stdfprintf(stderr, "couldn't create HDF file [%s]\n", sOutNameHDF);
                        }
                        //                        fclose(fOutCSV);
                        //                    } else {
                        //                        iResult = -1;
                        //                        stdfprintf(stderr, "couldn't open [%s] for writing\n", sOutNameCSV);
                        //                    }
                //                    delete[] sOutNameCSV;
                    delete[] sOutNameHDF;
                } else {
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            stdfprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        stdfprintf(stderr, "Couldn't create ParamReader\n");
    }

    return iResult;
}
