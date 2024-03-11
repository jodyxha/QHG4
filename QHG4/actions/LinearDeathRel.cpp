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
#include "LinearDeathRel.h"

template<typename T>
const std::string LinearDeathRel<T>::asNames[] = {
    ATTR_LINDEATHREL_D0_NAME,
    ATTR_LINDEATHREL_TURNOVER_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LinearDeathRel<T>::LinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths)
    : Action<T>(pPop,pCG,ATTR_LINDEATHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(0),
      m_dTheta(0),
      m_adK(NULL),
      m_iStride(1),
      m_adB(adBirthRates),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(1),
      m_pNumAgentsPerCell(NULL) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}


//-----------------------------------------------------------------------------
// constructor for use with varying K, e.g. VerhulstVarK action
//
template<typename T>
LinearDeathRel<T>::LinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double dD0, double dTheta, double* adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_LINDEATHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dD0(dD0),
      m_dTheta(dTheta),
      m_adK(adK),
      m_iStride(iStride),
      m_adB(adBirthRates),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(1),
      m_pNumAgentsPerCell(NULL) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LinearDeathRel<T>::~LinearDeathRel() {
    
    if (m_adD != NULL) {
        delete[] m_adD;
    }
    
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int LinearDeathRel<T>::preLoop() {
    int iResult = -1;
    if (m_pNumAgentsPerCell == NULL) {
        m_pNumAgentsPerCell = this->m_pPop->getNumAgentsArray();
    }

    if ((m_adK  != NULL) && 
        (m_adB  != NULL) && 
        (m_aiNumBirths != NULL)) {
            iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
//   We want N_death = N_birth*(b/d)
//   Therefore d_mod = N_death/N
//
template<typename T>
int LinearDeathRel<T>::initialize(float fT) {

    memset(m_adD,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());


    // loop for varying K
    //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        const double &dNumTot = m_pNumAgentsPerCell[iC]; // this + other
        const double &dNumLoc = this->m_pPop->getNumAgents(iC);
        if (m_adK[iC * m_iStride] <= 0) {
            m_adD[iC] = 1;
        } else {
            // space-varying K
            // we modify the birthrate by multiplying it with (num actual births)/(num avg births)
            // i.e. if more are born, more should die
            double d = m_dD0 + (m_dTheta - m_dD0) * (dNumTot / m_adK[iC * m_iStride]);
            m_adD[iC] = (m_aiNumBirths[m_iWhich][iC]*d)/(dNumLoc*m_adB[iC]);
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int LinearDeathRel<T>::finalize(float fT) {
    memset(m_aiNumBirths[m_iWhich], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
    m_iWhich = 1 - m_iWhich;
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int LinearDeathRel<T>::execute(int iAgentIndex, float fT) {

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
//    ATTR_LINDEATHREL_D0_NAME
//    ATTR_LINDEATHREL_TURNOVER_NAME
//
template<typename T>
int LinearDeathRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATHREL_D0_NAME, 1, &m_dD0);
        if (iResult != 0) {
            LOG_ERROR("[LinearDeathRel] couldn't read attribute [%s]", ATTR_LINDEATHREL_D0_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_LINDEATHREL_TURNOVER_NAME, 1, &m_dTheta);
           if (iResult != 0) {
               LOG_ERROR("[LinearDeathRel] couldn't read attribute [%s]", ATTR_LINDEATHREL_TURNOVER_NAME);
           }
    }
    
        
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_LINDEATHREL_D0_NAME
//    ATTR_LINDEATHREL_TURNOVER_NAME
//
template<typename T>
int LinearDeathRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATHREL_D0_NAME, 1, &m_dD0);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LINDEATHREL_TURNOVER_NAME, 1, &m_dTheta);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int LinearDeathRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_LINDEATHREL_D0_NAME) {
        m_dD0 = dValue;
    } else if (sAttrName == ATTR_LINDEATHREL_TURNOVER_NAME) {
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
int LinearDeathRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_LINDEATHREL_D0_NAME, &m_dD0);         
        iResult += getAttributeVal(mParams, ATTR_LINDEATHREL_TURNOVER_NAME, &m_dTheta);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool LinearDeathRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    LinearDeathRel<T>* pA = static_cast<LinearDeathRel<T>*>(pAction);
    if ((m_dD0    == pA->m_dD0) &&
        (m_dTheta == pA->m_dTheta)) {
        bEqual = true;
    } 
    return bEqual;
}

