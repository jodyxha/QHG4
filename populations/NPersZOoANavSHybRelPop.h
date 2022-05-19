#ifndef __NPERSZOOANAVSHYBRELPOP_H__
#define __NPERSZOOANAVSHYBRELPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "NPersWeightedMove.h"
//#include "ConfinedMove.h"
#include "NPersZHybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "PersOldAgeDeath.h"
#include "PersFertility.h"
#include "LocEnv.h"
#include "PrivParamMix.h"
#include "Navigate.h"


struct NPersZOoANavSHybRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
    float m_fGenHybM;
    float m_fGenHybP;
    float m_fPhenHyb;
    // dummy
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
    // for NPP
    double  m_dCC;
};

class NPersZOoANavSHybRelPop : public SPopulation<NPersZOoANavSHybRelAgent> {
public:
    NPersZOoANavSHybRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~NPersZOoANavSHybRelPop();

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
    NPersWeightedMove<NPersZOoANavSHybRelAgent> *m_pWM;
    LocEnv<NPersZOoANavSHybRelAgent> *m_pLE;
    PrivParamMix<NPersZOoANavSHybRelAgent> *m_pPPM;
    NPersZHybBirthDeathRel<NPersZOoANavSHybRelAgent> *m_pHybBirthDeathRel;
    RandomPair<NPersZOoANavSHybRelAgent> *m_pPair;
    GetOld<NPersZOoANavSHybRelAgent> *m_pGO;
    PersOldAgeDeath<NPersZOoANavSHybRelAgent> *m_pOAD;
    PersFertility<NPersZOoANavSHybRelAgent> *m_pFert;
    Navigate<NPersZOoANavSHybRelAgent> *m_pNavigate;
    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    double m_dMaternalContribAvg;
    double m_dMaternalContribSDev;
    double m_dPaternalContribAvg;
    double m_dPaternalContribSDev;
};


#endif
