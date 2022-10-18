#ifndef __OOANAVRELCOMPPOP_H__
#define __OOANAVRELCOMPPOP_H__

#include "BitGeneUtils.h"
//#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "BirthDeathRelComp.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
//@@@@@@@@@@@@@@@@@@@2#include "Phenetics.h"
#include "Phenetics2.h"
#include "Navigate.h"


struct OoANavRelCompAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavRelCompPop : public SPopulation<OoANavRelCompAgent> {
public:
    OoANavRelCompPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavRelCompPop();

    virtual int preLoop();

    virtual int setParams(const std::string sParams);
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

    virtual int initializeStep(float fTime);
    virtual int finalizeStep();
 protected:
    WeightedMove<OoANavRelCompAgent> *m_pWM;

    MultiEvaluator<OoANavRelCompAgent> *m_pME;
    BirthDeathRelComp<OoANavRelCompAgent> *m_pBirthDeathRelComp;
    RandomPair<OoANavRelCompAgent> *m_pPair;
    GetOld<OoANavRelCompAgent> *m_pGO;
    OldAgeDeath<OoANavRelCompAgent> *m_pOAD;
    Fertility<OoANavRelCompAgent> *m_pFert;
    NPPCapacity<OoANavRelCompAgent> *m_pNPPCap;
    Navigate<OoANavRelCompAgent> *m_pNavigate;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

};


#endif
