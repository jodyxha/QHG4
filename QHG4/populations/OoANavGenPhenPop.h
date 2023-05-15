#ifndef __OOANAVGENPHENPOP_H__
#define __OOANAVGENPhenPOP_H__

#include "BitGeneUtils.h"
//#include "GeneUtils.h"
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
#include "Genetics.h"
//@@@@@@@@@@@@@@@@@@@2#include "Phenetics.h"
#include "Phenetics2.h"
#include "Navigate.h"


struct OoANavGenPhenAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGenPhenPop : public SPopulation<OoANavGenPhenAgent> {
public:
    OoANavGenPhenPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGenPhenPop();

    virtual int preLoop();

    virtual int setParams(const std::string sParams);
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int dumpAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int restoreAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int copyAdditionalDataQDF(int iStart, int iCount, SPopulation *pPop);

    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

    virtual int initializeStep(float fTime);
 protected:
    WeightedMove<OoANavGenPhenAgent> *m_pWM;

    MultiEvaluator<OoANavGenPhenAgent> *m_pME;
    VerhulstVarK<OoANavGenPhenAgent> *m_pVerVarK;
    RandomPair<OoANavGenPhenAgent> *m_pPair;
    GetOld<OoANavGenPhenAgent> *m_pGO;
    OldAgeDeath<OoANavGenPhenAgent> *m_pOAD;
    Fertility<OoANavGenPhenAgent> *m_pFert;
    NPPCapacity<OoANavGenPhenAgent> *m_pNPPCap;
    Genetics<OoANavGenPhenAgent,BitGeneUtils> *m_pGenetics;
    //@@@@@@@@@@Phenetics<OoANavPhenAgent> *m_pPhenetics;
    Phenetics2<OoANavGenPhenAgent> *m_pPhenetics;
    Navigate<OoANavGenPhenAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    Geography *m_pGeography;
};


#endif
