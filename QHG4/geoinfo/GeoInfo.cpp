#include <cstring>
#include <cstdlib>
#include <cmath>

#include "qhg_consts.h"
#include "strutils.h"
//#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "MessLoggerT.h"
#include "GeoInfo.h"

#include "Projector.h"
#include "EQRProjector.h"
#include "OrthoProjector.h"
#include "AEProjector.h"
#include "TCEAProjector.h"
#include "LAEAProjector.h"
#include "LCCProjector.h"
#include "CEAProjector.h"
#include "LINProjector.h"

#include "GeoProvider.h"
#include "PrGeoProvider.h"
#include "PlaneGeoProvider.h"

/*
static const char *asProjNames[] = {
    "Equirectangular",
    "Orthographic Projector",
    "Azimuthal Equidistant Projector",
    "Transverse Cylindrical Equal Area Projector",
    "Lambert Azimuthal Equal Area Projector",
    "Lambert Conformal Conical Projector",
};

*/

GeoInfo* GeoInfo::s_pGeoInfo = NULL;

const double EPS = 1E-6;
double static relativeError(double d1, double d2) {
    double dDiff = fabs(d1-d2);
    if (d1 != 0) {
        dDiff /= d1;
    } else if (d2 != 0) {
        dDiff /= d2;
    }
    return dDiff;
}


//----------------------------------------------------------------------------
// ProjType::ToString
//
std::string ProjType::toString(bool bDegrees) const {
    std::string sPrType = xha_sprintf("%d [%s] %f %f %zd",
                                     m_iProjType,
                                     GeoInfo::getName(m_iProjType),
                                     m_dLambda0*(bDegrees?180/Q_PI:1),
                                     m_dPhi0*(bDegrees?180/Q_PI:1),
                                     m_vdAdd.size());
    for (uint i =0; i < m_vdAdd.size(); ++i) {
        sPrType += xha_sprintf(" %f", m_vdAdd[i]);
    }
    return sPrType;
}


