#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <omp.h>

#include "types.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"

#include "HybCollector.h"
#include "SubSpace.h"
#include "SubSpace.cpp"

//----------------------------------------------------------------------------
// usage
//
void usage(const std::string sApp) {
    stdprintf("usage:\n");
    stdprintf("   %s <hdf-file> <num-bins> [<slice-indexes>] [-v]\n", sApp);
    stdprintf("where\n");
    stdprintf("  hdf-file       hdf file to analyse\n");
    stdprintf("  num-bins       number of bins for histograms\n");
    stdprintf("  slice-indexes  index specification for reductions (if omitted, the dimension and coordinate names are shown)\n");
    stdprintf("  -v             verbose output\n");
    stdprintf("format for index-specifcations\n");
    stdprintf("    slice_indexes  ::= <slice_index> [\":\"<slice_index>]\n");
    stdprintf("    slice_index    ::= <numeric> | \"*\" | \"#\" \n");
    stdprintf("    numeric        ::= <number> [\"+\"] <number>}\n");
    stdprintf("If the format is numeric, slices will made for the specified numbers\n"); 
    stdprintf("If the format is \"*\" this dimension is kept (shorthand for 0+1+2+..+n)\n"); 
    stdprintf("If the format is \"#\" the SubSpace will be reduced along this dimension\n"); 
    stdprintf("requirements:\n");
    stdprintf("  * the number slice indexes must be equal to the number of dimensions\n"); 
    stdprintf("  * the numbers used in the i-th <numeric> must be less than the size of the i-th dimension\n"); 
    stdprintf("Example:\n");
    stdprintf("  %s bigrun_07163.hdf 51 '*:#:0+2+9:*' -v\n", sApp);
    //    stdprintf("  %s 4:2:3:5 1+2:*:#:4 -v\n", apArgV[0]);

}

