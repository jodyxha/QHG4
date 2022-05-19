/*============================================================================
| Permutator
| 
|  Permute an array (or part of an array)
|
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstring>
#include "WELL512.h"
#include "Permutator.h"



//-----------------------------------------------------------------------------
// createInstance
//
Permutator *Permutator::createInstance(uint iInitSize) {
    Permutator *pP = new Permutator(iInitSize);
    int iResult =  pP->init();
    if (iResult != 0) {
        delete pP;
        pP = NULL;
    }
    return pP;
}


//-----------------------------------------------------------------------------
// destructor
//
Permutator::~Permutator() {
    if (m_aiPerm != NULL) {
        delete[] m_aiPerm;
    }
}


//-----------------------------------------------------------------------------
// permute
//   fills the first iNumSel plaxces of m_aiPerm with a random selection
//   picked from [0, iNumTot-1]
//   If iNumTot == iNumSel, m_aiPerm is filled with a random permutation og
//   the integers from [0, iNumTot-1]
//
const uint *Permutator::permute(uint iNumTot, uint iNumSel, WELL512 *pWELL) {
    uint *pPerm = NULL;
    // check if we have to resize
    if (iNumTot > m_iSize) {
        resize(iNumTot);
    }
    
    if (iNumSel <= iNumTot) {
        // fill array with ascending numbers if the size has changed 
        if (iNumTot != m_iPrevTot) {
            uint i = 0;
            while (i < iNumTot) {
                m_aiPerm[i] = i;
                i++;
            }
            m_iPrevTot = iNumTot;
        }

        pPerm = permuteBuf(iNumTot, iNumSel, pWELL, m_aiPerm);
    }
    return pPerm;
}


//-----------------------------------------------------------------------------
// permute
//   fills the first iNumSel plaxces of m_aiPerm with a random selection
//   picked from [0, iNumTot-1]
//   If iNumTot == iNumSel, m_aiPerm is filled with a random permutation og
//   the integers from [0, iNumTot-1]
//
const uint *Permutator::permute(uint iNumTot, uint iNumSel) {
    uint *pPerm = NULL;
    // check if we have to resize
    if (iNumTot > m_iSize) {
        resize(iNumTot);
    }
    
    if (iNumSel <= iNumTot) {
        // fill array with ascending numbers if the size has changed 
        if (iNumTot != m_iPrevTot) {
            uint i = 0;
            while (i < iNumTot) {
                m_aiPerm[i] = i;
                i++;
            }
            m_iPrevTot = iNumTot;
        }

        pPerm = permuteBuf(iNumTot, iNumSel, m_aiPerm);
    }
    return pPerm;
}


//-----------------------------------------------------------------------------
// constructor
//
Permutator::Permutator(uint iInitSize) 
    : m_iSize(iInitSize),
      m_aiPerm(NULL),
      m_iPrevTot(0) {
    
}


//-----------------------------------------------------------------------------
// init
//
int Permutator::init() {
    int iResult = 0;
    
    resize(m_iSize);

    return iResult;
}


//-----------------------------------------------------------------------------
// resize
//   allocate array of specified size, and set m_iSize to new size
//   delete previous array if there is one
//
void Permutator::resize(uint iNewSize) {
    if (m_aiPerm != NULL) {
        delete[] m_aiPerm;
    }
    m_iSize = iNewSize;
    m_aiPerm = new uint[m_iSize];
    memset(m_aiPerm, 0, m_iSize*sizeof(uint));
}

