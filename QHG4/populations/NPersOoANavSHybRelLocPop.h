#ifndef __NPERSOOANAVSHYBRELLOCPOP_H__
#define __NPERSOOANAVSHYBRELLOCPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "SPopulation.h"
#include "NPersWeightedMove.h"
//#include "ConfinedMove.h"
#include "NPersHybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "PersOldAgeDeath.h"
#include "PersFertility.h"
#include "LocEnv.h"
#include "PrivParamMix.h"
#include "Navigate.h"


struct NPersOoANavSHybRelLocAgent : Agent {

    float m_fAge;
    int  m_iBirthCell;
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

class NPersOoANavSHybRelLocPop : public SPopulation<NPersOoANavSHybRelLocAgent> {
public:
    NPersOoANavSHybRelLocPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~NPersOoANavSHybRelLocPop();

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
    NPersWeightedMove<NPersOoANavSHybRelLocAgent> *m_pWM;
    LocEnv<NPersOoANavSHybRelLocAgent> *m_pLE;
    PrivParamMix<NPersOoANavSHybRelLocAgent> *m_pPPM;
    NPersHybBirthDeathRel<NPersOoANavSHybRelLocAgent> *m_pHybBirthDeathRel;
    RandomPair<NPersOoANavSHybRelLocAgent> *m_pPair;
    GetOld<NPersOoANavSHybRelLocAgent> *m_pGO;
    PersOldAgeDeath<NPersOoANavSHybRelLocAgent> *m_pOAD;
    PersFertility<NPersOoANavSHybRelLocAgent> *m_pFert;
    Navigate<NPersOoANavSHybRelLocAgent> *m_pNavigate;
    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

};


#endif
