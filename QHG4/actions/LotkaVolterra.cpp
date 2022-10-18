#include <cstring>
#include <omp.h>
#include <cmath>
#include <string>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "PopFinder.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "LotkaVolterra.h"

template<typename T>
const char *LotkaVolterra<T>::asNames[] = {
    ATTR_LOTKAVOLTERRA_SELFRATE_NAME,
    ATTR_LOTKAVOLTERRA_MIXRATE_NAME,
    ATTR_LOTKAVOLTERRA_OTHERPOP_NAME,
    ATTR_LOTKAVOLTERRA_K_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LotkaVolterra<T>::LotkaVolterra(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, PopFinder *pPopFinder)
    : Action<T>(pPop,pCG,ATTR_LOTKAVOLTERRA_NAME,sID),
      m_apWELL(apWELL),
      m_pPopFinder(pPopFinder),
      m_pOtherPop(NULL),
      m_dSelfRate(0),
      m_dMixRate(0),
      m_adB(NULL),
      m_adD(NULL),
      m_sOtherPopname("") {
    
    m_adB = new double[this->m_pCG->m_iNumCells];
    m_adD = new double[this->m_pCG->m_iNumCells];
      
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LotkaVolterra<T>::~LotkaVolterra() {
    
   if (m_adB != NULL) {
       delete[] m_adB;
   }
   if (m_adD != NULL) {
       delete[] m_adD;
   }

}

//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int LotkaVolterra<T>::preLoop() {
    int iResult = -1;
    m_pOtherPop = m_pPopFinder->getPopByName(m_sOtherPopname);
    if (m_pOtherPop != NULL) {
        iResult = 0;
    }
    printf("[LotkaVolterra<T>::preLoop()] got pointer for [%s]: %p\n", m_sOtherPopname, m_pOtherPop);

    return iResult;
}
    


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int LotkaVolterra<T>::initialize(float fT) {
    printf("[LotkaVolterra<T>::initialize]\n");
    memset(m_adB,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    memset(m_adD,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    
    
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
    //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        double d1 = m_dSelfRate;
        double d2 = m_dMixRate * ((double)(m_pOtherPop->getNumAgents(iC))) / m_dK;
       
        if (d1 > 0) {
           
            m_adB[iC] += d1;
            
        } else {
            m_adD[iC] -= d1;
        }

        if (d2 > 0) {
          
            m_adB[iC] += d2;
            
        } else {
            m_adD[iC] -= d2;
        }
        if (iC == 4) {
            printf("B: %f, D: %f (K:%f, NumAG %lu)\n", m_adB[iC], m_adD[iC], m_dK, m_pOtherPop->getNumAgents(iC));
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int LotkaVolterra<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
        int iCellIndex = pa->m_iCellIndex;
       

        int iThread = omp_get_thread_num();
        double dRB = this->m_apWELL[iThread]->wrandd();
          
        if (dRB < m_adB[iCellIndex]) {
            this->m_pPop->registerBirth(iCellIndex, iAgentIndex, -1);
            iResult = 1;
        } 

        double dRD = this->m_apWELL[iThread]->wrandd();
            
        if (dRD < m_adD[iCellIndex]) { // convert negative birth prob to death prob
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
    	}
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_LOTKAVOLTERRA_SELFRATE_NAME
//    ATTR_LOTKAVOLTERRA_MIXRATE_NAME
//    ATTR_LOTKAVOLTERRA_OTHERPOP_NAME
//    ATTR_LOTKAVOLTERRA_K_NAME
//
template<typename T>
int LotkaVolterra<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_SELFRATE_NAME, 1, &m_dSelfRate);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_SELFRATE_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_MIXRATE_NAME, 1, &m_dMixRate);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_MIXRATE_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_LOTKAVOLTERRA_OTHERPOP_NAME, m_sOtherPopname);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_OTHERPOP_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_K_NAME, 1, &m_dK);
        if (iResult != 0) {
            LOG_ERROR("[LotkaVolterra] couldn't read attribute [%s]", ATTR_LOTKAVOLTERRA_K_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_LOTKAVOLTERRA_SELFRATE_NAME
//    ATTR_LOTKAVOLTERRA_MIXRATE_NAME
//    ATTR_LOTKAVOLTERRA_OTHERPOP_NAME
//    ATTR_LOTKAVOLTERRA_K_NAME
//
template<typename T>
int LotkaVolterra<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_SELFRATE_NAME, 1, &m_dSelfRate);
    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_MIXRATE_NAME, 1, &m_dMixRate);
    iResult += qdf_insertSAttribute(hSpeciesGroup, ATTR_LOTKAVOLTERRA_OTHERPOP_NAME, m_sOtherPopname);
    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_LOTKAVOLTERRA_K_NAME, 1, &m_dK);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int LotkaVolterra<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;       
        iResult += this->getAttributeVal(mParams, ATTR_LOTKAVOLTERRA_SELFRATE_NAME, 1, &m_dSelfRate);
        iResult += this->getAttributeVal(mParams, ATTR_LOTKAVOLTERRA_MIXRATE_NAME, 1, &m_dMixRate);  
        iResult += this->getAttributeVal(mParams, ATTR_LOTKAVOLTERRA_OTHERPOP_NAME, m_sOtherPopname);
        iResult += this->getAttributeVal(mParams, ATTR_LOTKAVOLTERRA_K_NAME, 1, &m_dK);              
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool LotkaVolterra<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    LotkaVolterra<T>* pA = static_cast<LotkaVolterra<T>*>(pAction);
    if ((m_dSelfRate == pA->m_dSelfRate) &&
        (m_dMixRate  == pA->m_dMixRate) &&
        (m_dK        == pA->m_dK) &&
        (m_sOtherPopname == pA->m_sOtherPopname)) {
        bEqual = true;
    } 
    return bEqual;
}

