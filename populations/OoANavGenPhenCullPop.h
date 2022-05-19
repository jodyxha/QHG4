#ifndef __OOANAVGENPHENCULLPOP_H__
#define __OOANAVGENPHENCULLPOP_H__

#include "BitGeneUtils.h"
//#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarKCull.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
//@@@@@@@@@@@@@@@@@@@2#include "Phenetics.h"
#include "Phenetics2.h"
#include "Navigate.h"


struct OoANavGenPhenCullAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGenPhenCullPop : public SPopulation<OoANavGenPhenCullAgent> {
public:
    OoANavGenPhenCullPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGenPhenCullPop();

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
    WeightedMove<OoANavGenPhenCullAgent> *m_pWM;

    MultiEvaluator<OoANavGenPhenCullAgent> *m_pME;
    VerhulstVarKCull<OoANavGenPhenCullAgent> *m_pVerVarKCull;
    RandomPair<OoANavGenPhenCullAgent> *m_pPair;
    GetOld<OoANavGenPhenCullAgent> *m_pGO;
    OldAgeDeath<OoANavGenPhenCullAgent> *m_pOAD;
    Fertility<OoANavGenPhenCullAgent> *m_pFert;
    NPPCapacity<OoANavGenPhenCullAgent> *m_pNPPCap;
    Genetics<OoANavGenPhenCullAgent,BitGeneUtils> *m_pGenetics;
    //@@@@@@@@@@Phenetics<OoANavPhenAgent> *m_pPhenetics;
    Phenetics2<OoANavGenPhenCullAgent> *m_pPhenetics;
    Navigate<OoANavGenPhenCullAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

};


#endif
