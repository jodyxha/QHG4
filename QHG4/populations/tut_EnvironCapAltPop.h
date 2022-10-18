#ifndef __TUT_ENVIROMCAPALTPOP_H__
#define __TUT_ENVIRONCAPALTPOP_H__

#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "SingleEvaluator.h"
#include "WeightedMove.h"

#include "MultiEvaluator.h"
#include "NPPCapacity.h"

#include "SPopulation.h"

struct tut_EnvironCapAltAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int   m_iMateIndex;
};

class tut_EnvironCapAltPop : public SPopulation<tut_EnvironCapAltAgent> {
public:
    tut_EnvironCapAltPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~tut_EnvironCapAltPop();

  
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);

protected:
    WeightedMove<tut_EnvironCapAltAgent> *m_pWM;
    MultiEvaluator<tut_EnvironCapAltAgent> *m_pME;
    VerhulstVarK<tut_EnvironCapAltAgent> *m_pVerVarK;
    RandomPair<tut_EnvironCapAltAgent> *m_pPair;
    GetOld<tut_EnvironCapAltAgent> *m_pGO;
    OldAgeDeath<tut_EnvironCapAltAgent> *m_pOAD;
    Fertility<tut_EnvironCapAltAgent> *m_pFert;
    NPPCapacity<tut_EnvironCapAltAgent> *m_pNPPCap;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bUpdateNeeded;
};

#endif
