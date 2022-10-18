#ifndef __PERSOOANAVSHYBRELPOP_H__
#define __PERSOOANAVSHYBRELPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "PersWeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "PersHybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "PersOldAgeDeath.h"
#include "PersFertility.h"
#include "NPPCapacity.h"
#include "Navigate.h"
#include "PrivParamMix.h"

struct PersOoANavSHybRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
    float m_fHybridization;
    // for WeightedMove
    double  m_dMoveProb;
    //  for OldAgeDeath
    double  m_dMaxAge;
    double  m_dUncertainty;
    // for Fertility
    float   m_fFertilityMinAge;
    float   m_fFertilityMaxAge;
    float   m_fInterbirth;
    // for HybBiirthDeathRel
    double  m_dB0;
    double  m_dD0;
    double  m_dTheta;
    double  m_dBReal;

};

class PersOoANavSHybRelPop : public SPopulation<PersOoANavSHybRelAgent> {
public:
    PersOoANavSHybRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~PersOoANavSHybRelPop();

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
    PersWeightedMove<PersOoANavSHybRelAgent> *m_pWM;

    MultiEvaluator<PersOoANavSHybRelAgent> *m_pME;
    PersHybBirthDeathRel<PersOoANavSHybRelAgent> *m_pHybBirthDeathRel;
    RandomPair<PersOoANavSHybRelAgent> *m_pPair;
    GetOld<PersOoANavSHybRelAgent> *m_pGO;
    PersOldAgeDeath<PersOoANavSHybRelAgent> *m_pOAD;
    PersFertility<PersOoANavSHybRelAgent> *m_pFert;
    NPPCapacity<PersOoANavSHybRelAgent> *m_pNPPCap;
    Navigate<PersOoANavSHybRelAgent> *m_pNavigate;
    PrivParamMix<PersOoANavSHybRelAgent> *m_pPrivParamMix;

    double *m_adCapacities;
    double *m_adEnvWeights;

    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

};


#endif
