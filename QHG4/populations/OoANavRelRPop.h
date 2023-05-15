#ifndef __OOANAVRELRPOP_H__
#define __OOANAVRELRPOP_H__

#include "BitGeneUtils.h"
//#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
#include "RandomMove.h"
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


struct OoANavRelRAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavRelRPop : public SPopulation<OoANavRelRAgent> {
public:
    OoANavRelRPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavRelRPop();

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
    RandomMove<OoANavRelRAgent> *m_pRM;

    MultiEvaluator<OoANavRelRAgent> *m_pME;
    BirthDeathRel<OoANavRelRAgent> *m_pBirthDeathRel;
    RandomPair<OoANavRelRAgent> *m_pPair;
    GetOld<OoANavRelRAgent> *m_pGO;
    OldAgeDeath<OoANavRelRAgent> *m_pOAD;
    Fertility<OoANavRelRAgent> *m_pFert;
    NPPCapacity<OoANavRelRAgent> *m_pNPPCap;
    Navigate<OoANavRelRAgent> *m_pNavigate;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    Geography *m_pGeography;
};


#endif
