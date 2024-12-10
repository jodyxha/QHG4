#include <cstdio>

#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "GraphDesc.h"

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 2) {
        GraphDesc *pGD = GraphDesc::createInstance(apArgV[1]);
        if (pGD != NULL) {
            iResult = pGD->write(apArgV[2]);
            
            delete pGD;
        }
    } else {
        iResult = -1;
        stdprintf("usage: %s <infile> <outfile>\n", apArgV[0]);
    }

    return iResult;

}
