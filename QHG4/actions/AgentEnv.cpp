#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>


#include "MessLoggerT.h"
#include "EventConsts.h"
#include "WELLUtils.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "SPopulation.h"
#include "Action.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "AgentEnv.h"



template<typename T>
const stringvec AgentEnv<T>::s_vNames = {
    ATTR_AGENTENV_REQUIREMENTS_NAME};


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
AgentEnv<T>::AgentEnv(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, WELL512** apWELL, LBController *pAgentController, double *pdFoodAvailable, int *pAgentCounts) 
    : Action<T>(pPop, pCG, ATTR_AGENTENV_NAME, sID),
      m_pAgentController(pAgentController),
    m_bNeedUpdate(true),
    m_adCapacities(NULL),
    m_pEnvArr(NULL),
    m_pGroupArr(NULL),
    m_ppCounts(NULL),
    m_pFoodAvailable(pdFoodAvailable),
    m_pAgentCounts(pAgentCounts) {
    
    m_iNumThreads = omp_get_max_threads();
    m_iLocArrSize = this->m_pCG->m_iConnectivity + 1;    

    this->m_pPop->addObserver(this);


    this->m_vNames.insert(this->m_vNames.end(), s_vNames.begin(), s_vNames.end());

    m_ppCounts = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; ++i) {
        m_ppCounts[i] = new int[this->m_pCG->m_iNumCells]; 
        memset(m_ppCounts[i], 0, this->m_pCG->m_iNumCells*sizeof(int));
    }

}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
AgentEnv<T>::~AgentEnv() {
    delete[] m_adCapacities;


    if (m_pEnvArr != NULL) {
        delete[] m_pEnvArr;
    }

    if (m_pGroupArr != NULL) {
        delete[] m_pGroupArr;
    }

    if (m_ppCounts != NULL) {
        for (int i = 0; i  < m_iNumThreads; ++i) {
            delete[] m_ppCounts[i];
        }
        delete[] m_ppCounts;
    }
}


//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void AgentEnv<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if ((iEvent == EVENT_ID_GEO) || 
        (iEvent == EVENT_ID_CLIMATE) || 
        (iEvent == EVENT_ID_VEG)) { 
        m_bNeedUpdate = true;
    
    } else if (iEvent == EVENT_ID_FLUSH) {
        // no more events: recalculate
        getAgentCounts();
        recalculateGlobalCapacities();
    }
}


//-----------------------------------------------------------------------------
// getArr
//
template<typename T>
const double *AgentEnv<T>::getArr(int iAgentIndex) {
    return  &(m_aEnvVals[iAgentIndex]);
}


//-----------------------------------------------------------------------------
// init
//
template<typename T>
int AgentEnv<T>::init() {
    int iResult = -1;
    stdprintf("init called\n");
    
    if (this->m_pCG != NULL) {

        int iNumCells = this->m_pCG->m_iNumCells; 
        m_adCapacities = new double[iNumCells];
    

        // initialize the buffer ...
        stdprintf("initializing m_aEnvVals with (%d, %d)\n", m_pAgentController->getLayerSize(), m_iLocArrSize);
        m_aEnvVals.init(m_pAgentController->getLayerSize(), m_iLocArrSize);
        
        // ... and add it to the AgentController
        iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aEnvVals));
        if (iResult == 0) {
            // everything OK
        } else {
            stdprintf("Couldn't add buffer EnvVals to controller\n");
        }
    } else {
        if (this->m_pCG == NULL) {
            stdprintf("[AgentEnv] No cell grid!\n");
        }
    }

    m_pEnvArr   = new double[m_iLocArrSize];
    m_pGroupArr = new double[m_iLocArrSize];

    return iResult;
}


