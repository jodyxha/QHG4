#ifndef __GROUPMEMBERMOVE_H__
#define __GROUPMEMBERMOVE_H__

#include "Action.h"
#include "AgentEnv.h"
#include "ParamProvider2.h"

#define ATTR_GROUPMEMBERMOVE_NAME "GroupMemberMove"
#define ATTR_GROUPMEMBERMOVE_PROB_NAME "GroupMemberMove_prob"

template<typename T>
class GroupMemberMove : public Action<T> {
    
 public:
    GroupMemberMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, AgentEnv<T> *pAE);
    ~GroupMemberMove();
    int operator()(int iA, float fT);

    virtual int postLoop();


    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  

       bool isEqual(Action<T> *pAction, bool bStrict);
    
 protected:
    WELL512      **m_apWELL;
    AgentEnv<T>   *m_pAE;
    double m_dMoveProb;
    
};

#endif
