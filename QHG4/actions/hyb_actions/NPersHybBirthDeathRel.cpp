#include <cmath>
#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "NPersHybLinearBirthRel.cpp"
#include "NPersLinearDeathRel.cpp"
#include "NPersHybBirthDeathRel.h"


// this number must changed if the parameters change
template<typename T>
int NPersHybBirthDeathRel<T>::NUM_NPERSHYBBIRTHDEATHREL_PARAMS = 1;

template<typename T>
const char *NPersHybBirthDeathRel<T>::asNames[] = {
    ATTR_NPERSHYBBIRTHDEATHREL_HYBMINPROB_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
NPersHybBirthDeathRel<T>::NPersHybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths)
    : Action<T>(pPop,pCG,ATTR_NPERSHYBBIRTHDEATHREL_NAME,sID),
      m_dHybMinProb(0),
      m_bUseLog(false),
      m_iNumSetParams(0),
      m_apWELL(apWELL),
      m_aiNumBirths(aiNumBirths),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

  
    m_dHybMinProb = -1024;
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
NPersHybBirthDeathRel<T>::~NPersHybBirthDeathRel() {

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
int NPersHybBirthDeathRel<T>::preLoop() {
    int iResult = 0;

    if ((m_pLB != NULL) && (m_pLD != NULL)) {

        iResult += m_pLB->preLoop();
        iResult += m_pLD->preLoop();

    } else {
        printf("[NPersHybBirthDeathRel]LB or LD is null\n");
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int NPersHybBirthDeathRel<T>::initialize(float fT) {
    
    int iResult = 0;

    iResult += m_pLB->initialize(fT);
    iResult += m_pLD->initialize(fT);
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int NPersHybBirthDeathRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;

    iResult += m_pLB->execute(iAgentIndex,fT);
    iResult += m_pLD->execute(iAgentIndex,fT);

    return iResult;
}



//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int NPersHybBirthDeathRel<T>::finalize(float fT) {

    int iResult = 0;
    
    iResult += m_pLB->finalize(fT);
    iResult += m_pLD->finalize(fT);
    
    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  - tries to read the attribute
//      ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME
//    if this value is negative, it is considered to be the decimal log of the 
//    the actual value.  
//  - creates a LinearBirth and a LinearDeath object
// 
//
template<typename T>
int NPersHybBirthDeathRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPERSHYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
        if (iResult != 0) {
            LOG_ERROR("[NPersHybBirthDeathRel] couldn't read attribute [%s]", ATTR_NPERSHYBBIRTHDEATHREL_HYBMINPROB_NAME);
        }
    }

    // now that we have the parameters, we can actuall create life and death objects
    
    if (iResult == 0) {
        if (m_dHybMinProb < 0) {
            m_dHybMinProb = pow(10, m_dHybMinProb);
            m_bUseLog = true;
        }

        // just in case we're reading in new parameters,
        // delete old life and death actions

        if (m_pLB != NULL) {
            delete m_pLB;
        }
        if (m_pLD != NULL) {
            delete m_pLD;
        }
        
        m_pLB = new NPersHybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_dHybMinProb);
        m_pLD = new NPersLinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
////    ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME
//
template<typename T>
int NPersHybBirthDeathRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    double dWriteVal = m_dHybMinProb;
    if (m_bUseLog) {
        dWriteVal = log10(m_dHybMinProb);
    }
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPERSHYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &dWriteVal);

    return iResult;
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int NPersHybBirthDeathRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_NPERSHYBBIRTHDEATHREL_HYBMINPROB_NAME) {
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
int NPersHybBirthDeathRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_NPERSHYBBIRTHDEATHREL_HYBMINPROB_NAME, &m_dHybMinProb);
        if (iResult == 0) {
            // just in case we're reading in new parameters,
            // delete old birth and death actions
            
            if (m_pLB != NULL) {
                delete m_pLB;
            }
            if (m_pLD != NULL) {
                delete m_pLD;
            }
            
            m_pLB = new NPersHybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_dHybMinProb);
            m_pLD = new NPersLinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool NPersHybBirthDeathRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    NPersHybBirthDeathRel<T>* pA = static_cast<NPersHybBirthDeathRel<T>*>(pAction);
    if (m_dHybMinProb == pA->m_dHybMinProb) {
        bEqual = true;
    } 
    return bEqual;
}

