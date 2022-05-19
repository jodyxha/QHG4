#include <cstdio>
#include <string>

#include "LineReader.h"

int main(int iArgC, char *apArgV[]) {
    if (iArgC > 1) {
        LineReader *pLR = LineReader_std::createInstance(apArgV[1], "rt");
        if (pLR != NULL) {
            int i = 1;
            char *pLine = pLR->getNextLine(GNL_IGNORE_NONE);
            while ((pLine != NULL) && (!pLR->isEoF())) {
                printf("[%05d] %s", i++, pLine);
                pLine = pLR->getNextLine(GNL_IGNORE_NONE);
            }
            
            delete pLR;
        }
    } else {
        printf("Usage: %s <text-file>\n", apArgV[0]);
    }
    return 0;

}
