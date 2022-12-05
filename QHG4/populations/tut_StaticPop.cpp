/*
#include <omp.h>
#include <hdf5.h>

*/
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
//#include "Action.cpp"
#include "SPopulation.cpp"

#include "tut_StaticPop.h"

//----------------------------------------------------------------------------
// constructor
//
tut_StaticPop::tut_StaticPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<Agent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds) {
    
    
}

