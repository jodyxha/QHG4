#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include "types.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"

#include "SubSpace.h"
#include "SubSpace.cpp"

//----------------------------------------------------------------------------
// fill_rec
//  recursively fills he buffer such that the element at coordinates 
//    (x_0, x_1,...x_(n-1)) 
//  has the value
//    sum_0^n-1 x_i*10^i
//
void fill_rec(double **pCur, uintvec vSizes, uint iCurVal) {
    if (vSizes.size() > 0) {
        uint v = vSizes.back();
        vSizes.pop_back();
        for (uint i = 0; i < v; i++) {
            fill_rec(pCur, vSizes, 10*iCurVal+i);
        }
    } else {
        **pCur = (double)(iCurVal);
        *pCur = *pCur + 1;
    }
    
}


//----------------------------------------------------------------------------
// createBuffer
//
double *createBuffer(uintvec vSizes, uint *piNumVals) {
    *piNumVals = 1;
    // calculate number of values
    for (uint i = 0; i < vSizes.size(); ++i) {
        *piNumVals *= vSizes[i];
    }
    
    // allocate buffer
    double *pOut = new double[*piNumVals];
    double *pCur = pOut;
    // fill buffer 
    fill_rec(&pCur, vSizes, 0);
    
    return pOut;

}

/*
//----------------------------------------------------------------------------
// parseIndexElementOld
//
int parseIndexElementOld(uint iDim, uintvec vSizes, char *pIndexDesc, uintvecvec &vAllIndexes, bool bVerbose) {
    int iResult = -1;

    stringvec vsA;
    uintvec   vA;
    uint iDim2 = splitString(pIndexDesc, vsA, ":");
    if (iDim2 == iDim) {
        iResult = 0;
        // each part has the form "<int>[+<int>" or "
        for (uint i = 0; (iResult == 0) && (i < iDim); i++) {
                        
            stringvec vsX;
            uintvec   vX;
            if (vsA[i] == "*") {
                if (bVerbose) {stdprintf("found '*' for %d: range(%d)\n", i, vSizes[i]);}
                for (uint k = 0; k < vSizes[i]; ++k) {
                    vX.push_back(k);
                }
                if (bVerbose) {stdprintf("vX (%zd):", vX.size());for (uint z = 0; z < vX.size(); z++){ stdprintf(" %d", vX[z]);}printf("\n");}
            } else {
                uint iDim2 = splitString(vsA[i], vsX, "+");
                if (bVerbose) {stdprintf("vsX (%zd):", vsX.size());for (uint z = 0; z < vsX.size(); z++){ stdprintf(" %s", vsX[z]);}printf("\n");}
                for (uint i = 0; (iResult == 0) && (i < iDim2); i++) {
                    uint k = 0;
                    if (strToNum(vsX[i], &k)) {
                        vX.push_back(k);
                    } else {
                        stdprintf("invalid index configuration [%s]\n", vsX[i]);
                        iResult = -1;
                    }
                }
            }
            vAllIndexes.push_back(vX);
        }
    } else {
        stdprintf("there should be %u index combinations\n", iDim); 
    }
    return iResult;
}
*/

