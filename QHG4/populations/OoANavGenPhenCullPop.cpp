#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "BitGeneUtils.h"
//#include "GeneUtils.h"
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
#include "VerhulstVarKCull.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "Fertility.cpp"
#include "NPPCapacity.cpp"
#include "Genetics.cpp"
//@@@@@@@@@@@@@@@@@@#include "Phenetics.cpp"
#include "Phenetics2.cpp"
#include "GenomeCreator.cpp"
#include "Navigate.cpp"
#include "SequenceIOUtils.cpp"
#include "OoANavGenPhenCullPop.h"


//----------------------------------------------------------------------------
// constructor
//
OoANavGenPhenCullPop::OoANavGenPhenCullPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoANavGenPhenCullAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true),
      m_bPendingEvents(false),
      m_pGeography(pCG->m_pGeography) {

    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
  
    m_pNPPCap = new NPPCapacity<OoANavGenPhenCullAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<OoANavGenPhenCullAgent>::evaluatorinfos mEvalInfo;
 
    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoANavGenPhenCullAgent> *pSEAlt = new SingleEvaluator<OoANavGenPhenCullAgent>(this, m_pCG, "Alt", NULL, (double*)m_pGeography->m_adAltitude, "AltCapPref", true, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavGenPhenCullAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoANavGenPhenCullAgent> *pSECap = new SingleEvaluator<OoANavGenPhenCullAgent>(this, m_pCG, "NPP", NULL, m_adCapacities, "NPPPref", true, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoANavGenPhenCullAgent>*>("Multi_weight_npp", pSECap));

    m_pME = new MultiEvaluator<OoANavGenPhenCullAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true); //true: delete evaluators
    addObserver(m_pME);


    m_pVerVarKCull = new VerhulstVarKCull<OoANavGenPhenCullAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride);


    // evaluator for movement

    m_pWM = new WeightedMove<OoANavGenPhenCullAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<OoANavGenPhenCullAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<OoANavGenPhenCullAgent>(this, m_pCG, "");

    m_pOAD = new OldAgeDeath<OoANavGenPhenCullAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new Fertility<OoANavGenPhenCullAgent>(this, m_pCG, "");

    //m_pGenetics = new Genetics<OoANavGenPhenCullAgent,BitGeneUtils>(this, m_pCG, "", m_pAgentController, &m_vMergedDeadList, m_apWELL);
    m_pGenetics = new Genetics<OoANavGenPhenCullAgent,BitGeneUtils>(this, m_pCG, "", m_pAgentController, &m_vMergedDeadList, m_aiSeeds[1]);

    //m_pPhenetics = new Phenetics<OoANavGenPhenCullAgent>(this, m_pCG, "", m_pAgentController, &m_vMergedDeadList, m_apWELL);
    m_pPhenetics = new Phenetics2<OoANavGenPhenCullAgent>(this, m_pCG, "", m_pAgentController, &m_vMergedDeadList, m_aiSeeds[2]);

    m_pNavigate = new Navigate<OoANavGenPhenCullAgent>(this, m_pCG, "", m_apWELL);

    //    m_pCM = new ConfinedMove<OoANavGenPhenCullAgent>(this, m_pCG, "");

    // adding all actions to prioritizer

    m_prio.addAction(m_pME);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pVerVarKCull);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pNPPCap);
    m_prio.addAction(m_pGenetics);
    m_prio.addAction(m_pPhenetics);
    m_prio.addAction(m_pNavigate);

}



