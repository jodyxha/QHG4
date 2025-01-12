#include <cstdio>
#include "xha_strutilsT.h"
#include "GeoInfo.h"

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    std::string s1 = xha_sprintf("  %d   [%s]   33.1  -15.3     2   34.1  -16.3  ", PR_LAMBERT_CONFORMAL_CONIC, asProjNames[PR_LAMBERT_CONFORMAL_CONIC]);
    std::string s2 = xha_sprintf("%d:33.1:-15.3:2:34.1:-16.3", PR_LAMBERT_CONFORMAL_CONIC);

    ProjType *pT1 = new ProjType();
    if (pT1 != NULL) {
        xha_printf("pT1 ok\n");
        iResult = pT1->fromString(s1, true);
        if (iResult != 0) {
            xha_printf("T1 bad\n");
        }
    } else {
        xha_printf("pT1 failed\n");
    }

    ProjType *pT2 = ProjType::createPT(s2, true);
    if (pT2 != NULL) {
        xha_printf("pT2 ok\n");
    } else {
        xha_printf("pT2 failed\n");
    }


    xha_printf("original %s\n", s1);
    xha_printf("from old %s\n", pT1->toString(true));
    xha_printf("from new %s\n", pT2->toString(true));

    xha_printf("--------------------------------\n");

    std::string sA = "600 400 36 18 -300 0 3";
    std::string sB = "600:400:36:18:c:0:3";
    ProjGrid *pGA = new ProjGrid();
    iResult = pGA->fromString(sA);
    xha_printf("original %s\n", sA);
    xha_printf("fromString %s\n", pGA->toString());
    
    ProjGrid *pGB = ProjGrid::createPG(sB);
    xha_printf("original %s\n", sB);
    xha_printf("from createPG %s\n", pGB->toString());

    return iResult;
}
