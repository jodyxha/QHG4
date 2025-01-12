#include <cstdio>
#include <cstring>
#include <string>

#include "strutils.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "xha_strutils.cpp"
#include "LookUp.h"
#include "RainbowLookUp.h"
#include "Rainbow2LookUp.h"
#include "GeoLookUp.h"
#include "Geo2LookUp.h"
#include "TwoToneLookUp.h"
#include "FadeOutLookUp.h"
#include "FadeToLookUp.h"
#include "PNGImage.h"

#include "LookUpSampler.h"

const int LU_RAINBOW  = 0;
const int LU_RAINBOW2 = 1;
const int LU_GEO      = 2;
const int LU_GEO2     = 3;
const int LU_TWOTONE  = 4;
const int LU_FADEOUT  = 5;
const int LU_FADETO   = 6;

std::vector<std::string> vsLookUps{
    "Rainbow", 
    "Rainbow2", 
    "Geo", 
    "Geo2", 
    "TwoTone", 
    "FadeOut", 
    "FadeTo"
};


//----------------------------------------------------------------------------
// createInstance
//
LookUpSampler *LookUpSampler::createInstance(uint iW, uint iH, std::string sLookUpName) {
    LookUpSampler *pLUS = new LookUpSampler(iW, iH);
    int iResult = pLUS->init(sLookUpName);
    if (iResult != 0) {
        delete pLUS;
        pLUS =NULL;
    }
    return pLUS;
}


