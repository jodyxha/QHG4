#ifndef __TUT_MOVEPOP_H__
#define __TUT_MOVEPOP_H__


#include "OldAgeDeath.h"
#include "GetOld.h"

#include "RandomMove.h"

#include "SPopulation.h"


// our agents nmeed to have an age
struct tut_MoveAgent : Agent {
    float m_fAge;
};


class tut_MovePop : public SPopulation<tut_MoveAgent> {

public:
    tut_MovePop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    virtual ~tut_MovePop();

    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);



protected:
 
    GetOld<tut_MoveAgent> *m_pGO;
    OldAgeDeath<tut_MoveAgent> *m_pOAD;

    RandomMove<tut_MoveAgent> *m_pRM;
 

};

#endif
