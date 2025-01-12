#include <vector>
#include <map>

#include "types.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "GroupPop.h"
#include "GroupSplitter.h"
#include "GroupManager.h"


//----------------------------------------------------------------------------
// createInstance
//
GroupManager *GroupManager::createInstance(GroupPop *pPop,  GroupSplitter *pGroupSplitter) {
    GroupManager *pGM = new GroupManager(pPop, pGroupSplitter);
    
    return pGM;
}


//----------------------------------------------------------------------------
// constructor
//
GroupManager::GroupManager(GroupPop *pPop, GroupSplitter *pGroupSplitter)
    : m_iNextGroupID(0), 
      m_pPop(pPop),
      m_pGroupSplitter(pGroupSplitter) {

}


//----------------------------------------------------------------------------
// destructor
//
GroupManager::~GroupManager() {
}


//----------------------------------------------------------------------------
// getNextGroupID
//
int GroupManager::getNextGroupID() {
    int iCurID = m_iNextGroupID++;
    m_mGroupAgents[iCurID].clear();
    return iCurID;
}


//----------------------------------------------------------------------------
// addAgentToGroup
//
int GroupManager::addAgentToGroup(int iGroupID, int iAgentIdx) {
    int iResult = -1;
    if (!groupContainsAgent(iGroupID, iAgentIdx)) {
        m_mGroupAgents[iGroupID].push_back(iAgentIdx);
        m_mAgentGroup[iAgentIdx] = iGroupID;

        m_pPop->setGroupID(iGroupID, iAgentIdx);
        
        iResult = 0;
    } else {
        xha_printf("[GroupManager::addAgentToGroup] Agent %d already in gtroup %d\n", iAgentIdx, iGroupID);
    }
    return iResult;

}

//----------------------------------------------------------------------------
// removeAgentFromGroup
//
int GroupManager::removeAgentFromGroup(int iAgentIdx){
    int iResult = -1;
    agentgroup::iterator itg = m_mAgentGroup.find(iAgentIdx);
    if (itg != m_mAgentGroup.end()) {
        intvec &vCur = m_mGroupAgents[itg->second];
        intvec::const_iterator ita = std::find(vCur.begin(), vCur.end(), iAgentIdx);
        if (ita != vCur.end()) {
            vCur.erase(ita);

            m_mAgentGroup.erase(iAgentIdx);
            iResult = 0;
        } else {
            xha_printf("[GroupManager::removeAgentFromGroup] COuldn't erase agent %d\n", iAgentIdx);
        }
    } else {
        xha_printf("[GroupManager::removeAgentFromGroup] agebt %d not found %d\n", iAgentIdx);
    }
    return iResult;


}


//----------------------------------------------------------------------------
// groupContainsAgent
//
bool GroupManager::groupContainsAgent(int iGroupID, int iAgentIdx) {
    bool bContained = false;
    if (groupExists(iGroupID)) {
        intvec &vCur = m_mGroupAgents[iGroupID];
        intvec::const_iterator it = std::find(vCur.begin(), vCur.end(), iAgentIdx);
        bContained = (it != vCur.end());
    } else {
        xha_printf("[GroupManager::groupContainsAgent] no group with id %d\n", iGroupID);
    }
    return bContained;
}


//----------------------------------------------------------------------------
// groupExists
//
bool GroupManager::groupExists(int iGroupID) {
    groupagents::const_iterator itg = m_mGroupAgents.find(iGroupID);
    return (itg != m_mGroupAgents.end());
}

    
//----------------------------------------------------------------------------
// splitGroup
//
int GroupManager::splitGroup(int iGroupID) {
    int iResult = -1;

    if (groupExists(iGroupID)) {
        intvec vSplitOff;
        iResult = m_pGroupSplitter->split(m_mGroupAgents[iGroupID], vSplitOff);
        if (iResult == 0) {
            int iSplitGroupID = getNextGroupID();
            
            for (uint i = 0; (iResult == 0) && (i < vSplitOff.size()); ++i) {
                iResult = addAgentToGroup(iSplitGroupID, vSplitOff[i]);
            }
        } else {
            xha_printf("[ GroupManager::splitGroup] Couldn't split group %d\n", iGroupID);
        }
    } else {
        xha_printf("[ GroupManager::splitGroup] no group with id %d\n", iGroupID);
    }
    return iResult;
}