//----------------------------------------------------------------------------
// constructor
//
LookUpSampler::LookUpSampler(uint iW, uint iH)
  : m_iW(iW),
    m_iH(iH),
    m_iLU(-1),
    m_pLookUp(NULL),
    m_ppData(NULL),
    m_ppPNGData(NULL),
    m_pPNGImage(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
LookUpSampler::~LookUpSampler() {
    destroyArrays();
    if (m_pLookUp != NULL) {
        delete m_pLookUp;
    }
    if (m_pPNGImage != NULL) {
        delete m_pPNGImage;
    }
}


//----------------------------------------------------------------------------
// init
//
int LookUpSampler::init(std::string sLookUpName) {
    int iResult = -1;

    iResult = findLookUp(sLookUpName);
    if (iResult == 0) {
        createArrays();
    
    } else {
        xha_printf("Unknown look up [%s]\n", sLookUpName);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// makeImage
//
int LookUpSampler::makeImage(std::string sOutputName, uint iBorder) {
    int iResult = 0;

    
    m_pPNGImage = new PNGImage(m_iW, m_iH, 8, PNG_COLOR_TYPE_RGB_ALPHA);

    if ((m_pLookUp != NULL) && (m_ppData != NULL) && (m_ppPNGData != NULL)) {

        iResult = fillDataArray();
        if (iResult == 0) {
            iResult = fillPNGArray();
            if (iResult == 0) {
                iResult = insertBorder(iBorder);
                 if (iResult == 0) {
                     bool bOK = m_pPNGImage->createPNGFromData(m_ppPNGData, sOutputName.c_str());
                     iResult = bOK?0:-1;
                 } else {                         
                     iResult = -1;
                 }
            } else {
                xha_printf("Couldn't fill PNG arrays\n");
            }
        } else {
            xha_printf("Couldn't fill data arrays\n");
        }

    } else {
        xha_printf("LookUpSampler has not beein initialized\n");
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// findLookUp
//
int LookUpSampler::findLookUp(std::string sLookUpName) {
    int iResult = 0;
    m_iLU = -1;
    for (uint i = 0; (m_iLU < 0) && (i < vsLookUps.size()); i++) {
        if (vsLookUps[i] == sLookUpName) {
            m_iLU = i;
        }
    }
    if (m_iLU >= 0) {
        switch (m_iLU) {
        case LU_RAINBOW:
            m_pLookUp = new RainbowLookUp(0.0, 1.0);
            break;
        case LU_RAINBOW2:
            m_pLookUp = new Rainbow2LookUp(0.0, 1.0);
            break;
        case LU_GEO:
            m_pLookUp = new GeoLookUp(0.0, 0.5, 1.0);
            break;
        case LU_GEO2:
            m_pLookUp = new Geo2LookUp(0.0, 0.5, 1.0);
            break;
        case LU_TWOTONE:
            // from red to green
            m_pLookUp = new TwoToneLookUp(0.5, 
                                          1.0, 0.0, 0.0, 1.0, 
                                          0.0, 1.0, 0.0, 1.0);
            break;
        case LU_FADETO:
            m_pLookUp = new FadeToLookUp(0.0, 1.0,
                                         1.0, 0.0, 0.0, 1.0, 
                                         0.0, 1.0, 0.0, 1.0);
            break;
        case LU_FADEOUT:
            m_pLookUp = new FadeOutLookUp(0.0, 1.0,
                                          0.0, 0.0, 1.0, 1.0);
            break;
        default:
            m_pLookUp = NULL;
        }
    } else {
        iResult = -1;
    }  
 
    return iResult;
}


//----------------------------------------------------------------------------
// createArray
//
void LookUpSampler::createArrays() {
  m_ppPNGData = new uchar*[m_iH];
  for (uint i = 0; i < m_iH; i++) {
      m_ppPNGData[i] = new uchar[4*m_iW];
      memset(m_ppPNGData[i], 0, 4*m_iW*sizeof(uchar));
  }

  m_ppData = new double*[m_iH];
  for (uint i = 0; i < m_iH; i++) {
      m_ppData[i] = new double[m_iW];
      memset(m_ppData[i], 0, m_iW*sizeof(double));
  }
}

//----------------------------------------------------------------------------
// destroyArrays
//
void LookUpSampler::destroyArrays() {
    if (m_ppPNGData != NULL) {
        for (uint i = 0; i < m_iH; i++) {
            if (m_ppPNGData[i] != NULL) {
                delete[] m_ppPNGData[i];
            }
        }
        delete[] m_ppPNGData;
        m_ppPNGData = NULL;
    }

    if (m_ppData != NULL) {
        for (uint i = 0; i < m_iH; i++) {
            if (m_ppData[i] != NULL) {
                delete[] m_ppData[i];
            }
        }
        delete[] m_ppData;
        m_ppData = NULL;
    }

}


//----------------------------------------------------------------------------
// fillArray
//   all lines are equal, going from 0 to 1
//
int LookUpSampler::fillDataArray() {
    for (uint i = 0; i < m_iH; i++) {
        for (uint j = 0; j < m_iW; j++)  {
            m_ppData[i][j] = (j*1.0)/(m_iW-1);
        }
    }  
    return 0;
}

//----------------------------------------------------------------------------
// fillPNGArray
//   
//
int LookUpSampler::fillPNGArray() {
    int iResult = 0;

    for (uint i = 0; i < m_iH; i++) {
        for (uint j = 0; j < m_iW; j++) {
            uchar cRed   = 0;
            uchar cGreen = 0;
            uchar cBlue  = 0;
            uchar cAlpha = 0;

            m_pLookUp->getColor(m_ppData[i][j], cRed, cGreen, cBlue, cAlpha);
            
            m_ppPNGData[i][4*j]   = cRed;
            m_ppPNGData[i][4*j+1] = cGreen;
            m_ppPNGData[i][4*j+2] = cBlue;
            m_ppPNGData[i][4*j+3] = cAlpha;

        }
        
    }
    return iResult;
}


//----------------------------------------------------------------------------
// insertBorder
//
int LookUpSampler::insertBorder(uint iBorder) {
    int iResult = 0;

    if ((2*iBorder < m_iW) && (2*iBorder < m_iH)) {
        xha_printf("inserting border [%u]\n", iBorder);
        for (uint i = 0; i < iBorder; i++) {
            // insert a black line
            for (uint j = 0; j < m_iW; j++) {
                m_ppPNGData[i][4*j]   = 0;
                m_ppPNGData[i][4*j+1] = 0;
                m_ppPNGData[i][4*j+2] = 0;
                m_ppPNGData[i][4*j+3] = 255;

                m_ppPNGData[m_iH-i-1][4*j]   = 0;
                m_ppPNGData[m_iH-i-1][4*j+1] = 0;
                m_ppPNGData[m_iH-i-1][4*j+2] = 0;
                m_ppPNGData[m_iH-i-1][4*j+3] = 255;
            }
        }
        for (uint i = 0; i < m_iH; i++) {
            // insert a black line
            for (uint j = 0; j < iBorder; j++) {
                m_ppPNGData[i][4*j]   = 0;
                m_ppPNGData[i][4*j+1] = 0;
                m_ppPNGData[i][4*j+2] = 0;
                m_ppPNGData[i][4*j+3] = 255;

                m_ppPNGData[i][4*(m_iW-j-1)]   = 0;
                m_ppPNGData[i][4*(m_iW-j-1)+1] = 0;
                m_ppPNGData[i][4*(m_iW-j-1)+2] = 0;
                m_ppPNGData[i][4*(m_iW-j-1)+3] = 255;

            }
        }
    } else {
        xha_printf("Border too fat\n");
        iResult = -1;
    }
    return iResult;
}
