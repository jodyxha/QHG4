#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
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
#include "BirthDeathRelComp.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "Fertility.cpp"
#include "NPPCapacity.cpp"
#include "Navigate.cpp"
#include "OoANavRelCompPop.h"


//----------------------------------------------------------------------------
// constructor
//
OoANavRelCompPop::OoANavRelCompPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoANavRelCompAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true),
      m_bPendingEvents(false) {

    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
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

    m_pNPPCap = new NPPCapacity<OoANavRelCompAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<OoANavRelCompAgent>::evaluatorinfos mEvalInfo;
 
    intset sTriggerIDs;
    sTriggerIDs.insert(EVENT_ID_GEO);    
    sTriggerIDs.insert(EVENT_ID_VEG);
    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    // oriSingleEvaluator<OoANavRelCompAgent> *pSEAlt = new SingleEvaluator<OoANavRelCompAgent>(this, pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", true, EVENT_ID_GEO);
    SingleEvaluator<OoANavRelCompAgent> *pSEAlt = new SingleEvaluator<OoANavRelCompAgent>(this, m_pCG, "Alt", NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", false, sTriggerIDs);
    // for mult: SingleEvaluator<OoANavRelCompAgent> *pSEAlt = new SingleEvaluator<OoANavRelCompAgent>(this, pCG, "Alt", NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", false, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavRelCompAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    // ori SingleEvaluator<OoANavRelCompAgent> *pSECap = new SingleEvaluator<OoANavRelCompAgent>(this, pCG, "NPP", NULL, m_adCapacities, "NPPPref", true, EVENT_ID_VEG);
   
    // we want to evaluate the pure capacities - i.e. no PolyLine translation (NULL); no cumulation (false)
    SingleEvaluator<OoANavRelCompAgent> *pSECap = new SingleEvaluator<OoANavRelCompAgent>(this, pCG, "NPP", NULL, m_adCapacities, "", false, sTriggerIDs);
   
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavRelCompAgent>*>("Multi_weight_npp", pSECap));

    m_pME = new MultiEvaluator<OoANavRelCompAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_MUL_SIMPLE, true); //true: delete evaluators
    addObserver(m_pME);


    m_pBirthDeathRelComp = new BirthDeathRelComp<OoANavRelCompAgent>(this, m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_adCapacities, iCapacityStride);


    // evaluator for movement

    m_pWM = new WeightedMove<OoANavRelCompAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<OoANavRelCompAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<OoANavRelCompAgent>(this, m_pCG, "");

    m_pOAD = new OldAgeDeath<OoANavRelCompAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new Fertility<OoANavRelCompAgent>(this, m_pCG, "");

    m_pNavigate = new Navigate<OoANavRelCompAgent>(this, m_pCG, "", m_apWELL);

    //    m_pCM = new ConfinedMove<OoANavRelCompAgent>(this, m_pCG, "");

    // adding all actions to prioritizer

    m_prio.addAction(m_pME);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pBirthDeathRelComp);
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
OoANavRelCompPop::~OoANavRelCompPop() {

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
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
     if (m_pBirthDeathRelComp != NULL) {
        delete m_pBirthDeathRelComp;
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
int OoANavRelCompPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoANavRelCompPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoANavRelCompAgent>::preLoop();

    if (iResult == 0) {
        int iN = getNumAgentsEffective();
        if (iN > 0) {
                // ok
        } else {
            printf("[OoANavRelCompPop::preLoop] No agents found\n");
        }
    }

    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int OoANavRelCompPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    return iResult;
}


///----------------------------------------------------------------------------
//  finalizeStep
//
int OoANavRelCompPop::finalizeStep() {
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
int OoANavRelCompPop::updateEvent(int iEventID, char *pData, float fT) {
    if (iEventID == EVENT_ID_GEO) {
        // drown  or ice
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != LBController::NIL) {
            int iLastAgent = getLastAgentIndex();
                        
            //            int iChunk = (uint)ceil((iLastAgent-iFirstAgent+1)/(double)m_iNumThreads);
            //#pragma omp parallel for schedule(static, iChunk) 
            int iCount = 0;
            
#pragma omp parallel for reduction(+:iCount)
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    int iCellIndex = m_aAgents[iAgent].m_iCellIndex;
                    if ((this->m_pCG->m_pGeography->m_adAltitude[iCellIndex] < 0) ||
                        (this->m_pCG->m_pGeography->m_abIce[iCellIndex] > 0)) {
                        registerDeath(iCellIndex, iAgent);
                        iCount++;
                    }
                }
            }
  
            // make sure they are removed before step starts
            if (iCount > 0) {
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
        }

    }
    
    notifyObservers(iEventID, pData);
    m_bPendingEvents = true;
    return 0;
}


///----------------------------------------------------------------------------
// flushEvents
//
void OoANavRelCompPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoANavRelCompPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    

    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;

    // child & death stistics
    m_aAgents[iAgent].m_iNumBabies = 0;
    m_aAgents[iMother].m_iNumBabies++;
    return iResult;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoANavRelCompPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
void OoANavRelCompPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoANavRelCompAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}



