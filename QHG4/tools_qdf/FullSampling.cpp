
#include <vector>
#include <map>
#include "Sampling.h"
#include "FullSampling.h"

//----------------------------------------------------------------------------
//  constructor
//
FullSampling::FullSampling(int iNumCells) :
    m_iNumCells(iNumCells) {
    
    for (int i = 0; i < m_iNumCells; ++i) {
        m_mGroups[0].push_back(i);
    }
    makeRefs();
}

//----------------------------------------------------------------------------
//  destructor
//
FullSampling::~FullSampling() {};