//----------------------------------------------------------------------------
// ProjType::fromString
//   <projtype> "["<projname>"]" <lambda> <phi> <iNumAdd> [<additional>]* 
int ProjType::fromString(const std::string sLine, bool bDegrees) {
    int iResult = 0;
    uint iNumAdd = 0;
    stringvec vParts;
    size_t iOpen  = sLine.find("[");
    size_t iClose = sLine.find("]");

    std::string sLine1 = sLine;
    if ((iOpen != std::string::npos) && (iClose != std::string::npos)) {
        sLine1 = sLine.substr(0, iOpen-1) + sLine.substr(iClose+1);
    }

    uint iNum = splitString(sLine1, vParts, " ", false);
    if (iNum >= 4) {
        if (strToNum(vParts[0], &m_iProjType) && 
            strToNum(vParts[1], &m_dLambda0)  && 
            strToNum(vParts[2], &m_dPhi0)     && 
            strToNum(vParts[3], &iNumAdd)) {
            
            iResult = 0;
            if (bDegrees) {
                m_dLambda0 *= Q_PI/180;
                m_dPhi0    *= Q_PI/180;
            }
            
            if (iNumAdd > 0) {
                 if (iNum == (iNumAdd + 4)) {
                    for (uint i = 4; (iResult == 0) && (i < iNum); i++) {
                        double dTemp;
                        if (strToNum(vParts[i], &dTemp)) {
                            m_vdAdd.push_back(dTemp);
                        } else {
                        iResult = -1;
                        }
                    }
                } else {
                    // wrong number of additional params
                    iResult = -1;
                }
            } else {
                if (iNumAdd < 0) {
                    // negative number of addiionals
                    iResult = -1;
                }
            }
        } else {
            // couldn't convert to numbers
            iResult = -1;
        }
    } else {
        // not enough params
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// ProjType constructor
//
ProjType::ProjType()
    : m_iProjType(-1),
      m_dLambda0(dNaN),
      m_dPhi0(dNaN),
      m_iNumAdd(0) {
    for (int i =0; i < MAX_ADD; ++i) {
        m_adAdd[i] = dNaN;
    }
}


//----------------------------------------------------------------------------
// ProjType constructor
//
ProjType::ProjType(int iProjType, 
                   double dLambda0, 
                   double dPhi0, 
                   std::vector<double> vdAdd)
    : m_iProjType(iProjType),
      m_dLambda0(dLambda0),
      m_dPhi0(dPhi0),
      m_vdAdd(vdAdd) {
    
}


//----------------------------------------------------------------------------
// ProjType copy constructor
//
void ProjType::copy(const ProjType *pt) {
    m_iProjType = pt->m_iProjType;
    m_dLambda0  = pt->m_dLambda0;
    m_dPhi0     = pt->m_dPhi0;
    m_iNumAdd   = pt->m_iNumAdd;
    memcpy(m_adAdd, pt->m_adAdd, MAX_ADD*sizeof(double));
}


//----------------------------------------------------------------------------
// ProjType comparator isEqual
//
bool ProjType::isEqual(ProjType *pPT) {
    
    bool bEqual = (m_iProjType == pPT->m_iProjType) &&
        (relativeError(m_dLambda0, pPT->m_dLambda0) < EPS) &&
        (relativeError(m_dPhi0, pPT->m_dPhi0) < EPS)  &&
        (m_iNumAdd == pPT->m_iNumAdd);
    if (bEqual) {
        for (int i = 0; bEqual && (i < m_iNumAdd); ++i) {
            bEqual = bEqual && (relativeError(m_adAdd[i], pPT->m_adAdd[i]) < EPS);
        }
    }
    return bEqual;
}


//---------------------------------------------------
// serialize
//
unsigned char *ProjType::serialize(unsigned char *p) const {
    p = putMem(p, &m_iProjType, sizeof(int));
    p = putMem(p, &m_dLambda0, sizeof(double));
    p = putMem(p, &m_dPhi0, sizeof(double));
    p = putMem(p, &m_iNumAdd, sizeof(int));
    p = putMem(p, m_adAdd, MAX_ADD*sizeof(double));

    return p;
}


//---------------------------------------------------
// deserialize
//
unsigned char *ProjType::deserialize(unsigned char *p) {
    p = getMem(&m_iProjType, p, sizeof(int));
    p = getMem(&m_dLambda0,  p, sizeof(double));
    p = getMem(&m_dPhi0,     p, sizeof(double));
    p = getMem(&m_iNumAdd,   p, sizeof(int));
    p = getMem(m_adAdd,      p, MAX_ADD*sizeof(double));

    return p;
}


//---------------------------------------------------
// createPT
//   assumes input of form
//   <Type>:<long0>:<lat0>:<NumPar>(:<Par>)*
//   ptDef is a valid projection type to be used if string is invalid
//   bDegrees - if true inputs are degrees else radians
//
ProjType *ProjType::createPT(const std::string sPTData, bool bDegrees) {
    bool bOK=true;
    ProjType *pPT   = NULL;
    int iPType      = -1;
    double dLambda0 = dNaN;
    double dPhi0    = dNaN;
    uint iNumAdd     = 0;

    std::vector<double> vdAdds;
    stringvec vParts;
    uint iNum = splitString(sPTData, vParts, ":");

    if (iNum > 4) {
        if (strToNum(vParts[0], &iPType)) {
            if (strToNum(vParts[1], &dLambda0)) {
                if (strToNum(vParts[2], &dPhi0)) {
                    if (bDegrees) {
                        dLambda0 *= Q_PI/180;
                        dPhi0 *= Q_PI/180;
                    }   
                    if (strToNum(vParts[3], &iNumAdd)) {
                        
                        xha_printf("Type:    %s -> %f\n", vParts[0], iPType);
                        xha_printf("Lambda:  %s -> %f\n", vParts[1], dLambda0);
                        xha_printf("Phi:     %s -> %f\n", vParts[2], dPhi0);
                        xha_printf("INumAdd: %s -> %d\n", vParts[3], iNumAdd);

                        if (iNumAdd > 0) {
                            if (iNum == (iNumAdd + 4)) {
                                for (uint i = 4; bOK && (i < iNum); i++) {
                                    double dTemp;
                                    if (strToNum(vParts[i], &dTemp)) {
                                        vdAdds.push_back(dTemp);
                                    } else {
                                        LOG_ERROR("[createPT] invalid number for parameter #%d [%s]\n", i, vParts[i]);
                                        bOK = false;
                                    }
                                }
                            } else {
                                LOG_ERROR("[createPT] mismatch between additional + normal  and total [%d]+[%d] <-> [%d]\n", iNumAdd, 4, iNum);
                                bOK = false;
                            }
                        }

                    } else {
                        LOG_ERROR("[createPT] invalid number for parameter NumAdd [%s]\n", vParts[3]);
                        bOK = false;
                    }
                } else {
                    LOG_ERROR("[createPT] invalid number for parameter Phi0 [%s]\n", vParts[2]);
                    bOK = false;
                }
            } else {
                LOG_ERROR("[createPT] invalid number for parameter Lambda0 [%s]\n", vParts[1]);
                bOK = false;
            }
        } else {
            LOG_ERROR("[createPT] invalid number for parameter PType [%s]\n", vParts[0]);
            bOK = false;
        }
    } else {
        LOG_ERROR("[createPT] not enough params number [%s]\n",sPTData);
        bOK = false;
    }

    if (bOK) {
        pPT = new ProjType(iPType, dLambda0, dPhi0, vdAdds);
    }
    return pPT;
}



//----------------------------------------------------------------------------
// ProjGrid::toString
//
char *ProjGrid::toString() const {
    static char s_sPrData[256];
    sprintf(s_sPrData, "%d %d %f %f %f %f %f", 
            m_iGridW,
            m_iGridH,
            m_dRealW,
            m_dRealH,
            m_dOffsX,
            m_dOffsY,
            m_dRadius);
    return s_sPrData;
}
//----------------------------------------------------------------------------
// ProjGrid::fromString
//
int ProjGrid::fromString(const std::string sLine) {
    int iResult = 0;
    int iRead = sscanf(sLine.c_str(), "%d %d %lf %lf %lf %lf %lf",
            &m_iGridW,
            &m_iGridH,
            &m_dRealW,
            &m_dRealH,
            &m_dOffsX,
            &m_dOffsY,
            &m_dRadius);
    if (iRead != 7) {
        iResult = -1;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// ProjGrid constructor
//
ProjGrid::ProjGrid() 
    :    m_iGridW(0),
         m_iGridH(0),
         m_dRealW(0),
         m_dRealH(0),
         m_dOffsX(0),
         m_dOffsY(0),
         m_dRadius(0) {
}

//----------------------------------------------------------------------------
// ProjGrid constructor
//
ProjGrid::ProjGrid(int iGridW, 
                   int iGridH, 
                   double dRealW, 
                   double dRealH, 
                   double dOffsX,
                   double dOffsY,
                   double dRadius) 
    :    m_iGridW(iGridW),
         m_iGridH(iGridH),
         m_dRealW(dRealW),
         m_dRealH(dRealH),
         m_dOffsX(dOffsX),
         m_dOffsY(dOffsY),
         m_dRadius(dRadius) {
}

//----------------------------------------------------------------------------
// ProjGrid copy constructor
//
void ProjGrid::copy(const ProjGrid *ppg) {
    m_iGridW  = ppg->m_iGridW;
    m_iGridH  = ppg->m_iGridH;
    m_dRealW  = ppg->m_dRealW;
    m_dRealH  = ppg->m_dRealH;
    m_dOffsX  = ppg->m_dOffsX;
    m_dOffsY  = ppg->m_dOffsY;
    m_dRadius = ppg->m_dRadius;

}

//----------------------------------------------------------------------------
// ProjGrid comparator isEqual
//
bool ProjGrid::isEqual(ProjGrid *pPG) {
    
    bool bEqual = (m_iGridW == pPG->m_iGridW) &&
        (m_iGridH  == pPG->m_iGridH) &&
        (relativeError(m_dRealW, pPG->m_dRealW) < EPS) &&
        (relativeError(m_dRealH, pPG->m_dRealH) < EPS)  &&
        (relativeError(m_dOffsX, pPG->m_dOffsX) < EPS)  &&
        (relativeError(m_dOffsY, pPG->m_dOffsY) < EPS)  &&
        (relativeError(m_dRadius, pPG->m_dRadius) < EPS);
    return bEqual;
}
//---------------------------------------------------
// serialize
//
unsigned char *ProjGrid::serialize(unsigned char *p) const {
    p = putMem(p, &m_iGridW,  sizeof(double));
    p = putMem(p, &m_iGridH,  sizeof(double));
    p = putMem(p, &m_dRealW,  sizeof(double));
    p = putMem(p, &m_dRealH,  sizeof(double));
    p = putMem(p, &m_dOffsX,  sizeof(double));
    p = putMem(p, &m_dOffsY,  sizeof(double));
    p = putMem(p, &m_dRadius, sizeof(double));
    return p;
}

//---------------------------------------------------
// deserialize
//
unsigned char *ProjGrid::deserialize(unsigned char *p) {
    p = getMem(&m_iGridW,  p, sizeof(double));
    p = getMem(&m_iGridH,  p, sizeof(double));
    p = getMem(&m_dRealW,  p, sizeof(double));
    p = getMem(&m_dRealH,  p, sizeof(double));
    p = getMem(&m_dOffsX,  p, sizeof(double));
    p = getMem(&m_dOffsY,  p, sizeof(double));
    p = getMem(&m_dRadius, p, sizeof(double));

    return p;
}

//---------------------------------------------------
// createPD
//   assumes input of form
//   <GridW>:<GridH>:<RealW>:<RealH>:<OffX>:<OffY>:<R>
//   pdDef is a valid ProjGrid, to be used if string is invalid
//   (pd does not have any input in degrees/radians)
//
ProjGrid * ProjGrid::createPG(const std::string sPGData) {
   
    ProjGrid *pPG = NULL;
    
    bool bOK = true;
    std::vector<double> vdAdd;
    stringvec vParts;
    uint iNum = splitString(sPGData, vParts, ":");
    if (iNum ==  7) {
        for (uint i = 0; bOK && (i < iNum); i++) {
            if ((i == 4) && (vParts[i] == "c")) {
                vdAdd.push_back(dNaN);
            } else if ((i == 5) && (vParts[i] == "c")) {
                vdAdd.push_back(dNaN);
            } else {
                double dDummy = 0;
                if (strToNum(vParts[i], &dDummy)) {
                    vdAdd.push_back(dDummy);
                } else {
                    LOG_ERROR("[createPG] not a number for param %d: [%s]\n", i, vParts[i]);
                    vdAdd.push_back(dNaN);
                }
            }
        }

        if (std::isnan(vdAdd[4])) {
            vdAdd[4] = -vdAdd[0]/2;
        }
        if (std::isnan(vdAdd[5])) {
            vdAdd[5] = -vdAdd[1]/2;
        }
        pPG = new ProjGrid((int)vdAdd[0], (int)vdAdd[1],
                           vdAdd[2], vdAdd[3], vdAdd[4], vdAdd[5], vdAdd[6]);

    } else {
        LOG_ERROR("[createPG] wron number of patameters: ecpected 7 instead of %d\n", iNum);
        bOK = false;
    }

    return pPG;
}


//===========================================================================

//----------------------------------------------------------------------------
// constructor
//
GeoInfo::GeoInfo() {
}

//----------------------------------------------------------------------------
// instance
//
GeoInfo *GeoInfo::instance() {
    if (s_pGeoInfo == NULL) {
        s_pGeoInfo = new GeoInfo();
    }
    return s_pGeoInfo;
}

//----------------------------------------------------------------------------
// free
//
void GeoInfo::free() {
    if (s_pGeoInfo != NULL) {
        delete s_pGeoInfo;
        s_pGeoInfo = NULL;
    }
}

//----------------------------------------------------------------------------
// createProjector
//
Projector *GeoInfo::createProjector(int iType, double dLambda0, double dPhi0) {
    Projector *pr = NULL;
    
    switch (iType) {
    case PR_EQUIRECTANGULAR :
        pr = new EQRProjector(dLambda0, dPhi0);
        break;
    case PR_ORTHOGRAPHIC :
        pr = new OrthographicProjector(dLambda0, dPhi0);
        break;
    case PR_AZIMUTHAL_EQUIDISTANT :
        pr = new AzimuthalEquidistantProjector(dLambda0, dPhi0);
        break;
    case PR_TRANSVERSE_CYLINDRICAL_EQUAL_AREA :
        pr = new TransverseCylindricalEqualAreaProjector(dLambda0, dPhi0);
        break;
    case PR_LAMBERT_AZIMUTHAL_EQUAL_AREA :
        pr = new LambertAzimuthalEqualAreaProjector(dLambda0, dPhi0);
        break;
    case PR_LAMBERT_CONFORMAL_CONIC :
        pr = new LambertConformalConicalProjector(dLambda0, dPhi0);
        break;
    case PR_CYLINDRICAL_EQUAL_AREA :
        pr = new CylindricalEqualAreaProjector(dLambda0, dPhi0);
        break;
    case PR_LINEAR :
        pr = new LINProjector(dLambda0, dPhi0);
        break;
    }

    return pr;
}

//----------------------------------------------------------------------------
// createProjector
//
Projector *GeoInfo::createProjector(const ProjType *pPT) {

    Projector *pr = createProjector(pPT->m_iProjType, pPT->m_dLambda0, pPT->m_dPhi0);
  
    if (pr != NULL) {
        pr->setAdditional(pPT->m_iNumAdd, pPT->m_adAdd);
    }

    return pr;
}

//----------------------------------------------------------------------------
// createProjector
// 
GeoProvider *GeoInfo::createGeoProvider(const ProjType *pPT, const ProjGrid *pPG) {
    GeoProvider *pGP = NULL;
    if (pPT->m_iProjType == PR_LINEAR) {
        pGP = new PlaneGeoProvider(pPG);
    } else {
        Projector *pr = GeoInfo::instance()->createProjector(pPT);
        pGP = new PrGeoProvider(pPG, pr);
    }
    return pGP;
}
