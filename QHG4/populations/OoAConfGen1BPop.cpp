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
#include "ConfinedMove.cpp"
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
#include "Navigate.cpp"
#include "OoAConfGen1BPop.h"


//----------------------------------------------------------------------------
// constructor
//
OoAConfGen1BPop::OoAConfGen1BPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoAConfGen1BAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true) {

    int iCapacityStride = 1;
    m_adCapacities = new double[m_pCG->m_iNumCells];
    memset(m_adCapacities, 0, m_pCG->m_iNumCells*sizeof(double));
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
  
    m_pNPPCap = new NPPCapacity<OoAConfGen1BAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride); 

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    MultiEvaluator<OoAConfGen1BAgent>::evaluatorinfos mEvalInfo;

    // add altitude evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoAConfGen1BAgent> *pSEAlt = new SingleEvaluator<OoAConfGen1BAgent>(this, m_pCG, "Alt", NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref",true, EVENT_ID_GEO);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoAConfGen1BAgent>*>("Multi_weight_alt", pSEAlt));

    // add NPP evaluation - output is NULL because it will be set by MultiEvaluator
    SingleEvaluator<OoAConfGen1BAgent> *pSECap = new SingleEvaluator<OoAConfGen1BAgent>(this, m_pCG, "NPP", NULL, m_adCapacities, "NPPPref", true, EVENT_ID_VEG);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<OoAConfGen1BAgent>*>("Multi_weight_npp", pSECap));

    m_pME = new MultiEvaluator<OoAConfGen1BAgent>(this, m_pCG, "Alt+NPP", m_adEnvWeights, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators


    m_pVerVarK = new VerhulstVarK<OoAConfGen1BAgent>(this, m_pCG, "", m_apWELL, m_adCapacities, iCapacityStride);


    // evaluator for movement

    m_pWM = new WeightedMove<OoAConfGen1BAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);

    m_pPair = new RandomPair<OoAConfGen1BAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<OoAConfGen1BAgent>(this, m_pCG, "");

    m_pOAD = new OldAgeDeath<OoAConfGen1BAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new Fertility<OoAConfGen1BAgent>(this, m_pCG, "");

    m_pGenetics = new Genetics<OoAConfGen1BAgent,BitGeneUtils>(this, m_pCG, "", m_pAgentController, &m_vMergedDeadList, m_aiSeeds[1]);

    m_pNavigate = new Navigate<OoAConfGen1BAgent>(this, m_pCG, "", m_apWELL);

    m_pCM = new ConfinedMove<OoAConfGen1BAgent>(this, m_pCG, "");

    // adding all actions to prioritizer

    m_prio.addAction(m_pME);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pCM);
    m_prio.addAction(m_pVerVarK);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pNPPCap);
    m_prio.addAction(m_pGenetics);
    m_prio.addAction(m_pNavigate);

}



///----------------------------------------------------------------------------
// destructor
//
OoAConfGen1BPop::~OoAConfGen1BPop() {

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
    if (m_pCM != NULL) {
        delete m_pCM;
    }
    if (m_pGenetics != NULL) {
        delete m_pGenetics;
    }
    if (m_pNavigate != NULL) {
        delete m_pNavigate;
    }

}


///----------------------------------------------------------------------------
// setParams
//
int OoAConfGen1BPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoAConfGen1BPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoAConfGen1BAgent>::preLoop();
   
    int iN = getNumAgentsEffective();
    iResult = m_pGenetics->createInitialGenomes(iN);
        
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
int OoAConfGen1BPop::updateEvent(int iEventID, char *pData, float fT) {
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
       
    return 0;
};


///----------------------------------------------------------------------------
// flushEvents
//
void OoAConfGen1BPop::flushEvents(float fT) {
    notifyObservers(EVENT_ID_FLUSH, NULL);
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoAConfGen1BPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    

    m_pGenetics->initialize(m_fCurTime);
    iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;

    // child & death stistics
    m_aAgents[iAgent].m_iNumBabies = 0;
    m_aAgents[iMother].m_iNumBabies++;
    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int OoAConfGen1BPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
      
    return m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoAConfGen1BPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
void OoAConfGen1BPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoAConfGen1BAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int OoAConfGen1BPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pGenetics->readAdditionalDataQDF(hSpeciesGroup);

    /*
    if (iResult == 0) {
        // we read the genome from a QDF: no need to create it
        m_bCreateGenomes = false;
    } else {
        m_bCreateGenomes = true;
        iResult = 0;
    }
    */
    return iResult;
}


//----------------------------------------------------------------------------
// copyAdditionalDataQDF
//
int OoAConfGen1BPop::copyAdditionalDataQDF(int iStart, int iCount, SPopulation *pPop) {
    OoAConfGen1BPop *pO = static_cast<OoAConfGen1BPop *>(pPop);
    return m_pGenetics->getGenome()->copyBlock(iStart, pO->m_pGenetics->getGenome(), 0, (uint)iCount);
}
