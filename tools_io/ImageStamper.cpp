#include <cstdio>

#include "strutils.h"
#include "ParamReader.h"
#include "PNGImage.h"
#include "TextRenderer.h"
#include "AlphaComposer.h"


#define DEF_SIZE 24
#define DEF_OFF   7

//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - render text into image\n", pApp);
    printf("usage\n");
    printf("  %s -i <PNG_in> -t <text> -o <PNG_out> [-p <x>:<y>] [-s <size>] [-c <color_def>]\n", pApp);
    printf("where\n");
    printf("  PNG_in      PNG file to be written into\n");
    printf("  text        text to write\n");
    printf("  PNG_out     name of output PNG\n");
    printf("  x           x-offset at which to write text\n");
    printf("  y           y-offset at which to write text (refers to character baseline)\n");
    printf("  size        font size (default %d)\n", DEF_SIZE);
    printf("  color_def   text color definition with format RRGGBBAA (default: white)\n");
    printf("If x and y are omitted, the text is written close to the bottom left of the image\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// writeToPNG
//  write the text at position (iX,iY) (lower left corner of character base)
//  wit font size iSize
//
int addRenderText(AlphaComposer *pAC, char *pText, int iSize, int iX, int iY, int iW, int iH, double *pColor) {
    int iResult = 0;

    uchar **ppTimeData = NULL;
    if ((pText != NULL) && (*pText != '\0')) {
        TextRenderer *pTR = TextRenderer::createInstance(iW, iH);
        if (pTR != NULL) {
            pTR->setFontSize(iSize);
            pTR->setColor(pColor[0], pColor[1], pColor[2], pColor[3]);  // rgba

            if (iX < 0) {
                iX = DEF_OFF;
            }
            if (iY < 0) {
                iY = iH - DEF_OFF;
            }
            printf("adding text at pos (%d,%d)\n", iX, iY);
            pTR->addText(pText, iX, iY);
            
            ppTimeData = pTR->createData();

            pAC->addPNGData(ppTimeData);
            
            pTR->deleteArray(ppTimeData);
            
            delete pTR;
        } else {
            printf("Couldn't create TextRenderer\n");
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeToPNG
//   write the data toaNG file
//
int writeToPNG(const char *pOut, uchar **ppData, int iW, int iH) {
    int iResult = -1;
    PNGImage *pPI = new PNGImage(iW, iH);
    if (pPI != NULL) {
        printf("Have out PI %p\n", pPI);
        bool bOK = pPI->createPNGFromData(ppData, pOut);
        if (bOK) {
            iResult = 0;
        } else {
            iResult = -1;
        }
        
        delete pPI;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// deleteArray
//
void deleteArray(uchar **ppData, int iH) {
    for (int i = 0; i < iH; ++i) {
        delete[] ppData[i];
    }
    delete[] ppData;
}


//----------------------------------------------------------------------------
// splitColor
//
int splitColor(char *sColor, double *pColor) {
    int iResult = -1;
    int i = 0;

    if (strlen(sColor) == 8) {
        iResult = 0;
        char *p = sColor;
        char c[3];
        memset(c, 0, 3);
        int i = 0;
        while ((iResult == 0) && (i < 4)) {
            memcpy(c, p, 2);
            int iVal = -1;
            if (readHex(c, &iVal)) {
                pColor[i++] = (1.0*iVal)/255.0;
                printf("have #%d: %s=%d\n", i, c, iVal);
                p += 2;
            } else {
                printf("[%s] is not a hex number\n", p);
                iResult = -1;
            }
        }
    } else {
        printf("color definition should have length 8 (RRGGBBAA), not %zd\n", strlen(sColor));
    }
    return iResult;
}
        

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult   = 0;
    char *sInput  = NULL;
    char *sText   = NULL;
    char *sPos    = NULL;
    char *sOutput = NULL;
    char *sColor  = NULL;
    int iX  = -1;
    int iY  = -1;
    int iSize = 24;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(6,  
                               "-i:S!",   &sInput,
                               "-t:S!",   &sText,
                               "-p:S",    &sPos,
                               "-s:i",    &iSize,
                               "-c:S",    &sColor, 
                               "-o:S",    &sOutput);
    
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {

            iResult = -1;
            char *p = strchr(sPos, ':');
            if (p != 0) {
                *p = 0;
                p++;
                if (strToNum(sPos, &iX)) {
                    if (strToNum(p, &iY)) {
                        iResult = 0;
                    } else {
                        printf("Not a number : [%s]\n", p); 
                    }
                } else {
                    printf("Not a number : [%s]\n", sPos); 
                }
            } else {
                printf("Expected '<x>:<y>' instead of '%s'\n", sPos); 
            }

            double aColor[4];
            if (iResult  == 0) {
                for (int i = 0; i < 4; i++) {
                    aColor[i] = 1.0;
                }

                if (sColor != NULL) {
                    iResult = splitColor(sColor, aColor);
                }
            }

            if (iResult == 0) {
                PNGImage *pPI = new PNGImage();
                if (pPI != NULL) {
                    printf("Have in PI %p\n", pPI);
                
                    if (pPI->readPNGFile(sInput)) {
                        uchar **pucRows = pPI->getRows();;
                        int iW  = pPI->getWidth();
                        int iH  = pPI->getHeight();
                        int iBD = pPI->getBitDepth();
                        int iCT = pPI->getColorType();  
                        printf("%dx%d, bf %d, ct %d\n", iW, iH, iBD, iCT);
                    
                        AlphaComposer *pAC = AlphaComposer::createInstance(iW, iH);
                        if (pAC != NULL) {
                
                            pAC->addPNGData(pucRows);
                            iResult = addRenderText(pAC, sText, iSize, iX, iY, iW, iH, aColor);    
                            if (iResult == 0) {
                                iResult = writeToPNG(sOutput, pAC->getData(), iW, iH);
                            } else {
                                printf("couldn't add render text\n");
                            }
                            delete pAC;
                            //deleteArray(pucRows, iH);
                        } else {
                            printf("couldn't alphacomposer\n");
                        }
                    } else {
                        printf("couldn't read PNG file [%s]\n", sInput);
                    }
        
                    delete pPI;
                } else {
                    printf("couldn't create PNGImage\n");
                }
            }
        } else {
            printf("ParamReader result: %d\n", iResult);
            printf("%s: %s %s\n", pPR->getErrorMessage(iResult).c_str(),  pPR->getBadArg().c_str(), pPR->getBadVal().c_str());
            usage(apArgV[0]);
            iResult = -1;
       }
    
    } else {
        printf("Error in setOptions\n");
    }
    return iResult;
}
