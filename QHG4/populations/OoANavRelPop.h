#ifndef __OOANAVRELPOP_H__
#define __OOANAVRELPOP_H__

#include "BitGeneUtils.h"
//#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "BirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
//@@@@@@@@@@@@@@@@@@@2#include "Phenetics.h"
#include "Phenetics2.h"
#include "Navigate.h"


struct OoANavRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavRelPop : public SPopulation<OoANavRelAgent> {
public:
    OoANavRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavRelPop();

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
    WeightedMove<OoANavRelAgent> *m_pWM;

    MultiEvaluator<OoANavRelAgent> *m_pME;
    BirthDeathRel<OoANavRelAgent> *m_pBirthDeathRel;
    RandomPair<OoANavRelAgent> *m_pPair;
    GetOld<OoANavRelAgent> *m_pGO;
    OldAgeDeath<OoANavRelAgent> *m_pOAD;
    Fertility<OoANavRelAgent> *m_pFert;
    NPPCapacity<OoANavRelAgent> *m_pNPPCap;
    Navigate<OoANavRelAgent> *m_pNavigate;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    Geography *m_pGeography;
};


#endif