///----------------------------------------------------------------------------
// destructor
//
OoANavGenPhenCullPop::~OoANavGenPhenCullPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_adCapacities != NULL) {
        delete[] m_adCapacities;
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
     if (m_pVerVarKCull != NULL) {
        delete m_pVerVarKCull;
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
    if (m_pGenetics != NULL) {
        delete m_pGenetics;
    }
    if (m_pPhenetics != NULL) {
        delete m_pPhenetics;
    }

}


///----------------------------------------------------------------------------
// setParams
//
int OoANavGenPhenCullPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoANavGenPhenCullPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoANavGenPhenCullAgent>::preLoop();

    if (iResult == 0) {
        int iN = getNumAgentsEffective();
        iResult = m_pGenetics->createInitialGenomes(iN);
        if (iResult == 0) {
            iResult = m_pPhenetics->createInitialPhenomes(iN);
            if (iResult == 0) {
                // ok
            } else {
                printf("[OoANavGenPhenCullPop::preLoop] error creating intial phenomes\n");
            }
        } else {
            printf("[OoANavGenPhenCullPop::preLoop] error creating intial genomes\n");
        }
    }

    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int OoANavGenPhenCullPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    m_pGenetics->initialize(m_fCurTime);
    m_pPhenetics->initialize(m_fCurTime);
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
int OoANavGenPhenCullPop::updateEvent(int iEventID, char *pData, float fT) {
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
    m_bPendingEvents = true;
    return 0;
};


///----------------------------------------------------------------------------
// flushEvents
//
void OoANavGenPhenCullPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoANavGenPhenCullPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    

    iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    iResult = m_pPhenetics->makeOffspring(iAgent, iMother, iFather);

    OoANavGenPhenCullAgent &newAgent = m_aAgents[iAgent];
    newAgent.m_fAge = 0.0;
    newAgent.m_fLastBirth = 0.0;
    newAgent.m_iMateIndex = -3;

    // child & death stistics
    newAgent.m_iNumBabies = 0;
    m_aAgents[iMother].m_iNumBabies++;
    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int OoANavGenPhenCullPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);
    if (iResult == 0) {
        iResult = m_pPhenetics->writeAdditionalDataQDF(hSpeciesGroup);
    }
    return iResult;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoANavGenPhenCullPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
void OoANavGenPhenCullPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoANavGenPhenCullAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int OoANavGenPhenCullPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pGenetics->readAdditionalDataQDF(hSpeciesGroup);
    if (iResult == 0) {
        iResult = m_pPhenetics->readAdditionalDataQDF(hSpeciesGroup);
    }
    return iResult;
}



//----------------------------------------------------------------------------
// dumpAdditionalDataQDF
//  write additional data to the group 
//  (data to be stored separately from agent data, e.g. ancestors IDs)
//
int  OoANavGenPhenCullPop::dumpAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pGenetics->dumpAdditionalDataQDF(hSpeciesGroup);
    printf("[OoANavGenPhenCullPop::dumpAdditionalDataQDF] gendump returned %d\n", iResult);
    if (iResult == 0) {
        iResult = m_pPhenetics->dumpAdditionalDataQDF(hSpeciesGroup);
        printf("[OoANavGenPhenCullPop::dumpAdditionalDataQDF] phendump returned %d\n", iResult);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// restoreAdditionalDataQDF
//  read additional data from the group
//  (data stored separately from agent data)
//
int  OoANavGenPhenCullPop::restoreAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = m_pGenetics->restoreAdditionalDataQDF(hSpeciesGroup);
    if (iResult == 0) {
        iResult = m_pPhenetics->restoreAdditionalDataQDF(hSpeciesGroup);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// copyAdditionalDataQDF
//
int OoANavGenPhenCullPop::copyAdditionalDataQDF(int iStart, int iCount, SPopulation *pPop) {
    OoANavGenPhenCullPop *pO = static_cast<OoANavGenPhenCullPop *>(pPop);
    int iResult =  m_pGenetics->getGenome()->copyBlock(iStart, pO->m_pGenetics->getGenome(), 0, (uint)iCount);
    if (iResult == 0) {
        iResult = m_pPhenetics->getPhenome()->copyBlock(iStart, pO->m_pPhenetics->getPhenome(), 0, (uint)iCount);
    }
    return iResult;
}
