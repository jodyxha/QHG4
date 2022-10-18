#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PersHybBirthDeathRel.h"
#include "PersHybLinearBirthRel.cpp"
#include "PersLinearDeathRel.cpp"


// this number must changed if the parameters change
template<typename T>
int PersHybBirthDeathRel<T>::NUM_PERSHYBBIRTHDEATHREL_PARAMS = 21;

template<typename T>
const char *PersHybBirthDeathRel<T>::asNames[] = {
    ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME,
    ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PersHybBirthDeathRel<T>::PersHybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double *adK, int iStride)
    : Action<T>(pPop,pCG,ATTR_PERSHYBBIRTHDEATHREL_NAME,sID),
      m_adK(adK),
      m_iStride(iStride),
      m_apWELL(apWELL),
      m_aiNumBirths(aiNumBirths),
      m_pLB(NULL), 
      m_pLD(NULL) {
    

    m_iNumSetParams = 0;

    m_dHybMinProb   = -1024;
    m_dGroupAssProb = -1024;
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PersHybBirthDeathRel<T>::~PersHybBirthDeathRel() {

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
int PersHybBirthDeathRel<T>::preLoop() {
    int iResult = 0;

    if ((m_pLB != NULL) && (m_pLD != NULL)) {

        iResult += m_pLB->preLoop();
        iResult += m_pLD->preLoop();

    } else {
        printf("[PersHybBirthDeathRel]LB or LD is null\n");
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int PersHybBirthDeathRel<T>::initialize(float fT) {
    
    int iResult = 0;

    iResult += m_pLB->initialize(fT);
    iResult += m_pLD->initialize(fT);
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int PersHybBirthDeathRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;

    iResult += m_pLB->execute(iAgentIndex,fT);
    iResult += m_pLD->execute(iAgentIndex,fT);

    return iResult;
}



//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int PersHybBirthDeathRel<T>::finalize(float fT) {

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
//    ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME
//  and then creates a LinearBirth and a LinearDeath object
//
template<typename T>
int PersHybBirthDeathRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
        if (iResult != 0) {
            LOG_ERROR("[PersHybBirthDeathRel] couldn't read attribute [%s]", ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME);
        }
    }
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME, 1, &m_dGroupAssProb);
        if (iResult != 0) {
            LOG_ERROR("[PersHybBirthDeathRel] couldn't read attribute [%s]", ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME);
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
        
        m_pLB = new PersHybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_dHybMinProb, m_adK, m_iStride);
        m_pLD = new PersLinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_adK, m_iStride);
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
////    ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME
//
template<typename T>
int PersHybBirthDeathRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME,   1, &m_dGroupAssProb);

    return iResult;
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int PersHybBirthDeathRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME) {
        m_dHybMinProb = dValue;
    } else if (sAttrName == ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME) {
        m_dGroupAssProb = dValue;
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
int PersHybBirthDeathRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME, &m_dHybMinProb);
        iResult += getAttributeVal(mParams, ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME,   &m_dGroupAssProb);
        if (iResult == 0) {
            // just in case we're reading in new parameters,
            // delete old birth and death actions
            
            if (m_pLB != NULL) {
                delete m_pLB;
            }
            if (m_pLD != NULL) {
                delete m_pLD;
            }
            
            m_pLB = new PersHybLinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_dHybMinProb, m_adK, m_iStride);
            m_pLD = new PersLinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_aiNumBirths, m_adK, m_iStride);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool PersHybBirthDeathRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    PersHybBirthDeathRel<T>* pA = static_cast<PersHybBirthDeathRel<T>*>(pAction);
    if ((m_dHybMinProb == pA->m_dHybMinProb) &&
        (m_dGroupAssProb == pA->m_dGroupAssProb)) {
        bEqual = true;
    } 
    return bEqual;
}

