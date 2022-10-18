#ifndef __OOANAVHYBRELPOP_H__
#define __OOANAVHYBRELPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "HybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
#include "Hybrids.h"
#include "Navigate.h"


struct OoANavHybRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
    float m_fHybridization;
    
};

class OoANavHybRelPop : public SPopulation<OoANavHybRelAgent> {
public:
    OoANavHybRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavHybRelPop();

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
    virtual int finalizeStep();
 protected:
    WeightedMove<OoANavHybRelAgent> *m_pWM;

    MultiEvaluator<OoANavHybRelAgent> *m_pME;
    HybBirthDeathRel<OoANavHybRelAgent> *m_pHybBirthDeathRel;
    RandomPair<OoANavHybRelAgent> *m_pPair;
    GetOld<OoANavHybRelAgent> *m_pGO;
    OldAgeDeath<OoANavHybRelAgent> *m_pOAD;
    Fertility<OoANavHybRelAgent> *m_pFert;
    NPPCapacity<OoANavHybRelAgent> *m_pNPPCap;
    Hybrids<OoANavHybRelAgent,GeneUtils> *m_pHybrids;
    Navigate<OoANavHybRelAgent> *m_pNavigate;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

};


#endif
