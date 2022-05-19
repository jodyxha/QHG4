#include <cstdio>
#include "stdstrutilsT.h"
#include "GeoInfo.h"

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    std::string s1 = stdsprintf("  %d   [%s]   33.1  -15.3     2   34.1  -16.3  ", PR_LAMBERT_CONFORMAL_CONIC, asProjNames[PR_LAMBERT_CONFORMAL_CONIC]);
    std::string s2 = stdsprintf("%d:33.1:-15.3:2:34.1:-16.3", PR_LAMBERT_CONFORMAL_CONIC);

    ProjType *pT1 = new ProjType();
    if (pT1 != NULL) {
        stdprintf("pT1 ok\n");
        iResult = pT1->fromString(s1, true);
        if (iResult != 0) {
            stdprintf("T1 bad\n");
        }
    } else {
        stdprintf("pT1 failed\n");
    }

    ProjType *pT2 = ProjType::createPT(s2, true);
    if (pT2 != NULL) {
        stdprintf("pT2 ok\n");
    } else {
        stdprintf("pT2 failed\n");
    }


    stdprintf("original %s\n", s1);
    stdprintf("from old %s\n", pT1->toString(true));
    stdprintf("from new %s\n", pT2->toString(true));

    stdprintf("--------------------------------\n");

    std::string sA = "600 400 36 18 -300 0 3";
    std::string sB = "600:400:36:18:c:0:3";
    ProjGrid *pGA = new ProjGrid();
    iResult = pGA->fromString(sA);
    stdprintf("original %s\n", sA);
    stdprintf("fromString %s\n", pGA->toString());
    
    ProjGrid *pGB = ProjGrid::createPG(sB);
    stdprintf("original %s\n", sB);
    stdprintf("from createPG %s\n", pGB->toString());

    return iResult;
}
