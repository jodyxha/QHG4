#include <cstdio>
#include <cstdlib>
#include <hdf5.h>

#include "QDFUtils.h"
#include "CellBirths.h"

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    int iNumSteps = 5;
    int iNumCells = 40;
    int iNumBirths = 500;

    CellBirths *pCB = CellBirths::createInstance(iNumCells);
    if (pCB != NULL) {
        for (int i = 0; i < iNumBirths; i++) {
            int t = (int)((1.0*iNumSteps*rand())/RAND_MAX);
            gridtype iCell =  (int)((1.0*iNumCells*rand())/RAND_MAX);
            pCB->addBirth(5*t, iCell);
        }

        pCB->showAll();

        hid_t h = qdf_createFile("blabber.qdf", 0, 0, "nothing");
        printf("opened qdf\n"); fflush(stdout);
        hid_t g = qdf_opencreateGroup(h, "blubber");
        printf("opened group\n"); fflush(stdout);
        iResult = pCB->writeDataQDF(g);
        printf("written data\n"); fflush(stdout);
        qdf_closeGroup(g);
        qdf_closeFile(h);


        delete pCB;

        printf("**********************************************\n");
        iResult = -1;
        hid_t h2 = qdf_openFile("blabber.qdf", "r");
        printf("opened qdf\n"); fflush(stdout);
        hid_t g2 = qdf_openGroup(h2, "blubber");
        printf("opened group\n"); fflush(stdout);
        hid_t hKeys = qdf_openDataSet(g2, "CellBirth_keys");
        hid_t hS1 = H5Dget_space(hKeys);
        hsize_t iNumKeys; 

        int iNumDims = H5Sget_simple_extent_dims(hS1, &iNumKeys, NULL);
        if (iNumDims == 1) {
            printf("Got %lld keys\n", iNumKeys); fflush(stdout);
            float *pKeys = new float[iNumKeys];
            iResult = qdf_readArray(g2, "CellBirth_keys", iNumKeys, pKeys);
            if (iResult == 0) {
                hid_t hCounts = qdf_openDataSet(g2, "CellBirth_data");
                hid_t hS2 = H5Dget_space(hCounts);
                hsize_t iNumCounts; 

                iNumDims = H5Sget_simple_extent_dims(hS2, &iNumCounts, NULL);
                if (iNumDims == 1) {
                    printf("Got %lld counts\n", iNumKeys); fflush(stdout);
                    uint *pCounts = new uint[iNumCounts];
                    iResult = qdf_readArray(g2, "CellBirth_data", iNumCounts, pCounts);
                    if (iResult == 0) {
                        uint iNumCells = iNumCounts/iNumKeys;
                        uint iTot = 0;
                        uint j = 0;
                        for (uint i = 0; i < iNumKeys; i++) {
                            uint iSub = 0;
                            uint iNonZeros = 0;
                            for (uint k = 0; k < iNumCells; k++) {
                                if  (pCounts[k+j] > 0) {
                                    iSub += pCounts[k+j];
                                    iNonZeros++;
                                }
                            }

                            printf("Step [%f] (%u nonzeros, subtotal %u\n", pKeys[i], iNonZeros, iSub);
                            iTot += iSub;
                            for (uint k = 0; k < iNumCells; k++) {
                                printf("  [%4d]: %4d\n", k, pCounts[k+j]);
                            }
                            j += iNumCells;
                        }
                        printf("Total: %5d\n", iTot);

                        delete[] pCounts;
                        delete[] pKeys;
                    } else {
                        printf("Couldn't read data\n");
                    }
                } else {
                    printf("Bad dimension for data [%d]\n", iNumDims);
                }
                qdf_closeDataSpace(hS2);
                qdf_closeDataSet(hCounts);
            } else {
                printf("Couldn't read keys\n");
            }
        } else {
            printf("Bad dimension for keys [%d]\n", iNumDims);
            qdf_closeDataSpace(hS1);
            qdf_closeDataSet(hKeys);

        }
        qdf_closeGroup(g2);
        qdf_closeFile(h2);

    
    } else {
        printf("Couldn't create CellBirths\n");
    }

    return iResult;
}
