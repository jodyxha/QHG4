#ifndef __OOANAVPHENPOP_H__
#define __OOANAVPhenPOP_H__

#include "SPopulation.h"
#include "Geography.h"
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
//@@@@@@@@@@@@@@@@@@@2#include "Phenetics.h"
#include "Phenetics2.h"
#include "Navigate.h"


struct OoANavPhenAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavPhenPop : public SPopulation<OoANavPhenAgent> {
public:
    OoANavPhenPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavPhenPop();

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
    WeightedMove<OoANavPhenAgent> *m_pWM;

    MultiEvaluator<OoANavPhenAgent> *m_pME;
    VerhulstVarK<OoANavPhenAgent> *m_pVerVarK;
    RandomPair<OoANavPhenAgent> *m_pPair;
    GetOld<OoANavPhenAgent> *m_pGO;
    OldAgeDeath<OoANavPhenAgent> *m_pOAD;
    Fertility<OoANavPhenAgent> *m_pFert;
    NPPCapacity<OoANavPhenAgent> *m_pNPPCap;
    //@@@@@@@@@@Phenetics<OoANavPhenAgent> *m_pPhenetics;
    Phenetics2<OoANavPhenAgent> *m_pPhenetics;
    Navigate<OoANavPhenAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    Geography *m_pGeography;
};


#endif