//----------------------------------------------------------------------------
// parseIndexElement
//
int parseIndexElement(uint iDim, uintvec vSizes, char *pIndexDesc, uintvecvec &vAllIndexes, uintvec &vAllSums, bool bVerbose) {
    int iResult = -1;
    
    vAllIndexes.clear();
    vAllSums.clear();
    stringvec vsA;
    uintvec   vA;
    uint iDim2 = splitString(pIndexDesc, vsA, ":");
    if (iDim2 == iDim) {
        iResult = 0;
        // each part has the form "'[' <int>[<int>]" or "+" or "*"
        for (uint i = 0; (iResult == 0) && (i < iDim); i++) {
                        
            stringvec vsX;
            uintvec   vX;
            if ((vsA[i] == "*")  || (vsA[i] == "#")) {
                if (bVerbose) {stdprintf("found '*' for %d: range(%d)\n", i, vSizes[i]);}
                for (uint k = 0; k < vSizes[i]; ++k) {
                    vX.push_back(k);
                }
                if (bVerbose) {stdprintf("vX (%zd):", vX.size());for (uint z = 0; z < vX.size(); z++){ stdprintf(" %d", vX[z]);}printf("\n");}
                if (vsA[i] == "#") {
                    vAllSums.push_back(i);
                }
            } else {
                uint iDim2 = splitString(vsA[i], vsX, "+");
                if (bVerbose) {stdprintf("vsX (%zd):", vsX.size());for (uint z = 0; z < vsX.size(); z++){ stdprintf(" %s", vsX[z]);}printf("\n");}
                for (uint i = 0; (iResult == 0) && (i < iDim2); i++) {
                    int k = 0;
                    if (strToNum(vsX[i], &k)) {
                        vX.push_back(k);
                    } else {
                        stdprintf("invalid index configuration [%s]\n", vsX[i]);
                        iResult = -1;
                    }
                }

            }
            vAllIndexes.push_back(vX);

        }
    } else {
        stdprintf("there should be %u index combinations but only found %u in [%s]\n", iDim, iDim2, pIndexDesc); 
    }
    return iResult;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    bool bVerbose = false;

    if (iArgC > 2) {
        if ((iArgC > 3) && (strcmp(apArgV[3], "-v") == 0)) {
            bVerbose = true;
        }
        stringvec vsSizes;
        uintvec   vSizes;
        
        uint iDim = splitString(apArgV[1], vsSizes, ":");

        for (uint i = 0; (iResult == 0) && (i < iDim); i++) {
            int k = 0;
            if (strToNum(vsSizes[i], &k)) {
                vSizes.push_back(k);
            } else {
                stdprintf("invalid dimension size [%s]\n", vsSizes[i]);
                iResult = -1;
            }
        }

        if (iResult == 0) {
            uint iNumVals = 0;
            double *pData = createBuffer(vSizes, &iNumVals);

            SubSpace<double> *pSS = SubSpace<double>::create_instance(vSizes, bVerbose);
            if (pSS != NULL) {
                pSS->show_sizes();

                pSS->set_data(pData, 0, iNumVals);
                delete[] pData;
                //                if (bVerbose) {
                pSS->show_data_nice(DISP_FLOAT);
                //                }
                
                
                uintvecvec vAllIndexes;
                uintvec    vSumDims;

                iResult = parseIndexElement(iDim, vSizes, apArgV[2], vAllIndexes, vSumDims, bVerbose);
                if (iResult == 0) {

                    stdprintf("SliceIndexes:\n");
                    for (uint i = 0; i < vAllIndexes.size(); i++) {
                        uintvec::const_iterator it = std::find(vSumDims.begin(), vSumDims.end(), i);
                        std::string s = (it != vSumDims.end()?"+":" ");
                        stdprintf("  %s ", s);
                        for (uint j = 0; j < vAllIndexes[i].size(); j++) {
                            stdprintf("% d", vAllIndexes[i][j]);
                        }
                        stdprintf("\n");
                    }
                        
                    /*
                      int iPos = pSS->coordToPos(vAll[0]);
                      if (iPos >= 0) {
                      stdprintf("pos: %d\n", iPos);
                      //                    stdprintf("val: %d\n", aiData[iPos]);
                      
                      
                      intvec vY;
                      
                      iResult = pSS->posToCoord(iPos, vY);
                      
                      stdprintf("Y: ");
                      for (uint i = 0; i < vY.size(); i++) {
                      stdprintf(" %d", vY[i]);
                      }
                      stdprintf("\n");
                      }
                    */
                    
                    //                    pSS->calculate_slice_description(vAllIndexes);
                    SubSpace<double> *pSS2 = pSS->create_slice(vAllIndexes);
                    if (pSS2 != NULL) {
                        stdprintf("Have slice\n");
                        pSS2->show_sizes();
                        pSS2->show_data_nice(DISP_FLOAT);
                        
                        if (vSumDims.size() > 0) {
                            SubSpace<double> *pSSSum = pSS2->create_sum(vSumDims);
                            stdprintf("Have reduction\n");
                            pSSSum->show_sizes();
                            pSSSum->show_data_nice(DISP_FLOAT);
                            delete pSSSum;
                        }
                        delete pSS2;
                        
                    }


                }

                delete pSS;
            }    
        
        }
    } else {
        stdprintf("usage:\n");
        stdprintf("   %s <dimensions> <slice_indexes> [-v]\n", apArgV[0]);
        stdprintf("where\n");
        stdprintf("  dimensions     ::= <number> [\":\" <number>]\n");
        stdprintf("  slice_indexs   ::= <slice_index> [\":\"<slice_index>]\n");
        stdprintf("  slice_index    ::= <numeric> | \"*\" | \"#\" \n");
        stdprintf("    numeric      ::= <number> [\"+\"] <number>}\n");
        stdprintf("If the format is numeric, slices will made for the specified numbers\n"); 
        stdprintf("If the format is \"*\" this dimension is kept (shorthand for 0+1+2+..+n)\n"); 
        stdprintf("If the format is \"#\" the SubSpace will be reduced along this dimension\n"); 
        stdprintf("requirements:\n");
        stdprintf("  * the number slice indexes must be equal to the number of dimensions\n"); 
        stdprintf("  * the numbers used in the i-th <numeric> must be less than the size of the i-th dimension\n"); 
        stdprintf("  * the numbers used in a <numeric> should all differ from each other\n"); 
        stdprintf("Example:\n");
        stdprintf("  %s 4:2:3:5 1+2:*:#:4 -v\n", apArgV[0]);
    }
    return iResult;
}
