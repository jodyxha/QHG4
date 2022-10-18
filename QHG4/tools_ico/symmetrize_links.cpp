#include <cstdio>
#include <omp.h>

#include "LinkSymmetrizer.h"
#include "WELL512.h"
#include "WELLUtils.h"




int main(int iArgC, char *apArgV[]) {
       int iResult = 0;
       
       if (iArgC > 1) {
           LinkSymmetrizer *pLI = LinkSymmetrizer::createInstance(apArgV[1]);
           if (pLI != NULL) {
               double t0 = omp_get_wtime();
               WELL512 **pWELL = WELLUtils::buildWELLs(omp_get_max_threads(), 121311);

               
               double t1 = omp_get_wtime();
               iResult = pLI->checkAntipodeSymmetry();
               if (iResult == 0) {
                   printf("before: antipode symmetry ok\n");
               } else  {
                   printf("before: antipode symmetry broken\n");
                   iResult = 0;
               }
               pLI->symmetrize();
               /*
               for (int c = 0; c < 12; c++) {
                   pLI->findNeighborCoords(c);
               }
               */
               iResult = pLI->checkAntipodeSymmetry();
               if (iResult == 0) {
                   printf("after: antipode symmetry ok\n");
               } else  {
                   printf("after: antipode symmetry broken\n");
                   iResult = 0;
               }
               double t2 = omp_get_wtime();
               pLI->save();
               double t3 = omp_get_wtime();

               WELLUtils::destroyWELLs(pWELL, omp_get_max_threads());
               double t4 = omp_get_wtime();
               
               printf("Init well:  %fs\n", t1 -t0);
               printf("Scrambling: %fs\n", t2 -t1);
               printf("Saving:     %fs\n", t3 -t2);
               printf("Destroy:    %fs\n", t4 -t3);
               delete pLI;
           } else {
               printf("Couldn't create LinkSymmetrizer\n");
           }
           //           EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, true, NULL);
           // open qdf
       } else {
           printf("usage: %s <qdf_geogrid>\n", apArgV[0]);
       }
    
       return iResult;
}
