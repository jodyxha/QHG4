#ifndef __OOANAVSHYBRELPOP_H__
#define __OOANAVSHYBRELPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
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
#include "Navigate.h"


struct OoANavSHybRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
    float m_fHybridization;
    
};

class OoANavSHybRelPop : public SPopulation<OoANavSHybRelAgent> {
public:
    OoANavSHybRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavSHybRelPop();

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
    WeightedMove<OoANavSHybRelAgent> *m_pWM;

    MultiEvaluator<OoANavSHybRelAgent> *m_pME;
    HybBirthDeathRel<OoANavSHybRelAgent> *m_pHybBirthDeathRel;
    RandomPair<OoANavSHybRelAgent> *m_pPair;
    GetOld<OoANavSHybRelAgent> *m_pGO;
    OldAgeDeath<OoANavSHybRelAgent> *m_pOAD;
    Fertility<OoANavSHybRelAgent> *m_pFert;
    NPPCapacity<OoANavSHybRelAgent> *m_pNPPCap;
    Navigate<OoANavSHybRelAgent> *m_pNavigate;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    Geography *m_pGeography;
};


#endif
