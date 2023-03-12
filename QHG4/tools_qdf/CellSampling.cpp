
#include <vector>
#include <map>
#include "Sampling.h"
#include "CellSampling.h"

//----------------------------------------------------------------------------
//  constructor
//
CellSampling::CellSampling(int iNumCells) :
    m_iNumCells(iNumCells) {
    
    for (int i = 0; i < m_iNumCells; ++i) {
        m_mGroups[i].push_back(i);
    }
    makeRefs();
}

//----------------------------------------------------------------------------
//  destructor
//
CellSampling::~CellSampling() {};

