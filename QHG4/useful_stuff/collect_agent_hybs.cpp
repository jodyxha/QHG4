#include <cstdio>
#include <cstring>
#include <climits>
#include <omp.h>


#include <vector>
#include <string>
#include "strutils.h"
#include "LineReader.h"
#include "AgentHybCollector.h"

range *createRange(char *pRangeDef) {
    range *pRange = NULL;

    char *pCopy = new char[strlen(pRangeDef)+1];
    strcpy(pCopy, pRangeDef);
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
                            pRange = new range(dLon,  dLat, dRad);
                        } else {
                            fprintf(stderr, "[%s]: not a number\n", p3);
                        }
                    } else {
                        fprintf(stderr, "Expected two ':' in [%s]\n", pRangeDef);
                    }
                } else {
                    fprintf(stderr, "[%s]: not a number\n", p2);
                }
            } else {
                fprintf(stderr, "Expected ':' in [%s]\n", pRangeDef);
            }
        } else {
            fprintf(stderr, "[%s]: not a number\n", p1);
        }
    } else {
        fprintf(stderr, "Expected something in [%s]\n", pRangeDef);
    }
    return pRange;
}

int createNamedRanges(char *pLocations, named_ranges &mNamedRanges) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(pLocations, "rt");
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
                                       
                                        range *pRange = new range(dLon,  dLat, dRad);
                                        mNamedRanges[pName] = pRange;
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
        printf("Couldn't open [%s] for reading\n", pLocations);
    }
    return iResult;


}


void stringifyContext(bin_context *pContext, char *s) {
    sprintf(s, "\"%s\";%u;%u;%u;", pContext->m_sName.c_str(), pContext->m_iNumAgents, pContext->m_iNumChecked, pContext->m_iNumBins);
    
    char sb[256];
    for (uint k = 0; k < pContext->m_iNumBins; k++) {
        sprintf(sb, "%u;", pContext->m_piBins[k]);
        strcat(s, sb); 
    }
    char sc[128];
    sprintf(sc, "%u;%u;%.8f;%.8f",  pContext->m_iNumZeros,  pContext->m_iNumOnes,  pContext->m_fMinNonZero,  pContext->m_fMaxNonOne);
    strcat(s, sc);
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char *pLocations = NULL;
    
    if (iArgC > 3) {
        char *pGeoQDF   = apArgV[1];
        char *pPopQDF   = apArgV[2];
        char *pItemName = apArgV[3];
        if (iArgC > 4) {
            pLocations = apArgV[4];
        }
        uint iNumBins = 11;
        bool bHeader  = true;

        if (iResult == 0) {
            char *pSpecies = strchr(pPopQDF, ':');
            if (pSpecies != NULL) {
                *pSpecies++ = '\0';
                named_ranges mNamedRanges;
                named_ranges::const_iterator it;                            

                if (pLocations != NULL) {
                    iResult = createNamedRanges(pLocations, mNamedRanges);
                } else {
                    mNamedRanges["All"] = NULL;
                }
                //#pragma omp parallel for                
                //@@ loop over locations
                AgentHybCollector *pAIC = AgentHybCollector::createInstance(pGeoQDF, pPopQDF, pSpecies, pItemName);
                if (pAIC != NULL) {
                    fprintf(stderr, "calling pAIC->analyzeRange()\n");
                    float f0 = omp_get_wtime();
                    iResult = pAIC->analyzeRanges(mNamedRanges, iNumBins);
                    float f1 = omp_get_wtime();
                    if (iResult == 0) {
                        fprintf(stderr, "analyzeRange: %fs\n", f1 -f0);
                        if (bHeader) {
                            printf("LocName;NumAgents;NumChecked;NumBins;");
                            for (uint i = 0; i < iNumBins; i++) {
                                printf("bin_%02u;", i);
                            }
                            printf("NumZeros;NumOnes;MinNonzero;MaxNonZero");
                            printf("\n");
                        }
                        
                        for (uint i = 0; i < pAIC->getNumContexts(); i++) {
                            bin_context *pContext = pAIC->getContext(i);
                            char s[2048];
                            stringifyContext(pContext, s);
                            printf("%s\n", s);
                        }
                    }
                 
                    delete pAIC;
                } else {
                    printf("Couldn't create AgentHybCollector\n");
                }
                for (it = mNamedRanges.begin(); it != mNamedRanges.end(); ++it) {
                    delete it->second;
                }
                
            } else {
                printf("No Species given\n");
            }
        }
    } else {
        printf("%s <geo-qdf> <pop-qdf>:<species> <item-name> [<lon>:<lat>:<rad>]\n", apArgV[0]);
    }
    return iResult;
}
