#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "geomutils.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "ConfinedMove.h"

template<typename T>
const char *ConfinedMove<T>::asNames[] = {
    ATTR_CONFINEDMOVE_X_NAME,
    ATTR_CONFINEDMOVE_Y_NAME,
    ATTR_CONFINEDMOVE_R_NAME};


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
ConfinedMove<T>::ConfinedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID/*, std::vector<int>** vMoveList*/) 
    : Action<T>(pPop,pCG,ATTR_CONFINEDMOVE_NAME,sID),
    m_vMoveList(pPop->getMoveList()),
      m_bAllowed(NULL),
      m_bAbsorbing(false) {

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
ConfinedMove<T>::~ConfinedMove() {

    if (m_bAllowed != NULL) {
        delete[] m_bAllowed;
    }

}


//-----------------------------------------------------------------------------
// preLoop
//  the classification of allowed and forbidden cells must only be made once 
//
template<typename T>
int ConfinedMove<T>::preLoop() {
    

    m_bAllowed = new bool[this->m_pCG->m_iNumCells];

    memset(m_bAllowed, 0, this->m_pCG->m_iNumCells * sizeof(bool));

    double* dLon = this->m_pCG->m_pGeography->m_adLongitude;
    double* dLat = this->m_pCG->m_pGeography->m_adLatitude;

    if (this->m_pCG->m_iType == GRID_TYPE_FLAT4 ||
        this->m_pCG->m_iType == GRID_TYPE_FLAT6) {
        
        double dRC2 = m_dR * m_dR;

#pragma omp parallel for
        for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
            double dR2 = (m_dX - dLon[i]) * (m_dX - dLon[i]) + (m_dY - dLat[i]) * (m_dY - dLat[i]);
            if (dR2 < dRC2) {
                m_bAllowed[i] = true;
            } 
        }
    } else {  // icosahedral grid

        double dConv = 3.14159 / 180.0;

#pragma omp parallel for
        for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
            double dR = spherdist(dLon[i]*dConv, dLat[i]*dConv, m_dX*dConv, m_dY*dConv, RADIUS_EARTH_KM);
            if (dR < m_dR) {
                m_bAllowed[i] = true;
            }
        }
    }
        
    return 0;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int ConfinedMove<T>::finalize(float fT) {

    // it's VERY important that finalize is called before lists
    // are updated in SPopulation::finalizeStep()
    
#pragma omp parallel 
    {
        int iT = omp_get_thread_num();
        for (unsigned int iIndex = 0; iIndex < m_vMoveList[iT]->size(); iIndex += 3) {
            int iCellTo = (*m_vMoveList[iT])[iIndex+2];
            if ( ! m_bAllowed[iCellTo] ) {
                (*m_vMoveList[iT])[iIndex+2] = (*m_vMoveList[iT])[iIndex];
            }
        }
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_CONFINEDMOVE_X_NAME
//    ATTR_CONFINEDMOVE_Y_NAME
//    ATTR_CONFINEDMOVE_R_NAME
//
template<typename T>
int ConfinedMove<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
     int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_CONFINEDMOVE_X_NAME, 1, &m_dX);
        if (iResult != 0) {
            LOG_ERROR("[ConfinedMove] couldn't read attribute [%s]", ATTR_CONFINEDMOVE_X_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_CONFINEDMOVE_Y_NAME, 1, &m_dY);
        if (iResult != 0) {
            LOG_ERROR("[ConfinedMove] couldn't read attribute [%s]", ATTR_CONFINEDMOVE_Y_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_CONFINEDMOVE_R_NAME, 1, &m_dR);
        if (iResult != 0) {
            LOG_ERROR("[ConfinedMove] couldn't read attribute [%s]", ATTR_CONFINEDMOVE_R_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_CONFINEDMOVE_X_NAME
//    ATTR_CONFINEDMOVE_Y_NAME
//    ATTR_CONFINEDMOVE_R_NAME
//
template<typename T>
int ConfinedMove<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_CONFINEDMOVE_X_NAME, 1, &m_dX);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_CONFINEDMOVE_Y_NAME, 1, &m_dY);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_CONFINEDMOVE_R_NAME, 1, &m_dR);
    
    return iResult; 
}

//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int ConfinedMove<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_CONFINEDMOVE_X_NAME) {
        m_dX = dValue;
    } else if (sAttrName == ATTR_CONFINEDMOVE_Y_NAME) {
        m_dY = dValue;
    } else if (sAttrName == ATTR_CONFINEDMOVE_R_NAME) {
        m_dR = dValue;
    } else {
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int ConfinedMove<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_CONFINEDMOVE_X_NAME, &m_dX); 
        iResult += getAttributeVal(mParams, ATTR_CONFINEDMOVE_Y_NAME, &m_dY); 
        iResult += getAttributeVal(mParams, ATTR_CONFINEDMOVE_R_NAME, &m_dR); 
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool ConfinedMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    ConfinedMove<T>* pA = static_cast<ConfinedMove<T>*>(pAction);
    if ((m_dX == pA->m_dX) &&
        (m_dY == pA->m_dY) &&
        (m_dR == pA->m_dR)) {
        bEqual = true;
    } 
    return bEqual;
}

