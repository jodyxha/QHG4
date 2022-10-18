#include <cstring>
#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "WELL512.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "MassInterface.h"
#include "Birther.h"

template<typename T>
const char *Birther<T>::asNames[] = {
    ATTR_BIRTHER_ADULTMASS_NAME,
    ATTR_BIRTHER_BIRTHMASS_NAME,
    ATTR_BIRTHER_UNCERTAINTY_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Birther<T>::Birther(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL)
    : Action<T>(pPop,pCG,ATTR_BIRTHER_NAME,sID),
      m_apWELL(apWELL),
      m_dAdultMass(0),
      m_dBirthMass(0),
      m_dUncertainty(0),
      m_pMI(NULL) {
 
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int Birther<T>::preLoop() {
    int iResult = -1;
    m_pMI = dynamic_cast<MassInterface *>(this->m_pPop);
    if (m_pMI != NULL) {
        iResult = 0;
    } else {
        printf("[Birther<T>::initialize] Population must implement MassInterface\n");
    }
    return iResult;   
}


//-----------------------------------------------------------------------------
// execute
//
template<typename T>
int Birther<T>::execute(int iAgentIndex, float fT) {
    int iResult = 0;

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {

        double dR =  this->m_apWELL[omp_get_thread_num()]->wrandr(1 - m_dUncertainty, 1 + m_dUncertainty);
        double dMBaby = dR*m_dBirthMass;
        double dMAgent = m_pMI->getMass(iAgentIndex);

        if (dMAgent > m_dAdultMass) {
            m_pMI->setMass(iAgentIndex, dMAgent - dMBaby);
            m_pMI->setSecondaryMass(iAgentIndex, dMBaby);
            int iCellIndex = pa->m_iCellIndex;
            this->m_pPop->registerBirth(iCellIndex, iAgentIndex, -1);
            
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_BIRTHER_ADULTMASS_NAME
//    ATTR_BIRTHER_BIRTHMASS_NAME
//    ATTR_BIRTHER_UNCERTAINTY_NAME
//
template<typename T>
int Birther<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_extractAttribute(hSpeciesGroup, ATTR_BIRTHER_ADULTMASS_NAME,   1, &m_dAdultMass);
    iResult += qdf_extractAttribute(hSpeciesGroup, ATTR_BIRTHER_BIRTHMASS_NAME,   1, &m_dBirthMass);
    iResult += qdf_extractAttribute(hSpeciesGroup, ATTR_BIRTHER_UNCERTAINTY_NAME, 1, &m_dUncertainty);   

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_BIRTHER_ADULTMASS_NAME
//    ATTR_BIRTHER_BIRTHMASS_NAME
//    ATTR_BIRTHER_UNCERTAINTY_NAME
//
template<typename T>
int Birther<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_BIRTHER_ADULTMASS_NAME,   1, &m_dAdultMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_BIRTHER_BIRTHMASS_NAME,   1, &m_dBirthMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_BIRTHER_UNCERTAINTY_NAME, 1, &m_dUncertainty);   

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int Birther<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_BIRTHER_ADULTMASS_NAME,   &m_dAdultMass);   
        iResult += getAttributeVal(mParams, ATTR_BIRTHER_BIRTHMASS_NAME,   &m_dBirthMass);   
        iResult += getAttributeVal(mParams, ATTR_BIRTHER_UNCERTAINTY_NAME, &m_dUncertainty); 
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool Birther<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Birther<T>* pA = static_cast<Birther<T>*>(pAction);
    if ((m_dAdultMass   == pA->m_dAdultMass) &&
        (m_dBirthMass   == pA->m_dBirthMass) &&
        (m_dUncertainty == pA->m_dUncertainty)) {
        bEqual = true;
    } 
    return bEqual;
}

