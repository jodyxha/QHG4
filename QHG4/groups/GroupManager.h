#ifndef __GROUPMANAGER_H__
#define __GROUPMANAGER_H__

#include <vector>
#include <map>

#include "types.h"
#include "GroupSplitter.h"
class GroupPop;

typedef std::map<int, intvec>  groupagents;
typedef std::map<int, int>  agentgroup;

class GroupManager {
public:
    static GroupManager *createInstance(GroupPop *pPop, GroupSplitter *pGroupSplitter);
    virtual ~GroupManager();

    int getNextGroupID();

    int addAgentToGroup(int iGroupID, int iAgentIdx);
    int removeAgentFromGroup(int iAgentIdx);
  
    bool groupContainsAgent(int iGroupID, int iAgentIdx);
    bool groupExists(int iGroupID);
    
    int splitGroup(int iGroupID);
protected:
    GroupManager(GroupPop *pPop, GroupSplitter *pGroupSplitter);
    

    int            m_iNextGroupID;
    groupagents    m_mGroupAgents;  
    agentgroup     m_mAgentGroup;
    GroupPop      *m_pPop;
    GroupSplitter *m_pGroupSplitter;

};

#endif
