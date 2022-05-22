#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>
  

#include "strutils.h"
#include "HybChecker.h"
#include "BinFunc.h"
#include "LinBin.h"
#include "VecBin.h"

//----------------------------------------------------------------------------
// createLinBin
//  expects num_bins as parameter
//
BinFunc *createLinBin(char *pParams) {
    LinBin *pLB = NULL;

    char *pNumBins = nextWord(&pParams, ":");
    if (*pNumBins != '\0') {
        int iNumBins = 0;
        if (strToNum(pNumBins, &iNumBins)) {
            pLB =  new LinBin(iNumBins);
        } else {
            fprintf(stderr, "Expected a number, not: [%s]\n", pParams);
        }
    } else {
        fprintf(stderr, "Expected numbins: [%s]\n", pParams);
    }
    return pLB;
}

//----------------------------------------------------------------------------
// createVecBin
//  expects ':'-separated value list
// caller must delete bin func after use
//
BinFunc *createVecBin(char *pParams) {
    VecBin *pVB = NULL;

    std::vector<double> vValues;
    int iResult = 0;
    char *pVal = nextWord(&pParams, ":");
    while ((iResult == 0) && (*pVal != '\0'))  {
        double dVal;
        if (strToNum(pVal, &dVal)) {
            vValues.push_back(dVal);
            pVal = nextWord(&pParams, ":");
        } else {
            fprintf(stderr, "Expected a number, not: [%s]\n", pVal);
            iResult = -1;
        }
    }
    
    if (vValues.size() > 0) {
        pVB = new VecBin(vValues);
        
    } else {
        fprintf(stderr, "No value specified for vec bin\n");
    }


    return pVB;
}

double atanfunc(double x, double a, double slant, double shift) {
    return shift + 2*a*atan(slant*(x - 0.5))/M_PI;
}


//----------------------------------------------------------------------------
// createATanBin
//  expects "<num_bins":"<min_val>":"<slant>
// caller must delete bin func after use
//
BinFunc *createATanBin(char *pParams) {
    VecBin *pVB = NULL;

    char *pNumBins = nextWord(&pParams, ":");
    if (*pNumBins != '\0') {
        int iNumBins = 0;
        if (strToNum(pNumBins, &iNumBins)) {
            char *pMinVal = nextWord(&pParams, ":");
            if (*pMinVal != '\0') {
                double dMinVal = 0;
                if (strToNum(pMinVal, &dMinVal)) {
                    char *pSlant = nextWord(&pParams, ":");
                    if (*pSlant != '\0') {
                        double dSlant = 0;
                        if (strToNum(pSlant, &dSlant)) {
                            double a = (1-2*dMinVal)/(atanfunc(1, 1, dSlant, 0) - atanfunc(0, 1, dSlant, 0));

                            fprintf(stderr, "f0(-1): %f\n", atanfunc(0, 1, dSlant, 0)); 
                            fprintf(stderr, "f0(1): %f\n", atanfunc(1, 1, dSlant, 0)); 
                            fprintf(stderr, "a: %f\n", a);
                            fprintf(stderr, "slant : %f\n", dSlant);
                            fprintf(stderr, "atan(1) : %f\n", atan(1));
                            std::vector<double> vBordersATan;
                            for (int i = 0; i <= iNumBins; i++) {
                                double f = i*1.0/iNumBins;
                                f = atanfunc(f, a, dSlant, 0.5);
                                vBordersATan.push_back(f);
                                fprintf(stderr, "%d: %f\n", i, f); 
                            }
                            pVB = new VecBin(vBordersATan);

                        } else {
                            fprintf(stderr, "Expected a number for <slant>, not: [%s]\n", pParams);
                        }
                        
                    } else {
                        fprintf(stderr, "Expected slant_val: [%s]\n", pParams);
                    }
                } else {
                    fprintf(stderr, "Expected a number for <min_val>, not: [%s]\n", pParams);
                }
            } else {
                fprintf(stderr, "Expected min_val: [%s]\n", pParams);
            }
            
        } else {
            fprintf(stderr, "Expected a number for <num_bins>, not: [%s]\n", pParams);
        }
    } else {
        fprintf(stderr, "Expected numbins: [%s]\n", pParams);
    }
    return pVB;
}

double tanfunc(double x, double a, double slant, double dRange, double shift) {
    return shift + a*tan(slant*(dRange*x - dRange/2.0));
}

