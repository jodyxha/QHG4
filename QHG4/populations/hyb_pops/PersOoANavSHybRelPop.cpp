#include <hdf5.h>

#include "EventConsts.h"
#include "GeneUtils.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "PersWeightedMove.cpp"
//#include "ConfinedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "PersHybBirthDeathRel.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "PersOldAgeDeath.cpp"
#include "PersFertility.cpp"
#include "NPPCapacity.cpp"
#include "GenomeCreator.cpp"
#include "OrigomeCreator.cpp"
#include "Navigate.cpp"
#include "SequenceIOUtils.cpp"
#include "PrivParamMix.cpp"
#include "PersOoANavSHybRelPop.h"


//----------------------------------------------------------------------------
// constructor
//
PersOoANavSHybRelPop::PersOoANavSHybRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<PersOoANavSHybRelAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true),
      m_bPendingEvents(false),
      m_pGeography(pCG->m_pGeography) {

    m_adCapacities = new double[m_pCG->m_iNumCells];
    int iCapacityStride = 1;
    //    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
  
    m_aiNumBirths  = new int*[2];
    m_aiNumBirths[0]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[0], 0, m_pCG->m_iNumCells*sizeof(int));
    m_aiNumBirths[1]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[1], 0, m_pCG->m_iNumCells*sizeof(int));

    m_pNPPCap = new NPPCapacity<PersOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<PersOoANavSHybRelAgent>::evaluatorinfos mEvalInfo;
    
    intset sTriggerIDs;
    sTriggerIDs.insert(EVENT_ID_GEO);    
    sTriggerIDs.insert(EVENT_ID_VEG);

    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<PersOoANavSHybRelAgent> *pSEAlt = new SingleEvaluator<PersOoANavSHybRelAgent>(this, pCG, "Alt", NULL, (double*)m_pGeography->m_adAltitude, "AltCapPref", false, sTriggerIDs);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<PersOoANavSHybRelAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<PersOoANavSHybRelAgent> *pSECap = new SingleEvaluator<PersOoANavSHybRelAgent>(this, pCG, "NPP", NULL, m_adCapacities, "", false, sTriggerIDs);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<PersOoANavSHybRelAgent>*>("Multi_weight_npp", pSECap));

    m_pME = new MultiEvaluator<PersOoANavSHybRelAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_MUL_SIMPLE, true); //true: delete evaluators
    addObserver(m_pME);

    m_pHybBirthDeathRel = new PersHybBirthDeathRel<PersOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_aiNumBirths, m_adCapacities, iCapacityStride);

    // evaluator for movement

    m_pWM = new PersWeightedMove<PersOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<PersOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<PersOoANavSHybRelAgent>(this, m_pCG, "");

    m_pOAD = new PersOldAgeDeath<PersOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new PersFertility<PersOoANavSHybRelAgent>(this, m_pCG, "");

    m_pNavigate = new Navigate<PersOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    //    m_pCM = new ConfinedMove<PersOoANavSHybRelAgent>(this, m_pCG, "");

    // adding all actions to prioritizer

    m_prio.addAction(m_pME);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pHybBirthDeathRel);
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
PersOoANavSHybRelPop::~PersOoANavSHybRelPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
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
     if (m_pHybBirthDeathRel != NULL) {
        delete m_pHybBirthDeathRel; 
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
    if (m_adCapacities != NULL) {
        delete[] m_adCapacities;
    }
}


///----------------------------------------------------------------------------
// setParams
//
int PersOoANavSHybRelPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int PersOoANavSHybRelPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<PersOoANavSHybRelAgent>::preLoop();

    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int PersOoANavSHybRelPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);

    return iResult;
}

///----------------------------------------------------------------------------
//  finalizeStep
//
int PersOoANavSHybRelPop::finalizeStep() {
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
int PersOoANavSHybRelPop::updateEvent(int iEventID, char *pData, float fT) {
    if (iEventID == EVENT_ID_GEO) {
        // drown  or ice
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != LBController::NIL) {
            int iLastAgent = getLastAgentIndex();
            
            int iCount = 0;
            
#pragma omp parallel for reduction(+:iCount)
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    int iCellIndex = m_aAgents[iAgent].m_iCellIndex;
                    if ((m_pGeography->m_adAltitude[iCellIndex] < 0) ||
                        (m_pGeography->m_abIce[iCellIndex] > 0)) {
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
};


///----------------------------------------------------------------------------
// flushEvents
//
void PersOoANavSHybRelPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int PersOoANavSHybRelPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    
    PersOoANavSHybRelAgent &curMother = m_aAgents[iMother];
    PersOoANavSHybRelAgent &curFather = m_aAgents[iFather];
    
    PersOoANavSHybRelAgent &curAgent = m_aAgents[iAgent];
    curAgent.m_fAge = 0.0;
    curAgent.m_fLastBirth = 0.0;
    curAgent.m_iMateIndex = -3;

    //@@TODO: use christophs [B(h1,R1)+B(h2,R2)]/2
    curAgent.m_fHybridization    = (curMother.m_fHybridization + curFather.m_fHybridization)/2.0;


    // if mix mode is 4 (MODE_WEIGHTEDMIX) or 5 (MODE_PUREMIX), the mix parameter should be the hybridisation;
    // for other modes the parameter is ignored 
    m_pPrivParamMix->calcParams(curMother, curFather, curAgent, curAgent.m_fHybridization); 


    // child & death stistics
    curAgent.m_iNumBabies = 0;
    curMother.m_iNumBabies++;
    
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int PersOoANavSHybRelPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fAge from [%s]\n", *ppData);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fHybridization);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fLastBirth from [%s]\n", *ppData);
    }

    //@@ TODO: same for all the other fileds
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dMoveProb);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fHybridization from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dMaxAge);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dMoveProb from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dUncertainty);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dMaxAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fFertilityMinAge);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dUncertainty from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fFertilityMaxAge);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fFertilityMinAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fInterbirth);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fFertilityMaxAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dB0);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fInterbirth from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dD0);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dB0 from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dTheta);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dD0 from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dBReal);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dD0 from [%s]\n", *ppData);
    }

    if (iResult != 0) {
        printf("[addPopSpecificAgentData] Couldn't read m_dBReal from [%s]\n", *ppData);
    }
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void PersOoANavSHybRelPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    PersOoANavSHybRelAgent agent;
    H5Tinsert(*hAgentDataType, "Age",           qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth",     qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Hybridization", qoffsetof(agent, m_fHybridization), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "MoveProb",      qoffsetof(agent, m_dMoveProb), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "MaxAge",        qoffsetof(agent, m_dMaxAge), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "Uncertainty",   qoffsetof(agent, m_dUncertainty), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "FertilityMinAge", qoffsetof(agent, m_fFertilityMinAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "FertilityMaxAge", qoffsetof(agent, m_fFertilityMaxAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Interbirth",    qoffsetof(agent, m_fInterbirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "B0",            qoffsetof(agent, m_dB0), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "D0",            qoffsetof(agent, m_dD0), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "Theta",         qoffsetof(agent, m_dTheta), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "BReal",         qoffsetof(agent, m_dBReal), H5T_NATIVE_DOUBLE);
}
