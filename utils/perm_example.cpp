#include <cstdio>
#include <utility>

#include "WELL512.h"
#include "Permutator.h"

int main(int iArgC, char *apArgV[]) {

    int iResult = 0;


    // WELL512 seeded with current time
    WELL512 *pWELL = new WELL512();
    
    std::pair<int, std::string> apairs[] ={{0,"zero"},  {1,"one"},  {2,"two"}, 
                                           {3,"three"}, {4,"four"}, {5,"five"},{6,"six"}};

    Permutator *pPerm = Permutator::createInstance(5);

    // select 3 random elements from [0,4]
    unsigned int *pRes1 = pPerm->permute(5, 3, pWELL);
    for (unsigned i = 0; i < 3; i++) {
        printf(" %d", pRes1[i]);
    }
    printf("\n");

    // get a random permutation of [0,9]
    unsigned int *pRes2 = pPerm->permute(9, 9, pWELL);
    for (unsigned i = 0; i < 9; i++) {
        printf(" %d", pRes2[i]);
    }
    printf("\n");

    // select 4 random elements from the array apairs
    std::pair<int, std::string> *pRes3 = pPerm->permuteBuf(7, 4, pWELL, apairs);
    for (unsigned i = 0; i < 4; i++) {
        printf(" %d:%s", pRes3[i].first, pRes3[i].second.c_str());
    }
    printf("\n");

    delete pPerm;
    delete pWELL;
    return iResult;
}
