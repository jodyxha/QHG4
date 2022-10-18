#ifndef __OOAPOP_H__
#define __OOAPOP_H__

#include "BitGeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"


struct OoAAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoAPop : public SPopulation<OoAAgent> {
public:
    OoAPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoAPop();

    int preLoop();
    int setParams(const std::string sParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, float fT);
    virtual void flushEvents(float fT);

 protected:
    WeightedMove<OoAAgent> *m_pWM;
    //    ConfinedMove<OoAAgent> *m_pCM;
    MultiEvaluator<OoAAgent> *m_pME;
    VerhulstVarK<OoAAgent> *m_pVerVarK;
    RandomPair<OoAAgent> *m_pPair;
    GetOld<OoAAgent> *m_pGO;
    OldAgeDeath<OoAAgent> *m_pOAD;
    Fertility<OoAAgent> *m_pFert;
    NPPCapacity<OoAAgent> *m_pNPPCap;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bPendingEvents;

    bool m_bUpdateNeeded;
};


#endif
