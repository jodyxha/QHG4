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
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "LineReader.h"
#include "ParamReader.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"
#include "CellSampler.h"
#include "AgentItemCollector.h"
#include "HybHDFWriter.h"

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
    fprintf(stderr, "%s - collect agent-id, cell-id and item value from a population's AgentDataSet\n", pApp);
    fprintf(stderr, "Creates an HDF file containing the list, as well as a\n"); 
    fprintf(stderr, "CSV file containing number, min-val, max-val and median for each node.\n");
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s  -p '<pop-dir-pattern>:<species>'  -g <geo-qdf> -n <item-name>:<item-type>\n", pApp);
    fprintf(stderr, "      -l '<location-file> -o <out-body>] [-h] [-v]\n");
    fprintf(stderr, "where\n");
    fprintf(stderr, "  geo-qdf          a QDF with Geography and Grid\n");
    fprintf(stderr, "  pop-dir-pattern  a pattern for directories containing qdf (only one '*' allowed)\n");
    fprintf(stderr, "  species          the species name for which to do the analysis (if omitted, 'sapiens' is used\n");
    fprintf(stderr, "  item-name        the name of the agents hybrdidization value\n");
    fprintf(stderr, "  item-type        the data type of the item ('byte', 'int', 'long', 'float', 'double')\n");
    fprintf(stderr, "  location-file    a text wile containing sampling information. Consists of liens of the form\n");
    fprintf(stderr, "                     <name> <lon> <lat> <radius> <num>\n");
    fprintf(stderr, "  -h               prepend CSV style headerline to output\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s -p '/data/batchA_00*:sapiens' -n PheneticHyb:float -o testing\n", pApp);
    fprintf(stderr, "Will create output files testing.hdf and testing.csv.\n");
    fprintf(stderr, "Assumes the existence of files ooa_pop-<species>_SG_00[01]000.qdf in /data/batchA_00*\n");
    fprintf(stderr, "\n");
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
                xha_fprintf(stderr, "[calcLandWater] Read %d Altitudes\n", iCount);
                iResult = 0;
            } else {
                iResult = -1;
                xha_fprintf(stderr, "[calcLandWater] Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", pPopQDF, GEOGROUP_NAME, GEO_DS_ALTITUDE, iCount, iNumCells);
            }
        } else {
            iResult = -1;
            xha_fprintf(stderr, "[calcLandWater] Couldn't open data set [%s]\n", GEO_DS_ALTITUDE);
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
        xha_fprintf(stderr, "[calcLandWater] Couldn't create QDFArray\n");
  
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
                fprintf(stderr, "[getDirName] Part mismatch   [%s] != [%s]\n", vPatParts[i].c_str(), vMatchParts[i].c_str());
                bEqual = false;
            }
        }
        if (bEqual) {
            strcpy(pName, vMatchParts.back().c_str());
            iResult = 0;
        }
    } else {
        fprintf(stderr, "[getDirName] number of parts don't match: %zd != %zd\n",vPatParts.size(), vMatchParts.size());
    }

    delete[] sPat;
    delete[] sMatch;
    return iResult;
}


//----------------------------------------------------------------------------
//  addCSVLine
//
//int addCSVLine(FILE *fOutCSV, const char *pName, uchar *pData, uint iNumAgs, const loc_cells &mSelected) {
int addCSVLine(FILE *fOutCSV, const char *pName, int iCurStep, uint iNumAgs, const locvalvec &mLocHybs) {
    int iResult = 0;

    
    

    // mStats will contain numags, min, max, and median for all cells
    statmap mStats;
    // make sure mStats has an entry for every cell in the region 
    locvalvec::const_iterator itLoc;
    for (itLoc = mLocHybs.begin(); itLoc != mLocHybs.end(); ++itLoc)  {
        std::vector<float> vDummy;
        vDummy.clear();
        mStats[itLoc->first] = statitems(0, vDummy);
    }

    // prepare output line
    // space for name + ";"+162 times "<val>;<val>;<val>;<val>;"
    char *sLine = new char[MAX_NAME+1+mLocHybs.size()*(64+1)+1];
    sprintf(sLine, "%s;%d;", pName, iCurStep);

    if (iNumAgs > 0) {
        //@@@@ here we loop over mLocHybs to do the stats
        xha_fprintf(stderr, "[addCSVLine] adding data for %zd regions\n", mLocHybs.size());
        locvalvec::const_iterator itLoc2;
        for (itLoc2 = mLocHybs.begin(); itLoc2 != mLocHybs.end(); ++itLoc2)  {
            // make sorted version to find median
            std::vector<float> vHybs(itLoc2->second);
            std::sort(vHybs.begin(), vHybs.end());
 
            // calculate min, max,median for all reguions
            float fMin=  2;
            float fMax=  0;
            float fMedian = -1;
            if (vHybs.size() > 0) {

                uint iMid = (vHybs.size())/2;

                fMedian = vHybs[iMid];
                for (uint j = 0; j < vHybs.size(); j++) {
                    if (vHybs[j] < fMin) {
                        fMin = vHybs[j];
                    }
                    if (vHybs[j] > fMax) {
                        fMax = vHybs[j];
                    }
                }

            } else {
                fMin = -1;
                fMax = -1;
                fMedian = -1;
            }

            std::vector<float> &vStats=mStats[itLoc2->first].second;
            vStats.push_back(fMin);
            vStats.push_back(fMax);
            vStats.push_back(fMedian);
            mStats[itLoc2->first].first = vHybs.size();
        }
        xha_fprintf(stderr, "[addCSVLine] Stats collected\n");

        // loop mstats! 
        statmap::const_iterator itStat;

        for (itStat = mStats.begin(); itStat != mStats.end(); ++itStat) {
            char sNum[32];
            // num
            sprintf(sNum, "%u;",   itStat->second.first);
            strcat(sLine, sNum);
            if  (itStat->second.first > 0) {
                // min
                sprintf(sNum, "%.6f;", itStat->second.second[0]);
                strcat(sLine, sNum);
                // max
                sprintf(sNum, "%.6f;", itStat->second.second[1]);
                strcat(sLine, sNum);
                // median
                sprintf(sNum, "%.6f;", itStat->second.second[2]);
                strcat(sLine, sNum);
            } else {
                strcat(sLine, "-1;-1;-1;");
            }
        }
    } else {
        // loop mstats! 
        statmap::const_iterator itStat;
        
        for (itStat = mStats.begin(); itStat != mStats.end(); ++itStat) {
            strcat(sLine, "0;-1;-1;-1;");
        }
    }
  
    fprintf(fOutCSV, "%s\n", sLine);
    fprintf(stderr, "[addCSVLine] one line written\n");
    delete[] sLine;
    return iResult;
}


