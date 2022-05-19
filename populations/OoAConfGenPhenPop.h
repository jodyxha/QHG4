#ifndef __OOACONFGENPHENPOP_H__
#define __OOACONFGENPHENPOP_H__

#include "BitGeneUtils.h"
//#include "GeneUtils.h"
#include "SPopulation.h"
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
#include "Genetics.h"
//@@@@@@@@@@@@@@@@@@@2#include "Phenetics.h"
#include "Phenetics2.h"
#include "Navigate.h"



struct OoAConfGenPhenAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoAConfGenPhenPop : public SPopulation<OoAConfGenPhenAgent> {
public:
    OoAConfGenPhenPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoAConfGenPhenPop();

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
    WeightedMove<OoAConfGenPhenAgent> *m_pWM;

    MultiEvaluator<OoAConfGenPhenAgent> *m_pME;
    VerhulstVarK<OoAConfGenPhenAgent> *m_pVerVarK;
    RandomPair<OoAConfGenPhenAgent> *m_pPair;
    GetOld<OoAConfGenPhenAgent> *m_pGO;
    OldAgeDeath<OoAConfGenPhenAgent> *m_pOAD;
    Fertility<OoAConfGenPhenAgent> *m_pFert;
    NPPCapacity<OoAConfGenPhenAgent> *m_pNPPCap;
    Genetics<OoAConfGenPhenAgent,BitGeneUtils> *m_pGenetics;
    //@@@@@@@@@@Phenetics<OoAConfPhenAgent> *m_pPhenetics;
    Phenetics2<OoAConfGenPhenAgent> *m_pPhenetics;
    Navigate<OoAConfGenPhenAgent> *m_pNavigate;
    ConfinedMove<OoAConfGenPhenAgent> *m_pCM;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

};


#endif
