#ifndef __NPERSZOOANAVSHYBRELMCPOP_H__
#define __NPERSZOOANAVSHYBRELMCPOP_H__

//#include "BitGeneUtils.h"
#include "GeneUtils.h"
#include "Vec3D.h"
#include "SPopulation.h"
#include "Geography.h"
#include "NPersWeightedMove.h"
#include "NPersZHybBirthDeathRel.h"
#include "RandomPair.h"
#include "GetOld.h"
#include "PersOldAgeDeath.h"
#include "PersFertility.h"
#include "LocEnv.h"
#include "PrivParamMix.h"
#include "Navigate.h"

typedef std::pair<int,int> cellpair;
typedef std::map<cellpair,int> migcounts;

struct NPersZOoANavSHybRelMCAgent : Agent {

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

class NPersZOoANavSHybRelMCPop : public SPopulation<NPersZOoANavSHybRelMCAgent> {
public:
    NPersZOoANavSHybRelMCPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~NPersZOoANavSHybRelMCPop();

    virtual int preLoop();

    virtual int setParams(const std::string sParams);
    virtual int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    virtual int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    virtual void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

    virtual int initializeStep(float fTime);
    virtual int finalizeStep();

    // MC special
    virtual int performMoves();
    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int condenseCounts();
 protected:
    NPersWeightedMove<NPersZOoANavSHybRelMCAgent> *m_pWM;
    LocEnv<NPersZOoANavSHybRelMCAgent> *m_pLE;
    PrivParamMix<NPersZOoANavSHybRelMCAgent> *m_pPPM;
    NPersZHybBirthDeathRel<NPersZOoANavSHybRelMCAgent> *m_pHybBirthDeathRel;
    RandomPair<NPersZOoANavSHybRelMCAgent> *m_pPair;
    GetOld<NPersZOoANavSHybRelMCAgent> *m_pGO;
    PersOldAgeDeath<NPersZOoANavSHybRelMCAgent> *m_pOAD;
    PersFertility<NPersZOoANavSHybRelMCAgent> *m_pFert;
    Navigate<NPersZOoANavSHybRelMCAgent> *m_pNavigate;
    bool m_bCreateGenomes;
    bool m_bPendingEvents;

    int    **m_aiNumBirths;
    double  *m_adBirthRates;

    double m_dMaternalContribAvg;
    double m_dMaternalContribSDev;
    double m_dPaternalContribAvg;
    double m_dPaternalContribSDev;

    migcounts m_mvMigCounts;
    std::map<int, Vec3D> m_mVectors;
    std::map<int, Vec3D> m_mDiffs;

    Geography *m_pGeography;
};


#endif
