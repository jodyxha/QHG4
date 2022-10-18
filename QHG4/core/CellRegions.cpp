#include <omp.h>

#include "types.h"
#include "geomutils.h"
#include "SCellGrid.h"
#include "CellRegions.h"

//-----------------------------------------------------------------------------
// createInstance
//
CellRegions *CellRegions::createInstance(SCellGrid *pCG, double dRadius) {
    CellRegions *pCR = new CellRegions(pCG, dRadius);
    int iResult = pCR->init();
    if (iResult != 0) {
        delete pCR;
        pCR = NULL;
    }
    return pCR;
}

//-----------------------------------------------------------------------------
// constructor
//
CellRegions::CellRegions(SCellGrid *pCG, double dRadius) 
    : m_pCG(pCG),
      m_dRadius(dRadius),
      m_asCandidates(NULL) {
}
 

//-----------------------------------------------------------------------------
// destructor
//
CellRegions::~CellRegions() {
    if (m_asCandidates != NULL) {
        delete[] m_asCandidates;
    }
}
 
//-----------------------------------------------------------------------------
// init
//
int CellRegions::init() {
    int iResult = 0;

    m_asCandidates = new intset[omp_get_max_threads()];
#pragma omp parallel
{
    m_asCandidates[omp_get_thread_num()].clear();
}
    return iResult;
}


//-----------------------------------------------------------------------------
// findInside
//
int CellRegions::findInside(double dLon, double dLat, double dDist, intset &sCells) {
    int iResult = -1;
    // loop through all nodes of  EQ
    printf("Doing %u cells\n", m_pCG->getNumCells());
#pragma omp parallel for
    for (uint i = 0; i < m_pCG->getNumCells(); i++) {
        if (spherdistDeg(dLon, dLat, 
                      m_pCG->getGeography()->getLongitude()[i], 
                      m_pCG->getGeography()->getLatitude()[i], m_dRadius) < dDist) {
            m_asCandidates[omp_get_thread_num()].insert(i);
        }
    }
    // merge into sCells
      for (int i = 0; i < omp_get_max_threads(); i++) {
        sCells.insert(m_asCandidates[i].begin(), m_asCandidates[i].end());
    }
    return iResult;
}
