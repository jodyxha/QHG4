#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "LinearBirth.h"

template<typename T>
const std::string LinearBirth<T>::asNames[] = {
    ATTR_LINBIRTH_B0_NAME,
    ATTR_LINBIRTH_TURNOVER_NAME,
    ATTR_LINBIRTH_CAPACITY_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LinearBirth<T>::LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_LINBIRTH_NAME,sID),
      m_apWELL(apWELL),
      m_dB0(0),
      m_dTheta(0),
      m_adK(NULL),
      m_iStride(1) {
    
    m_adB = new double[this->m_pCG->m_iNumCells];
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}

//-----------------------------------------------------------------------------
// constructor for use with Verhulst action
//
template<typename T>
LinearBirth<T>::LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dB0, double dTheta, double dK) 
    : Action<T>(pPop,pCG,ATTR_LINBIRTH_NAME,sID),
      m_apWELL(apWELL),
      m_dB0(dB0),
      m_dTheta(dTheta),
      m_dK(dK), 
      m_adK(NULL),
      m_iStride(1) {
    
    m_adB = new double[this->m_pCG->m_iNumCells];
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}

//-----------------------------------------------------------------------------
// constructor for use with VerhulstVarK action
//
template<typename T>
LinearBirth<T>::LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dB0, double dTheta, double* adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_LINBIRTH_NAME,sID),
      m_apWELL(apWELL),
      m_dB0(dB0),
      m_dTheta(dTheta),
      m_dK(-1024),
      m_adK(adK), 
      m_iStride(iStride) {
    
    m_adB = new double[this->m_pCG->m_iNumCells];
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LinearBirth<T>::~LinearBirth() {
    
    if (m_adB != NULL) {
        delete[] m_adB;
    }
    
}


//-----------------------------------------------------------------------------
 // initialize
//
template<typename T>
int LinearBirth<T>::initialize(float fT) {
    
    memset(m_adB,0,sizeof(double) * (this->m_pCG->m_iNumCells));
   

    //	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
    //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
  for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {

        if (m_adK == NULL) {
            // case with constant K
            m_adB[iC] = m_dB0 + (m_dTheta - m_dB0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_dK);
        } else {
            if ( m_adK[iC * m_iStride] <= 0) {
                m_adB[iC] = 0;
            } else {
                // case with space-dependent K
                m_adB[iC] = m_dB0 + (m_dTheta - m_dB0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_adK[iC * m_iStride]);            
            }
        }
        
    }
    
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int LinearBirth<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
	
    	int iCellIndex = pa->m_iCellIndex;
        
    	// if reproduction is in couples, use mate index, 
    	// otherwise use agent index also as father
	
    	// note: the offset is needed because if the agent structure 
    	// does not have a m_iMateIndex member, the compiler 
    	// would not know what to do
	
    	int iMateIndex = pa->m_iMateIndex;

    	if (m_adB[iCellIndex] > 0) { // positive birth prob
            if ((pa->m_iGender == 0) && (iMateIndex >= 0)) {
            	
                int iThread = omp_get_thread_num();
                
                double dR = this->m_apWELL[iThread]->wrandd();
                
                if (dR < m_adB[iCellIndex]) {
                    //printf("[LinearBirth::execute] have a deady (%d)\n", iAgentIndex);
                    this->m_pPop->registerBirth(iCellIndex, iAgentIndex, iMateIndex);
                    iResult = 1;
            	} 
            }
    	} else if (m_adB[iCellIndex] < 0) {
            
            int iThread = omp_get_thread_num();
            
            double dR = this->m_apWELL[iThread]->wrandd();

            if (dR < -m_adB[iCellIndex]) { // convert negative birth prob to death prob
                this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            }
    	}

    }

    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_LINBIRTH_B0_NAME
//    ATTR_LINBIRTH_TURNOVER_NAME
//    ATTR_LINBIRTH_CAPACITY_NAME
//
template<typename T>
int LinearBirth<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTH_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirth] couldn't read attribute [%s]", ATTR_LINBIRTH_B0_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTH_TURNOVER_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirth] couldn't read attribute [%s]", ATTR_LINBIRTH_TURNOVER_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTH_CAPACITY_NAME, 1, &m_dK);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirth] couldn't read attribute [%s]", ATTR_LINBIRTH_CAPACITY_NAME);
        }
    }
    
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_LINBIRTH_B0_NAME
//    ATTR_LINBIRTH_TURNOVER_NAME
//    ATTR_LINBIRTH_CAPACITY_NAME
//
template<typename T>
int LinearBirth<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTH_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTH_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTH_CAPACITY_NAME, 1, &m_dK);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int LinearBirth<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_LINBIRTH_B0_NAME) {
        m_dB0 = dValue;
    } else if (sAttrName == ATTR_LINBIRTH_TURNOVER_NAME) {
        m_dTheta = dValue;

    } else if (sAttrName == ATTR_LINBIRTH_CAPACITY_NAME) {
        m_dK = dValue;
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
int LinearBirth<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_LINBIRTH_B0_NAME, &m_dB0);         
        iResult += getAttributeVal(mParams, ATTR_LINBIRTH_TURNOVER_NAME, &m_dTheta);
        iResult += getAttributeVal(mParams, ATTR_LINBIRTH_CAPACITY_NAME, &m_dK);    
    }
    return iResult;
}




//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool LinearBirth<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    LinearBirth<T>* pA = static_cast<LinearBirth<T>*>(pAction);
    if ((m_dB0    == pA->m_dB0) &&
        (m_dTheta == pA->m_dTheta) &&
        (m_dK     == pA->m_dK)) {
        bEqual = true;
    } 
    return bEqual;
}

