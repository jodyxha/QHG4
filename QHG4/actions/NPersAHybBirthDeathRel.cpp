#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "NPersAHybLinearBirthRel.cpp"
#include "NPersLinearDeathRel.cpp"
#include "NPersAHybBirthDeathRel.h"


// this number must changed if the parameters change
template<typename T>
int NPersAHybBirthDeathRel<T>::NUM_NPERSAHYBBIRTHDEATHREL_PARAMS = 1;

template<typename T>
const char *NPersAHybBirthDeathRel<T>::asNames[] = {
    ATTR_NPERSAHYBBIRTHDEATHREL_HYBMINPROB_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
NPersAHybBirthDeathRel<T>::NPersAHybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths)
    : Action<T>(pPop,pCG,ATTR_NPERSAHYBBIRTHDEATHREL_NAME,sID),
      m_apWELL(apWELL),
      m_aiNumBirths(aiNumBirths),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

    m_iNumSetParams = 0;

    m_dHybMinProb = -1024;
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
NPersAHybBirthDeathRel<T>::~NPersAHybBirthDeathRel() {

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
int NPersAHybBirthDeathRel<T>::preLoop() {
    int iResult = 0;

    if ((m_pLB != NULL) && (m_pLD != NULL)) {

        iResult += m_pLB->preLoop();
        iResult += m_pLD->preLoop();

    } else {
        printf("[NPersAHybBirthDeathRel]LB or LD is null\n");
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int NPersAHybBirthDeathRel<T>::initialize(float fT) {
    
    int iResult = 0;

    iResult += m_pLB->initialize(fT);
    iResult += m_pLD->initialize(fT);
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int NPersAHybBirthDeathRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;

    iResult += m_pLB->execute(iAgentIndex,fT);
    iResult += m_pLD->execute(iAgentIndex,fT);

    return iResult;
}



//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int NPersAHybBirthDeathRel<T>::finalize(float fT) {

    int iResult = 0;
    
    iResult += m_pLB->finalize(fT);
    iResult += m_pLD->finalize(fT);
    
    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME
//  and then creates a LinearBirth and a LinearDeath object
//
template<typename T>
int NPersAHybBirthDeathRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPERSAHYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
        if (iResult != 0) {
            LOG_ERROR("[NPersAHybBirthDeathRel] couldn't read attribute [%s]", ATTR_NPERSAHYBBIRTHDEATHREL_HYBMINPROB_NAME);
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
        
        m_pLB = new NPersAHybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_dHybMinProb);
        m_pLD = new NPersLinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
////    ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME
//
template<typename T>
int NPersAHybBirthDeathRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPERSAHYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);

    return iResult;
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int NPersAHybBirthDeathRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_NPERSAHYBBIRTHDEATHREL_HYBMINPROB_NAME) {
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
int NPersAHybBirthDeathRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_NPERSAHYBBIRTHDEATHREL_HYBMINPROB_NAME, &m_dHybMinProb);
        if (iResult == 0) {
            // just in case we're reading in new parameters,
            // delete old birth and death actions
            
            if (m_pLB != NULL) {
                delete m_pLB;
            }
            if (m_pLD != NULL) {
                delete m_pLD;
            }
            
            m_pLB = new NPersAHybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_dHybMinProb);
            m_pLD = new NPersLinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool NPersAHybBirthDeathRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    NPersAHybBirthDeathRel<T>* pA = static_cast<NPersAHybBirthDeathRel<T>*>(pAction);
    if (m_dHybMinProb == pA->m_dHybMinProb) {
        bEqual = true;
    } 
    return bEqual;
}

