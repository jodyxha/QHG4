#ifndef __OOACONFPHENPOP_H__
#define __OOACONFPHENPOP_H__

#include "SPopulation.h"
#include "Geography.h"
#include "WeightedMove.h"
#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
//@@@@@@@@@@@@@@@@@@@2#include "Phenetics.h"
#include "Phenetics2.h"
#include "Navigate.h"



struct OoAConfPhenAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoAConfPhenPop : public SPopulation<OoAConfPhenAgent> {
public:
    OoAConfPhenPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoAConfPhenPop();

    virtual int preLoop();

    virtual int setParams(const std::string sParams);
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int dumpAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int restoreAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

    virtual int copyAdditionalDataQDF(int iStart, int iCount, SPopulation *pPop);

    virtual int initializeStep(float fTime);
 protected:
    WeightedMove<OoAConfPhenAgent> *m_pWM;

    MultiEvaluator<OoAConfPhenAgent> *m_pME;
    VerhulstVarK<OoAConfPhenAgent> *m_pVerVarK;
    RandomPair<OoAConfPhenAgent> *m_pPair;
    GetOld<OoAConfPhenAgent> *m_pGO;
    OldAgeDeath<OoAConfPhenAgent> *m_pOAD;
    Fertility<OoAConfPhenAgent> *m_pFert;
    NPPCapacity<OoAConfPhenAgent> *m_pNPPCap;
    //@@@@@@@@@@Phenetics<OoAConfPhenAgent> *m_pPhenetics;
    Phenetics2<OoAConfPhenAgent> *m_pPhenetics;
    Navigate<OoAConfPhenAgent> *m_pNavigate;
    ConfinedMove<OoAConfPhenAgent> *m_pCM;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    Geography *m_pGeography;
};


#endif
