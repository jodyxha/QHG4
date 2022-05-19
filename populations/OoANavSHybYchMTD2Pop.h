#ifndef __OOANAVSHYBYCHMTD2POP_H__
#define __OOANAVSHYBYCHMTD2POP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "NPersWeightedMove2.h"
//#include "ConfinedMove.h"
#include "NPersZHybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "PersOldAgeDeath.h"
#include "PersFertility.h"
#include "LocEnv2.h"
#include "PrivParamMix2.h"
#include "Navigate.h"


struct OoANavSHybYchMTD2Agent : Agent {

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
    float m_fParentalHybridization; 
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

class OoANavSHybYchMTD2Pop : public SPopulation<OoANavSHybYchMTD2Agent> {
public:
    OoANavSHybYchMTD2Pop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavSHybYchMTD2Pop();

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
    NPersWeightedMove2<OoANavSHybYchMTD2Agent> *m_pWM;
    LocEnv2<OoANavSHybYchMTD2Agent> *m_pLE;
    PrivParamMix2<OoANavSHybYchMTD2Agent> *m_pPPM;
    NPersZHybBirthDeathRel<OoANavSHybYchMTD2Agent> *m_pHybBirthDeathRel;
    RandomPair<OoANavSHybYchMTD2Agent> *m_pPair;
    GetOld<OoANavSHybYchMTD2Agent> *m_pGO;
    PersOldAgeDeath<OoANavSHybYchMTD2Agent> *m_pOAD;
    PersFertility<OoANavSHybYchMTD2Agent> *m_pFert;
    Navigate<OoANavSHybYchMTD2Agent> *m_pNavigate;
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
