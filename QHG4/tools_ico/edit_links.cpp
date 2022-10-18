#include <cstdio>
#include <fcntl.h>   // open
#include <unistd.h>  // read, write, close
#include <cstring>   // strcmp
#include <omp.h>

#include "LinkTools.h"
#include "strutils.h"
#include "WELL512.h"
#include "WELLUtils.h"


#define BUFSIZE 65536

//----------------------------------------------------------------------------
// copyFile
//
int copyFile(const char *pFrom, const char *pTo) { 
    int iResult = -1;
    char sCommand[1024];
    
    sprintf(sCommand, "cp %s %s", pFrom, pTo);

    iResult = system(sCommand);

    return iResult;
}


//----------------------------------------------------------------------------
// doSymmetrize
//
int doSymmetrize(const char *pIn, const char *pOut) {
    int iResult = 0;
    if (pOut != NULL) {
        iResult = copyFile(pIn, pOut);
    } else {
        pOut = pIn;
    }
    if (iResult == 0) {
        LinkTools *pLT = LinkTools::createInstance(pOut);
        if (pLT != NULL) {
            iResult = pLT->symmetrize();
            if (iResult == 0) {
                iResult = pLT->checkAntipodeSymmetry();
                if (iResult == 0) {
                    pLT->save();
                } else {
                   printf("antipode symmetry broken\n");
                }
            }
            delete pLT;
        } else {
            printf("Couldn't create LinkSymmetrizer\n");
        }
    } else {
        printf("Couldn't copy [%s] to [%s]\n", pIn, pOut);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// doCheck
//
int doCheck(const char *pIn) {
    int iResult = 0;

    LinkTools *pLT = LinkTools::createInstance(pIn);
    if (pLT != NULL) {
        iResult = pLT->checkAntipodeSymmetry();
        if (iResult == 0) {
            printf("after: antipode symmetry ok\n");
        } else  {
            printf("after: antipode symmetry broken\n");
        }
    } else {
        printf("Couldn't create LinkSymmetrizer\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// doScramble
//
int doScramble(const char *pIn, const char *pOut, int iSeed) {
    int iResult = 0;
    if (pOut != NULL) {
        iResult = copyFile(pIn, pOut);
    } else {
        pOut = pIn;
    }
    if (iResult == 0) {
        LinkTools *pLT = LinkTools::createInstance(pOut);
        if (pLT != NULL) {
            //double t0 = omp_get_wtime();
            WELL512 **pWELL = WELLUtils::buildWELLs(omp_get_max_threads(), iSeed);
                    
            //double t1 = omp_get_wtime();
            pLT->scramble(pWELL);
            //double t2 = omp_get_wtime();
            pLT->save();
            printf("You might want to do 'h5repack %s'\n", pOut);
            //double t3 = omp_get_wtime();
                    
            WELLUtils::destroyWELLs(pWELL, omp_get_max_threads());
            //double t4 = omp_get_wtime();
            /*        
            printf("Init well:  %fs\n", t1 -t0);
            printf("Scrambling: %fs\n", t2 -t1);
            printf("Saving:     %fs\n", t3 -t2);
            printf("Destroy:    %fs\n", t4 -t3);
            */
            delete pLT;
        } else {
            printf("Couldn't create LinkSymmetrizer\n");
        }
    } else {
        printf("Couldn't copy [%s] to [%s]\n", pIn, pOut);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// doNeighbors
//
int doNeighbors(const char *pIn) {
    int iResult = 0;

    LinkTools *pLT = LinkTools::createInstance(pIn);
    if (pLT != NULL) {
        for (int c = 0; c < 12; c++) {
            pLT->findNeighborCoords(c);
        }
        delete pLT;
    } else {
        printf("Couldn't create LinkSymmetrizer\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
       
    if (iArgC > 2) {
        char *pIn  = apArgV[2]; 
        if (strcmp(apArgV[1],"scramble")==0) { 
	    int offs=3;
            char *pOut = NULL;
	    if (iArgC > 4) {
                pOut = apArgV[3];
	        offs++;
            }

            int iSeed = -1;
            if (strToNum(apArgV[offs], &iSeed)) {
                iResult = doScramble(pIn, pOut, iSeed);
            } else {
                iResult = -1;
                printf("Expected integer for seed, but got [%s]\n", apArgV[3]);
            }
        } else if (strcmp(apArgV[1],"nodeneighbors")==0) {
            iResult = doNeighbors(pIn);

        } else if (strcmp(apArgV[1],"symmetrize")==0) {
	    char *pOut = NULL;
	    if (iArgC > 3) {
                pOut = apArgV[3];
	    }	
            iResult = doSymmetrize(pIn, pOut);
           
        } else if (strcmp(apArgV[1],"check")==0) {
            iResult = doCheck(pIn);
           
        } else {
            printf("unknown command  [%s]\n", apArgV[1]);
            iResult = -1;
        }

    } else {
        printf("%s - display or modify icosa hedron links\n", apArgV[0]);
        printf("usage: %s scramble <qdf_grid> <qdf_out> <seed>\n", apArgV[0]);
        printf("       (random ordering of links at every node)\n");
        printf("usage: %s nodeneighbors <qdf_grid>\n", apArgV[0]);
        printf("       (show neighbor lists of each node)\n");
        printf("usage: %s symmetrize <qdf_grid> <qdf_out> \n", apArgV[0]);
        printf("       (reorder nopdes to attain antipodal symmetry)\n");
        printf("usage: %s check <qdf_grid> \n", apArgV[0]);
        printf("       (check antipodal symmetry neighbor(antipode(n)=antipode(neighbor(n))))\n");
    }
    
    return iResult;
}
