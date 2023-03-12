#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"

#include "GetOld.cpp"
#include "ATanDeath.cpp"
#include "Fertility.cpp"
#include "Verhulst.cpp"
#include "RandomPair.cpp"

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"

#include "tut_EnvironAltPop.h"

//----------------------------------------------------------------------------
// constructor
//
tut_EnvironAltPop::tut_EnvironAltPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<tut_EnvironAltAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_pGeography(pCG->m_pGeography) {

    m_pGO       = new GetOld<tut_EnvironAltAgent>(this, m_pCG, "");
    m_pAD       = new ATanDeath<tut_EnvironAltAgent>(this, m_pCG, "", m_apWELL);
    m_pFert     = new Fertility<tut_EnvironAltAgent>(this, m_pCG, "");
    m_pVerhulst = new Verhulst<tut_EnvironAltAgent>(this, m_pCG, "", m_apWELL);
    m_pPair     = new RandomPair<tut_EnvironAltAgent>(this, m_pCG, "", m_apWELL);

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    memset(m_adEnvWeights, 0, m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)*sizeof(double));
    m_pSEAlt = new SingleEvaluator<tut_EnvironAltAgent>(this, m_pCG, "Alt", m_adEnvWeights, (double*)m_pGeography->m_adAltitude, "AltCapPref", true, EVENT_ID_GEO);
   
    m_pWM = new WeightedMove<tut_EnvironAltAgent>(this, m_pCG, "", m_apWELL, m_adEnvWeights);





    // adding all actions to prioritizer

    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pVerhulst);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pSEAlt);
    m_prio.addAction(m_pWM);
}

//----------------------------------------------------------------------------
// destructor
//
tut_EnvironAltPop::~tut_EnvironAltPop() {

    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pAD != NULL) {
        delete m_pAD;
    }
    if (m_pFert != NULL) {
        delete m_pFert;
    }
    if (m_pVerhulst != NULL) {
        delete m_pVerhulst;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pSEAlt != NULL) {
        delete m_pSEAlt;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }  
}


//----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agent if under ice or water
//
int tut_EnvironAltPop::updateEvent(int iEventID, char *pData, float fT) { 
    if (iEventID == EVENT_ID_GEO) { 
        // drown
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != LBController::NIL) {
            int iLastAgent = getLastAgentIndex();

#pragma omp parallel for
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
void tut_EnvironAltPop::flushEvents(float fT) {
    notifyObservers(EVENT_ID_FLUSH, NULL);
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int tut_EnvironAltPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;

    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;

    return iResult;
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int tut_EnvironAltPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
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
void tut_EnvironAltPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    tut_EnvironAltAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);

}
