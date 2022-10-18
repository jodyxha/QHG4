#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "RandomMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "BirthDeathRel.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "Fertility.cpp"
#include "NPPCapacity.cpp"
#include "Navigate.cpp"
#include "OoANavRelRPop.h"


//----------------------------------------------------------------------------
// constructor
//
OoANavRelRPop::OoANavRelRPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoANavRelRAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true),
      m_bPendingEvents(false) {

    m_adCapacities = new double[m_pCG->m_iNumCells];
    int iCapacityStride = 1;
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
  
    m_adBirthRates = new double[m_pCG->m_iNumCells];
    memset(m_adBirthRates, 0, m_pCG->m_iNumCells*sizeof(double));
    m_aiNumBirths  = new int*[2];
    m_aiNumBirths[0]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[0], 0, m_pCG->m_iNumCells*sizeof(int));
    m_aiNumBirths[1]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[1], 0, m_pCG->m_iNumCells*sizeof(int));

    m_pNPPCap = new NPPCapacity<OoANavRelRAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<OoANavRelRAgent>::evaluatorinfos mEvalInfo;
 
    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    // ori SingleEvaluator<OoANavRelRAgent> *pSEAlt = new SingleEvaluator<OoANavRelRAgent>(this, m_pCG, "Alt", NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", true, EVENT_ID_GEO);
    SingleEvaluator<OoANavRelRAgent> *pSEAlt = new SingleEvaluator<OoANavRelRAgent>(this, m_pCG, "Alt", NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", false, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavRelRAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    // ori SingleEvaluator<OoANavRelRAgent> *pSECap = new SingleEvaluator<OoANavRelRAgent>(this, m_pCG, "NPP", NULL, m_adCapacities, "NPPPref", true, EVENT_ID_VEG);
    SingleEvaluator<OoANavRelRAgent> *pSECap = new SingleEvaluator<OoANavRelRAgent>(this, m_pCG, "NPP", NULL, m_adCapacities, "", false, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavRelRAgent>*>("Multi_weight_npp", pSECap));

    // ori m_pME = new MultiEvaluator<OoANavRelRAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true); //true: delete evaluators
    m_pME = new MultiEvaluator<OoANavRelRAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_MUL_SIMPLE, true); //true: delete evaluators
    addObserver(m_pME);


    m_pBirthDeathRel = new BirthDeathRel<OoANavRelRAgent>(this, m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_adCapacities, iCapacityStride);


    // evaluator for movement

    m_pRM = new RandomMove<OoANavRelRAgent>(this, m_pCG, "", m_apWELL);

    m_pPair = new RandomPair<OoANavRelRAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<OoANavRelRAgent>(this, m_pCG, "");

    m_pOAD = new OldAgeDeath<OoANavRelRAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new Fertility<OoANavRelRAgent>(this, m_pCG, "");

    m_pNavigate = new Navigate<OoANavRelRAgent>(this, m_pCG, "", m_apWELL);

    //    m_pCM = new ConfinedMove<OoANavRelRAgent>(this, m_pCG, "");

    // adding all actions to prioritizer

    m_prio.addAction(m_pME);
    m_prio.addAction(m_pRM);
    m_prio.addAction(m_pBirthDeathRel);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pNPPCap);
    m_prio.addAction(m_pNavigate);

}



///----------------------------------------------------------------------------
// destructor
//
OoANavRelRPop::~OoANavRelRPop() {

    if (m_adCapacities != NULL) {
        delete[] m_adCapacities;
    }
    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_adBirthRates != NULL) {
        delete[] m_adBirthRates;
    }
    if (m_aiNumBirths != NULL) {
        delete[] m_aiNumBirths[0];
        delete[] m_aiNumBirths[1];
        delete[] m_aiNumBirths;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pRM != NULL) {
        delete m_pRM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
     if (m_pBirthDeathRel != NULL) {
        delete m_pBirthDeathRel;
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
    if (m_pNavigate != NULL) {
        delete m_pNavigate;
    }

}


///----------------------------------------------------------------------------
// setParams
//
int OoANavRelRPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoANavRelRPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoANavRelRAgent>::preLoop();

    if (iResult == 0) {
        int iN = getNumAgentsEffective();
        if (iN > 0) {
                // ok
        } else {
            printf("[OoANavRelRPop::preLoop] No agents found\n");
        }
    }

    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int OoANavRelRPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    return iResult;
}


///----------------------------------------------------------------------------
//  finalizeStep
//
int OoANavRelRPop::finalizeStep() {
    int iResult = SPopulation::finalizeStep();
    return iResult;
}


///----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agents in water or ice
//    EVENT_ID_CLIMATE  : (NPP is given, so no NPP calculation
//    EVENT_ID_VEG      : update NPP
//    EVENT_ID_NAV      : update seaways
//
int OoANavRelRPop::updateEvent(int iEventID, char *pData, float fT) {
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

    notifyObservers(iEventID, pData);
    m_bPendingEvents = true;
    return 0;
}


///----------------------------------------------------------------------------
// flushEvents
//
void OoANavRelRPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoANavRelRPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    
    printf("[OoANavRelRPop::makeOffspringAtIndex]T%f start\n", m_fCurTime);
    WELLUtils::showStates(m_apWELL, omp_get_max_threads(), false);

    OoANavRelRAgent &newAgent = m_aAgents[iAgent];
    newAgent.m_fAge = 0.0;
    newAgent.m_fLastBirth = 0.0;
    newAgent.m_iMateIndex = -3;

    // child & death stistics
    newAgent.m_iNumBabies = 0;
    m_aAgents[iMother].m_iNumBabies++;
    printf("[OoANavRelRPop::makeOffspringAtIndex]T%f end\n", m_fCurTime);
    WELLUtils::showStates(m_apWELL, omp_get_max_threads(), false);
    return iResult;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoANavRelRPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
void OoANavRelRPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoANavRelRAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}



