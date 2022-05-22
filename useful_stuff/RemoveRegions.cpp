#include <cstdio>
#include <cstring>
#include "types.h"
#include "RegionRemover.h"


//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - remove specified regions from QHG file\n", pApp);
    printf("Usage:\n");
    printf("  %s <QDFFile> [-i] <RegionName>[\":\"<RegionName>]*\n", pApp);
    printf("where\n");
    printf("QDFFile     QDF file to be modified\n");
    printf("-i          invert (i.e. remove everything except named region)\n");
    printf("RegionName  one of the following'\n");
    stringvec vs;
    RegionRemover::getRegionNames(vs);
    for (uint i = 0; i < vs.size(); i++) {
        printf("              %s\n", vs[i].c_str());
    }
}

//----------------------------------------------------------------------------
// splitNameString
//
int splitNameString(char *pNameList, stringvec &vs) {
    int iResult = 0;
    char *p = strtok(pNameList, ":");
    while (p != NULL) {
        vs.push_back(p);
        p = strtok(NULL, ":");
    }
    if (vs.size() == 0) {
        iResult = -1;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    if (iArgC > 2) {
        bool bInvert = false;
        int iLast = 2;
        if (strcmp(apArgV[2], "-i") == 0) {
            bInvert = true;
            iLast = 3;
        }
        
        if (iArgC > iLast) {
            printf("Invalid name list\n");
            stringvec vs;
            iResult = splitNameString(apArgV[iLast], vs);
            if (iResult == 0) {
                RegionRemover *pRR = RegionRemover::createInstance(apArgV[1], vs);
                if (pRR != NULL) {
                    pRR->displayPolys();
                    printf("Starting to remove\n");
                    if (bInvert) {
                        iResult = pRR->removeRegionsInvert();
                    } else {
                        iResult = pRR->removeRegions();
                    }
                    if (iResult == 0) {
                        printf("+++ success +++\n");
                    } else {
                        printf("--- failure ---\n");
                    }
                    delete pRR;
                }
            } else {
                printf("Invalid name list\n");
            }
        } else {
            printf("Expected regions after '-v'\n");
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
