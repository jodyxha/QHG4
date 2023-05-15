#ifndef __OOANAVGENPHENRELCOMPPOP_H__
#define __OOANAVGENPHENRELCompPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "BirthDeathRelComp.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
#include "Navigate.h"


struct OoANavGenRelCompAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGenRelCompPop : public SPopulation<OoANavGenRelCompAgent> {
public:
    OoANavGenRelCompPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGenRelCompPop();

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
    WeightedMove<OoANavGenRelCompAgent> *m_pWM;

    MultiEvaluator<OoANavGenRelCompAgent> *m_pME;
    BirthDeathRelComp<OoANavGenRelCompAgent> *m_pBirthDeathRelComp;
    RandomPair<OoANavGenRelCompAgent> *m_pPair;
    GetOld<OoANavGenRelCompAgent> *m_pGO;
    OldAgeDeath<OoANavGenRelCompAgent> *m_pOAD;
    Fertility<OoANavGenRelCompAgent> *m_pFert;
    NPPCapacity<OoANavGenRelCompAgent> *m_pNPPCap;
    Genetics<OoANavGenRelCompAgent,GeneUtils> *m_pGenetics;
    Navigate<OoANavGenRelCompAgent> *m_pNavigate;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    Geography *m_pGeography;
};


#endif
