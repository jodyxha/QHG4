#include <cstdio>
#include <cstring>
#include <omp.h>
#include <map>

#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "MoveStats_old.h"


//-----------------------------------------------------------------------------
// constructor
//
MoveStats_old::MoveStats_old()
    : m_iNumCells(0),
      m_aiHops(NULL),
      m_adDist(NULL),
      m_adTime(NULL),
      m_aiHopsTemp(NULL),
      m_adDistTemp(NULL),
      m_asChanged(NULL) {

    
}


//-----------------------------------------------------------------------------
// constructor
//
MoveStats_old::MoveStats_old(uint iNumCells)
    : m_iNumCells(iNumCells),
      m_aiHops(NULL),
      m_adDist(NULL),
      m_adTime(NULL),
      m_aiHopsTemp(NULL),
      m_adDistTemp(NULL),
      m_asChanged(NULL)  {

    init();
}


//---------------------------------------------------------arrays for jho--------------------
// init
//
int MoveStats_old::init() {
        
    // make all hops -1
    m_aiHops = new int[m_iNumCells];
    memset(m_aiHops, 0xff, m_iNumCells*sizeof(uint));
    
    // make all dists 0
    m_adDist = new double[m_iNumCells];
    memset(m_adDist, 0x00, m_iNumCells*sizeof(double));
    
    // make all times -1
    m_adTime = new double[m_iNumCells];
    for (uint i = 0; i < m_iNumCells; i++) {
        m_adTime[i] = -1;
    }

    // make all hops -1
    m_aiHopsTemp = new int[m_iNumCells];
    memset(m_aiHopsTemp, 0xff, m_iNumCells*sizeof(uint));
    
    // make all dists 0
    m_adDistTemp = new double[m_iNumCells];
    for (uint i = 0; i < m_iNumCells; i++) {
        m_adDistTemp[i] = 1e9;
    }

    
    m_asChanged = new intset[omp_get_max_threads()];
   
        
    return 0;
}


//-----------------------------------------------------------------------------
// destructor
//
MoveStats_old::~MoveStats_old() {
    if (m_aiHops != NULL) {
        delete[] m_aiHops;
    }
    if (m_adDist != NULL) {
        delete[] m_adDist;
    }
    if (m_adTime != NULL) {
        delete[] m_adTime;
    }

    if (m_aiHopsTemp != NULL) {
        delete[] m_aiHopsTemp;
    }
    if (m_adDistTemp != NULL) {
        delete[] m_adDistTemp;
    }
    if (m_asChanged != NULL) {
        delete[] m_asChanged;
    }
 
}



//-----------------------------------------------------------------------------
// addStatCandidates
//
int MoveStats_old::addStatCandidates(int iCell, double dDist, int iHops, double dTime) {
    int iResult = 0;
    
    
    if (dDist < m_adDistTemp[iCell]) {

        m_adDistTemp[iCell] = dDist;
        m_aiHopsTemp[iCell] = iHops;
     
        m_asChanged[omp_get_thread_num()].insert(iCell);
        
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// insertNewStats
//   this must be called at the end of an iteration
//
int MoveStats_old::insertNewStats(double fCurTime) {
    int iResult = 0;

    for (int i = 1; i < omp_get_max_threads(); ++i) {
        m_asChanged[0].insert(m_asChanged[i].begin(), m_asChanged[i].end());
    }
    intset::const_iterator it;
    for (it = m_asChanged[0].begin(); it != m_asChanged[0].end(); ++it) {
        m_aiHops[*it] = m_aiHopsTemp[*it];
        m_adDist[*it] = m_adDistTemp[*it];
        m_adTime[*it] = fCurTime;
        m_adDistTemp[*it] = 1e9;
    }

    for (int i = 0; i < omp_get_max_threads(); ++i) {
        m_asChanged[i].clear();
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readData
//
int MoveStats_old::readDataQDF(hid_t hGroup) {
    int iResult = 0;
  

    if (iResult == 0) {
        iResult = qdf_readArray(hGroup, SPOP_DS_HOPS,   m_iNumCells, m_aiHops);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(hGroup, SPOP_DS_DIST,   m_iNumCells, m_adDist);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(hGroup, SPOP_DS_TIME,   m_iNumCells, m_adTime);
    }

    return iResult;

}


//----------------------------------------------------------------------------
// write
//
int MoveStats_old::writeDataQDF(hid_t hGroup) {
    int iResult = 0;
     
    if (iResult == 0) {
        printf("[ MoveStats_old::writeDataQDF] writing Hops[%d]\n", m_iNumCells);
        iResult = qdf_writeArray(hGroup, SPOP_DS_HOPS, m_iNumCells, m_aiHops);
    }
            
    if (iResult == 0) {
        printf("[ MoveStats_old::writeDataQDF] writing Dist[%d]\n", m_iNumCells);
        iResult = qdf_writeArray(hGroup, SPOP_DS_DIST, m_iNumCells, m_adDist);
    }
            
    if (iResult == 0) {
        printf("[ MoveStats_old::writeDataQDF] writing Time[%d]\n", m_iNumCells);
        iResult = qdf_writeArray(hGroup, SPOP_DS_TIME, m_iNumCells, m_adTime);
    }
            
    return iResult;
}
