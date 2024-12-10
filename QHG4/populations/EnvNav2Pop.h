#ifndef __ENVNAV2POP_H__
#define __ENVNAV2POP_H__

#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "SingleEvaluator.h"
#include "WeightedMove.h"

#include "MultiEvaluator.h"
#include "NPPCapacity.h"
#include "Navigate2.h"

struct EnvNav2Agent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int   m_iMateIndex;
};

class EnvNav2Pop : public SPopulation<EnvNav2Agent> {
public:
    EnvNav2Pop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~EnvNav2Pop();


    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);

protected:
    WeightedMove<EnvNav2Agent> *m_pWM;
    MultiEvaluator<EnvNav2Agent> *m_pME;
    VerhulstVarK<EnvNav2Agent> *m_pVerVarK;
    RandomPair<EnvNav2Agent> *m_pPair;
    GetOld<EnvNav2Agent> *m_pGO;
    OldAgeDeath<EnvNav2Agent> *m_pOAD;
    Fertility<EnvNav2Agent> *m_pFert;
    NPPCapacity<EnvNav2Agent> *m_pNPPCap;
    Navigate2<EnvNav2Agent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bUpdateNeeded;
};

#endif
