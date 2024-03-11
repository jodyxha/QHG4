#include <cstring>
#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "MassInterface.h"
#include "MassManager.h"

template<typename T>
const std::string MassManager<T>::asNames[] = {
    ATTR_MASSMANAGER_MIN_NAME,
    ATTR_MASSMANAGER_MAX_NAME,
    ATTR_MASSMANAGER_DELTA_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
MassManager<T>::MassManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID)
    : Action<T>(pPop,pCG,ATTR_MASSMANAGER_NAME,sID),
      m_dMinMass(0),
      m_dMaxMass(0),
      m_dDelta(0),
      m_pMI(NULL) {

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));

}



//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int MassManager<T>::preLoop() {
    int iResult = -1;
    m_pMI = dynamic_cast<MassInterface *>(this->m_pPop);
    if (m_pMI != NULL) {
        iResult = 0;
    } else {
        printf("[MassManager<T>::preLoop] Population must implement MassInterface\n");
    }
    return iResult;   
}


//-----------------------------------------------------------------------------
// execute
//
template<typename T>
int MassManager<T>::execute(int iAgentIndex, float fT) {
    int iResult = 0;

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > LIFE_STATE_DEAD) {
        double dM = m_pMI->getMass(iAgentIndex);
        dM += m_dDelta;
        if (dM < m_dMinMass) {
            int iCellIndex = pa->m_iCellIndex;
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
        } else if (dM > m_dMaxMass) {
            m_pMI->setMass(iAgentIndex, m_dMaxMass);
        } else {
            m_pMI->setMass(iAgentIndex, dM);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_MASSMANAGER_MIN_NAME, 
//    ATTR_MASSMANAGER_MAX_NAME, 
//    ATTR_MASSMANAGER_DELTA_NAME
//
template<typename T>
int MassManager<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_MASSMANAGER_MIN_NAME,   1, &m_dMinMass);
        if (iResult != 0) {
            LOG_ERROR("[MassManager] couldn't read attribute [%s]", ATTR_MASSMANAGER_MIN_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_MASSMANAGER_MAX_NAME,   1, &m_dMaxMass);
        if (iResult != 0) {
            LOG_ERROR("[MassManager] couldn't read attribute [%s]", ATTR_MASSMANAGER_MAX_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_MASSMANAGER_DELTA_NAME, 1, &m_dDelta);   
        if (iResult != 0) {
            LOG_ERROR("[MassManager] couldn't read attribute [%s]", ATTR_MASSMANAGER_DELTA_NAME);
        }
    }


    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_MASSMANAGER_MIN_NAME, 
//    ATTR_MASSMANAGER_MAX_NAME, 
//    ATTR_MASSMANAGER_DELTA_NAME
//
template<typename T>
int MassManager<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_MASSMANAGER_MIN_NAME,   1, &m_dMinMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_MASSMANAGER_MAX_NAME,   1, &m_dMaxMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_MASSMANAGER_DELTA_NAME, 1, &m_dDelta);   

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int MassManager<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_MASSMANAGER_MIN_NAME,   &m_dMinMass); 
        iResult += getAttributeVal(mParams, ATTR_MASSMANAGER_MAX_NAME,   &m_dMaxMass); 
        iResult += getAttributeVal(mParams, ATTR_MASSMANAGER_DELTA_NAME, &m_dDelta);   
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool MassManager<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    MassManager<T>* pA = static_cast<MassManager<T>*>(pAction);
    if ((m_dMinMass == pA->m_dMinMass) &&
        (m_dMaxMass == pA->m_dMaxMass) &&
        (m_dDelta   == pA->m_dDelta)) {
        bEqual = true;
    } 
    return bEqual;
}

