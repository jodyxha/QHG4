#include <cstdio>
#include <cstdlib>

#include "EQConnectivity.h"




int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 1) {
        char *pEnd;
        int iSubDivs = strtol(apArgV[1], &pEnd, 10);
        if (*pEnd == '\0') {
            EQConnectivity *pEQC = EQConnectivity::createInstance(iSubDivs);
            if (pEQC != NULL) {
                iResult = 0;
                int  iNTri = pEQC->getNumTriangles();
                int *piTri = pEQC->getTriangles();

                for (int i = 0; i < iNTri; i++) {
                    printf("%7d, %7d, %7d\n", piTri[3*i], piTri[3*i+1], piTri[3*i+2]);
                }
                
                delete pEQC;
            }
        } else {
            printf("Subdivs should be an integer >= 0\n");
        }
    } else {
        printf("Usage\n");
        printf("%s <numsubdivs>\n", apArgV[0]);
    }
    return iResult;
}