//----------------------------------------------------------------------------
// parseIndexElement
//
int parseIndexElement(SubSpace<int> *pSS, uint iDim, uintvec vSizes, char *pIndexDesc, uintvecvec &vAllIndexes, uintvec &vAllSums, bool bVerbose) {
    int iResult = -1;
    
    vAllIndexes.clear();
    vAllSums.clear();
    stringvec vsA;
    std::vector<int> vA;
    uint iDim2 = splitString(pIndexDesc, vsA, ":");
    if (iDim2 == iDim) {
        iResult = 0;
        // each part has the form "'[' <int>[<int>]" or "+" or "*"
        for (uint i = 0; (iResult == 0) && (i < iDim); i++) {
                        
            stringvec vsX;
            uintvec vX;
            if ((vsA[i] == "*")  || (vsA[i] == "#")) {
                if (bVerbose) {stdfprintf(stderr, "found '*' or '#' for %d: range(%d)\n", i, vSizes[i]);}
                for (uint k = 0; k < vSizes[i]; ++k) {
                    vX.push_back(k);
                }
                if (bVerbose) {stdfprintf(stderr, "vX (%zd):", vX.size());for (uint z = 0; z < vX.size(); z++){ stdfprintf(stderr, " %d", vX[z]);}printf("\n");}
                if (vsA[i] == "#") {
                    vAllSums.push_back(i);
                }
            } else {
                uint iDim2 = splitString(vsA[i], vsX, "+");
                if (bVerbose) {stdfprintf(stderr, "vsX (%zd):", vsX.size());for (uint z = 0; z < vsX.size(); z++){ stdfprintf(stderr, " %s", vsX[z]);}printf("\n");}
                for (uint j = 0; (iResult == 0) && (j < iDim2); j++) {
                    int k = 0;
                    if (!vsX[j].empty()) {
                        if (strToNum(vsX[j], &k)) {
                            vX.push_back(k);
                        } else {

                            const stringvec &cn = pSS->get_coord_names()[i];
                            bool bSearching = true;
                            for (uint z = 0; bSearching && (z < cn.size()); z++) {
                                if (cn[z] == vsX[j]) {
                                    vX.push_back(z);
                                    bSearching = false;
                                }
                            }
                            if (bSearching) {
                                stdfprintf(stderr, "unknown coordinate name [%s]\n", vsX[j]);
                                pSS->show_names();
                                iResult = -1;
                            }
                        }
                    } else {
                        stdfprintf(stderr, "invalid index configuration [%s]\n", vsX[i]);
                        iResult = -1;
                    }
                }

            }
            vAllIndexes.push_back(vX);

        }
    } else {
        stdfprintf(stderr, "there should be %u index combinations but only found %u in [%s]\n", iDim, iDim2, pIndexDesc); 
    }
    return iResult;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    bool bVerbose = false;

    double dT_hyb   = 0;
    double dT_slice = 0;
    double dT_sum   = 0;

    int iNumBins = 9;
    if (iArgC > 2) {
        char *pFileHDF  = apArgV[1];
        char *pNumBins  = apArgV[2];
        char *pIndexDef = NULL;
        if (iArgC > 3) {
            pIndexDef = apArgV[3];
        }
        if ((iArgC > 4) && (strcmp(apArgV[4], "-v") == 0)) {
            bVerbose = true;
        }
        if (strToNum(pNumBins, &iNumBins)) {
            HybCollector *pHC = HybCollector::create_instance(pFileHDF, iNumBins);
            if (pHC != NULL) {
                dT_hyb = omp_get_wtime();
                pHC->collect_hybridisations();
                dT_hyb = omp_get_wtime() - dT_hyb;
                uintvec vSizes;
                vSizes.push_back(pHC->getNumBins());
                vSizes.push_back(pHC->getNumRegions());
                vSizes.push_back(pHC->getNumSteps());
                vSizes.push_back(pHC->getNumSims());
                
                int iNumVals = pHC->getNumSims()*pHC->getNumSteps()*pHC->getNumRegions()*pHC->getNumBins();
                SubSpace<int> *pSS = SubSpace<int>::create_instance(vSizes, bVerbose);
                if (pSS != NULL) {
                    SubSpace<int> *pSSOutput=NULL;
                    
                    pSS->show_sizes();

                    stringvec vDimNames{"bin", "region", "time_step", "simulation"};
                    stringvecvec vvCoordNames( {{}, pHC->getRegionNames(), pHC->getStepNames(), pHC->getSimNames()});

                    pSS->set_data(pHC->getData(), 0, iNumVals);
                    pSS->set_dim_names(vDimNames);
                    pSS->set_coord_names(vvCoordNames);
                    //pSS->show_names();
                    if (bVerbose) {
                        pHC->show_data();
                        pSS->show_data_nice(DISP_INT);
                    }
                    //                    pSS->show_data_csv();
                    
                   
                    uintvecvec vAllIndexes;
                    uintvec    vSumDims;
                    if (pIndexDef != NULL) {
                        iResult = parseIndexElement(pSS, vSizes.size(), vSizes, pIndexDef, vAllIndexes, vSumDims, bVerbose);
                        if (iResult == 0) {
                            if (bVerbose) {
                                stdfprintf(stderr, "SliceIndexes:\n");
                                for (uint i = 0; i < vAllIndexes.size(); i++) {
                                    uintvec::const_iterator it = std::find(vSumDims.begin(), vSumDims.end(), i);
                                    std::string s = (it != vSumDims.end()?"+":" ");
                                    stdfprintf(stderr, "  %s ", s);
                                    for (uint j = 0; j < vAllIndexes[i].size(); j++) {
                                        stdfprintf(stderr, "% d", vAllIndexes[i][j]);
                                    }
                                    stdfprintf(stderr, "\n");
                                }
                            }
                            
                            dT_slice = omp_get_wtime();
                            //                        pSS->calculate_slice_description(vAllIndexes);
                            SubSpace<int> *pSS2 = pSS->create_slice(vAllIndexes);
                            dT_slice = omp_get_wtime() - dT_slice;
                            
                            
                            if (pSS2 != NULL) {
                                // use the vAllIndexes to create a new coordnames
                                // and pass them to pSS2
                                
                                pSSOutput = pSS2;
                                
                                if (bVerbose) {
                                    stdprintf("Have slice\n");
                                    pSS2->show_sizes();
                                    pSS2->show_data_nice(DISP_INT);
                                }
                                if (vSumDims.size() > 0) {
                                    dT_sum = omp_get_wtime();
                                    SubSpace<int> *pSSSum = pSS2->create_sum(vSumDims);
                                    dT_sum = omp_get_wtime() - dT_sum;
                                    
                                    delete pSS2;
                                    pSSOutput = pSSSum;
                                    if (bVerbose) {
                                        stdprintf("Have reduction\n");
                                        pSSSum->show_sizes();
                                        pSSSum->show_data_nice(DISP_INT);
                                    }
                                    //delete pSSSum;
                                }
                                
                            
                                //if (bVerbose) {
                                stdprintf("Have final output slice\n");
                                pSSOutput->show_sizes();
                                //                            pSSOutput->show_data_nice(DISP_INT );
                                pSSOutput->show_data_csv();
                                //}
                           } 
                            
                        }
                        
                    } else {
                        pSS->show_names();
                    } 
                    delete pSS;
                    delete pSSOutput;
                }
                delete pHC;
            } else {
                stdfprintf(stderr, "couldn't create HybCollector\n");
            }
        
        } else {
            stdfprintf(stderr, "num bins should be a number [%s]\n", apArgV[2]);
        }
        /*
        stdfprintf(stderr, "dT_hyb:   %fs\n", dT_hyb);
        stdfprintf(stderr, "dT_slice: %fs\n", dT_slice);
        stdfprintf(stderr, "dT_sum:   %fs\n", dT_sum);
        */
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
