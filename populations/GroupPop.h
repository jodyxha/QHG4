#ifndef __GROUPPOP_H__
#define __GROUPPOP_H__

#include "SPopulation.h"
#include "GroupMemberMove.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "FoodManager.h"
#include "AgentEnv.h"


#include "ChildManager.h"
#include "GroupInterface.h"
class GroupManager;


struct GroupAgent : public Agent {

    float  m_fAge;
    float  m_fLastBirth;
    int    m_iMateIndex;

    int    m_iGroupID;
    int    m_iHomeID;
    idtype m_iParentIdx;
    float  m_fHealth;
};


class GroupPop : public SPopulation<GroupAgent>, GroupInterface {
public:
    GroupPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    virtual ~GroupPop();

    virtual int setParams(const std::string sParams);

    virtual int initializeStep(float fTime);

    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    //    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    //    virtual int raedAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual void registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo);
    virtual int makePopSpecificMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo);
    virtual int makePopSpecificDeath(int iAgentIndex);

    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

    virtual int setGroupID(int iGroupID, idtype iAgentIdx);

    virtual int setParent(int iChildIdx, int iParentIdx);
protected:
    
    GroupMemberMove<GroupAgent> *m_pGMM;
    RandomPair<GroupAgent>      *m_pPair;
    GetOld<GroupAgent>          *m_pGO;
    OldAgeDeath<GroupAgent>     *m_pOAD;
    Fertility<GroupAgent>       *m_pFert;
    AgentEnv<GroupAgent>        *m_pAE;
    FoodManager<GroupAgent>     *m_pFM;

    float  *m_pFoodAvailable;
    int    *m_pAgentCounts;
    
    ChildManager<GroupAgent>     *m_pCM;
    GroupManager     *m_pGM;
  
    bool m_bPendingEvents;
  
};

#endif
