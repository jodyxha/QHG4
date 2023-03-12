#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

#include "GetOld.cpp"
#include "ATanDeath.cpp"
#include "Fertility.cpp"
#include "VerhulstVarK.cpp"
#include "RandomPair.cpp"
#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"

#include "MultiEvaluator.cpp"
#include "NPPCapacity.cpp"

#include "tut_EnvironCapAltPop.h"

//----------------------------------------------------------------------------
// constructor
//
tut_EnvironCapAltPop::tut_EnvironCapAltPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<tut_EnvironCapAltAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_pGeography(pCG->m_pGeography) {

    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells * sizeof(double));

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<tut_EnvironCapAltAgent>::evaluatorinfos mEvalInfo;

    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<tut_EnvironCapAltAgent> *pSEAlt = new SingleEvaluator<tut_EnvironCapAltAgent>(this, m_pCG, "Alt", NULL, (double*)m_pGeography->m_adAltitude, "AltPref", true, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<tut_EnvironCapAltAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<tut_EnvironCapAltAgent> *pSECap = new SingleEvaluator<tut_EnvironCapAltAgent>(this, m_pCG, "NPP", NULL, m_adCapacities, "", true, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<tut_EnvironCapAltAgent>*>("Multi_weight_npp", pSECap));

    m_pME      = new MultiEvaluator<tut_EnvironCapAltAgent>(this, m_pCG, "NPP+Alt", m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators
    m_pNPPCap  = new NPPCapacity<tut_EnvironCapAltAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    m_pWM       = new WeightedMove<tut_EnvironCapAltAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);

    m_pGO       = new GetOld<tut_EnvironCapAltAgent>(this, m_pCG, "");
    m_pAD       = new ATanDeath<tut_EnvironCapAltAgent>(this, m_pCG, "", m_apWELL);
    m_pFert     = new Fertility<tut_EnvironCapAltAgent>(this, m_pCG, "");
    m_pVerVarK  = new VerhulstVarK<tut_EnvironCapAltAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride);
    m_pPair     = new RandomPair<tut_EnvironCapAltAgent>(this, m_pCG, "", m_apWELL);


    // adding all actions to prioritizer

    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pVerVarK);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pME);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pNPPCap);

}

//----------------------------------------------------------------------------
// destructor
//
tut_EnvironCapAltPop::~tut_EnvironCapAltPop() {

    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pAD != NULL) {
        delete m_pAD;
    }
    if (m_pFert != NULL) {
        delete m_pFert;
    }
    if (m_pVerVarK != NULL) {
        delete m_pVerVarK;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pNPPCap != NULL) {
        delete m_pNPPCap;
    }

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }  

    if (m_adCapacities != NULL) {
        delete[] m_adCapacities;
    }

}


//----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agent if under ice or water
//
int tut_EnvironCapAltPop::updateEvent(int iEventID, char *pData, float fT) { 
    if (iEventID == EVENT_ID_GEO) { 
        // drown
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != LBController::NIL) {
            int iLastAgent = getLastAgentIndex();

            int iChunk = (uint)ceil((iLastAgent-iFirstAgent+1)/(double)m_iNumThreads);
#pragma omp parallel for schedule(static, iChunk) 
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    int iCellIndex = m_aAgents[iAgent].m_iCellIndex;
                    if ((m_pGeography->m_adAltitude[iCellIndex] < 0) ||
                        (m_pGeography->m_abIce[iCellIndex] > 0)) {
                        registerDeath(iCellIndex, iAgent);
                    }
                }
            }
        }
        // make sure they are removed before step starts
        if (m_bRecycleDeadSpace) {
            recycleDeadSpaceNew();
        } else {
            performDeaths();
        }
        // update counts
        updateTotal();
        updateNumAgentsPerCell();
        // clear lists to avoid double deletion
	initListIdx();
    }
    
    notifyObservers(iEventID, pData);
       
    return 0;
}


//----------------------------------------------------------------------------
// flushEvents
//
void tut_EnvironCapAltPop::flushEvents(float fT) {
    notifyObservers(EVENT_ID_FLUSH, NULL);
}


//----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int tut_EnvironCapAltPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;

    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;

    return iResult;
}


//----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int tut_EnvironCapAltPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void tut_EnvironCapAltPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    tut_EnvironCapAltAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}


//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//  write additional data to the group 
//  Here we only write the capacity array to see the combined effects of
//  NPP, water, and coastal.
//
int tut_EnvironCapAltPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    iResult = qdf_readArray(hSpeciesGroup, SPOP_DS_CAP, m_iNumCells, m_adCapacities);
    return iResult;
}
