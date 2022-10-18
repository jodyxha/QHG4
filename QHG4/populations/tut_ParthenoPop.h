#ifndef __TUT_PARTHENOPOP_H__
#define __TUT_PARTHENOPOP_H__


#include "GetOld.h"
#include "OldAgeDeath.h"
#include "RandomMove.h"

#include "Fertility.h"
#include "Verhulst.h"

#include "SPopulation.h"

struct tut_ParthenoAgent : Agent {

    float m_fAge;

    float m_fLastBirth;
    int m_iMateIndex;
};


class tut_ParthenoPop : public  SPopulation<tut_ParthenoAgent> {
public:
    tut_ParthenoPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    virtual ~tut_ParthenoPop();

    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);

protected:
    GetOld<tut_ParthenoAgent>       *m_pGO;
    OldAgeDeath<tut_ParthenoAgent>  *m_pOAD;
    RandomMove<tut_ParthenoAgent>   *m_pRM;

    Verhulst<tut_ParthenoAgent>     *m_pVerhulst;
    Fertility<tut_ParthenoAgent>    *m_pFert;

};

#endif
