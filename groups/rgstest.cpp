#include <stdio.h>
#include <time.h>
#include "types.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "WELL512.h"
#include "WELLUtils.h"

#include "RandomGroupSplitter.h"


int main(int iArgC, char * apArgV[]) {
    int iResult = 0;
    
    intvec vOrig;
    for (uint i = 0; i < 20; i++) {
        vOrig.push_back(i);
    }

    stdprintf("vorig before: %bv\n", vOrig);
    char *pPhrase;
    time_t tloc;
    time_t t= time(&tloc);
    if (iArgC > 1) {
        pPhrase = apArgV[1];
    } else {
        pPhrase = ctime(&t);
    }
    
    stdprintf("Phrase:[%s]\n", pPhrase);

    WELL512 *pWELL = WELLUtils::createWELL(pPhrase);
    WELLUtils::showState(pWELL);

    RandomGroupSplitter *prgs = new RandomGroupSplitter(pWELL, vOrig.size()/3, 2*vOrig.size()/3);
    intvec vSplit;

    iResult = prgs->split(vOrig, vSplit);
    stdprintf("vorig after:  %bv\n", vOrig);
    stdprintf("vsplit:       %bv\n", vSplit);
    
    delete prgs;
    delete pWELL;
    return iResult;
}

