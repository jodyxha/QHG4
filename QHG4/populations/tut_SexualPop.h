#ifndef __TUT_SEXUALPOP_H__
#define __TUT_SEXUALPOP_H__


#include "GetOld.h"
#include "OldAgeDeath.h"
#include "RandomMove.h"
#include "Fertility.h"
#include "Verhulst.h"

#include "RandomPair.h"

#include "SPopulation.h"

struct tut_SexualAgent : Agent {

    float m_fAge;

    float m_fLastBirth;
    int m_iMateIndex;
};


class tut_SexualPop : public  SPopulation<tut_SexualAgent> {
public:
    tut_SexualPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    virtual ~tut_SexualPop();

    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);

protected:
    GetOld<tut_SexualAgent>       *m_pGO;
    OldAgeDeath<tut_SexualAgent>  *m_pOAD;
    RandomMove<tut_SexualAgent>   *m_pRM;
    Fertility<tut_SexualAgent>    *m_pFert;
    Verhulst<tut_SexualAgent>     *m_pVerhulst;
  
    RandomPair<tut_SexualAgent>   *m_pPair;

};

#endif
