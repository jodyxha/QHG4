#include <cstdio>
#include <cstdlib>

#include "ParamReader.h"
#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
//#include "stdstrutils.cpp"

#include "LookUpSampler.h"
void usage(std::string sApp) {
    stdprintf("%s - create small sample pngs of the available lookupsn", sApp);
    stdprintf("usage;\n");
    stdprintf("  %s -w <width> -h <height> -l <lookup-name> -ö <output-name>  {-b <border-width>]n", sApp);
    stdprintf("where\n");
    stdprintf("  width         width of image\n");
    stdprintf("  height        height of image\n");
    stdprintf("  lookup-name   name of LookUp\n");
    stdprintf("  output-name   name of output image\n");
    stdprintf("  border-width  thockness of border\n");
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

                        stdprintf("W:    %u\n", iW);
                        stdprintf("H:    %u\n", iH);
                        stdprintf("L:    %s\n", sLookUp);
                        stdprintf("O:    %s\n", sOutput);
                        stdprintf("B:    %u\n", iBorder);
                        
                        LookUpSampler *pLU = LookUpSampler::createInstance(iW, iH, sLookUp);
                        if (pLU != NULL) {
                            
                            iResult = pLU->makeImage(sOutput, iBorder);
                            
                            delete pLU;
                        } else {
                            stdprintf("couldn't create LookUpSampler\n");
                            iResult = -1;
                        }
                    } else{
                        stdprintf("Bad height [%s]\n", vParts[1]);
                    }
                    
                } else{
                    stdprintf("Bad width [%s]\n", vParts[0]);
                }
            } else {
                stdprintf("Expected #<aidth>x<height>\n");
            }
        } else {
            stdprintf("ParamReader result: %d\n", iResult);
            stdprintf("%s: %s %s\n", pPR->getErrorMessage(iResult),  pPR->getBadArg(), pPR->getBadVal());
            usage(apArgV[0]);
            iResult = -1;
        }
        
    } else {
       stdprintf("Error in setOptions\n");
    }
    delete pPR;
    return iResult;
}
