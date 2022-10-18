#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "OldAgeDeath.h"

template<typename T>
const char *OldAgeDeath<T>::asNames[] = {
    ATTR_OLDAGEDEATH_MAXAGE_NAME,
    ATTR_OLDAGEDEATH_UNCERTAINTY_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
OldAgeDeath<T>::OldAgeDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_OLDAGEDEATH_NAME,sID),
      m_apWELL(apWELL),
      m_dMaxAge(0),
      m_dUncertainty(0) {
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
OldAgeDeath<T>::~OldAgeDeath() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int OldAgeDeath<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        pa->m_fAge = fT - pa->m_fBirthTime;
        // death possible starting at 0.8 times m_dMaxAge
        double dR =  this->m_apWELL[omp_get_thread_num()]->wrandr(1 - m_dUncertainty*m_dMaxAge, 1 + m_dUncertainty*m_dMaxAge);
        //        printf("OldAgeDeath a%d: age %f, rand %f, max %f\n", iAgentIndex,  pa->m_fAge, dR, m_dMaxAge);
        if (pa->m_fAge > m_dMaxAge+dR) {
            this->m_pPop->registerDeath(pa->m_iCellIndex, iAgentIndex);
        }
        // variation:
        // kill_age = maxAge + wrandr(-m_dUncertainty, m_dUncertainty),
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_OLDAGEDEATH_MAXAGE_NAME
//    ATTR_OLDAGEDEATH_UNCERTAINTY_NAME
//
template<typename T>
int OldAgeDeath<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_MAXAGE_NAME, 1, &m_dMaxAge);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_OLDAGEDEATH_MAXAGE_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_UNCERTAINTY_NAME, 1, &m_dUncertainty);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_OLDAGEDEATH_UNCERTAINTY_NAME);
        }
    }

    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_OLDAGEDEATH_MAXAGE_NAME
//    ATTR_OLDAGEDEATH_UNCERTAINTY_NAME
//
template<typename T>
int OldAgeDeath<T>:: writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_MAXAGE_NAME, 1, &m_dMaxAge);
    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_OLDAGEDEATH_UNCERTAINTY_NAME, 1, &m_dUncertainty);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int OldAgeDeath<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams,  ATTR_OLDAGEDEATH_MAXAGE_NAME, &m_dMaxAge);           
        iResult += getAttributeVal(mParams,  ATTR_OLDAGEDEATH_UNCERTAINTY_NAME, &m_dUncertainty); 
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool OldAgeDeath<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    OldAgeDeath<T>* pA = static_cast<OldAgeDeath<T>*>(pAction);
    if ((m_dMaxAge      == pA->m_dMaxAge) && 
        (m_dUncertainty == pA->m_dUncertainty)) {
        bEqual = true;
    } 
    return bEqual;
}

