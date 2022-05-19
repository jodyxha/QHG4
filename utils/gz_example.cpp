#include <cstdio>

#include "gzutils.h"


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    bool bgzip = true;
    const int GZ_BLOCK_SIZE = 8192;

    if (iArgC > 3) {
        std::string sMode(apArgV[1]);
        std::string sInput(apArgV[2]);
        std::string sOutput(apArgV[3]);

        gzUtils *pgz = gzUtils::createInstance(GZ_BLOCK_SIZE);
        if (pgz != NULL) {
            if (sMode == "-g") {
                iResult = pgz->do_gzip(sInput, sOutput);
            } else if (sMode == "-u") {
                iResult = pgz->do_gunzip(sInput, sOutput);
            } else {
                iResult = -1;
            }
        
            delete pgz;
        } else {
            printf("Couldn't gzUtils instance\n");
            iResult = -1;
        }
    } else {
        printf("usage: %s [\"-g\" | \"-u\"] <input-file> <output-file>\n", apArgV[0]);
        iResult = -1;
    }
    
    return iResult;
}
