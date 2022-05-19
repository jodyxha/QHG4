#include <cstdio>
#include <string>
#include <vector>

#include "ParamReader.h"

typedef unsigned int             uint;
typedef std::vector<std::string> stringvec;

int main (int iArgC, char *apArgV[]) {
    int iResult = -1;
 
    bool   bHelp       = false;
    bool   bGaga       = false;
    double dValue      =  -1;
    int    iSel        =  0;
    char   c1          = '\0';
    std::string sName  = "default_name";
    char  *psOpt       = NULL;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(7,  
                               "-h:0",          &bHelp,
                               "-b:b",          &bGaga,
                               "-v:d!",         &dValue,
                               "--selector:i!", &iSel,
                               "-n:s",          &sName,
                               "--optional:S",  &psOpt,
                               "-t:c",          &c1);
    
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            printf("Successs!\n");
            printf("  bHelp:     %s\n", bHelp?"true":"false"); 
            printf("  dValue:    %f\n", dValue); 
            printf("  iSel:      %i\n", iSel); 
            printf("  sName:     %s\n", sName.c_str()); 
            printf("  psOpt:     %s\n", psOpt);
            if (iResult == 2) {
                stringvec vFree;
                uint iNum = pPR->getFreeParams(vFree);
                    printf("Found %d free param%s\n", iNum, (iNum==1)?"":"s"); 
                for (uint i = 0; i < iNum; i++) {
                    printf("  %s\n", vFree[i].c_str());
                }
                iResult = 0;


            }
            printf("*****\n");
            pPR->display();
            printf("*****\n");
            pPR->display(stdout, false);
        } else {
            printf("Have Error\n");
                printf("code    : %d\n", iResult);
                printf("message : %s\n", pPR->getErrorMessage(iResult).c_str());
        }
        delete pPR;
    } else {
        printf("Error in setOptions\n");
    }
    return iResult;
}