//----------------------------------------------------------------------------
//  openCSV
//    space for <name> + ';' + 162*("count_"+<num>+";min_"+<num>+";max_"+<num>+";median_"+<num>+";"
//
FILE *openCSV(const std::string sName, loc_cells &mSelected, bool bCSVHeader) {
    bool bNew = true;
    struct stat statbuf;
    int iEx = stat(sName.c_str(), &statbuf);
    bNew = (iEx != 0);

    FILE *fCSV = fopen(sName.c_str(), "at");
    if (fCSV != NULL) {
        if (bNew && bCSVHeader)  {
            // doesn't exist yet: writa header
            uint iLen = 9; //  "sim_name"
            loc_cells::const_iterator it;
            for (it = mSelected.begin(); it != mSelected.end(); ++it)  {
                
                iLen += 4*strlen(it->first.c_str())+7+5+5+8; 
            }

            // write header line
            std::string sHeader{"sim_name;step"};
            for (it = mSelected.begin(); it != mSelected.end(); ++it)  {
                const char *pName = it->first.c_str();
                sHeader += xha_sprintf(";%s_count;%s_min;%s_max;%s_median;", pName, pName, pName, pName);

            }
            fprintf(fCSV, "%s\n", sHeader.c_str());
            
        }
    } else {
        xha_fprintf(stderr, "[openCSV] Couldn't open [%s] for writng\n", sName);
    }
    return fCSV;
}



