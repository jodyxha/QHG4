#include <stdio.h>
#include <time.h>
#include "types.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "WELL512.h"
#include "WELLUtils.h"

#include "RandomGroupSplitter.h"


int main(int iArgC, char * apArgV[]) {
    int iResult = 0;
    
    intvec vOrig;
    for (uint i = 0; i < 20; i++) {
        vOrig.push_back(i);
    }

    xha_printf("vorig before: %bv\n", vOrig);
    char *pPhrase;
    time_t tloc;
    time_t t= time(&tloc);
    if (iArgC > 1) {
        pPhrase = apArgV[1];
    } else {
        pPhrase = ctime(&t);
    }
    
    xha_printf("Phrase:[%s]\n", pPhrase);

    WELL512 *pWELL = WELLUtils::createWELL(pPhrase);
    WELLUtils::showState(pWELL);

    RandomGroupSplitter *prgs = new RandomGroupSplitter(pWELL, vOrig.size()/3, 2*vOrig.size()/3);
    intvec vSplit;

    iResult = prgs->split(vOrig, vSplit);
    xha_printf("vorig after:  %bv\n", vOrig);
    xha_printf("vsplit:       %bv\n", vSplit);
    
    delete prgs;
    delete pWELL;
    return iResult;
}

