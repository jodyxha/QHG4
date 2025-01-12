#include <cstdio>
#include <cstring>


#include "types.h"
#include "strutils.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "ParamReader.h"
#include "CubemapImages.h"


#include "QDFUtils.h"
#include "QDFUtilsT.h"

//----------------------------------------------------------------------------
// usage
//
void usage(std::string sApp) {
    xha_printf("%s - create 6 images for OGL cubemaps\n", sApp);
    xha_printf("Usage:\n");
    xha_printf("CVompile: %s\n", "g++ -g -Wall QDF2CubemapImages.cpp CubemapImages.cpp -I../utils -I../icosa  -I../io -I../visual  -L../icosa -lIcosa -L../io -lIO -L../visual -lVisual -L../utils -lUtils -lhdf5 -lz -lpng");
}


//----------------------------------------------------------------------------
// readArray
//
double *readArray(const char *pInputQDF, char *pArrayPath, int *piNumCells) {
    double *pData = NULL;
    int iResult = -1;
    char *pArrayName = strchr(pArrayPath, '/');
    if (pArrayName != NULL) {
        *pArrayName++ = '\0';
    }
    hid_t hFile = qdf_openFile(pInputQDF);
    if (hFile != H5P_DEFAULT) {
        printf("file opened: %ld\n", hFile);
        hid_t hGridGroup =  qdf_openGroup(hFile, GRIDGROUP_NAME);
        if (hGridGroup != H5P_DEFAULT) {
            printf("grid group opened: %ld\n", hGridGroup);
            int iNumCells = 0;
            iResult = qdf_extractAttribute(hGridGroup, GRID_ATTR_NUM_CELLS, 1, piNumCells);
            if (iResult == 0) {
                qdf_closeGroup(hGridGroup);
                printf("NumCells: %d\n", *piNumCells);
                pData = new double[*piNumCells];

                hid_t hDataGroup =  qdf_openGroup(hFile, pArrayPath);
                if (hDataGroup != H5P_DEFAULT) {
                    printf("group2 opened: %ld\n", hDataGroup);
                    iResult = qdf_readArray(hDataGroup, pArrayName, *piNumCells, pData);
                    if (iResult == 0) {
                        printf("array read r:%d\n", iResult);
                        
                    } else {
                        printf("Couldn't read array\n");
                    }
                    qdf_closeGroup(hDataGroup);
                } else {
                    printf("Couldn't open group [%s]\n", pArrayPath);
                }
            } else {
                printf("couldn't extract num cells\n");
            }
            //qdf_closeGroup(hGridGroup);
        } else {
            printf("couldn't open group [%s]\n", GRIDGROUP_NAME);
        }
        qdf_closeFile(hFile);
    } else {
        printf("couldn't open file [%s]\n", pInputQDF);
            
    }
    
    if ((iResult != 0) && (pData != NULL)) {
        printf("deleting\n");
        delete[] pData;
        pData = NULL;
    }
    printf("returning [%p]\n", pData);
    return pData;

}

//int getValues(int *active_coords, int clamp_val, 
/*
int main1(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char *sInputQDF   = NULL;
    char *sArrayPath  = NULL;
    char *sOutputBody = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(3,
                                   "-i:S!",      &sInputQDF,
                                   "-a:S!",      &sArrayPath,
                                   "-o:S!",      &sOutputBody);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                int iNumCells = 0;
                int iNumSubDivs = 0;
                //     EQsahedron *pEQ = NULL;
                double *pData = readArray(sInputQDF, sArrayPath, &iNumCells &iNumSubDivs);
                if (pData != NULL) {
                    printf("Got arary with %d elements\n", iNumCells);
                    for (uint i = 0; i < 50; i++) {
                        printf("%f  ", pData[i]);
                    }
                    printf("\n");
                    iResult = 0;
                    iNumSubDivs = sqrt((iNumCells-2)/10) - 1;
                    printf("Calculated subdivs: %dn", iNumSubDivs);
                    //    pEQ = EQsahedron::createInstance(iNumSubDivs, true);
                } else {
                    printf("Couldn´t read arrays\n");
                }

                if (iResult == 0) {
                    
                    //must create images in this order:
                    // GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 

                }
                // todo load eqsahedron
                // need additional input iSize
                // project each pixel of an array corresponding to a unit cube pixel to sphere
                // use eqsahedron to determine node, get value for array 
            }
        }
    
    }            
    return iResult;
}
*/

int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char *sInputQDF   = NULL;
    char *sArrayPath  = NULL;
    char *sOutputBody = NULL;
    uint iSize = 0;

    ParamReader *pPR = new ParamReader();
   
    pPR->setVerbose(true);
    bool bOK = pPR->setOptions(4,
                               "-i:S!",      &sInputQDF,
                               "-a:S!",      &sArrayPath,
                               "-s:i!",      &iSize,
                               "-o:S!",      &sOutputBody);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            iResult = -1;
            CubemapImages *pCI = CubemapImages::createInstance(sInputQDF, sArrayPath, iSize);
            if (pCI != NULL) {
                    
                iResult = pCI->createImages(sOutputBody);
                if (iResult == 0) {
                    xha_printf("success");
                } else {
                    xha_printf("couldn#t create images\n");
                }
                delete pCI;
            } else {
                xha_printf("Couldn#t extract array [%s] from [%s]\n", sArrayPath, sInputQDF);
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
    delete pPR;
    return iResult;
}
