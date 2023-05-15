#ifndef __NPERSOOANAVSHYBRELRPOP_H__
#define __NPERSOOANAVSHYBRELRPOP_H__

//#include "BitGeneUtils.h"
#include "WELL512.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
#include "NPersRandomMove.h"

#include "NPersHybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "PersOldAgeDeath.h"
#include "PersFertility.h"
#include "LocEnv.h"
#include "PrivParamMix.h"
#include "Navigate.h"


struct NPersOoANavSHybRelRAgent : Agent {

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
    // for NPP
    double  m_dCC;
};

class NPersOoANavSHybRelRPop : public SPopulation<NPersOoANavSHybRelRAgent> {
public:
    NPersOoANavSHybRelRPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~NPersOoANavSHybRelRPop();

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
    NPersRandomMove<NPersOoANavSHybRelRAgent> *m_pWM;
    LocEnv<NPersOoANavSHybRelRAgent> *m_pLE;
    PrivParamMix<NPersOoANavSHybRelRAgent> *m_pPPM;
    NPersHybBirthDeathRel<NPersOoANavSHybRelRAgent> *m_pHybBirthDeathRel;
    RandomPair<NPersOoANavSHybRelRAgent> *m_pPair;
    GetOld<NPersOoANavSHybRelRAgent> *m_pGO;
    PersOldAgeDeath<NPersOoANavSHybRelRAgent> *m_pOAD;
    PersFertility<NPersOoANavSHybRelRAgent> *m_pFert;
    Navigate<NPersOoANavSHybRelRAgent> *m_pNavigate;
    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    Geography *m_pGeography;
};


#endif
