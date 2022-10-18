#include <cstdio>
#include <cstring>
#include <climits>
#include <omp.h>


#include <vector>
#include <string>
#include "strutils.h"
#include "LineReader.h"
#include "ParamReader.h"
#include "AgentItemCollector.h"

#define DEF_NUM_BINS 11

//----------------------------------------------------------------------------
//  usage
//
void usage(const char *pApp) {
    fprintf(stderr, "%s - create sistribution foe an element of a population's AgentDataSet\n", pApp);
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s  -p <pop-qdf>:<species>  -n <item-name>:<item-type>\n", pApp);
    fprintf(stderr, "          [-g <geo-qdf>] [-b <num-bins>[:<min-bin:<max-bin>] [-l <locations-file>]\n") ;
    fprintf(stderr, "where\n");
}


//----------------------------------------------------------------------------
// createSamplingRange
//  create a sampling range structre from the string.
//  expect <lon>:<lat>:<radius>
//
sampling_range *createSamplingRange(char *pSamplingRangeDef) {
    sampling_range *pRange = NULL;

    char *pCopy = new char[strlen(pSamplingRangeDef)+1];
    strcpy(pCopy, pSamplingRangeDef);
    char *p1 = strtok(pCopy, ":");
    if (p1 != NULL) {
        double dLon;
        if (strToNum(p1, &dLon)) {
            char *p2 = strtok(NULL, ":");
            if (p2 != NULL) {
                double dLat;
                if (strToNum(p2, &dLat)) {
                    char *p3 = strtok(NULL, ":");
                    if (p3 != NULL) {
                        double dRad;
                        if (strToNum(p3, &dRad)) {
                            pRange = new sampling_range(dLon,  dLat, dRad);
                        } else {
                            fprintf(stderr, "[%s]: not a number\n", p3);
                        }
                    } else {
                        fprintf(stderr, "Expected two ':' in [%s]\n", pSamplingRangeDef);
                    }
                } else {
                    fprintf(stderr, "[%s]: not a number\n", p2);
                }
            } else {
                fprintf(stderr, "Expected ':' in [%s]\n", pSamplingRangeDef);
            }
        } else {
            fprintf(stderr, "[%s]: not a number\n", p1);
        }
    } else {
        fprintf(stderr, "Expected something in [%s]\n", pSamplingRangeDef);
    }
    return pRange;
}



//----------------------------------------------------------------------------
// createNamedSamplingRange
//   expect pLocationFIle to consist of lines
//      <name> <lon> <lat> <radius>
//
int createNamedSamplingRanges(char *pLocationsFile, named_sampling_ranges &mNamedSamplingRanges) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(pLocationsFile, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine();
        while ((iResult == 0) && (pLine != NULL)) {
            iResult = -1;
            char *pCopy = new char[strlen(pLine)+1];
            strcpy(pCopy, pLine);
            char *pName = strtok(pCopy, " \t\n");
            if (pName != NULL) {
                char *p1 = strtok(NULL, " \t\n");
                if (p1 != NULL) {

                    double dLon;
                    if (strToNum(p1, &dLon)) {
                        char *p2 = strtok(NULL, " \t\n");
                        if (p2 != NULL) {
                            double dLat;
                            if (strToNum(p2, &dLat)) {
                                char *p3 = strtok(NULL, " \t\n");
                                if (p3 != NULL) {
                                    double dRad;
                                    if (strToNum(p3, &dRad)) {
                                       
                                        sampling_range *pSamplingRange = new sampling_range(dLon,  dLat, dRad);
                                        mNamedSamplingRanges[pName] = pSamplingRange;
                                        iResult = 0;
                                    } else {
                                        fprintf(stderr, "[%s]: not a number\n", p3);
                                    }
                                } else {
                                    fprintf(stderr, "Expected two ':' in [%s]\n", pLine);
                                }
                            } else {
                                fprintf(stderr, "[%s]: not a number\n", p2);
                            }
                        } else {
                            fprintf(stderr, "Expected ':' in [%s]\n", pLine);
                        }
                    }   else {
                        fprintf(stderr, "[%s]: not a number\n", p1);
                    }
                } else {
                    fprintf(stderr, "Expected ':' in [%s]\n", pLine);
                }
            } else {
                fprintf(stderr, "Expected something in [%s]\n", pLine);
            }
            delete[] pCopy;
            pLine = pLR->getNextLine();
        }
            
        delete pLR;
        
    } else {
        fprintf(stderr, "Couldn't open [%s] for reading\n", pLocationsFile);
    }
    return iResult;


}


