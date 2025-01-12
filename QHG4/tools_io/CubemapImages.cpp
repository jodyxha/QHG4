#include <cstdio>
#include <cstring>
#include <cmath>
#include <hdf5.h>

#include "types.h"
#include "strutils.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "EQsahedron.h"
#include "PNGImage.h"
#include "Geo2LookUp.h"
#include "Rainbow2LookUp.h"
#include "RainbowLookUp.h"
#include "CubemapImages.h"

// these reflect the  GL_TEXTURE_CUBE_MAP_POSITIVE_X,GL_TEXTURE_CUBE_MAP_NEGATIVE_X, etc
int active_clamp[][3] = {
    {1, 2, 1},
    {1, 2,-1},
    {0, 2, 1},
    {0, 2,-1},
    {0, 1, 1},
    {0, 1,-1},
};

//----------------------------------------------------------------------------
// createInstance
//
CubemapImages *CubemapImages::createInstance(std::string sInputQDF, std::string sArrayPath, uint iSize) {
    CubemapImages *pCI = new CubemapImages(iSize);
    int iResult = pCI->init(sInputQDF, sArrayPath);
    if (iResult != 0) {
        delete pCI;
        pCI = NULL;
    }
    return pCI;
}


//----------------------------------------------------------------------------
// destructor
//
CubemapImages::~CubemapImages() {
    if (m_pData != NULL) {
        delete[] m_pData;
    }

    if (m_pImgData != NULL) {
        for (uint i = 0; i < m_iSize; i++) {
            delete[] m_pImgData[i];
        }
        delete[] m_pImgData;
    }

    if (m_pEQ != NULL) {
        delete m_pEQ;
    }
}

//----------------------------------------------------------------------------
// constructor
//
CubemapImages::CubemapImages(uint iSize)
    : m_iNumCells(0),
      m_iNumSubDivs(0),
      m_iSize(iSize),
      m_pData(NULL),
      m_pImgData(NULL),
      m_pEQ(NULL) {
}


