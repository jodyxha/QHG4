#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "HybBirthDeathRel.h"
#include "HybLinearBirthRel.cpp"
#include "LinearDeathRel.cpp"


// this number must changed if the parameters change
template<typename T>
int HybBirthDeathRel<T>::NUM_HYBBIRTHDEATHREL_PARAMS = 3;

template<typename T>
const char *HybBirthDeathRel<T>::asNames[] = {
    ATTR_HYBBIRTHDEATHREL_B0_NAME,
    ATTR_HYBBIRTHDEATHREL_D0_NAME,
    ATTR_HYBBIRTHDEATHREL_THETA_NAME,
    ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
HybBirthDeathRel<T>::HybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double *adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_HYBBIRTHDEATHREL_NAME,sID),
      m_adK(adK),
      m_iStride(iStride),
      m_apWELL(apWELL),
      m_aiNumBirths(aiNumBirths),
      m_adBirthRates(adBirthRates),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

    m_iNumSetParams = 0;

    m_dB0 = -1024;
    m_dD0 = -1024;
    m_dTheta = -1024;
    m_dHybMinProb = -1024;
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
HybBirthDeathRel<T>::~HybBirthDeathRel() {

    if (m_pLB != NULL) {
        delete m_pLB;
    }
    if (m_pLD != NULL) {
        delete m_pLD;
    }

}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int HybBirthDeathRel<T>::preLoop() {
    int iResult = 0;

    if ((m_pLB != NULL) && (m_pLD != NULL)) {

        iResult += m_pLB->preLoop();
        iResult += m_pLD->preLoop();

    } else {
        printf("[HybBirthDeathRel]LB or LD is null\n");
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int HybBirthDeathRel<T>::initialize(float fT) {
    
    int iResult = 0;

    iResult += m_pLB->initialize(fT);
    iResult += m_pLD->initialize(fT);
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int HybBirthDeathRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;

    iResult += m_pLB->execute(iAgentIndex,fT);
    iResult += m_pLD->execute(iAgentIndex,fT);

    return iResult;
}



//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int HybBirthDeathRel<T>::finalize(float fT) {

    int iResult = 0;
    
    iResult += m_pLB->finalize(fT);
    iResult += m_pLD->finalize(fT);
    
    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_HYBBIRTHDEATHREL_B0_NAME
//    ATTR_HYBBIRTHDEATHREL_D0_NAME
//    ATTR_HYBBIRTHDEATHREL_THETA_NAME
//  and then creates a LinearBirth and a LinearDeath object
//
template<typename T>
int HybBirthDeathRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[HybBirthDeathRel] couldn't read attribute [%s]", ATTR_HYBBIRTHDEATHREL_B0_NAME);
        }
    }
     
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[HybBirthDeathRel] couldn't read attribute [%s]", ATTR_HYBBIRTHDEATHREL_D0_NAME);
        }
    }
     
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_THETA_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[HybBirthDeathRel] couldn't read attribute [%s]", ATTR_HYBBIRTHDEATHREL_THETA_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
        if (iResult != 0) {
            LOG_ERROR("[HybBirthDeathRel] couldn't read attribute [%s]", ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME);
        }
    }

    // now that we have the parameters, we can actuall create life and death objects
    
    if (iResult == 0) {

        // just in case we're reading in new parameters,
        // delete old life and death actions

        if (m_pLB != NULL) {
            delete m_pLB;
        }
        if (m_pLD != NULL) {
            delete m_pLD;
        }
        
        m_pLB = new HybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dB0, m_dTheta, m_dHybMinProb, m_adK, m_iStride);
        m_pLD = new LinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dD0, m_dTheta, m_adK, m_iStride);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_HYBBIRTHDEATHREL_B0_NAME
//    ATTR_HYBBIRTHDEATHREL_D0_NAME
//    ATTR_HYBBIRTHDEATHREL_THETA_NAME
//    ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME
//
template<typename T>
int HybBirthDeathRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_THETA_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);

    return iResult;
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int HybBirthDeathRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_HYBBIRTHDEATHREL_B0_NAME) {
        m_dB0 = dValue;
    } else if (sAttrName == ATTR_HYBBIRTHDEATHREL_D0_NAME) {
        m_dD0 = dValue;
    } else if (sAttrName == ATTR_HYBBIRTHDEATHREL_THETA_NAME) {
        m_dTheta = dValue;
    } else if (sAttrName == ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME) {
        m_dHybMinProb = dValue;
    } else {
        iResult = -1;
    }
    if (iResult == 0) {
        iResult += m_pLB->modifyAttributes(sAttrName, dValue);
        iResult += m_pLD->modifyAttributes(sAttrName, dValue);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int HybBirthDeathRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_HYBBIRTHDEATHREL_B0_NAME, &m_dB0);                
        iResult += getAttributeVal(mParams, ATTR_HYBBIRTHDEATHREL_D0_NAME, &m_dD0);                
        iResult += getAttributeVal(mParams, ATTR_HYBBIRTHDEATHREL_THETA_NAME, &m_dTheta);          
        iResult += getAttributeVal(mParams, ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME, &m_dHybMinProb);
        if (iResult == 0) {
            // just in case we're reading in new parameters,
            // delete old birth and death actions
            
            if (m_pLB != NULL) {
                delete m_pLB;
            }
            if (m_pLD != NULL) {
                delete m_pLD;
            }
            
            m_pLB = new HybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dB0, m_dTheta, m_dHybMinProb, m_adK, m_iStride);
            m_pLD = new LinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dD0, m_dTheta, m_adK, m_iStride);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool HybBirthDeathRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    HybBirthDeathRel<T>* pA = static_cast<HybBirthDeathRel<T>*>(pAction);
    if ((m_dB0    == pA->m_dB0) &&
        (m_dD0    == pA->m_dD0) &&
        (m_dTheta == pA->m_dTheta) && 
        (m_dHybMinProb == pA->m_dHybMinProb)) {
        bEqual = true;
    } 
    return bEqual;
}