//----------------------------------------------------------------------------
//  stringifyBinContext
//
void stringifyBinContext(bin_context *pBinContext, char *s) {
    sprintf(s, "\"%s\";%u;%u;%.4f;%.4f;%u;", pBinContext->m_sName.c_str(), pBinContext->m_iNumAgents, pBinContext->m_iNumChecked, pBinContext->m_dMinBin, pBinContext->m_dMaxBin, pBinContext->m_iNumBins);
    
    char sb[256];
    for (uint k = 0; k < pBinContext->m_iNumBins; k++) {
        sprintf(sb, "%u;", pBinContext->m_piBins[k]);
        strcat(s, sb); 
    }
    char sc[128];
    sprintf(sc, "%u;%u;%.8f;%.8f",  pBinContext->m_iNumZeros,  pBinContext->m_iNumOnes,  pBinContext->m_fMinNonZero,  pBinContext->m_fMaxNonOne);
    strcat(s, sc);
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
        char *sBinInfo       = NULL;
        int   iNumBins       = DEF_NUM_BINS;
        double dMin = 0;
        double dMax = 1;
        char *pDataType      = NULL;
        char *pSpecies       = NULL;

        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(5,
                                       "-p:S!",     &sPopQDF,
                                       "-g:S",      &sGeoQDF,
                                       "-n:S!",     &sItemName,
                                       "-b:S",      &sBinInfo,
                                       "-l:S",     &sLocationsFile);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
                
            if (iResult >= 0) {


                bool bHeader  = true;
                
                if (iResult == 0) {
                    
                    pSpecies = strchr(sPopQDF, ':');
                    if (pSpecies != NULL) {
                        *pSpecies++ = '\0';

                        if (sGeoQDF == NULL) {
                            sGeoQDF = sPopQDF;
                        }
                    } else {
                        fprintf(stderr, "No Species given\n");
                    }
                }

                if (iResult == 0) {
                    pDataType = strchr(sItemName, ':');
                    if (pDataType != NULL) {
                        *pDataType++ = '\0';
                    } else {
                        fprintf(stderr, "expected ':' followed by datatype\n");
                        iResult = -1;
                    }
                }
                        
                if (iResult == 0) {
                    char *pMin = NULL;
                    char *pMax = NULL;
                    if (sBinInfo != NULL) {
                        iResult = -1;
                        pMin = strchr(sBinInfo, ':');
                        if (pMin != NULL) {
                            *pMin++ = '\0';
                            pMax = strchr(pMin, ':');
                            if (pMax != NULL) {
                                *pMax++ = '\0';
                            }
                        }
                        if (strToNum(sBinInfo, &iNumBins)) {
                            if ((pMin != NULL) && (strToNum(pMin, &dMin))) {
                                if ((pMax != NULL) && (strToNum(pMax, &dMax))) {
                                    iResult = 0;
                                }
                            }
                        }
                    }
                }
                if (iResult == 0) {
                    named_sampling_ranges mNamedSamplingRanges;
                    named_sampling_ranges::const_iterator it;                            
                    
                    if (sLocationsFile != NULL) {
                        iResult = createNamedSamplingRanges(sLocationsFile, mNamedSamplingRanges);
                    } else {
                        mNamedSamplingRanges["All"] = NULL;
                    }
                    
                    //@@ loop over locations
                    AgentItemCollector *pAIC = AgentItemCollector::createInstance(sGeoQDF, sPopQDF, pSpecies, sItemName, pDataType);
                    if (pAIC != NULL) {
                        fprintf(stderr, "calling pAIC->analyzeRanges()\n");
                        float f0 = omp_get_wtime();
                        iResult = pAIC->analyzeRanges(mNamedSamplingRanges, iNumBins, dMin, dMax);
                        float f1 = omp_get_wtime();
                        if (iResult == 0) {
                            fprintf(stderr, "analyzeRange: %fs\n", f1 -f0);
                            if (bHeader) {
                                printf("LocName;NumAgents;NumChecked;MinBin;MaxBin;NumBins;");
                                for (uint i = 0; i < iNumBins; i++) {
                                    printf("bin_%02u;", i);
                                }
                                printf("NumZeros;NumOnes;MinNonzero;MaxNonZero");
                                printf("\n");
                            }
                            
                            for (uint i = 0; i < pAIC->getNumContexts(); i++) {
                                bin_context *pBinContext = pAIC->getContext(i);
                                char s[2048];
                                stringifyBinContext(pBinContext, s);
                                printf("%s\n", s);
                            }
                        }
                        uint iNumAgs = pAIC->getNumValues();
                        double *p = pAIC->getValues();
                        for (uint j = 0 ; j < iNumAgs; j++) {
                            printf("%f\n", p[j]);
                        }
                        delete pAIC;
                    } else {
                        fprintf(stderr, "Couldn't create AgentItemCollector\n");
                    }
                    
                    for (it = mNamedSamplingRanges.begin(); it != mNamedSamplingRanges.end(); ++it) {
                        delete it->second;
                    }
                    
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }

    return iResult;
}
