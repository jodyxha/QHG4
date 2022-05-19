#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "EQConnectivity.h"
#include "EQsahedron.h"




int main(int iArgC, char *apArgV[]) {
    int iResult    = -1;
    int iSubDivs   = 0;
    bool bTegmark  = true;
    FILE *fOutput  = stdout;
    
    if (iArgC > 1) {
        char *pEnd;
        int iSubDivs = strtol(apArgV[1], &pEnd, 10);
        if (*pEnd == '\0') {
            if (iArgC > 2) {
                if ((strcmp(apArgV[2], "teg") == 0) || (strcmp(apArgV[2], "ico") == 0)) {
                    iResult = 0;
                    if (strcmp(apArgV[2], "teg") == 0) {
                        bTegmark = true;
                    } else {
                        bTegmark = false;
                    }
                    if (iArgC > 3) {
                        fOutput = fopen(apArgV[3], "wt");
                        if  (fOutput != NULL) {
                            iResult = 0;
                        } else {
                            printf("Couldn't open [%s] for writing\n", apArgV[3]);
                            iResult = -1;
                        }
                    } else {
                       iResult = 0;
                    }
                } else {
                    fprintf(stdout, "unknnown argument [%s]\n", apArgV[2]);
                    iResult = -1;
                }
            
            }  else {
                iResult = 0;
            }
        } else {
            printf("Subdivs should be an integer >= 0\n");
        }
        

        if (iResult == 0) {
            
            EQConnectivity *pEQC = EQConnectivity::createInstance(iSubDivs);
            if (pEQC != NULL) {
                iResult = 0;
                int  iNTri = pEQC->getNumTriangles();
                int *piTri = pEQC->getTriangles();

                EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, bTegmark);
                if (pEQ != NULL) {
                    pEQ->relink();
                    VertexLinkage *pVL = pEQ->getLinkage();
                    
                    int iNumVerts = pVL->getNumVertices();
                    Vec3D **apVerts = new Vec3D*[iNumVerts];
                    for (int i  = 0; i < iNumVerts; i++) {
                        Vec3D *pV  = pVL->getVertex(i);
                        if (pV != NULL) {
                            apVerts[i] = pV;;
                        } else { 
                            printf("error no vertex with id %dn", i);
                            iResult = -1;
                        }
                    }

                    fprintf(fOutput, "VERTICES:%d\n", iNumVerts);
                    for (int i  = 0; i < iNumVerts; i++) {
                        fprintf(fOutput, "  %9.6f %9.6f %9.6f\n", apVerts[i]->m_fX, apVerts[i]->m_fY, apVerts[i]->m_fZ);
                    }
                    
                    fprintf(fOutput, "TRIANGLES:%d\n", iNTri);

                    for (int i = 0; i < iNTri; i++) {
                        fprintf(fOutput, "  %6d %7d %7d\n", piTri[3*i], piTri[3*i+1], piTri[3*i+2]);
                    }
                    delete[] apVerts;
                    delete pEQ;
                    if (iArgC > 3) {
                        fclose(fOutput);
                    }
                }
                delete pEQC;
            }
        }
    } else {
        printf("Usage\n");
        printf("%s <numsubdivs> [ \"teg\" | \"ico\" [<outfile>]]\n", apArgV[0]);
    }
    return iResult;
}
