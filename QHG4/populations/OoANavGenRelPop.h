#ifndef __OOANAVGENRELPOP_H__
#define __OOANAVGENRELPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
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
#include "Navigate.h"


struct OoANavGenRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGenRelPop : public SPopulation<OoANavGenRelAgent> {
public:
    OoANavGenRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGenRelPop();

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
    WeightedMove<OoANavGenRelAgent> *m_pWM;

    MultiEvaluator<OoANavGenRelAgent> *m_pME;
    BirthDeathRel<OoANavGenRelAgent> *m_pBirthDeathRel;
    RandomPair<OoANavGenRelAgent> *m_pPair;
    GetOld<OoANavGenRelAgent> *m_pGO;
    OldAgeDeath<OoANavGenRelAgent> *m_pOAD;
    Fertility<OoANavGenRelAgent> *m_pFert;
    NPPCapacity<OoANavGenRelAgent> *m_pNPPCap;
    Genetics<OoANavGenRelAgent,GeneUtils> *m_pGenetics;
    Navigate<OoANavGenRelAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

};


#endif
