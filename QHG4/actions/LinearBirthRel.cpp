#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "WELL512.h"
#include "WELLUtils.h"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "LinearBirthRel.h"

template<typename T>
const char *LinearBirthRel<T>::asNames[] = {
    ATTR_LINBIRTHREL_B0_NAME,
    ATTR_LINBIRTHREL_TURNOVER_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LinearBirthRel<T>::LinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths) 
    : Action<T>(pPop,pCG,ATTR_LINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_adB(adBirthRates),
      m_dB0(0),
      m_dTheta(0),
      m_adK(NULL),
      m_iStride(1),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0) ,
      m_pNumAgentsPerCell(NULL) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// constructor for use with VerhulstVarK action
//
template<typename T>
LinearBirthRel<T>::LinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double dB0, double dTheta, double* adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_LINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_adB(adBirthRates),
      m_dB0(dB0),
      m_dTheta(dTheta),
      m_adK(adK), 
      m_iStride(iStride),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0) ,
      m_pNumAgentsPerCell(NULL) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LinearBirthRel<T>::~LinearBirthRel() {
    if (m_aaiNumTemp != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            if (m_aaiNumTemp != NULL) {
                delete[] m_aaiNumTemp[i];
            }
        }
        delete[] m_aaiNumTemp;
    }
 
    // we don't delete m_aiNumBirths because it comes from outside
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int LinearBirthRel<T>::preLoop() {
    int iResult = -1;
    if (m_pNumAgentsPerCell == NULL) {
        m_pNumAgentsPerCell = this->m_pPop->getNumAgentsArray();
    }

    if ((m_adK != NULL) && 
        (m_adB != NULL) && 
        (m_aiNumBirths != NULL)) {
#pragma omp parallel for
        for (int i = 0; i < m_iNumThreads; i++) {
            memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
        }       
        memset(m_aiNumBirths[0], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        memset(m_aiNumBirths[1], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int LinearBirthRel<T>::initialize(float fT) {

    memset(m_adB,0,sizeof(double) * (this->m_pCG->m_iNumCells));
    
#pragma omp parallel for
    for (int i = 0; i < m_iNumThreads; i++) {
        memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
    }

    // set 
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
    //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        if ( m_adK[iC * m_iStride] <= 0) {
            m_adB[iC] = 0;
        } else {
            // case with space-dependent K (m_pNumAgentsPerCell: number of this *and* other pop
            m_adB[iC] = m_dB0 + (m_dTheta - m_dB0) * (this->m_pNumAgentsPerCell[iC] / m_adK[iC * m_iStride]); 
        }
    }
    
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int LinearBirthRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
  
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);
    
    if (pa->m_iLifeState > 0) {
	
    	int iCellIndex = pa->m_iCellIndex;
        int iThread    = omp_get_thread_num();
        // we'll need a random  number in any case

        double dR = this->m_apWELL[iThread]->wrandd();

    	// if reproduction is in couples, use mate index, 
    	// otherwise use agent index also as father
	
        // note: if this doesn't compile, the agent has no field m_iMateIndex, 
        // and therefore the population should not use this action class.
    	int iMateIndex = pa->m_iMateIndex;

    	if (m_adB[iCellIndex] > 0) { // positive birth prob
            if ((pa->m_iGender == 0) && (iMateIndex >= 0)) {

                if (dR < m_adB[iCellIndex]) {
                    this->m_pPop->registerBirth(iCellIndex, iAgentIndex, iMateIndex);
                    m_aaiNumTemp[iThread][iCellIndex]++;
                    iResult = 1;
            	} 
            }
    	} else if (m_adB[iCellIndex] < 0) {

            if (dR < -m_adB[iCellIndex]) { // convert negative birth prob to death prob
                this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            }
    	}

    }

    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int LinearBirthRel<T>::finalize(float fT) {
#pragma omp parallel for
    for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
        m_aiNumBirths[m_iWhich][i] = 0;
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            m_aiNumBirths[m_iWhich][i] += m_aaiNumTemp[iT][i];
        }
    }
    m_iWhich = 1 - m_iWhich;
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_LINBIRTHREL_B0_NAME
//    ATTR_LINBIRTHREL_TURNOVER_NAME
//
template<typename T>
int LinearBirthRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTHREL_B0_NAME, 1, &m_dB0);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirthRel] couldn't read attribute [%s]", ATTR_LINBIRTHREL_B0_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINBIRTHREL_TURNOVER_NAME, 1, &m_dTheta);
        if (iResult != 0) {
            LOG_ERROR("[LinearBirthRel] couldn't read attribute [%s]", ATTR_LINBIRTHREL_TURNOVER_NAME);
        }
    }
    
    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_LINBIRTHREL_B0_NAME
//    ATTR_LINBIRTHREL_TURNOVER_NAME
//
template<typename T>
int LinearBirthRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTHREL_B0_NAME, 1, &m_dB0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINBIRTHREL_TURNOVER_NAME, 1, &m_dTheta);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int LinearBirthRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_LINBIRTHREL_B0_NAME) {
        m_dB0 = dValue;
    } else if (sAttrName == ATTR_LINBIRTHREL_TURNOVER_NAME) {
        m_dTheta = dValue;
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
int LinearBirthRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_LINBIRTHREL_B0_NAME, &m_dB0);         
        iResult += getAttributeVal(mParams, ATTR_LINBIRTHREL_TURNOVER_NAME, &m_dTheta);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool LinearBirthRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    LinearBirthRel<T>* pA = static_cast<LinearBirthRel<T>*>(pAction);
    if ((m_dB0    == pA->m_dB0) &&
        (m_dTheta == pA->m_dTheta)) {
        bEqual = true;
    } 
    return bEqual;
}

