#include <omp.h>
#include <cmath>
#include <cstring>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "LinearDeathCull.h"

template<typename T>
const std::string LinearDeathCull<T>::asNames[] = {
    ATTR_LINDEATHCULL_D0_NAME,
    ATTR_LINDEATHCULL_TURNOVER_NAME,
    ATTR_LINDEATHCULL_CAPACITY_NAME,
    ATTR_LINDEATHCULL_EPS_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LinearDeathCull<T>::LinearDeathCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL)
    : Action<T>(pPop,pCG,ATTR_LINDEATHCULL_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(0),
      m_dTheta(0),
      m_dK(0),
      m_adK(NULL),
      m_dEps(-1024),
      m_iStride(1) {
    
    m_adD        = new double[this->m_pCG->m_iNumCells];
    m_adCullProb = new double[this->m_pCG->m_iNumCells];

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));

}

//-----------------------------------------------------------------------------
// constructor for use with e.g. Verhulst action
//
template<typename T>
LinearDeathCull<T>::LinearDeathCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double dK, double dEps) 
    : Action<T>(pPop,pCG,ATTR_LINDEATHCULL_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(dD0),
      m_dTheta(dTheta),
      m_dK(dK),
      m_adK(NULL),
      m_dEps(dEps),
      m_iStride(1) {
    
    m_adD        = new double[this->m_pCG->m_iNumCells];
    m_adCullProb = new double[this->m_pCG->m_iNumCells];
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}

//-----------------------------------------------------------------------------
// constructor for use with e.g. VerhulstVarK action
//
template<typename T>
LinearDeathCull<T>::LinearDeathCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double* adK, double dEps, int iStride) 
    : Action<T>(pPop,pCG,ATTR_LINDEATHCULL_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(dD0),
      m_dTheta(dTheta),
      m_dK(-1024),
      m_adK(adK),
      m_dEps(dEps),
      m_iStride(iStride) {
    
    m_adD        = new double[this->m_pCG->m_iNumCells];
    m_adCullProb = new double[this->m_pCG->m_iNumCells];
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LinearDeathCull<T>::~LinearDeathCull() {
    
    if (m_adD != NULL) {
        delete[] m_adD;
    }
    
    if (m_adCullProb != NULL) {
        delete[] m_adCullProb;
    }
    
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int LinearDeathCull<T>::initialize(float fT) {
    
    memset(m_adD,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    memset(m_adCullProb,0,sizeof(double)*(this->m_pCG->m_iNumCells));

    if (m_adK == NULL) {

        // loop for constant K
        //	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
        //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            const double &dNum = (double)(this->m_pPop->getNumAgents(iC));
            // constant K
            m_adD[iC] = m_dD0 + (m_dTheta - m_dD0) * dNum/ m_dK;
            // cull probability
            if (dNum > m_dK*(1 - m_dEps)) {
                m_adCullProb[iC] = 1 -  m_dK*(1 - m_dEps)/dNum;
            }
        }
    } else {
        // loop for varying K

        //	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
        //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            double &dKCur = m_adK[iC * m_iStride];
            if (dKCur <= 0) {
                m_adD[iC]     = 1.0;
                m_adCullProb[iC]  = 1.0;
            } else {
                const double &dNum = (double)(this->m_pPop->getNumAgents(iC));

                // space-varying K
                m_adD[iC] = m_dD0 + (m_dTheta - m_dD0) * dNum / dKCur;
                // cull probability
                if (dNum > dKCur*(1 - m_dEps)) {
                    m_adCullProb[iC] = 1 - dKCur*(1 - m_dEps)/dNum;
                }
            }
        }
    }
    
    return 0;
}



//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int LinearDeathCull<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
    	int iCellIndex = pa->m_iCellIndex;
    	
    	int iThread = omp_get_thread_num();
    	
    	double dR = this->m_apWELL[iThread]->wrandd();

    	if (dR < m_adD[iCellIndex]) {
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            iResult = 1;
    	} else {
            // not dead: do we have to cull?
            if (m_adCullProb[iCellIndex] > 0) {
                dR = this->m_apWELL[iThread]->wrandd();
                
                if (dR < m_adCullProb[iCellIndex]) {
                    this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
                    iResult = 1;
                }
            }
        }
    }
	
    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_LINDEATHCULL_D0_NAME
//    ATTR_LINDEATHCULL_TURNOVER_NAME
//    ATTR_LINDEATHCULL_CAPACITY_NAME
//    ATTR_LINDEATHCULL_EPS_NAME
//
template<typename T>
int LinearDeathCull<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[LinearDeathCull] couldn't read attribute [%s]", ATTR_LINDEATHCULL_D0_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_TURNOVER_NAME, 1, &m_dTheta);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeathCull] couldn't read attribute [%s]", ATTR_LINDEATHCULL_TURNOVER_NAME);
           }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_CAPACITY_NAME, 1, &m_dK);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeathCull] couldn't read attribute [%s]", ATTR_LINDEATHCULL_CAPACITY_NAME);
           }
    }
        
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_EPS_NAME, 1, &m_dEps);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeathCull] couldn't read attribute [%s]", ATTR_LINDEATHCULL_EPS_NAME);
           }
    }
        
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_LINDEATHCULL_D0_NAME
//    ATTR_LINDEATHCULL_TURNOVER_NAME
//    ATTR_LINDEATHCULL_CAPACITY_NAME
//    ATTR_LINDEATHCULL_EPS_NAME
//
template<typename T>
int LinearDeathCull<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_CAPACITY_NAME, 1, &m_dK);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATHCULL_EPS_NAME, 1, &m_dEps);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int LinearDeathCull<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_LINDEATHCULL_D0_NAME) {
        m_dD0 = dValue;
    } else if (sAttrName == ATTR_LINDEATHCULL_TURNOVER_NAME) {
        m_dTheta = dValue;

    } else if (sAttrName == ATTR_LINDEATHCULL_CAPACITY_NAME) {
        m_dK = dValue;

    } else if (sAttrName == ATTR_LINDEATHCULL_EPS_NAME) {
        m_dEps = dValue;
    } else {
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int LinearDeathCull<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_LINDEATHCULL_D0_NAME, &m_dD0);         
        iResult += getAttributeVal(mParams, ATTR_LINDEATHCULL_TURNOVER_NAME, &m_dTheta);
        iResult += getAttributeVal(mParams, ATTR_LINDEATHCULL_CAPACITY_NAME, &m_dK);    
        iResult += getAttributeVal(mParams, ATTR_LINDEATHCULL_EPS_NAME, &m_dEps);       
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool LinearDeathCull<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    LinearDeathCull<T>* pA = static_cast<LinearDeathCull<T>*>(pAction);
    if ((m_dD0    == pA->m_dD0) &&
        (m_dTheta == pA->m_dTheta) &&
        (m_dK     == pA->m_dK) &&
        (m_dEps   == pA->m_dEps)) {
        bEqual = true;
    } 
    return bEqual;
}