//----------------------------------------------------------------------------
//  structureData
//    here we are assuming that we are looking at a float item like hybridization
//
int structureData(uchar *pData, uint iNumAgs,  const loc_cells &mSelected, locvalvec& mLocHybs){

    // store item values by cell id
    xha_fprintf(stderr, "[structurData] filling %u agent items to map\n", iNumAgs);
    std::map<int, std::vector<float>> mAllHybs;

    aginfo_float ai;
    uchar *p = pData;
    for (uint i = 0; i < iNumAgs; i++) {
        memcpy(&ai, p, sizeof(aginfo_float));
        p += sizeof(aginfo_float);
        mAllHybs[ai.m_ulCellID].push_back(ai.m_tItem);
    }
    
    uint iV = 0;
    loc_cells::const_iterator itSel;
    for (itSel = mSelected.begin(); itSel != mSelected.end(); ++itSel)  {
        std::vector<float> vHybVals;
        const std::vector<int> v = itSel->second;
        for (uint j = 0; j < v.size(); j++) {
            vHybVals.insert(vHybVals.end(), mAllHybs[v[j]].begin(), mAllHybs[v[j]].end());
        }
        iV += vHybVals.size();
        mLocHybs[itSel->first] = vHybVals;
    }
    xha_fprintf(stderr, "[structurData] selected total of %u\n", iV);
    return 0;
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
        char *sItemName      = NULL;
        char *sOutbody       = NULL;
        char *pDataType      = NULL;
        //int iTimeStep        = DEF_STEP;
        const char *pSpecies       = NULL;
        bool bCSVHeader = false;
        bool bVerbose   = false;

        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(7,
                                   "-p:S!",     &sPopQDF,
                                   "-g:S",      &sGeoQDF,
                                   "-n:S!",     &sItemName,
                                   "-l:S!",     &sLocationsFile,
                                   "-o:S",      &sOutbody,
                                   "-v:0",      &bVerbose,
                                   "-h:0",      &bCSVHeader);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
                
            if (iResult >= 0) {

                if (iResult == 0) {
                    // separate species from pattern
                    char *pSpc = strchr(sPopQDF, ':');
                    if (pSpc != NULL) {
                        *pSpc++ = '\0';
                        pSpecies = pSpc;
                    } else {
                        xha_fprintf(stderr, "No Species given - %s is used\n", DEF_SPC);
                        pSpecies = DEF_SPC;
                    }

                }

                if (iResult == 0) {
                    // separate item type from item name
                    pDataType = strchr(sItemName, ':');
                    if (pDataType != NULL) {
                        *pDataType++ = '\0';
                    } else {
                        xha_fprintf(stderr, "expected ':' followed by datatype\n");
                        iResult = -1;
                    }
                }
                
                //@@glob_t gt;
                //@@iResult = glob(sPopQDFPat, 0, NULL, &gt);
                //int iNumDirs = gt.gl_pathc
                //int iNumDirs = 1;

                if (iResult == 0) {

                    // create output names
                    std::string sOutNameHDF = xha_sprintf("%s.hdf", sOutbody);
                    std::string sOutNameCSV = xha_sprintf("%s.csv", sOutbody);

                    stringvec vNames;
                    loc_cells mSelected;
                    CellSampler *pCS = CellSampler::createInstance(sGeoQDF, sLocationsFile, 0);
                    if (pCS != NULL) {
                        
                        mSelected = pCS->getSelected();
                        vNames = pCS->getNames();
                        
                        xha_fprintf(stderr, "CellSampler found %zd regions\n", mSelected.size());
                        if (bVerbose) {
                            pCS->showSelected(stderr, 10);
                        }
                        delete pCS;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "Couldn't create CellSampler\n");
                    }

                    FILE *fOutCSV = openCSV(sOutNameCSV, mSelected, bCSVHeader);
                    if (fOutCSV != NULL) {
                        HybHDFWriter *pHHW = HybHDFWriter::createInstance(sOutNameHDF, vNames.size());
                        if (pHHW != NULL)  {
                            // loop through directories
                            // get qdf file or determine if we have a die out
                            bool bDiedOut = false;

                            if (iResult == 0) {

                                stringvec vParts;
                                splitString(sPopQDF, vParts, "/");
                                const char *pSimName = vParts[vParts.size()-2].c_str();

                                xha_fprintf(stderr, "sPopDir [%s], pSimName [%s]\n", sPopQDF, pSimName);
                                locvalvec mLocHybs;
                                loc_landwater mLandWater;
                                if (bDiedOut) {
                                    xha_fprintf(stderr, "doing diedout outputs\n"); fflush(stderr);
                                    pHHW->writeDataRegionized(pSimName, -1, -1, NULL, 0, mSelected, mLandWater);
                                    iResult = addCSVLine(fOutCSV, pSimName, -1, 0, mLocHybs);
                                } else {
                                    xha_fprintf(stderr, "doing normal outputs\n"); fflush(stderr);
                                    if (sGeoQDF == NULL) {
                                        sGeoQDF = sPopQDF;
                                    }
                                    
                                    AgentItemCollector *pAIC = AgentItemCollector::createInstance(sGeoQDF, sPopQDF, pSpecies, sItemName, pDataType);
                                    if (pAIC != NULL) {
                                        
                                        uchar *pData       = pAIC->getArray();
                                        uint   iNumAgs     = pAIC->getNumValues();
                                        int    iCurStep    = pAIC->getCurStep();
                                        float  fStartTime  = pAIC->getStartTime();

                                        iResult = calcLandWater(sPopQDF, mSelected, mLandWater);
                                        xha_fprintf(stderr, "AgentItemCollector found %u values\n", iNumAgs);

                                        std::vector<uint> vSelIndexes;
                                        structureData(pData, iNumAgs, mSelected, mLocHybs);
                                        iResult = pHHW->writeDataRegionized(pSimName, iCurStep, fStartTime, pData, iNumAgs, mSelected, mLandWater);
                                        if (iResult == 0) {
                                            //iResult = addCSVLine(fOutCSV, sSimName, pData, iNumAgs, mSelected);
                                            iResult = addCSVLine(fOutCSV, pSimName,  iCurStep, iNumAgs, mLocHybs);
                                            if (iResult == 0) {
                                                xha_fprintf(stderr, "+++ success for [%s] +++\n", sPopQDF);
                                            } else {
                                                xha_fprintf(stderr, "couldn't add csv line\n");
                                            }
                                        } else {
                                            iResult = -1;
                                            xha_fprintf(stderr, "couldn't write data to HDF file\n");
                                        }
                                        delete pAIC;
                                    } else {
                                        iResult = -1;
                                        xha_fprintf(stderr, "Couldn't create AgentItemCollector\n");
                                    }
                                }
                            }

                            delete pHHW;
                        } else {
                            iResult = -1;
                            xha_fprintf(stderr, "couldn't create HDF file [%s]\n", sOutNameHDF);
                        }
                        fclose(fOutCSV);
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "couldn't open [%s] for writing\n", sOutNameCSV);
                    }
                } else {
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            xha_fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        xha_fprintf(stderr, "Couldn't create ParamReader\n");
    }

    return iResult;
}