//----------------------------------------------------------------------------
// createAanBin
//  expects "<num_bins":"<min_val>:<slant>:<range>">
// caller must delete bin func after use
//
BinFunc *createTanBin(char *pParams) {
    VecBin *pVB = NULL;

    char *pNumBins = nextWord(&pParams, ":");
    if (*pNumBins != '\0') {
        int iNumBins = 0;
        if (strToNum(pNumBins, &iNumBins)) {
            char *pMinVal = nextWord(&pParams, ":");
            if (*pMinVal != '\0') {
                double dMinVal = 0;
                if (strToNum(pMinVal, &dMinVal)) {
                    char *pSlant = nextWord(&pParams, ":");
                    if (*pSlant != '\0') {
                        double dSlant = 0;
                        if (strToNum(pSlant, &dSlant)) {
                            char *pRange = nextWord(&pParams, ":");
                            if (*pRange != '\0') {
                                double dRange = 0;
                                if (strToNum(pRange, &dRange)) {
                                    double a = (1-2*dMinVal)/(tanfunc(1, 1, dSlant, dRange, 0) - tanfunc(0, 1, dSlant, dRange, 0));

                                    fprintf(stderr, "f0(-1): %f\n", tanfunc(0, 1, dSlant, dRange, 0)); 
                                    fprintf(stderr, "f0(1): %f\n", tanfunc(1, 1, dSlant, dRange, 0)); 
                                    fprintf(stderr, "a: %f\n", a);
                                    std::vector<double> vBordersTan;
                                    for (int i = 0; i <= iNumBins; i++) {
                                        double f = i*1.0/iNumBins;
                                        f = tanfunc(f, a, dSlant, dRange, 0.5);
                                        vBordersTan.push_back(f);
                                        fprintf(stderr, "%d: %f\n", i, f); 
                                    }
                                    pVB = new VecBin(vBordersTan);
                                    
                                } else {
                                    fprintf(stderr, "Expected a number for <range>, not: [%s]\n", pParams);
                                }
                                
                            } else {
                                fprintf(stderr, "Expected slant_val: [%s]\n", pParams);
                            }
                        } else {
                            fprintf(stderr, "Expected a number for <range>, not: [%s]\n", pParams);
                        }
                        
                    } else {
                        fprintf(stderr, "Expected slant_val: [%s]\n", pParams);
                    }
                        
                } else {
                    fprintf(stderr, "Expected a number for <min_val>, not: [%s]\n", pParams);
                }
            } else {
                fprintf(stderr, "Expected min_val: [%s]\n", pParams);
            }
            
        } else {
            fprintf(stderr, "Expected a number for <num_bins>, not: [%s]\n", pParams);
        }
    } else {
        fprintf(stderr, "Expected numbins: [%s]\n", pParams);
    }
    return pVB;
}


