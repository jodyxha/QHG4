#ifndef __TUT_ENVIRONALT_H__
#define __TUT_ENVIRONALT_H__

#include "GetOld.h"
#include "ATanDeath.h"
#include "Fertility.h"
#include "Verhulst.h"
#include "RandomPair.h"

#include "SingleEvaluator.h"
#include "WeightedMove.h"

#include "SPopulation.h"
#include "Geography.h"

struct tut_EnvironAltAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int   m_iMateIndex;
};

class tut_EnvironAltPop : public SPopulation<tut_EnvironAltAgent> {
public:
    tut_EnvironAltPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~tut_EnvironAltPop();

  
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

 protected:
    GetOld<tut_EnvironAltAgent>          *m_pGO;
    ATanDeath<tut_EnvironAltAgent>       *m_pAD;
    Fertility<tut_EnvironAltAgent>       *m_pFert;
    Verhulst<tut_EnvironAltAgent>        *m_pVerhulst;
    RandomPair<tut_EnvironAltAgent>      *m_pPair;
     
    WeightedMove<tut_EnvironAltAgent>    *m_pWM;
    SingleEvaluator<tut_EnvironAltAgent> *m_pSEAlt;
  
    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bUpdateNeeded;

    Geography *m_pGeography;
};


#endif
