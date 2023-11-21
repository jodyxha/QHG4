#ifndef __OOANAVSHYBYCHMTDPOP_H__
#define __OOANAVSHYBYCHMTDPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
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
#include "MoveStats.h"

struct OoANavSHybYchMTDAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
    float m_fGenHybM;
    float m_fGenHybP;
    float m_fPhenHyb;
    // dummy
    float m_fHybridization;
    uchar m_iYchr;    // 0: sapiens, 1: neander
    uchar m_imtDNA;   // 0: sapiens, 1: neander
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

class OoANavSHybYchMTDPop : public SPopulation<OoANavSHybYchMTDAgent> {
public:
    OoANavSHybYchMTDPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavSHybYchMTDPop();

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
    NPersWeightedMove<OoANavSHybYchMTDAgent> *m_pWM;
    LocEnv<OoANavSHybYchMTDAgent> *m_pLE;
    PrivParamMix<OoANavSHybYchMTDAgent> *m_pPPM;
    NPersZHybBirthDeathRel<OoANavSHybYchMTDAgent> *m_pHybBirthDeathRel;
    RandomPair<OoANavSHybYchMTDAgent> *m_pPair;
    GetOld<OoANavSHybYchMTDAgent> *m_pGO;
    PersOldAgeDeath<OoANavSHybYchMTDAgent> *m_pOAD;
    PersFertility<OoANavSHybYchMTDAgent> *m_pFert;
    Navigate<OoANavSHybYchMTDAgent> *m_pNavigate;
    MoveStats<OoANavSHybYchMTDAgent> *m_pMoveStats;
    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    double m_dMaternalContribAvg;
    double m_dMaternalContribSDev;
    double m_dPaternalContribAvg;
    double m_dPaternalContribSDev;
    Geography *m_pGeography;
};


#endif
