#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "Verhulst.h"
#include "LinearBirth.cpp"
#include "LinearDeath.cpp"


// this number must changed if the parameters change
template<typename T>
int Verhulst<T>::NUM_VERHULST_PARAMS = 4;

template<typename T>
const char *Verhulst<T>::asNames[] = {
    ATTR_VERHULST_B0_NAME,
    ATTR_VERHULST_D0_NAME,
    ATTR_VERHULST_TURNOVER_NAME,
    ATTR_VERHULST_CAPACITY_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Verhulst<T>::Verhulst(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_VERHULST_NAME,sID),
      m_apWELL(apWELL),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

    m_iNumSetParams = 0;

    m_dB0 = -1024;
    m_dD0 = -1024;
    m_dTheta = -1024;
    m_dK = -1024;

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Verhulst<T>::~Verhulst() {

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
int Verhulst<T>::initialize(float fT) {
    
    int iResult = 0;

    if (m_pLB != NULL && m_pLD != NULL) {

        iResult += m_pLB->initialize(fT);
        iResult += m_pLD->initialize(fT);
        

    } else {
       if (m_pLB != NULL) {
            delete m_pLB;
        }
        if (m_pLD != NULL) {
            delete m_pLD;
        }
        
        m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dB0, m_dTheta, m_dK);
        m_pLD = new LinearDeath<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dD0, m_dTheta, m_dK);
        iResult = -1;
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int Verhulst<T>::operator()(int iAgentIndex, float fT) {

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
int Verhulst<T>::finalize(float fT) {

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
//    ATTR_VERHULST_B0_NAME
//    ATTR_VERHULST_D0_NAME
//    ATTR_VERHULST_TURNOVER_NAME
//    ATTR_VERHULST_CAPACITY_NAME
//  and creates a LinearBirth and LinearDeath object
//
template<typename T>
int Verhulst<T>::extractParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_B0_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_D0_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_TURNOVER_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_TURNOVER_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VERHULST_CAPACITY_NAME, 1, &m_dK);
        if (iResult != 0) {
            LOG_ERROR("[Verhulst] couldn't read attribute [%s]", ATTR_VERHULST_CAPACITY_NAME);
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
        
        m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dB0, m_dTheta, m_dK);
        m_pLD = new LinearDeath<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dD0, m_dTheta, m_dK);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//  tries to write the attributes
//    ATTR_VERHULST_B0_NAME
//    ATTR_VERHULST_D0_NAME
//    ATTR_VERHULST_TURNOVER_NAME
//    ATTR_VERHULST_CAPACITY_NAME
//
template<typename T>
int Verhulst<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VERHULST_CAPACITY_NAME, 1, &m_dK);

    return iResult;
}


//-----------------------------------------------------------------------------
// modifyParams
//
//
template<typename T>
int Verhulst<T>::modifyParams(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_VERHULST_B0_NAME) {
        m_dB0 = dValue;
    } else if (sAttrName == ATTR_VERHULST_D0_NAME) {
        m_dD0 = dValue;
    } else if (sAttrName == ATTR_VERHULST_TURNOVER_NAME) {
        m_dTheta = dValue;
    } else if (sAttrName == ATTR_VERHULST_CAPACITY_NAME) {
        m_dK = dValue;
    } else {
        iResult = -1;
    }
    if (iResult == 0) {
        iResult += m_pLB->modifyParams(sAttrName, dValue);
        iResult += m_pLD->modifyParams(sAttrName, dValue);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetParams
//   
//
template<typename T>
int Verhulst<T>::tryGetParams(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getParams();
    if (this->checkParams(mParams) == 0) {
        iResult = 0;
        iResult += getParamVal(mParams, ATTR_VERHULST_B0_NAME, &m_dB0);          
        iResult += getParamVal(mParams, ATTR_VERHULST_D0_NAME, &m_dD0);          
        iResult += getParamVal(mParams, ATTR_VERHULST_TURNOVER_NAME, &m_dTheta); 
        iResult += getParamVal(mParams, ATTR_VERHULST_CAPACITY_NAME, &m_dK);     

	if (iResult == 0) {
            if (m_pLB != NULL) {
                delete m_pLB;
            }
            if (m_pLD != NULL) {
                delete m_pLD;
            }

            m_pLB = new LinearBirth<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dB0, m_dTheta, m_dK);
            m_pLD = new LinearDeath<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_dD0, m_dTheta, m_dK);

        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool Verhulst<T>::isEqual(Action<T> *pAction, bool bStrict) {
    Verhulst<T>* pA = static_cast<Verhulst<T>*>(pAction);
    return m_pLB->isEqual(pA->m_pLB, bStrict) && m_pLD->isEqual(pA->m_pLD, bStrict);
}



