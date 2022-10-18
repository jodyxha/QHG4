#ifndef __NEWMOVESTATTESTPOP_H__
#define __NEWMOVESTATTESTPOP_H__

#include "BitGeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
#include "Navigate.h"
#include "MoveStats.h"


struct NewMoveStatTestAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class NewMoveStatTestPop : public SPopulation<NewMoveStatTestAgent> {
public:
    NewMoveStatTestPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~NewMoveStatTestPop();

    virtual int setParams(const std::string sParams);
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);
 
 
 protected:
    WeightedMove<NewMoveStatTestAgent> *m_pWM;
    MultiEvaluator<NewMoveStatTestAgent> *m_pME;
    VerhulstVarK<NewMoveStatTestAgent> *m_pVerVarK;
    RandomPair<NewMoveStatTestAgent> *m_pPair;
    GetOld<NewMoveStatTestAgent> *m_pGO;
    OldAgeDeath<NewMoveStatTestAgent> *m_pOAD;
    Fertility<NewMoveStatTestAgent> *m_pFert;
    NPPCapacity<NewMoveStatTestAgent> *m_pNPPCap;
    Navigate<NewMoveStatTestAgent> *m_pNavigate;
    MoveStats<NewMoveStatTestAgent> *m_pMoveStats;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

};


#endif
