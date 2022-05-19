#include <cstdio>
#include <cstdarg>
#include "types.h"
#include "WELL512.h"
#include "WELLUtils.h"

#include "omp.h"


int main(int iArgC, char *apArgV[]) {

    long iN = 0;
    iN = atol(apArgV[1]);    
    WELL512 *pW = WELLUtils::createWELL("glubber");
    
    float t1 = omp_get_wtime();
    for (long i = 0; i < iN; i++) {
        rand();
    }
    t1 = omp_get_wtime() -t1;
    printf("%ld rands %f s, 1 rand %e s\n", iN, t1, t1/iN);
    float t2 = omp_get_wtime();
    for (long i = 0; i < iN; i++) {
        pW->wrand();
    }
    t2 = omp_get_wtime() -t2;
    printf("%ld wrands %f s, 1 rand %e s\n", iN, t2, t2/iN);
    delete pW;



    return 0;
}
