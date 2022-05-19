#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "GroupMemberMove.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "Fertility.cpp"
#include "AgentEnv.cpp"
#include "FoodManager.cpp"

#include "RandomGroupSplitter.h"
#include "GroupManager.h"
#include "ChildManager.h"
#include "ChildManager.cpp"
#include "GroupPop.h"


//----------------------------------------------------------------------------
// constructor
//
GroupPop::GroupPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<GroupAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
    m_bPendingEvents(false) {

    m_pAgentCounts   = new int[m_pCG->m_iNumCells];
    memset(m_pAgentCounts, 0, m_pCG->m_iNumCells*sizeof(int));

    m_pAE   = new AgentEnv<GroupAgent>(this, m_pCG, "", m_apWELL, m_pAgentController, m_pCG->m_pVegetation->m_adTotalANPP, m_pAgentCounts);

    m_pFM   = new FoodManager<GroupAgent>(this, m_pCG, "", m_apWELL, m_pCG->m_pVegetation->m_adTotalANPP, m_pAgentCounts);

    m_pGMM  = new GroupMemberMove<GroupAgent>(this, m_pCG, "", m_apWELL, m_pAE);

    m_pPair = new RandomPair<GroupAgent>(this, m_pCG, "", m_apWELL);

    m_pGO   = new GetOld<GroupAgent>(this, m_pCG, "");

    m_pOAD  = new OldAgeDeath<GroupAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new Fertility<GroupAgent>(this, m_pCG, "");

    m_pCM   = new ChildManager<GroupAgent>(this, m_pCG, "", m_apWELL);

    RandomGroupSplitter *pRGS = new RandomGroupSplitter(m_apWELL[0], 3, 8);
    m_pGM =GroupManager::createInstance(this, pRGS);
    
    // adding all actions to prioritizer

    m_prio.addAction(m_pGMM);
    m_prio.addAction(m_pAE);
    m_prio.addAction(m_pFM);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pFert);
  
}


//----------------------------------------------------------------------------
// destructor
//
GroupPop::~GroupPop() {

    if (m_pFoodAvailable != NULL) {
        delete[] m_pFoodAvailable;
    }
    if (m_pAgentCounts != NULL) {
        delete[] m_pAgentCounts;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pAE != NULL) {
        delete m_pAE;
    }
    if (m_pFM != NULL) {
        delete m_pFM;
    }
    if (m_pGMM != NULL) {
        delete m_pGMM;
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
    if (m_pCM != NULL) {
        delete m_pCM;
    }
    if (m_pGM != NULL) {
        delete m_pGM;
    }
}

//----------------------------------------------------------------------------
// setParams
//
int GroupPop::setParams(const std::string sParams) {
    int iResult = 0;

    return iResult;
}


//----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agents in water or ice
//    EVENT_ID_CLIMATE  : (NPP is given, so no NPP calculation
//    EVENT_ID_VEG      : update NPP
//    EVENT_ID_NAV      : update seaways
//
int GroupPop::updateEvent(int iEventID, char *pData, float fT) {
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
};


//----------------------------------------------------------------------------
// flushEvents
//
void GroupPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


//----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int GroupPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    

    GroupAgent &newAgent = m_aAgents[iAgent];
    newAgent.m_fAge = 0.0;
    newAgent.m_fLastBirth = 0.0;
    newAgent.m_iMateIndex = -3;

    newAgent.m_iParentIdx = iMother;
    newAgent.m_iGroupID   =  m_aAgents[iMother].m_iGroupID;
    newAgent.m_iHomeID    =  m_aAgents[iMother].m_iHomeID;
    newAgent.m_fHealth    = 1;

    m_pCM->addChild(iMother, iAgent);

    // TODO: add baby to group!!
    m_pGM->addAgentToGroup(newAgent.m_iGroupID, iAgent);
    return iResult;
}


//----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int GroupPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_iGroupID);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_iHomeID);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_iParentIdx);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fHealth);
    }
    // TODO: children how? numchildren + N children? Or separate action with array of children?
    return iResult;
}



//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void GroupPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    GroupAgent agent;
    H5Tinsert(*hAgentDataType, "Age",       qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "GroupID",   qoffsetof(agent, m_iGroupID), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "HomeID",    qoffsetof(agent, m_iHomeID), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "ParentID",  qoffsetof(agent, m_iParentIdx), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Health",    qoffsetof(agent, m_fHealth), H5T_NATIVE_FLOAT);
    
}


//----------------------------------------------------------------------------
// registerMove
//  only adult agents can register their move
//  children of an adult are forced to nove to the same cell
//
void GroupPop::registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo) {
    if (m_aAgents[iAgentIndex].m_fAge >= m_pFert->getMinAge()) {
        this->registerMove(iCellIndexFrom, iAgentIndex, iCellIndexTo);
        
        const std::vector<int> &vChildren = m_pCM->getChildren(iAgentIndex);
        // move all children along
        for (uint i = 0; i < vChildren.size(); i++) {
            SPopulation::registerMove(iCellIndexFrom, vChildren[i], iCellIndexTo);
            
        }
    }
}

//----------------------------------------------------------------------------
// makePopSpecificMove
//
int GroupPop::makePopSpecificMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo) {
    
    //don't need this?
   
    return 0;
}


//----------------------------------------------------------------------------
// makePopSpecificDeath
//
int GroupPop::makePopSpecificDeath(int iAgentIndex) {
    // remove from group
    m_pGM->removeAgentFromGroup(iAgentIndex); 
    // TODO
    // if has children, find a different parent
    return 0;
    
}


//----------------------------------------------------------------------------
// initializeStep
//
int GroupPop::initializeStep(float fTime) {
    int iResult = SPopulation::initializeStep(fTime);

    m_pCM->checkAdult();
    return iResult;
}


//----------------------------------------------------------------------------
// setGroupID
//
int GroupPop::setGroupID(int iGroupID, idtype iAgentIdx) {
    m_aAgents[iAgentIdx].m_iGroupID = iGroupID;
    return 0;
}


//----------------------------------------------------------------------------
// setParent
//
int GroupPop::setParent(int iChildIdx, int iParentIdx)  {
    m_aAgents[iChildIdx].m_iParentIdx = iParentIdx;
    return 0;
}

/*
//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int GroupPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = SPopulation::writeAdditionalDataQDF(hSpeciesGroup);
    if (iResult == 0) {
        //@@TODO        iResult = m_pCM->writeToQDF(hSpeciesGroup);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int GroupPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = SPopulation::writeAdditionalDataQDF(hSpeciesGroup);
    if (iResult == 0) {
        //@@TODO        iResult = m_pCM->readFromQDF(hSpeciesGroup);
    }
    return iResult;
}
*/
