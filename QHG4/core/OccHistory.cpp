
#include <cstdio>
#include <cstring>
#include <omp.h>

#include <map>
#include <vector>

#include "types.h"
#include "utils.h"
#include "strutils.h"
#include "OccHistory.h"


//----------------------------------------------------------------------------
//  creeateInstance
//
OccHistory *OccHistory::createInstance(intvec &vCellIDs, int iNumPops) {
    OccHistory *pOH = new OccHistory(vCellIDs, iNumPops);
    int iResult = pOH->init();
    if (iResult != 0) {
        delete pOH;
        pOH = NULL;
    }
    return pOH;
}


//----------------------------------------------------------------------------
//  destructor
//
OccHistory::~OccHistory() {
    if (m_pTB != NULL) {
        delete[] m_pTB;
    }
}


//----------------------------------------------------------------------------
//  constructor
//
OccHistory::OccHistory(intvec vCellIDs, int iNumPops)
   : m_vCellIDs(vCellIDs),
     m_iNumPops(iNumPops),
     m_pTB(NULL),
     m_iDataSize(0) {

    for (uint i = 0; i < m_vCellIDs.size(); i++) {
        m_mID2Pos[m_vCellIDs[i]] = i;
    }
              
}

//----------------------------------------------------------------------------
//  setNumPops
//
int OccHistory::setNumPops(int iNumPops) {
    int iResult = 0;
    m_iNumPops = iNumPops;
    if (m_iNumPops < MAX_POPS) {
        initializeCounts(NEG_INF);
    } else {
        printf("Too many populations: %d >= %d\n", m_iNumPops, MAX_POPS);
        iResult = -1;
    }
    return iResult;
}

