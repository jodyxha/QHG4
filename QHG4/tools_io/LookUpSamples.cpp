#include <cstdio>
#include <cstdlib>

#include "ParamReader.h"
#include "strutils.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
//#include "xha_strutils.cpp"

#include "LookUpSampler.h"
void usage(std::string sApp) {
    xha_printf("%s - create small sample pngs of the available lookupsn", sApp);
    xha_printf("usage;\n");
    xha_printf("  %s -w <width> -h <height> -l <lookup-name> -ö <output-name>  {-b <border-width>]n", sApp);
    xha_printf("where\n");
    xha_printf("  width         width of image\n");
    xha_printf("  height        height of image\n");
    xha_printf("  lookup-name   name of LookUp\n");
    xha_printf("  output-name   name of output image\n");
    xha_printf("  border-width  thockness of border\n");
}

int main(int iArgC, char *apArgV[]) {

    int iResult = 0;
    uint iH  = 0;
    uint iW  = 0;
    std::string sSize   = "";
    std::string sLookUp = "";
    std::string sOutput = "";
    uint iBorder = 0;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(4,  
                               "-s:s!",   &sSize,
                               "-l:s!",   &sLookUp,
                               "-o:s!",   &sOutput,
                               "-b:i",    &iBorder);
    
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            iResult = 1;
            stringvec vParts;
            uint iNum = splitString(sSize, vParts, "x", false);
            if (iNum == 2) {
                if (strToNum(vParts[0], &iW)) {
                    if (strToNum(vParts[1], &iH)) {

                        xha_printf("W:    %u\n", iW);
                        xha_printf("H:    %u\n", iH);
                        xha_printf("L:    %s\n", sLookUp);
                        xha_printf("O:    %s\n", sOutput);
                        xha_printf("B:    %u\n", iBorder);
                        
                        LookUpSampler *pLU = LookUpSampler::createInstance(iW, iH, sLookUp);
                        if (pLU != NULL) {
                            
                            iResult = pLU->makeImage(sOutput, iBorder);
                            
                            delete pLU;
                        } else {
                            xha_printf("couldn't create LookUpSampler\n");
                            iResult = -1;
                        }
                    } else{
                        xha_printf("Bad height [%s]\n", vParts[1]);
                    }
                    
                } else{
                    xha_printf("Bad width [%s]\n", vParts[0]);
                }
            } else {
                xha_printf("Expected #<aidth>x<height>\n");
            }
        } else {
            xha_printf("ParamReader result: %d\n", iResult);
            xha_printf("%s: %s %s\n", pPR->getErrorMessage(iResult),  pPR->getBadArg(), pPR->getBadVal());
            usage(apArgV[0]);
            iResult = -1;
        }
        
    } else {
       xha_printf("Error in setOptions\n");
    }
    delete pPR;
    return iResult;
}
