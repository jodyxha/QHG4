#include <cstdio>

#include "EQsahedron.h"
#include "EQVertexFaces.h"

void printset(intset s, const char *pCaption, int iLim) {
    printf("%s\n", pCaption);
        int i = 0;
        intset::const_iterator it;
        for (it = s.begin(); it != s.end(); ++it) {
            printf("%d ", *it);
            i++;
            if (i>iLim) {
                printf("...");
                break;
            }
        }
        printf("\n");
    
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    int iNumSubDivs = atoi(apArgV[1]); 
    int iCellID     = atoi(apArgV[2]);
    double dLon     = atof(apArgV[3]);
    double dLat     = atof(apArgV[4]);
    EQVertexFaces *pEQV = new EQVertexFaces(iNumSubDivs);
    intset sv; 
    iResult = pEQV->getFaceNeighborIDs(iCellID, sv);
    if (iResult >= 0) {
        char sCaption[256];


        sprintf(sCaption, "found %d verts in neighboring faces of %d\n", iResult, iCellID);
        printset(sv, sCaption, 99);
            
        sv.clear();    
        iResult = pEQV->getFaceNeighborIDs(dLon, dLat, sv);
        if (iResult >= 0) {
            sprintf(sCaption, "found %d verts in neighboring faces of (%f,%f)\n", iResult, dLon, dLat);
            printset(sv, sCaption, 99);
        } else {
            printf("Bad cell coords %f,%f\n", dLon, dLat);
        }
    } else {
        printf("Bad cell ID %d\n", iCellID);
    }
    delete pEQV;
    return iResult;
}