//-----------------------------------------------------------------------------
// recalculate
//   here we simply look at pfFoodAvailable to determine the carryinig capacity
//   (simple approach: capacity = available food divided by daily requirements)
//
template<typename T>
int AgentEnv<T>::recalculateGlobalCapacities() {
    if (m_bNeedUpdate) {
        

        stdprintf("AgentEnv::recalculateGlobalCapacities\n");
        int iNumCells = this->m_pCG->m_iNumCells; 
        // fill m_adCapacities with npp miami-values for current climate

        memset(m_adCapacities, 0, iNumCells*sizeof(double));
       
#pragma omp parallel for
        for (int i = 0; i < iNumCells; i++) {
            m_adCapacities[i] = m_pFoodAvailable[i] / m_dRequirements;
        }
        m_bNeedUpdate = false;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int AgentEnv<T>::preLoop() {
    int iResult = 0;
    getAgentCounts();
    recalculateGlobalCapacities();
    return iResult;
}


//----------------------------------------------------------------------------
// getAgentCounts
//   calculates the hyb-weighted parameter averages for all agents
//   should be called before recalculateGlobalCapacities, 
//   otherwise the agfents act on yesterday's state
//
template<typename T>
int AgentEnv<T>::getAgentCounts() { 
    int iResult = 0;
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

    // clean counters
    for (int i = 0; i < m_iNumThreads; ++i) {
        memset(m_ppCounts[i], 0, this->m_pCG->m_iNumCells*sizeof(int));
    }
#pragma omp parallel for 
   for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
       int iCellID = this->m_pPop->m_aAgents[iA].m_iCellIndex;
       m_ppCounts[omp_get_thread_num()][iCellID]++;
   }
   // accumulate
   for (int iT = 1; iT < m_iNumThreads; ++iT) {
#pragma omp parallel for
       for (uint i = 0; i < this->m_pCG->m_iNumCells; ++i) {
           m_ppCounts[0][i] += m_ppCounts[iT][i];
       }
   }

   return iResult;
}


//----------------------------------------------------------------------------
// initialize
//   calculates the hyb-weighted parameter averages for all agents
//
template<typename T>
int AgentEnv<T>::initialize(float fTime) { 
    int iResult = 0;

    getAgentCounts();
    recalculateGlobalCapacities();

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {

        // calculate the local environment
        calculateEnvArr(iA);

        // now do group array
        calculateGroupArr(iA);
        
        // now mix them with agent's health factor
        // low health: seek food; good health: seek people
        double *pLocEnvArr =  &(m_aEnvVals[iA]);
        float fHealth = this->m_pPop->m_aAgents[iA].m_fHealth;
        for (int k = 0; k < m_iLocArrSize; k++) {
            pLocEnvArr[k] = fHealth*m_pGroupArr[k] + (1-fHealth)*pLocEnvArr[k];
        }
        
    } 

    return iResult; 
}


//----------------------------------------------------------------------------
// calculateEnvArr
//   calculates the hyb-weighted parameter averages for all agents
//
template<typename T>
int AgentEnv<T>::calculateEnvArr(int iAgentIndex) {
    int iResult = 0;

    T pA = this->m_pPop->m_aAgents[iAgentIndex];
    double *pLocEnvArr =  &(m_aEnvVals[iAgentIndex]);
    memset(pLocEnvArr, 0, m_iLocArrSize*sizeof(double));

    //    float fH     = pA.m_fHybridization;
    int iCurCell = pA.m_iCellIndex;
    for (int iN = 0; iN <= this->m_pCG->m_iConnectivity; iN++) {
        int iC = iCurCell;
        if (iN <= this->m_pCG->m_aCells[iCurCell].m_iNumNeighbors) {
            if (iN > 0) {
                iC = this->m_pCG->m_aCells[iC].m_aNeighbors[iN-1];
            }
            pLocEnvArr[iN] = m_adCapacities[iC];
                
            if (pLocEnvArr[iN] < 0) {
                pLocEnvArr[iN]  = 0;
            }
        } else {
            pLocEnvArr[iN]  = 0;
        }
    }
    //pA.m_dCC = m_pEnvArr[0];
  
    // accumulate for wighted move
    for (int iN = 1; iN < m_iLocArrSize; iN++) {
        pLocEnvArr[iN] += pLocEnvArr[iN -1];
    }
    return iResult;
}



//----------------------------------------------------------------------------
// calculateGroupArr
//   calculates the hyb-weighted parameter averages for all agents
//
template<typename T>
int AgentEnv<T>::calculateGroupArr(int iAgentIndex) {
    int iResult = 0;

    T pA = this->m_pPop->m_aAgents[iAgentIndex];

    memset(m_pGroupArr, 0, m_iLocArrSize*sizeof(double));
    int iCurCell = pA.m_iCellIndex;
    
    for (int iN = 0; iN <= this->m_pCG->m_iConnectivity; iN++) {
        if (iN <= this->m_pCG->m_aCells[iCurCell].m_iNumNeighbors) {
            if (iN > 0) {
                m_pGroupArr[iN] = m_ppCounts[0][this->m_pCG->m_aCells[iCurCell].m_aNeighbors[iN-1]];
            } else {
                m_pGroupArr[iN] = m_ppCounts[0][iCurCell];
            }
        } else {
            m_pGroupArr[iN]  = 0;
        }
    }
    const static std::string ATTR_AGENTENV_NAME = "AgentEnv";

    // accumulate
    for (int iN = 1; iN < m_iLocArrSize; iN++) {
        m_pGroupArr[iN] += m_pGroupArr[iN -1];
    }
    return iResult;
}


//----------------------------------------------------------------------------
// decreaseHealth
//   decreases health of each agent
//
template<typename T>
int AgentEnv<T>::decreaseHealth() {
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
         T pA = this->m_pPop->m_aAgents[iA];
         pA.m_fHealth -= m_dRequirements;
         if (pA.m_fHealth <= 0) {
             this->m_pPop->registerDeath(iA, pA->m_iCellIndex);
         }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_AGENTENV_REQUIREMENTS_NAME
//
template<typename T>
int AgentEnv<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_AGENTENV_REQUIREMENTS_NAME, 1, &m_dRequirements);
        if (iResult != 0) {
            LOG_ERROR("[FoodManager<T>::extractAttributesQDF] couldn't read attribute [%s]", ATTR_AGENTENV_REQUIREMENTS_NAME);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to read the attributes
//    ATTR_AGENTENV_REQUIREMENTS_NAME
//
template<typename T>
int AgentEnv<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_AGENTENV_REQUIREMENTS_NAME, 1, &m_dRequirements);
   
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   need no params
//
template<typename T>
int AgentEnv<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;
 
    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult += getAttributeVal(mParams, ATTR_AGENTENV_REQUIREMENTS_NAME, &m_dRequirements);         
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool AgentEnv<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    AgentEnv<T>* pA = static_cast<AgentEnv<T>*>(pAction);
    if (m_dRequirements != pA->m_dRequirements) {
        bEqual = false;
    }
    return bEqual;
}

