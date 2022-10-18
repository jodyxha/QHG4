#ifndef __TUT_STATICPOP_H__
#define __TUT_STATICPOP_H__



#include "SPopulation.h"

// we do *Not* extend the agent structure


class tut_StaticPop : public SPopulation<Agent> {

 public:
    tut_StaticPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);

};

#endif