//----------------------------------------------------------------------------
// init
//
int CubemapImages::init(std::string sInputQDF, std::string sArrayPath) {
    int iResult = 0;
    
    iResult = readArray(sInputQDF, sArrayPath);
    
    if (iResult == 0) {
        m_pImgData = new double*[m_iSize];
        for (uint i = 0; i < m_iSize; i++) {
            m_pImgData[i] = new double[m_iSize];
            memset(m_pImgData[i], 0, m_iSize*sizeof(double));
        }

        m_pEQ = EQsahedron::createInstance(m_iNumSubDivs, true);
    } else {
        xha_printf("Couldn´t get Array [%s] from [%s]\n", sInputQDF, sArrayPath);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readARray
//
int CubemapImages::readArray(std::string sInputQDF, std::string sArrayPath) {
    int iResult = -1;

    // separates path and array name
    char sArrayGroup[sArrayPath.size()+1];
    strcpy(sArrayGroup, sArrayPath.c_str());
    char *pArrayName = strchr(sArrayGroup, '/');
    if (pArrayName != NULL) {
        *pArrayName++ = '\0';
    }
           
    hid_t hFile = qdf_openFile(sInputQDF.c_str());
    if (hFile != H5P_DEFAULT) {
        xha_printf("file opened: %ld\n", hFile);
        hid_t hGridGroup =  qdf_openGroup(hFile, GRIDGROUP_NAME);
        if (hGridGroup != H5P_DEFAULT) {
            printf("grid group opened: %ld\n", hGridGroup);

            iResult = qdf_extractAttribute(hGridGroup, GRID_ATTR_NUM_CELLS, 1, &m_iNumCells);
            if (iResult == 0) {
                m_iNumSubDivs = sqrt((m_iNumCells-2)/10) - 1;
                qdf_closeGroup(hGridGroup);
                printf("NumCells: %d\n", m_iNumCells);
                m_pData = new double[m_iNumCells];

                xha_printf("opening datsa group [%s]\n", sArrayGroup);
                hid_t hDataGroup =  qdf_openGroup(hFile, sArrayGroup);
                if (hDataGroup != H5P_DEFAULT) {
                    xha_printf("group2 opened: %ld\n", hDataGroup);
                    iResult = qdf_readArray(hDataGroup, pArrayName, m_iNumCells, m_pData);
                    if (iResult == 0) {
                        printf("array read r:%d\n", iResult);
                        
                    } else {
                        iResult = -1;
                        xha_printf("Couldn't read array [%s]\n", pArrayName);
                    }
                    qdf_closeGroup(hDataGroup);
                } else {
                    iResult = -1;
                    xha_printf("Couldn't open group [%s]\n", sArrayPath);
                }
            } else {
                xha_printf("couldn't extract num cells\n");
            }
            //qdf_closeGroup(hGridGroup);
        } else {
            xha_printf("couldn't open group [%s]\n", GRIDGROUP_NAME);
        }
        qdf_closeFile(hFile);
    } else {
        xha_printf("couldn't open file [%s]\n", sInputQDF);
            
    }
 
    return iResult;
}




//----------------------------------------------------------------------------
// createProjections
//
int CubemapImages::createImages(std::string sOutBody) {
    int iResult = -1;

    uint aActives[2];
    PNGImage *pPI = new PNGImage(m_iSize, m_iSize, 8, PNG_COLOR_TYPE_RGB_ALPHA);
    pPI->setVerbose(true);
        Geo2LookUp *pRL = new Geo2LookUp(-5000,0, 5000);
        //Rainbow2LookUp *pRL = new Rainbow2LookUp(-5000,5000);
    //RainbowLookUp *pRL = new RainbowLookUp(-50,30);
    double ***aadData = new double **[m_iSize];
    for (uint i = 0; i < m_iSize; i++) {
        aadData[i] = new double *[m_iSize];
        for (uint j = 0; j < m_iSize; j++) {
            aadData[i][j] = new double[4];
            memset(aadData[i][j], 0, 4*sizeof(double));
        }
    }

    for (uint i = 0; i < 6; i++) {
        aActives[0] = active_clamp[i][0];
        aActives[1] = active_clamp[i][1];
        int iClamp  = active_clamp[i][2];

        iResult = getValues(aActives, iClamp);
        if (iResult == 0) {
            xha_printf("data for image #%u created\n", i);
            for (uint u = 0; u < m_iSize; u++) {
                for (uint v = 0; v < m_iSize; v++) {
                    double dRed   = 0;
                    double dGreen = 0;
                    double dBlue  = 0;
                    double dAlpha = 0;
                    pRL->getColor(m_pImgData[u][v], dRed, dGreen, dBlue, dAlpha);
                    aadData[u][v][0] = dRed;
                    aadData[u][v][1] = dGreen;
                    aadData[u][v][2] = dBlue;
                    aadData[u][v][3] = dAlpha;
                }
            }
            std::string sCurOut =  xha_sprintf("%s_%d.png", sOutBody, i);
            bool bOK = pPI->createPNGFromData(aadData, sCurOut.c_str());
            if (bOK) { // now create images from data
                xha_printf("Image [%s] written ok\n", sCurOut);
                iResult = 0;
            } else {
                xha_printf("COuldn't write image #%d\n", i);
            }
        }

    }
    for (uint i = 0; i < m_iSize; i++) {
        aadData[i] = new double *[m_iSize];
        for (uint j = 0; j < m_iSize; j++) {
            delete[] aadData[i][j];
        }
        delete [] aadData[i];
    }
    delete aadData;

    delete pPI;
    delete pRL;
    return iResult;
}
 

//----------------------------------------------------------------------------
// createProjections
//
int CubemapImages::getValues(uint *pActiveCoords, int iClampVal) {
    int iResult = 0;

    float coords[3];
    // we know pActiveCoords has size 3
    for (uint i = 0; i < 3; i++) {
        coords[i] = iClampVal;
    }
    for (uint u = 0; u < m_iSize; u++) {
        memset(m_pImgData[u], 0, m_iSize*sizeof(double));
        coords[pActiveCoords[1]] = 2.0*u/m_iSize - 1;
        for (uint v = 0; v < m_iSize; v++) {
            coords[pActiveCoords[0]] = 2.0*v/m_iSize - 1;
            Vec3D vCur(coords[0], coords[1], coords[2]);
            int iNode = m_pEQ->findNode(&vCur);
            m_pImgData[u][v] = m_pData[iNode];
            //        printf("img[%d][%d] -> %d: %f\n", u, v, iNode, m_pImgData[u][v]);
                
        } 
    } 

    return iResult;
}

