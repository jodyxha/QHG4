#ifndef __OOANAVGENPHENRELPOP_H__
#define __OOANAVGENPHENRELPOP_H__

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
#include "Phenetics2.h"
#include "Navigate.h"


struct OoANavGenPhenRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGenPhenRelPop : public SPopulation<OoANavGenPhenRelAgent> {
public:
    OoANavGenPhenRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGenPhenRelPop();

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
    WeightedMove<OoANavGenPhenRelAgent> *m_pWM;

    MultiEvaluator<OoANavGenPhenRelAgent> *m_pME;
    BirthDeathRel<OoANavGenPhenRelAgent> *m_pBirthDeathRel;
    RandomPair<OoANavGenPhenRelAgent> *m_pPair;
    GetOld<OoANavGenPhenRelAgent> *m_pGO;
    OldAgeDeath<OoANavGenPhenRelAgent> *m_pOAD;
    Fertility<OoANavGenPhenRelAgent> *m_pFert;
    NPPCapacity<OoANavGenPhenRelAgent> *m_pNPPCap;
    Genetics<OoANavGenPhenRelAgent,BitGeneUtils> *m_pGenetics;
    //@@@@@@@@@@Phenetics<OoANavPhenAgent> *m_pPhenetics;
    Phenetics2<OoANavGenPhenRelAgent> *m_pPhenetics;
    Navigate<OoANavGenPhenRelAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    Geography *m_pGeography;
};


#endif
