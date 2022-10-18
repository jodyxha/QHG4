#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "ArrayShare.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "BirthDeathRelComp.h"
#include "LinearBirthRel.cpp"
#include "LinearDeathRel.cpp"


// this number must changed if the parameters change
template<typename T>
int BirthDeathRelComp<T>::NUM_BIRTHDEATHRELCOMP_PARAMS = 5;

template<typename T>
const char *BirthDeathRelComp<T>::asNames[] = {
    ATTR_BIRTHDEATHRELCOMP_B0_NAME,
    ATTR_BIRTHDEATHRELCOMP_D0_NAME,
    ATTR_BIRTHDEATHRELCOMP_THETA_NAME,
    ATTR_BIRTHDEATHRELCOMP_THIS_NAME,
    ATTR_BIRTHDEATHRELCOMP_OTHER_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
BirthDeathRelComp<T>::BirthDeathRelComp(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double *adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_BIRTHDEATHRELCOMP_NAME,sID),
      m_adK(adK),
      m_iStride(iStride),
      m_apWELL(apWELL),
      m_aiNumBirths(aiNumBirths),
      m_adBirthRates(adBirthRates),
      m_pLB(NULL), 
      m_pLD(NULL),
      m_sThis(""),
      m_sOther(""),
      m_pTotalCounts(NULL) ,
      m_bReadDone(false) {
    

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
BirthDeathRelComp<T>::~BirthDeathRelComp() {

    if (m_pLB != NULL) {
        delete m_pLB;
    }
    if (m_pLD != NULL) {
        delete m_pLD;
    }

    if (m_pTotalCounts != NULL) {
        delete[] m_pTotalCounts;
    }

    ArrayShare::freeInstance();
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int BirthDeathRelComp<T>::preLoop() {
    int iResult = 0;
    
    if ((m_pLB != NULL) && (m_pLD != NULL)) {

        m_pOtherCounts = (ulong*) ArrayShare::getInstance()->getArray(m_sOther);
        m_pTotalCounts = new ulong[this->m_pCG->m_iNumCells];
        m_pLB->setAgentNumArray(m_pTotalCounts);
        m_pLD->setAgentNumArray(m_pTotalCounts);

        iResult += m_pLB->preLoop();
        iResult += m_pLD->preLoop();
    } else {
        printf("LB or LD is null\n");
        iResult = -1;
    }
    return iResult;
}
    

//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int BirthDeathRelComp<T>::initialize(float fT) {
    
    int iResult = 0;

    if (m_pOtherCounts != NULL) {
#pragma omp parallel for    
        for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
            m_pTotalCounts[i] = this->m_pPop->getNumAgents(i) + m_pOtherCounts[i];
        }
    } else {
        memcpy(m_pTotalCounts, this->m_pPop->getNumAgentsArray(), this->m_pCG->m_iNumCells*sizeof(ulong));
    }
    iResult += m_pLB->initialize(fT);
    iResult += m_pLD->initialize(fT);
        
    return iResult;
}


//-----------------------------------------------------------------------------
// action
//
template<typename T>
int BirthDeathRelComp<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    iResult += m_pLB->execute(iAgentIndex,fT);
    iResult += m_pLD->execute(iAgentIndex,fT);

    return iResult;
}



//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int BirthDeathRelComp<T>::finalize(float fT) {

    int iResult = 0;
    
    iResult += m_pLB->finalize(fT);
    iResult += m_pLD->finalize(fT);

    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_BIRTHDEATHRELCOMP_B0_NAME