//----------------------------------------------------------------------------
// createBinFunc
//   handles "lin:"<num_bins> | "vec:"<value>+ | "atan:"<num_bins":"<min_val>":"<slant>
// caller must delete bin func after use
//
BinFunc *createBinFunc(char *sBinMode) {
    BinFunc *pBF = NULL;
    char *pType = nextWord(&sBinMode, ":");
    if (*pType != '\0') {
        if (strcmp(pType, "lin") == 0) {
            pBF = createLinBin(sBinMode);

        } else if  (strcmp(pType, "vec") == 0) {
            pBF = createVecBin(sBinMode);

        } else if  (strcmp(pType, "atan") == 0) {
            pBF = createATanBin(sBinMode);

        } else if  (strcmp(pType, "tan") == 0) {
            pBF = createTanBin(sBinMode);

        } else {
            fprintf(stderr, "Unknown bin type: [%s]\n", pType);
        }
    } else {
        fprintf(stderr, "Empty String? (shouldn't happen)\n");
    }

    return pBF;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;


    //   check_hybrids <pop_qdf> <bin_mode> <out_mode>
    //where
    //  pop_qdf     qdf-file with at least one population
    //  bin_mode    <lin_bin> <vec_bin> <atan_bin>
    //  lin_bin     "lin:"<num_bins>
    //  vec_bin     "vec:"<value>+
    //  atan_bin    "atan:<NumBins>:<scale><slant>
    //  out_mode    "list":"csv":"hdr"
    
    if (iArgC > 3) {
        const char *pPop     = apArgV[1];
        char       *pBinMode = apArgV[2];
        const char *pOutMode = apArgV[3];

        /*
        std::vector<double> vBorders {0.01, 0.02, 0.03, 0.04, 0.05, 0.10, 0.20, 0.40, 0.60, 0.80, 0.90, 0.95, 0.96, 0.97, 0.98, 0.99};
        std::vector<double> vBordersLin {0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90};
        std::vector<double> vBordersLin2 {0.20, 0.40, 0.60, 0.80};
        */        
        BinFunc *pBF = createBinFunc(pBinMode);
        if (pBinMode != NULL) {
            
            // pop file must exist
            struct stat statbuf;
            iResult = stat(pPop, &statbuf);
            if ((iResult == 0) && (S_ISREG(statbuf.st_mode))) {
                int iMode = HybChecker::MODE_NONE;
                if (strcmp("list", pOutMode) == 0) {
                    iMode = HybChecker::MODE_TXT;
                } else if (strcmp("csv", pOutMode) == 0) {
                    iMode = HybChecker::MODE_CSV;
                } else if (strcmp("hdr", pOutMode) == 0) {
                    iMode = HybChecker::MODE_HDR;
                } else {
                    iMode = HybChecker::MODE_NONE;
                    fprintf(stderr, "%s doesn't exist or is not a regular file\n", pPop);
                }
                
                if (iMode != HybChecker::MODE_NONE) {
                    HybChecker *pHC = HybChecker::createInstance(pPop, pBF);
                    // new                    HybChecker *pHC = HybChecker::createInstance(pPop, pBF);
                    if (pHC != NULL) {
                        pHC->analyze();
                        
                        if (iMode ==  HybChecker::MODE_HDR) {
                            pHC->showCSVHeader();
                            pHC->showCSVLine();
                            //pHC->showSimple();
                            
                        } else if (iMode ==  HybChecker::MODE_CSV) {
                            pHC->showCSVLine();
                        } else if  (iMode ==  HybChecker::MODE_TXT) {
                            pHC->showSimple();
                        }
                        
                        
                        delete pHC;
                    }                    
                }
            } else {
                fprintf(stderr, "%s doesn't exist or is not a regular file\n", pPop);
            }
        }

        delete pBF;

    } else  {
        printf("%s - do histograms for hybridisation of population qdf\n", apArgV[0]);
        printf("Usage:\n");
        printf("  %s <pop-qdf> <bin-mode> <out-mode>\n", apArgV[0]);
        printf("where\n");
        printf("  pop-qdf         a qdf files containing agets which have an attribute \"PheneticHyb\"\n");
        printf("  bin-mode        lin:<num_bins> | vec:<values>* |  atan:<num_bins>:<min_val>:<slant>\n");
        printf("                    lin   all bins have same size\n");
        printf("                    vec   the bins have sizes determined by values\n");
        printf("                    atan   bin sizes determined atan (small at the edges,  large in the middle\n");
        printf("                    tan    bin sizes determined tan (large at the edges small iin the middle\n");
        printf(" out-mode          'txt' | 'csv' | 'hdr':\n");
        printf("                    txt: simple list\n");
        printf("                    csv: a csv line\n");
        printf("                    hdr: a csv line precede by a csv header\n");
    }
    return iResult;
}



//----------------------------------------------------------------------------
// main
//
int main2(int iArgC, char *apArgV[]) {
    int iResult = 0;



    std::vector<double> vBorders {0.01, 0.02, 0.03, 0.04, 0.05, 0.10, 0.20, 0.40, 0.60, 0.80, 0.90, 0.95, 0.96, 0.97, 0.98, 0.99};
    std::vector<double> vBordersLin {0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90};
    std::vector<double> vBordersLin2 {0.20, 0.40, 0.60, 0.80};

    std::vector<double> &vCur = vBorders ;
    if (vCur.back() < 1.0) {
        vCur.push_back(1.0);
    }

    BinFunc *pVB = new VecBin(vCur);

    printf("size: %zd \n", vCur.size());
    for (int i = 0; i < 102; i++) {
        double v = (i*1.0)/101.0;
        printf("v %f -> b %d\n", v, pVB->calcBin(v));
               
    }
    delete pVB;
    return iResult;
}
