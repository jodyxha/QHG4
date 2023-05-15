#ifndef __NPERSAOOANAVSHYBRELPOP_H__
#define __NPERSAOOANAVSHYBRELPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "Geography.h"
#include "NPersWeightedMove.h"
//#include "ConfinedMove.h"
#include "NPersXHybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "PersOldAgeDeath.h"
#include "PersFertility.h"
#include "LocEnv.h"
#include "PrivParamMix.h"
#include "Navigate.h"


struct NPersXOoANavSHybRelAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
    float m_fNeaFrac;
    float m_fNeaSDev;
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

class NPersXOoANavSHybRelPop : public SPopulation<NPersXOoANavSHybRelAgent> {
public:
    NPersXOoANavSHybRelPop(SCellGrid *pCG, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~NPersXOoANavSHybRelPop();

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
    NPersWeightedMove<NPersXOoANavSHybRelAgent> *m_pWM;
    LocEnv<NPersXOoANavSHybRelAgent> *m_pLE;
    PrivParamMix<NPersXOoANavSHybRelAgent> *m_pPPM;
    NPersXHybBirthDeathRel<NPersXOoANavSHybRelAgent> *m_pHybBirthDeathRel;
    RandomPair<NPersXOoANavSHybRelAgent> *m_pPair;
    GetOld<NPersXOoANavSHybRelAgent> *m_pGO;
    PersOldAgeDeath<NPersXOoANavSHybRelAgent> *m_pOAD;
    PersFertility<NPersXOoANavSHybRelAgent> *m_pFert;
    Navigate<NPersXOoANavSHybRelAgent> *m_pNavigate;
    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    double m_dSDev;

    Geography *m_pGeography;
};


#endif
