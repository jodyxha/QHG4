#include "EQsahedron.h"


int main(int iArgC, char *apArgV[]) {
    std::string s("orig_ico.txt");
    if (iArgC > 1) {
        s = apArgV[1];
    }
    int iResult = 0;
    int iSubDivs = 256;
    EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, true);
    if (pEQ != NULL)  {
        int iNumCells = EQsahedron::calcNumVerts(iSubDivs);
        printf("Have EQsahedron with %d verts\n", iNumCells);
        
        pEQ->dump(s);
    } else {
        printf("Couldn't make EQsahedron with subdiv %d\n", iSubDivs);
        iResult = -1;
    }


    return iResult;
}