//    ATTR_BIRTHDEATHRELCOMP_D0_NAME
//    ATTR_BIRTHDEATHRELCOMP_THETA_NAME
//    ATTR_BIRTHDEATHRELCOMP_THIS_NAME
//    ATTR_BIRTHDEATHRELCOMP_OTHER_NAME
//  and then creates a LinearBirth and a LinearDeath object
//
template<typename T>
int BirthDeathRelComp<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[BirthDeathRelComp] couldn't read attribute [%s]", ATTR_BIRTHDEATHRELCOMP_B0_NAME);
        }
    }
     
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[BirthDeathRelComp] couldn't read attribute [%s]", ATTR_BIRTHDEATHRELCOMP_D0_NAME);
        }
    }
     
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_THETA_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[BirthDeathRelComp] couldn't read attribute [%s]", ATTR_BIRTHDEATHRELCOMP_THETA_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_THIS_NAME, m_sThis);
        if (iResult != 0) {
            LOG_ERROR("[BirthDeathRelComp] couldn't read attribute [%s]", ATTR_BIRTHDEATHRELCOMP_THIS_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_OTHER_NAME, m_sOther);
        if (iResult != 0) {
            LOG_ERROR("[BirthDeathRelComp] couldn't read attribute [%s]", ATTR_BIRTHDEATHRELCOMP_OTHER_NAME);
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
        
        m_pLB = new LinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dB0, m_dTheta, m_adK, m_iStride);
        m_pLD = new LinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dD0, m_dTheta, m_adK, m_iStride);

        ArrayShare::getInstance()->shareArray(m_sThis, this->m_pCG->m_iNumCells,  this->m_pPop->getNumAgentsArray());
        ArrayShare::getInstance()->display();
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_BIRTHDEATHRELCOMP_B0_NAME
//    ATTR_BIRTHDEATHRELCOMP_D0_NAME
//    ATTR_BIRTHDEATHRELCOMP_THETA_NAME
//    ATTR_BIRTHDEATHRELCOMP_THIS_NAME
//    ATTR_BIRTHDEATHRELCOMP_OTHER_NAME
//
template<typename T>
int BirthDeathRelComp<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_THETA_NAME, 1, &m_dTheta);
    iResult += qdf_insertSAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_THIS_NAME,  m_sThis);
    iResult += qdf_insertSAttribute(hSpeciesGroup, ATTR_BIRTHDEATHRELCOMP_OTHER_NAME, m_sOther);

    return iResult;
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int BirthDeathRelComp<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_BIRTHDEATHRELCOMP_B0_NAME) {
        m_dB0 = dValue;
    } else if (sAttrName == ATTR_BIRTHDEATHRELCOMP_D0_NAME) {
        m_dD0 = dValue;
    } else if (sAttrName == ATTR_BIRTHDEATHRELCOMP_THETA_NAME) {
        m_dTheta = dValue;
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
int BirthDeathRelComp<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_BIRTHDEATHRELCOMP_B0_NAME, &m_dB0);
        iResult += getAttributeVal(mParams, ATTR_BIRTHDEATHRELCOMP_D0_NAME, &m_dD0);
        iResult += getAttributeVal(mParams, ATTR_BIRTHDEATHRELCOMP_THETA_NAME, &m_dTheta);
        iResult += getAttributeStr(mParams, ATTR_BIRTHDEATHRELCOMP_THIS_NAME, m_sThis);
        iResult += getAttributeStr(mParams, ATTR_BIRTHDEATHRELCOMP_OTHER_NAME, m_sOther);

        if (iResult == 0) {
            if (m_pLB != NULL) {
                delete m_pLB;
            }
            if (m_pLD != NULL) {
                delete m_pLD;
            }
            
            m_pLB = new LinearBirthRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dB0, m_dTheta, m_adK, m_iStride);
            m_pLD = new LinearDeathRel<T>(this->m_pPop, this->m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_dD0, m_dTheta, m_adK, m_iStride);
            
            ArrayShare::getInstance()->shareArray(m_sThis, this->m_pCG->m_iNumCells, this->m_pPop->getNumAgentsArray());
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool BirthDeathRelComp<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    BirthDeathRelComp<T>* pA = static_cast<BirthDeathRelComp<T>*>(pAction);
    if ((m_dB0    == pA->m_dB0) &&
        (m_dD0    == pA->m_dD0) &&
        (m_dTheta == pA->m_dTheta) &&
        (m_sThis  == pA->m_sThis) && 
        (m_sOther == pA->m_sOther)) {
        bEqual = true;
    } 
    return bEqual;
}