//----------------------------------------------------------------------------
//  init
//
int OccHistory::init() {
    int iResult = 0;
    if (m_vCellIDs.size() > 0) {
        m_pTB = new timed_bits[m_vCellIDs.size()];
    } else {
        m_pTB = NULL;
    }

    
    if (m_iNumPops > 0) {
        if (m_iNumPops < MAX_POPS) {
            initializeCounts(NEG_INF);
        } else {
            printf("Too many populations: %d >= %d\n", m_iNumPops, MAX_POPS);
            iResult = -1;
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
//  initializeCounts
//
void OccHistory::initializeCounts(float fT) {
    
    for (uint iCell = 0; iCell < m_vCellIDs.size(); iCell++)  {
        m_pTB[iCell][fT] = 0;
        //        m_pTB[iCell].push_back(std::pair<float, uint>(fT, uMask));
    }
}


//----------------------------------------------------------------------------
//  addEntry
//    iCellID must be pne of the ID passed to constructor
//    pCOunts must have at least m_iNumPops entries
//
int OccHistory::addEntry(int iStep, int iCellIndex, uchar uMask, bool bIgnoreZero) {
    int iResult = 0;

    if (m_iNumPops > 0) {
        timed_bits &tb = m_pTB[iCellIndex];
        uint uMaskOld = tb.rbegin()->second;
        
        if (uMask != uMaskOld) {
            if (!bIgnoreZero || (uMask != 0)) {
                tb[iStep] = uMask;
            }
        }
    } else {
        printf("Number of populations is 0\n");
        iResult = -1;
    }
    return iResult;
}

//----------------------------------------------------------------------------
//  addEntry
//    iCellID must be pne of the ID passed to constructor
//    pCOunts must have at least m_iNumPops entries
//
int OccHistory::addEntry(int iStep, int iCellIndex, uchar *pCounts) {
    int iResult = 0;

    if (m_iNumPops > 0) {
        uchar uMask=0;
        for (int i = 0; i < m_iNumPops; i++) {
            uMask <<=1;
            uMask += (pCounts[i] > 0)?1:0;
        }
        m_pTB[iCellIndex][iStep] = uMask;
    } else {
        printf("Number of populations is 0\n");
        iResult = -1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
//  addEntries
//    pvCounts must have at least m_iNumPops entries for every cell passed in constr
//
int OccHistory::addEntries(int iStep, ucharpvec &pvCounts) {
    int iResult = 0;
#pragma omp parallel for
    for (uint i = 0; i < m_vCellIDs.size(); i++) {
        iResult = addEntry(iStep, i, pvCounts[i]);
    }
    return iResult;
}


//----------------------------------------------------------------------------
//  ID2Idx
int OccHistory::ID2Idx(int iCellID) {
    int iIndex = -1;
    intintmap::const_iterator iim = m_mID2Pos.find(iCellID);
    if (iim != m_mID2Pos.end()) {
        iIndex = iim->second;
    }
    return iIndex;
}


//----------------------------------------------------------------------------
//  serialize
//  format      ::= <header><data>
//  header      ::= <num_pops> <numcells>
//  data        ::= <cellentry>*
//  cellentry   ::= <CellID><num_entries><entry>*
//  entry       ::= <time_mask>
//
//  num_pops    : int
//  num_cells   : int
//  CellID      : int
//  num_entries : int
//  time_mask   : uint
//
uchar *OccHistory::serialize() {
    
    int iSize = 2*sizeof(int); //num pops+numcells
    
    for (uint i = 0; i < m_vCellIDs.size(); i++) {
        iSize += 2*sizeof(int);  // cell ID + num entries
        int iCell = m_mID2Pos[i];
        timed_bits &tt=m_pTB[iCell];
        iSize += tt.size()*sizeof(uint);
    }
    m_iDataSize = iSize;

    uchar *pBuf = new uchar[iSize];
    memset(pBuf, 0x77, iSize);
    uchar *p = pBuf;

    int ic = m_vCellIDs.size();
    p = putMem(p, &m_iNumPops, sizeof(int));  
    p = putMem(p, &ic, sizeof(int));  
    printf("written npops %d, ncells %d\n", m_iNumPops, ic);
    for (uint i = 0; i < m_vCellIDs.size(); i++) {
        p = putMem(p, &(m_vCellIDs[i]), sizeof(int));
            
        printf("|%d", m_vCellIDs[i]);
        int iCell = m_mID2Pos[i];
        timed_bits &tt=m_pTB[iCell];

        int is = tt.size();
        p = putMem(p, &is, sizeof(int));
        
        printf("|%zd", tt.size());
        timed_bits::const_iterator it;
        //    for (uint i = 0; i < tb.size(); i++) {
        for (it = tt.begin(); it != tt.end(); ++it) {
            printf("|%08d|%04x", it->first, it->second);
            uint u = 0;
            //            if (it->first == NEG_INF) {
            //    u = (NEG_INF_C << MAX_POPS)  + it->second;
            //} else {
            u = (it->first << MAX_POPS)  + it->second;
                //}
            printf("(%08x)", u);
            p = putMem(p, &u, sizeof(uint));  
        }
        printf("|\n");
    }
    printf("-> %d bytes\n", iSize);
    printf("\n");
    return pBuf;

}


//----------------------------------------------------------------------------
//  deserialize
//  format      ::= <header><data>
//  header      ::= <num_pops> <numcells>
//  data        ::= <cellentry>*
//  cellentry   ::= <CellID><num_entries><entry>*
//  entry       ::= <time_mask>
//
//  num_pops    : int
//  num_cells   : int
//  CellID      : int
//  num_entries : int
//  time_mask   : uint
//
int OccHistory::deserialize(uchar *pBuf) {
    int iResult = 0;
    printf("deserializing\n");
    m_vCellIDs.clear();
    m_mID2Pos.clear();
    if (m_pTB != NULL) {
        delete[] m_pTB;
    }

    uchar *p = pBuf;

    int iNumPops;
    int iNumCells;
    
    p = getMem(&iNumPops, p, sizeof(int));
    p = getMem(&iNumCells, p, sizeof(int));

    m_pTB = new timed_bits[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        int iCellID;
        p = getMem(&iCellID, p, sizeof(int));
        m_vCellIDs.push_back(iCellID);
        m_mID2Pos[iCellID] = i;

        int iTTsize;
        p = getMem(&iTTsize, p, sizeof(int));
        timed_bits tt;
        for (int j = 0; j < iTTsize; j++) {
            uint uComp;
            p = getMem(&uComp, p, sizeof(uint));
            uchar uMask = uComp & MSK_CROP;
            int  iStep  = uComp >> MAX_POPS;
            if (iStep == NEG_INF_C) {
                iStep = NEG_INF;
            }
            
            tt[iStep] = uMask;
        }

        m_pTB[i] = tt;
    }
    return iResult;
}


