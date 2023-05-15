
#include <vector>
#include <map>
#include "Sampling.h"
#include "EachSampling.h"

//----------------------------------------------------------------------------
//  constructor
//
EachSampling::EachSampling(int iNumCells) :
    m_iNumCells(iNumCells) {
    
    for (int i = 0; i < m_iNumCells; ++i) {
        m_mGroups[i].push_back(i);
    }
    makeRefs();
}

//----------------------------------------------------------------------------
//  destructor
//
EachSampling::~EachSampling() {};

