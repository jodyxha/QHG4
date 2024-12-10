#ifndef __ENVNAVPOP_H__
#define __ENVNAVPOP_H__

#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "SingleEvaluator.h"
#include "WeightedMove.h"

#include "MultiEvaluator.h"
#include "NPPCapacity.h"
#include "Navigate.h"

struct EnvNavAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int   m_iMateIndex;
};

class EnvNavPop : public SPopulation<EnvNavAgent> {
public:
    EnvNavPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~EnvNavPop();


    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);

protected:
    WeightedMove<EnvNavAgent> *m_pWM;
    MultiEvaluator<EnvNavAgent> *m_pME;
    VerhulstVarK<EnvNavAgent> *m_pVerVarK;
    RandomPair<EnvNavAgent> *m_pPair;
    GetOld<EnvNavAgent> *m_pGO;
    OldAgeDeath<EnvNavAgent> *m_pOAD;
    Fertility<EnvNavAgent> *m_pFert;
    NPPCapacity<EnvNavAgent> *m_pNPPCap;
    Navigate<EnvNavAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bUpdateNeeded;
};

#endif
