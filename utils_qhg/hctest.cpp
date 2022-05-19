#include <cstdio>

#include "stdstrutils.h"
#include "stdstrutilsT.h"

#include "HybCollector.h"


int main(int iArgC, char *apArgV[]) {
#ifdef  H5_HAVE_THREADSAFE
    stdprintf("HDF5 threadsafe\n");
#else
    stdprintf("HDF5 not threadsafe\n");
#endif
    int iResult = 0;
    int iNumBins = 9;
    if (iArgC > 1) {
        if (iArgC > 2) {
            if (!strToNum(apArgV[2], &iNumBins)) {
                iResult = -1;
            }
        }
        if (iResult == 0) {
            HybCollector *pHC = HybCollector::create_instance(apArgV[1], iNumBins);
            
            if (pHC != NULL) {
                //                pHC->show_names();
                
                pHC->collect_hybridisations();
                
                                pHC->show_data();
                delete pHC;
                
            }
        }
    } else {
        stdprintf("%s <hdf-file> [<num-bins>]\n", apArgV[0]);
    }
    return iResult;
}


