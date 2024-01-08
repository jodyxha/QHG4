#include <cstdio>
#include <cstring>
#include <omp.h>

#include "SCellGrid.h"
#include "Navigation2.h"


//-----------------------------------------------------------------------------
// constructor
//
Navigation2::Navigation2(SCellGrid *pCG)
    : Environment(pCG),
      m_iNumCells(0),
      m_iNumWaterWays(0),
      m_dSampleDist(0),
      m_iNumBridges(0) {

          m_iNumCells = pCG->m_iNumCells;
}


//-----------------------------------------------------------------------------
// setData
//
int Navigation2::setData(const waterwaymap &mWaterWays/*, uint iNumWaterways*/, double dSampleDist) {
    m_mWaterWays    = mWaterWays;
    m_iNumWaterWays = m_mWaterWays.size();
    m_dSampleDist   = dSampleDist;
    
    return 0;
}


//-----------------------------------------------------------------------------
// setBridges
//
int Navigation2::setBridges(const bridgelist &vBridges) {
    m_vBridges    = vBridges;
    m_iNumBridges = m_vBridges.size();
    return 0;
}

//-----------------------------------------------------------------------------
// destructor
//
Navigation2::~Navigation2() {
}


