#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <climits>

#include <glob.h>
#include <omp.h>
#include <hdf5.h>
 
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "strutils.h"
#include "xha_strutilsT.h"
#include "LineReader.h"
#include "ParamReader.h"
#include "QDFUtils.h"
#include "AgentItemCollector.h"
#include "HybHDFWriter.h"

#define NUM_CELLS 162   // for ico3.qdf
#define MAX_NAME  256

const char DEF_SPC[] = "sapiens";
#define DEF_STEP     1000
#define DEF_QDF_PAT  "ooa_pop-sapiens_SG_%06d.qdf"


//----------------------------------------------------------------------------
//  usage
//
void usage(const char *pApp) {
    xha_fprintf(stderr, "%s - collect agent-id, cell-id and item value from a population's AgentDataSet\n", pApp);
    xha_fprintf(stderr, "Creates an HDF file containing the list, as well as a\n"); 
    xha_fprintf(stderr, "CSV file containing number, min-val, max-val and median for each node.\n");
    xha_fprintf(stderr, "usage:\n");
    xha_fprintf(stderr, "  %s  -p '<pop-dir-pattern>:<species>'  -n <item-name>:<item-type> -o <out-body-csv>]\n", pApp);
    xha_fprintf(stderr, "where\n");
    xha_fprintf(stderr, "  pop-dir-pattern  a pattern for directories containing qdf (only one '*' allowed)\n");
    xha_fprintf(stderr, "  species          the species name for which to do the analysis (if omitted, 'sapiens' is used\n");
    xha_fprintf(stderr, "  item-name        the name of the agents hybrdidization value\n");
    xha_fprintf(stderr, "  item-type        the data type of the item ('byte', 'int', 'long', 'float', 'double')\n");
    xha_fprintf(stderr, "Example:\n");
    xha_fprintf(stderr, "  %s -p '/data/batchA_00*:sapiens' -n PheneticHyb:float -o testing\n", pApp);
    xha_fprintf(stderr, "Will create output files testing.hdf and testing.csv.\n");
    xha_fprintf(stderr, "Assumes the existence of files ooa_pop-<species>_SG_00[01]000.qdf in /data/batchA_00*\n");
    xha_fprintf(stderr, "\n");
}

//----------------------------------------------------------------------------
//  getDirName
//
int getDirName(const std::string sPat, std::string sMatch, std::string &sName) {
    int iResult = -1;

    stringvec vPatParts;
    stringvec vMatchParts;
    
    uint iNumP = splitString(sPat,   vPatParts,   "/");
    
    uint iNumM = splitString(sMatch, vMatchParts, "/");
    
    if (iNumP == iNumM) { 
        for (uint i = 0; (iResult <0) && (i < iNumP); i++) {
            if (vPatParts[i] == vMatchParts[i]) {
                // that's ok
            } else if (vPatParts[i].find("*") != std::string::npos) {
                sName =  vMatchParts[i];
                iResult = 0;
            } else {
                xha_fprintf(stderr, "Part mismatch   [%s] != [%s]\n", vPatParts[i], vMatchParts[i]);
            }
        }
    } else {
        xha_fprintf(stderr, "number of parts don't match: %zd != %zd\n",vPatParts.size(), vMatchParts.size());
    }

    return iResult;
}



//----------------------------------------------------------------------------
//  main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        char *sGeoQDF        = NULL;
        char *sPopQDFPat     = NULL;
        char *sItemName      = NULL;
        char *sOutbody       = NULL;
        char *pDataType      = NULL;
        const char *pSpecies       = NULL;

        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(4,
                                   "-p:S!",     &sPopQDFPat,
                                   "-g:S",      &sGeoQDF,
                                   "-n:S!",     &sItemName,
                                   "-o:S",      &sOutbody);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
                
            if (iResult >= 0) {

                if (iResult == 0) {
                    // separate species from pattern
                    char *pSpc = strchr(sPopQDFPat, ':');
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

                glob_t gt;
                iResult = glob(sPopQDFPat, 0, NULL, &gt);
                if (iResult == 0) {

                    xha_fprintf(stderr, "Matching dirs found: %zd\n", gt.gl_pathc);

                    // create output names
                    char *sOutNameHDF = new char[strlen(sOutbody)+5]; // ".hdf"+NUL
                    sprintf(sOutNameHDF, "%s.hdf", sOutbody);

                    HybHDFWriter *pHHW = HybHDFWriter::createInstance(sOutNameHDF, gt.gl_pathc);
                    if (pHHW != NULL) {
                        // loop through directories
                        for (uint i = 0; (iResult == 0) && (i < gt.gl_pathc); i++) {
                            // get qdf file or determine if we have a die out
                            int iStep = DEF_STEP;
                            bool bDiedOut = false;
                            char sPopFile[1024];
                            char sTail[256];

                            sprintf(sTail, DEF_QDF_PAT, iStep);
                            sprintf(sPopFile, "%s/%s",gt.gl_pathv[i], sTail);
                            struct stat statbuf;
                            int iResult = stat(sPopFile, &statbuf);
                            if (iResult != 0) {
                                xha_fprintf(stderr, "pop %s doesn't exist out\n", sPopFile);

                                sprintf(sTail, DEF_QDF_PAT, 0);
                                sprintf(sPopFile, "%s/%s",gt.gl_pathv[i], sTail);
                                iResult = stat(sPopFile, &statbuf);
                                if (iResult == 0)  {
                                    xha_fprintf(stderr, "pop %s does exist -> died out\n", sPopFile);
                                    bDiedOut = true;
                                }
                                iResult = 0;
                            }

                            std::string sName = "";
                            iResult = getDirName(sPopQDFPat, gt.gl_pathv[i], sName);
                            if (iResult == 0) {
                                xha_fprintf(stderr, "Found match [%s]\n", sName);
                            } else {
                                iResult = -1;
                                xha_fprintf(stderr, "Couldn't get dir name\n");
                            }


                            if (iResult == 0) {
                                if (bDiedOut) {
                                    xha_fprintf(stderr, "have a die-out\n");
                                    pHHW->writeData(sName, NULL, 0);
                                } else {
                                    xha_fprintf(stderr, "have a normal\n");
                                    if (sGeoQDF == NULL) {
                                        sGeoQDF = sPopFile;
                                    }
                                    xha_fprintf(stderr, "calling: AgentItemCollector::createInstance(%s, %s, %s, %s, %s);\n", sPopFile, sGeoQDF, pSpecies, sItemName, pDataType);
                                    AgentItemCollector *pAIC = AgentItemCollector::createInstance(sPopFile, sGeoQDF, pSpecies, sItemName, pDataType);
                                    if (pAIC != NULL) {
                                            
                                        uchar *pData = pAIC->getArray();
                                    
                                        uint iNumAgs  = pAIC->getNumValues();
                                    
                                        iResult = pHHW->writeData(sName, pData, iNumAgs);
                                        if (iResult == 0) {
                                            xha_fprintf(stderr, "+++ success for [%s] +++\n", gt.gl_pathv[i]);
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
                        }
                        delete pHHW;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "couldn't create HDF file [%s]\n", sOutNameHDF);
                    }
                    delete[] sOutNameHDF;
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "couldn't glob patternn");
                }
                globfree(&gt);
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
