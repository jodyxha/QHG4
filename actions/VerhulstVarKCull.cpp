#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "VerhulstVarKCull.h"
#include "LinearBirth.cpp"
#include "LinearDeathCull.cpp"


// this number must changed if the parameters change
template<typename T>
int VerhulstVarKCull<T>::NUM_VERHULSTVARKCULL_PARAMS = 4;

template<typename T>
const char *VerhulstVarKCull<T>::asNames[] = {
    ATTR_VERHULSTVARKCULL_B0_NAME,
    ATTR_VERHULSTVARKCULL_D0_NAME,
    ATTR_VERHULSTVARKCULL_TURNOVER_NAME,
    ATTR_VERHULSTVARKCULL_EPS_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
VerhulstVarKCull<T>::VerhulstVarKCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_VERHULSTVARKCULL_NAME,sID),
      m_adK(adK),
      m_dEps(-1024),
      m_iStride(iStride),
      m_apWELL(apWELL),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

    m_iNumSetParams = 0;

    m_dB0 = -1024;
    m_dD0 = -1024;
    m_dTheta = -1024;

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
VerhulstVarKCull<T>::~VerhulstVarKCull() {

    if (m_pLB != NULL) {
        delete m_pLB;
    }
    if (m_pLD != NULL) {
        delete m_pLD;
    }

}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int VerhulstVarKCull<T>::initialize(float fT) {
    
    int iResult = 0;

    if (m_pLB != NULL && m_pLD != NULL) {

        iResult += m_pLB->initialize(fT);
        iResult += m_pLD->initialize(fT);
        

    } else {
        iResult = -1;
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int VerhulstVarKCull<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    if (m_pLB != NULL && m_pLD != NULL) {

        iResult += (*m_pLB)(iAgentIndex,fT);
        iResult += (*m_pLD)(iAgentIndex,fT);

    } else {
        iResult = -1;
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int VerhulstVarKCull<T>::finalize(float fT) {

    int iResult = 0;
    
    if (m_pLB != NULL && m_pLD != NULL) {

        iResult += m_pLB->finalize(fT);
        iResult += m_pLD->finalize(fT);

    } else {
        iResult = -1;
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_VERHULSTVARKCULL_B0_NAME
//    ATTR_VERHULSTVARKCULL_D0_NAME
//    ATTR_VERHULSTVARKCULL_TURNOVER_NAME
//    ATTR_VERHULSTVARKCULL_EPS_NAME
//  and then creates a LinearBirth and a LinearDeath object
//
template<typename T>
int VerhulstVarKCull<T>::extractParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[VerhulstVarKCull] couldn't read attribute [%s]", ATTR_VERHULSTVARKCULL_B0_NAME);
        }
    }
     
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[VerhulstVarKCull] couldn't read attribute [%s]", ATTR_VERHULSTVARKCULL_D0_NAME);
        }
    }
     
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_TURNOVER_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[VerhulstVarKCull] couldn't read attribute [%s]", ATTR_VERHULSTVARKCULL_TURNOVER_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_EPS_NAME, 1, &m_dEps);
        if (iResult != 0) {
            LOG_ERROR("[VerhulstVarKCull] couldn't read attribute [%s]", ATTR_VERHULSTVARKCULL_EPS_NAME);
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
        
        m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dB0, m_dTheta, m_adK, m_iStride);
        m_pLD = new LinearDeathCull<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dD0, m_dTheta, m_adK, m_dEps, m_iStride);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_VERHULSTVARKCULL_B0_NAME
//    ATTR_VERHULSTVARKCULL_D0_NAME
//    ATTR_VERHULSTVARKCULL_TURNOVER_NAME
//    ATTR_VERHULSTVARKCULL_EPS_NAME
//
template<typename T>
int VerhulstVarKCull<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULSTVARKCULL_EPS_NAME, 1, &m_dEps);

    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetParams
//   
//
template<typename T>
int VerhulstVarKCull<T>::tryGetParams(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getParams();
    if (this->checkParams(mParams) == 0) {
        iResult = 0;
        iResult += getParamVal(mParams,  ATTR_VERHULSTVARKCULL_B0_NAME, &m_dB0);  
        iResult += getParamVal(mParams,  ATTR_VERHULSTVARKCULL_D0_NAME, &m_dD0);  
        iResult += getParamVal(mParams,  ATTR_VERHULSTVARKCULL_TURNOVER_NAME, &m_dTheta);
        iResult += getParamVal(mParams,  ATTR_VERHULSTVARKCULL_EPS_NAME, &m_dEps);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool VerhulstVarKCull<T>::isEqual(Action<T> *pAction, bool bStrict) {
    
    VerhulstVarKCull<T>* pA = static_cast<VerhulstVarKCull<T>*>(pAction);
    return m_pLB->isEqual(pA->m_pLB, bStrict) && m_pLD->isEqual(pA->m_pLD, bStrict);
}


