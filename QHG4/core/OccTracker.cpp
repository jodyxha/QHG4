#include <cstdio>
#include <omp.h>

#include <map>
#include <vector>

#include "types.h"
#include "qhg_consts.h"
#include "strutils.h"
#include "stdstrutilsT.h"
#include "PopBase.h"
#include "PopLooper.h"
#include "SCellGrid.h"
#include "OccHistory.h"
#include "OccTracker.h"

//----------------------------------------------------------------------------
//  creeateInstance
//
OccTracker *OccTracker::createInstance(SCellGrid *pCG, intvec &vCellIDs, PopLooper *pPL) {
    OccTracker *pOC = new OccTracker(pCG, vCellIDs);
    int iResult = pOC->init(pPL);
    if (iResult != 0) {
        delete pOC;
        pOC = NULL;
    }
    return pOC;
}


//----------------------------------------------------------------------------
//  destructor
//
OccTracker::~OccTracker() {
    if (m_pOH != NULL) {
        delete[] m_pOH;
    }
}



//----------------------------------------------------------------------------
//  updateCounts
//
int OccTracker::updateCounts(float fT) {
    int iResult = 0;
    //#pragma omp parallel for
    for (uint iCell = 0; iCell < m_vCellIDs.size(); iCell++)  {
        uint uMask = calcBitMap(m_vCellIDs[iCell]);
        m_pOH->addEntry(fT, iCell, uMask, true);
    }
    return iResult;
}



//----------------------------------------------------------------------------
//  constructor
//
OccTracker::OccTracker(SCellGrid *pCG, intvec vCellIDs)
    : Environment(pCG),
      m_vCellIDs(vCellIDs),
      m_pOH(NULL) {

}
    

//----------------------------------------------------------------------------
//  init
//
int OccTracker::init(PopLooper *pPL) {
    int iResult = 0;

    m_pOH = OccHistory::createInstance(m_vCellIDs, pPL->getNumPops());

    m_vPops.clear();
    m_vPopNames.clear();
    
    popmap::const_iterator it; 
    for (it = pPL->begin(); it != pPL->end(); it++) {
        if (m_vPops.size() < MAX_POPS) {
            m_vPops.push_back(it->second);
            m_vPopNames.push_back(it->second->getSpeciesName());
        } else {
            stdprintf("Too many populations: ignoring [%s]\n", it->second->getSpeciesName());
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
//  calcBitMap
//
uint OccTracker::calcBitMap(int iCellID) {
    uint uMask = 0;
    for (uint i = 0; i < m_vPops.size(); i++) {
        PopBase *pPB = m_vPops[i];
        uMask <<= 1;
        if (pPB->getNumAgents(iCellID)>0) {
            uMask++;
        }
    }
    return uMask;
}
 



//----------------------------------------------------------------------------
//  serialize
//  format      ::= <header><data>
//  header      ::= <num_pops><num_cells>
//  data        ::= <cellentry>*
//  cellentry   ::= <CellID><NumEntries><entry>*
//  entry       ::= <time_mask>
//
//  num_pops    : int
//  num_cells   : int
//  CellID      : int
//  num_entries : int
//  time_mask   : uint
//
// the time (step) and the mask are packed into a single uint
//
uchar *OccTracker::serialize() {
    return m_pOH->serialize();
}

//----------------------------------------------------------------------------
//  deserialize
//  format      ::= <header><data>
//  header      ::= <num_pops><num_cells>
//  format      ::= <cellentry>*
//  cellentry   ::= <CellID><NumEntries><entry>*
//  entry       ::= <time><mask>
//  CellID      : int
//  num_entries : int
//  time_mask   : uint
//
int OccTracker::deserialize(uchar *pBuf) {
    return m_pOH->deserialize(pBuf);
}
