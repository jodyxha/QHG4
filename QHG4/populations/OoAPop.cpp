#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "BitGeneUtils.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
//#include "ConfinedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "VerhulstVarK.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "Fertility.cpp"
#include "NPPCapacity.cpp"
#include "Genetics.cpp"
#include "GenomeCreator.cpp"
#include "OoAPop.h"


//----------------------------------------------------------------------------
// constructor
//
OoAPop::OoAPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoAAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bPendingEvents(false) {
 
    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
  
    m_pNPPCap = new NPPCapacity<OoAAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<OoAAgent>::evaluatorinfos mEvalInfo;

    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoAAgent> *pSEAlt = new SingleEvaluator<OoAAgent>(this, m_pCG, "Alt", NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", false, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoAAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoAAgent> *pSECap = new SingleEvaluator<OoAAgent>(this, m_pCG, "NPP", NULL, m_adCapacities, "", false, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoAAgent>*>("Multi_weight_npp", pSECap));

    m_pME = new MultiEvaluator<OoAAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators


    m_pVerVarK = new VerhulstVarK<OoAAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride);


    // evaluator for movement

    m_pWM = new WeightedMove<OoAAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<OoAAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<OoAAgent>(this, m_pCG, "");

    m_pOAD = new OldAgeDeath<OoAAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new Fertility<OoAAgent>(this, m_pCG, "");

    //    m_pCM = new ConfinedMove<OoAAgent>(this, m_pCG, "");

    // adding all actions to prioritizer

    m_prio.addAction(m_pME);
    m_prio.addAction(m_pWM);
    //    m_prio.addAction(m_pCM);
    m_prio.addAction(m_pVerVarK);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pNPPCap);

}



///----------------------------------------------------------------------------
// destructor
//
OoAPop::~OoAPop() {

    if (m_adCapacities != NULL) {
        delete[] m_adCapacities;
    }
    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
     if (m_pVerVarK != NULL) {
        delete m_pVerVarK;
    }
    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pOAD != NULL) {
        delete m_pOAD;
    }
    if (m_pFert != NULL) {
        delete m_pFert;
    }
    if (m_pNPPCap != NULL) {
        delete m_pNPPCap;
    }

}


///----------------------------------------------------------------------------
// setParams
//
int OoAPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoAPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoAAgent>::preLoop();
   
    int iN = getNumAgentsEffective();
    if (iN > 0) {
        // ok
    } else {
        printf("[OoAPop::preLoop] No agents found\n");
    }
    return iResult;
}


///----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agents in water or ice
//    EVENT_ID_CLIMATE  : update NPP
//
int OoAPop::updateEvent(int iEventID, float fT) { 
    if (iEventID == EVENT_ID_CLIMATE) {
        m_pNPPCap->recalculate();
    } 
    if (iEventID == EVENT_ID_GEO) {
        // drown  or ice
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != LBController::NIL) {
            int iLastAgent = getLastAgentIndex();

            int iChunk = (uint)ceil((iLastAgent-iFirstAgent+1)/(double)m_iNumThreads);
#pragma omp parallel for schedule(static, iChunk) 
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    int iCellIndex = m_aAgents[iAgent].m_iCellIndex;
                    if ((this->m_pCG->m_pGeography->m_adAltitude[iCellIndex] < 0) ||
                        (this->m_pCG->m_pGeography->m_abIce[iCellIndex] > 0)) {
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
    return 0;
};

///----------------------------------------------------------------------------
// flushEvents
//
void OoAPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}



///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int OoAPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    return 0;

}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoAPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    

    OoAAgent &newAgent = m_aAgents[iAgent];
    newAgent.m_fAge = 0.0;
    newAgent.m_fLastBirth = 0.0;
    newAgent.m_iMateIndex = -3;

    // child & death stistics
    newAgent.m_iNumBabies = 0;
    m_aAgents[iMother].m_iNumBabies++;
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoAPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
    }
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void OoAPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoAAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}
