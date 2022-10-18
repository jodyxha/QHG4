#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "Fertility.h"

template<typename T>
const char *Fertility<T>::asNames[] = {
    ATTR_FERTILITY_MIN_AGE_NAME,
    ATTR_FERTILITY_MAX_AGE_NAME,
    ATTR_FERTILITY_INTERBIRTH_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Fertility<T>::Fertility(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID) 
    : Action<T>(pPop,pCG,ATTR_FERTILITY_NAME,sID),
      m_fFertilityMinAge(0),
      m_fFertilityMaxAge(0),
      m_fInterbirth(0) {

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Fertility<T>::~Fertility() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//  must be called after GetOld
//
template<typename T>
int Fertility<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        if (pa->m_iGender == 0) {
            if ((pa->m_fAge > m_fFertilityMinAge) && 
                (pa->m_fAge < m_fFertilityMaxAge) && 
                ((fT - pa->m_fLastBirth) > m_fInterbirth)) {
                pa->m_iLifeState = LIFE_STATE_FERTILE;
            } else {
                pa->m_iLifeState = LIFE_STATE_ALIVE;
            }
        } else {            
            if (pa->m_fAge > m_fFertilityMinAge) {
                pa->m_iLifeState = LIFE_STATE_FERTILE;
            } else {
                pa->m_iLifeState = LIFE_STATE_ALIVE;
            }
        }
    }

    return iResult;
}




//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_FERTILITY_MIN_AGE_NAME
//    ATTR_FERTILITY_MAX_AGE_NAME
//    ATTR_FERTILITY_INTERBIRTH_NAME
//
template<typename T>
int Fertility<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FERTILITY_MIN_AGE_NAME, 1, &m_fFertilityMinAge);
        if (iResult != 0) {
            LOG_ERROR("[Fertility] couldn't read attribute [%s]", ATTR_FERTILITY_MIN_AGE_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FERTILITY_MAX_AGE_NAME, 1, &m_fFertilityMaxAge);
        if (iResult != 0) {
            LOG_ERROR("[Fertility] couldn't read attribute [%s]", ATTR_FERTILITY_MAX_AGE_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FERTILITY_INTERBIRTH_NAME, 1, &m_fInterbirth);
        if (iResult != 0) {
            LOG_ERROR("[Fertility] couldn't read attribute [%s]", ATTR_FERTILITY_INTERBIRTH_NAME);
        }
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_FERTILITY_MIN_AGE_NAME
//    ATTR_FERTILITY_MAX_AGE_NAME
//    ATTR_FERTILITY_INTERBIRTH_NAME
//
template<typename T>
int Fertility<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FERTILITY_MIN_AGE_NAME, 1, &m_fFertilityMinAge);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FERTILITY_MAX_AGE_NAME, 1, &m_fFertilityMaxAge);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FERTILITY_INTERBIRTH_NAME, 1, &m_fInterbirth);

    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int Fertility<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_FERTILITY_MIN_AGE_NAME, &m_fFertilityMinAge);
        iResult += getAttributeVal(mParams, ATTR_FERTILITY_MAX_AGE_NAME, &m_fFertilityMaxAge);
        iResult += getAttributeVal(mParams, ATTR_FERTILITY_INTERBIRTH_NAME, &m_fInterbirth);  
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool Fertility<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Fertility<T>* pA = static_cast<Fertility<T>*>(pAction);
    if ((m_fFertilityMaxAge == pA->m_fFertilityMaxAge) &&
        (m_fFertilityMinAge == pA->m_fFertilityMinAge) &&
        (m_fInterbirth      == pA->m_fInterbirth)) {
        bEqual = true;
    } 
    return bEqual;
}


