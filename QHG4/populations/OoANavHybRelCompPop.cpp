#include <hdf5.h>

#include "EventConsts.h"
#include "GeneUtils.h"
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
#include "Genetics.cpp"
#include "GenomeCreator.cpp"
#include "Hybrids.cpp"
#include "OrigomeCreator.cpp"
#include "Navigate.cpp"
#include "SequenceIOUtils.cpp"
#include "OoANavHybRelCompPop.h"


//----------------------------------------------------------------------------
// constructor
//
OoANavHybRelCompPop::OoANavHybRelCompPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoANavHybRelCompAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
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

    m_pNPPCap = new NPPCapacity<OoANavHybRelCompAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<OoANavHybRelCompAgent>::evaluatorinfos mEvalInfo;
    
    intset sTriggerIDs;
    sTriggerIDs.insert(EVENT_ID_GEO);    
    sTriggerIDs.insert(EVENT_ID_VEG);

    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoANavHybRelCompAgent> *pSEAlt = new SingleEvaluator<OoANavHybRelCompAgent>(this, m_pCG, "Alt", NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref", false, sTriggerIDs);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavHybRelCompAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoANavHybRelCompAgent> *pSECap = new SingleEvaluator<OoANavHybRelCompAgent>(this, m_pCG, "NPP", NULL, m_adCapacities, "", false, sTriggerIDs);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavHybRelCompAgent>*>("Multi_weight_npp", pSECap));

    m_pME = new MultiEvaluator<OoANavHybRelCompAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_MUL_SIMPLE, true); //true: delete evaluators
    addObserver(m_pME);


    m_pBirthDeathRelComp = new BirthDeathRelComp<OoANavHybRelCompAgent>(this, m_pCG, "", m_apWELL, m_adBirthRates, m_aiNumBirths, m_adCapacities, iCapacityStride);

    // evaluator for movement

    m_pWM = new WeightedMove<OoANavHybRelCompAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<OoANavHybRelCompAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<OoANavHybRelCompAgent>(this, m_pCG, "");

    m_pOAD = new OldAgeDeath<OoANavHybRelCompAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new Fertility<OoANavHybRelCompAgent>(this, m_pCG, "");

    m_pHybrids = new Hybrids<OoANavHybRelCompAgent,GeneUtils>(this, m_pCG, "", m_pAgentController, &m_vMergedDeadList, m_aiSeeds[1]);

    m_pNavigate = new Navigate<OoANavHybRelCompAgent>(this, m_pCG, "", m_apWELL);

    //    m_pCM = new ConfinedMove<OoANavHybRelCompAgent>(this, m_pCG, "");

    // adding all actions to prioritizer

    m_prio.addAction(m_pME);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pBirthDeathRelComp);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pNPPCap);
    m_prio.addAction(m_pHybrids);
    m_prio.addAction(m_pNavigate);

}



///----------------------------------------------------------------------------
// destructor
//
OoANavHybRelCompPop::~OoANavHybRelCompPop() {

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
    if (m_pHybrids != NULL) {
        delete m_pHybrids;
    }

}


///----------------------------------------------------------------------------
// setParams
//
int OoANavHybRelCompPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoANavHybRelCompPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoANavHybRelCompAgent>::preLoop();

    if (iResult == 0) {
        int iN = getNumAgentsEffective();
        iResult = m_pHybrids->createInitialGenomes(iN);
        if (iResult == 0) {
            // ok
        } else {
            printf("[OoANavHybRelCompPop::preLoop] error creating intial genomes\n");
        }
    }

    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int OoANavHybRelCompPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    m_pHybrids->initialize(m_fCurTime);

    return iResult;
}

///----------------------------------------------------------------------------
//  finalizeStep
//
int OoANavHybRelCompPop::finalizeStep() {
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
int OoANavHybRelCompPop::updateEvent(int iEventID, char *pData, float fT) {
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
};


///----------------------------------------------------------------------------
// flushEvents
//
void OoANavHybRelCompPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoANavHybRelCompPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    

    iResult = m_pHybrids->makeOffspring(iAgent, iMother, iFather);
    OoANavHybRelCompAgent &newAgent = m_aAgents[iAgent];
    newAgent.m_fAge = 0.0;
    newAgent.m_fLastBirth = 0.0;
    newAgent.m_iMateIndex = -3;
    newAgent.m_fHybridization = 0.0;;

    // child & death stistics
    newAgent.m_iNumBabies = 0;
    m_aAgents[iMother].m_iNumBabies++;
    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int OoANavHybRelCompPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    printf("@@@@ [OoANavHybRelPop::writeAdditionalDataQDF]\n");
    int iResult = m_pHybrids->writeAdditionalDataQDF(hSpeciesGroup);
    return iResult;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoANavHybRelCompPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
    if (iResult != 0) {
        printf("[addPopSpecificAgentData] Couldn't read m_fHybridization from [%s]\n", *ppData);
    }
    
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void OoANavHybRelCompPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoANavHybRelCompAgent agent;
    H5Tinsert(*hAgentDataType, "Age",           qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth",     qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Hybridization", qoffsetof(agent, m_fHybridization), H5T_NATIVE_FLOAT);
}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int OoANavHybRelCompPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pHybrids->readAdditionalDataQDF(hSpeciesGroup);
    return iResult;
}



//----------------------------------------------------------------------------
// dumpAdditionalDataQDF
//  write additional data to the group 
//  (data to be stored separately from agent data, e.g. ancestors IDs)
//
int  OoANavHybRelCompPop::dumpAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pHybrids->dumpAdditionalDataQDF(hSpeciesGroup);
    printf("[OoANavHybRelCompPop::dumpAdditionalDataQDF] gendump returned %d\n", iResult);
    return iResult;
}


//----------------------------------------------------------------------------
// restoreAdditionalDataQDF
//  read additional data from the group
//  (data stored separately from agent data)
//
int  OoANavHybRelCompPop::restoreAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = m_pHybrids->restoreAdditionalDataQDF(hSpeciesGroup);
    
    return iResult;
}

//----------------------------------------------------------------------------
// copyAdditionalDataQDF
//
int OoANavHybRelCompPop::copyAdditionalDataQDF(int iStart, int iCount, SPopulation *pPop) {
    OoANavHybRelCompPop *pO = static_cast<OoANavHybRelCompPop*>(pPop);
    int iResult = m_pHybrids->getGenome()->copyBlock(iStart, pO->m_pHybrids->getGenome(), 0, (uint)iCount);
    if (iResult == 0) {
        iResult = m_pHybrids->getOrigome()->copyBlock(iStart, pO->m_pHybrids->getOrigome(), 0, (uint)iCount);
    }
    return iResult;
}
