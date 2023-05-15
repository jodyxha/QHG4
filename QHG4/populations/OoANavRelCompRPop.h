#ifndef __OOANAVRELCOMPRPOP_H__
#define __OOANAVRELCOMPRPOP_H__

#include "BitGeneUtils.h"
//#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
#include "RandomMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "BirthDeathRelComp.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Navigate.h"


struct OoANavRelCompRAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavRelCompRPop : public SPopulation<OoANavRelCompRAgent> {
public:
    OoANavRelCompRPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavRelCompRPop();

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
    RandomMove<OoANavRelCompRAgent> *m_pRM;

    MultiEvaluator<OoANavRelCompRAgent> *m_pME;
    BirthDeathRelComp<OoANavRelCompRAgent> *m_pBirthDeathRelComp;
    RandomPair<OoANavRelCompRAgent> *m_pPair;
    GetOld<OoANavRelCompRAgent> *m_pGO;
    OldAgeDeath<OoANavRelCompRAgent> *m_pOAD;
    Fertility<OoANavRelCompRAgent> *m_pFert;
    NPPCapacity<OoANavRelCompRAgent> *m_pNPPCap;
    Navigate<OoANavRelCompRAgent> *m_pNavigate;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    Geography *m_pGeography;
};


#endif
