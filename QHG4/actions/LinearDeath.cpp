#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "LinearDeath.h"

template<typename T>
const char *LinearDeath<T>::asNames[] = {
    ATTR_LINDEATH_D0_NAME,
    ATTR_LINDEATH_TURNOVER_NAME,
    ATTR_LINDEATH_CAPACITY_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LinearDeath<T>::LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL)
    : Action<T>(pPop,pCG,ATTR_LINDEATH_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(0),
      m_dTheta(0),
      m_dK(0),
      m_adK(NULL),
      m_iStride(1) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// constructor for use with e.g. Verhulst action
//
template<typename T>
LinearDeath<T>::LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double dK) 
    : Action<T>(pPop,pCG,ATTR_LINDEATH_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(dD0),
      m_dTheta(dTheta),
      m_dK(dK),
      m_adK(NULL),
      m_iStride(1) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// constructor for use with e.g. VerhulstVarK action
//
template<typename T>
LinearDeath<T>::LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double* adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_LINDEATH_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(dD0),
      m_dTheta(dTheta),
      m_dK(-1024),
      m_adK(adK),
      m_iStride(iStride) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LinearDeath<T>::~LinearDeath() {
    
    if (m_adD != NULL) {
        delete[] m_adD;
    }
    
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int LinearDeath<T>::initialize(float fT) {
    
    memset(m_adD,0,sizeof(double)*(this->m_pCG->m_iNumCells));

    if (m_adK == NULL) {

        // loop for constant K
        //	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
        //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            // constant K
            m_adD[iC] = m_dD0 + (m_dTheta - m_dD0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_dK);
        }
    } else {
        // loop for varying K

        //	int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
        //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            if (m_adK[iC * m_iStride] <= 0) {
                m_adD[iC] = 1;
            } else {
                // space-varying K
                m_adD[iC] = m_dD0 + (m_dTheta - m_dD0) * ((double)(this->m_pPop->getNumAgents(iC)) / m_adK[iC * m_iStride]);
            }
        }
    }
    
    return 0;
}



//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int LinearDeath<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
    	int iCellIndex = pa->m_iCellIndex;
    	
    	int iThread = omp_get_thread_num();
    	
    	double dR = this->m_apWELL[iThread]->wrandd();

    	if (dR < m_adD[iCellIndex]) {
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            iResult = 1;
    	}
    }
	
    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_LINDEATH_D0_NAME
//    ATTR_LINDEATH_TURNOVER_NAME
//    ATTR_LINDEATH_CAPACITY_NAME
//
template<typename T>
int LinearDeath<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATH_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[LinearDeath] couldn't read attribute [%s]", ATTR_LINDEATH_D0_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATH_TURNOVER_NAME, 1, &m_dTheta);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeath] couldn't read attribute [%s]", ATTR_LINDEATH_TURNOVER_NAME);
           }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATH_CAPACITY_NAME, 1, &m_dK);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeath] couldn't read attribute [%s]", ATTR_LINDEATH_CAPACITY_NAME);
           }
    }
        
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_LINDEATH_D0_NAME
//    ATTR_LINDEATH_TURNOVER_NAME
//    ATTR_LINDEATH_CAPACITY_NAME
//
template<typename T>
int LinearDeath<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATH_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATH_TURNOVER_NAME, 1, &m_dTheta);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATH_CAPACITY_NAME, 1, &m_dK);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int LinearDeath<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_LINDEATH_D0_NAME) {
        m_dD0 = dValue;
    } else if (sAttrName == ATTR_LINDEATH_TURNOVER_NAME) {
        m_dTheta = dValue;

    } else if (sAttrName == ATTR_LINDEATH_CAPACITY_NAME) {
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
int LinearDeath<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_LINDEATH_D0_NAME, &m_dD0);           
        iResult += getAttributeVal(mParams, ATTR_LINDEATH_TURNOVER_NAME, &m_dTheta);  
        iResult += getAttributeVal(mParams, ATTR_LINDEATH_CAPACITY_NAME, &m_dK);      
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool LinearDeath<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    LinearDeath<T>* pA = static_cast<LinearDeath<T>*>(pAction);
    if ((m_dD0    == pA->m_dD0) &&
        (m_dTheta == pA->m_dTheta) &&
        (m_dK     == pA->m_dK)) {
        bEqual = true;
    } 
    return bEqual;
}

