#include <cstdio>
#include <cstring>


#include "strutils.h"
#include "LineReader.h"
#include "AgentCounter.h"


//----------------------------------------------------------------------------
// writeCounts
//
int writeCounts(AgentCounter *pAC, const char *pOut) {
    int iResult = -1;

    int iNumCells = pAC->getNumCells();
    const double *pdLon = pAC->getLongitude();
    const double *pdLat = pAC->getLatitude();
    const double *pdAlt = pAC->getAltitude();
    const double *pdIce = pAC->getIceCover();
    int          *piCnt = pAC->getPopCounts(NULL);

    FILE *fOut = fopen(pOut, "wt");
    if (fOut != NULL) {
        iResult = 0;
        for (int i = 0; i < iNumCells; i++) {
            if ((pdAlt[i] > 0) && (pdIce[i] == 0)) {
                fprintf(fOut, "%d  %f %f %d\n", i, pdLon[i], pdLat[i], piCnt[i]);
            }
        }
        fclose(fOut);
    } else {
        printf("Failed to open [%s]\n", pOut);
        iResult = -1;
    }


    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
 
    if (iArgC > 3) {

        const char *pPop = apArgV[1];
        const char *pGeo = apArgV[2];
        
        const char *pOut = apArgV[3];
        
        
        AgentCounter *pAC = AgentCounter::createInstance(pPop, pGeo, NULL);  //NULL not interested in NPP/CC
        if (pAC != NULL) {
            int iTotalCount  = pAC->countAgentsInCells();
            printf("TotalCount: %d\n", iTotalCount);
            iResult = writeCounts(pAC, pOut);
                            
            delete pAC;
        }
            
    } else  {
        printf("Usage:\n");
        printf("  %s <pop-qdf> <geo-qdf> <output>\n", apArgV[0]);
    }
    return iResult;
}
