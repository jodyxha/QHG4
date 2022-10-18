#ifndef __TUT_OLDAGEDIEPOP_H__
#define __TUT_OLDAGEDIEPOP_H__

//#include "OldAgeDeath.h"
#include "AtanDeath.h"
#include "GetOld.h"


#include "SPopulation.h"

// our agents need to have an age
struct tut_OldAgeDieAgent : Agent {
    float m_fAge;
};

class tut_OldAgeDiePop : public SPopulation<tut_OldAgeDieAgent> {

 public:
    tut_OldAgeDiePop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    virtual ~tut_OldAgeDiePop();

    int addPopSpecificAgentData(int iAgentIndex, char **ppData);

    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);



protected:
 
    GetOld<tut_OldAgeDieAgent> *m_pGO;
    AtanDeath<tut_OldAgeDieAgent> *m_pAD;
    

};

#endif
