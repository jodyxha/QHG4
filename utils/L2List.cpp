/*=============================================================================
| L2List
| 
|  Two doubly linked lists in a single array.
|  Used to keep track of used and unused elements in aarrays such as 
|  LayerBuf or LayerArrBuf,and provide fast access to unused indexes.
| 
|  Internally this is managed by two doubly linked lists of indexes,
|  "ACTIVE" and "PASSIVE". The nodes of both lists are  elements of 
|  a single array.
|  When a new index is used, the corresponding node is moved from 
|  the PASSIVE list to the ACTIVE list. If an index is not needed 
|  anymore, the corresponding node is moved from the "ACTIVE"
|  list to the "PASSIVE" list.
|  
|  It is the users responsability to make sure that 
|  no element is deleted twice, as this corrupts the linkage 
|  
|  Author: Jody Weissmann
\============================================================================*/

#include <cstdio>
#include <cstring>
#include <cstdlib>  // for abort()

#ifdef L2L_DEBUG
#include "stacktrace.h"
#endif

#include "strutils.h"
#include "L2List.h"


//----------------------------------------------------------------------------
// constructor
//
L2List::L2List(int iSize) 
    : m_iSize(iSize),
      m_alList(NULL),
      m_iCurHole(NIL),
      m_iCurActive(NIL),
      m_iSerType(DUMP_MODE_NONE) {
 
    m_alList = new L2Node[m_iSize];
    clear();
}


//----------------------------------------------------------------------------
// copy constructor
//
L2List::L2List(const L2List *pL2L) 
    : m_iSize(pL2L->m_iSize),
      m_alList(NULL),
      m_iCurHole(pL2L->m_iCurHole),
      m_iCurActive(pL2L->m_iCurActive) {
 
    m_alList = new L2Node[m_iSize];
    memcpy(m_alList, pL2L->m_alList,  m_iSize*sizeof(L2Node));
    memcpy(m_iFirst, pL2L->m_iFirst,  2*sizeof(int));
    memcpy(m_iLast,  pL2L->m_iLast,   2*sizeof(int));
}


//----------------------------------------------------------------------------
// destructor
//
L2List::~L2List() {
    if (m_alList != NULL) {
        delete[] m_alList;
    }
}


//----------------------------------------------------------------------------
// copy
//
int L2List::copy(const L2List *pL2L) {
    int iResult = 0;
    m_iSize      = pL2L->m_iSize;
    m_iCurHole   = pL2L->m_iCurHole;
    m_iCurActive = pL2L->m_iCurActive;
 
    // m_alList has almost certainly been allocated
    if (m_alList != NULL){
        delete[] m_alList;
    }
    m_alList = new L2Node[m_iSize];
    memcpy(m_alList, pL2L->m_alList,  m_iSize*sizeof(L2Node));
    memcpy(m_iFirst, pL2L->m_iFirst,  2*sizeof(int));
    memcpy(m_iLast,  pL2L->m_iLast,   2*sizeof(int));
    return iResult;
}


//----------------------------------------------------------------------------
// clear
//
void L2List::clear() {
    m_alList[0].iPrev = NIL;
    if (m_iSize > 1) {
        m_alList[0].iNext = 1;
    } else {
        m_alList[0].iNext = NIL;
    }
 
    for (int i = 1; i < m_iSize-1; i++) {
        m_alList[i].iPrev = i-1;
        m_alList[i].iNext = i+1;
     }
            
    if (m_iSize > 1) {
        m_alList[m_iSize-1].iPrev = m_iSize-2;
    } else {
        m_alList[m_iSize-1].iPrev = NIL;
    }
    m_alList[m_iSize-1].iNext = NIL;
 
    m_iFirst[ACTIVE]  = NIL;
    m_iLast[ACTIVE]   = NIL;
    m_iFirst[PASSIVE] = 0;
    m_iLast[PASSIVE]  = m_iSize-1;
}


//----------------------------------------------------------------------------
// reserveSpace2
//   Assumption: there are at least iNum consecutive elements after last active
//
//   activates iNum elements after the last active element
//   return index of first reserved element
//
int L2List::reserveSpace2(uint iNum) {
    int iStart = -1;

    if ((m_iLast[ACTIVE] == NIL) || (m_iLast[ACTIVE]+(int)iNum) < m_iSize) {
        // start postion of block
        if (m_iLast[ACTIVE] == NIL) {
            iStart = m_iFirst[PASSIVE];
        } else {
            iStart = m_iLast[ACTIVE]+1;
        }
        // last position of block
        int iEnd = iStart+iNum-1;

        
        if ((iStart == 0) && (iEnd == m_iSize-1)) {
            // block is entire layer: no linking required
            m_iFirst[ACTIVE]  = iStart;
            m_iLast[ACTIVE]   = iEnd;
            m_iFirst[PASSIVE] = NIL;
            m_iLast[PASSIVE]  = NIL;
        
        } else {

            int iPrevActive = m_iLast[ACTIVE];
            // the passive element previous to the block begin
            int iPrevPassive = m_alList[iStart].iPrev;
            int iPostPassive = (iEnd < m_iSize-1)?iEnd+1:NIL;

                    
            // link block into active
            m_iLast[ACTIVE] = iEnd;
            m_alList[iEnd].iNext = NIL;
            m_alList[iStart].iPrev = iPrevActive;
            if (iPrevActive != NIL) {
                m_alList[iPrevActive].iNext = iStart;
            } else {
                m_iFirst[ACTIVE] = iStart;
            }
        
            
            // unlink block from passive
            if (iPrevPassive != NIL) {
                m_alList[iPrevPassive].iNext = iPostPassive;
            } else {
                m_iFirst[PASSIVE] = iPostPassive;
            }
            if (iPostPassive != NIL) {
                m_alList[iPostPassive].iPrev = iPrevPassive;
            } else {
                m_iLast[PASSIVE] = iPrevPassive;
            }

        }
    
    }
    return iStart;
}


//----------------------------------------------------------------------------
// getNumEndFree
//   get size of largest passive block touching the layer End
//   which is iLayerSize-1-lastActive
//
int L2List::getNumEndFree() {
    int iSize = -1;
    if (m_iLast[ACTIVE] != NIL) {
        iSize = m_iSize - m_iLast[ACTIVE] - 1;
    } else {
        iSize = m_iSize;
    }
    return iSize;
}


//----------------------------------------------------------------------------
// unlink
//   connect node's previous and next with each other
//   taking care of special cases (first or last).
//   The node's links are not changed
//
int L2List::unlink(uchar uState, int iE) {
    if ((iE < 0) || (iE > m_iSize)) {
        printf("unlink: iE is %d!!!!!!!!!!!!!!\n", iE);
#ifdef L2L_DEBUG
        print_stacktrace(stdout);
#endif
        printf("aborting\n");
        fflush(stdout);
        abort(); 
    }
    int iP = m_alList[iE].iPrev; 
    int iN = m_alList[iE].iNext; 
 
    if (iP != NIL) {
        // link previous to next
        m_alList[iP].iNext = iN;
    } else {
        // node was first in list
        m_iFirst[uState] =  iN;
    }

    if (iN != NIL) {
        // link next to previous
        m_alList[iN].iPrev = iP;
    } else {
        // node was last in list
        m_iLast[uState] =  iP;
    }
    return iE;
}


//----------------------------------------------------------------------------
// linkAfter
//   link iE after index iTarget taking care of special cases
//
int L2List::linkAfter(uchar uState, int iE, int iTarget) {

    // iE's previous will always be iTarget (even if NIL)
    m_alList[iE].iPrev = iTarget;

    if (iTarget != NIL) {
        //  "normal" insertion 
        int iN = m_alList[iTarget].iNext; 
        if (iN != NIL) {
            // link iE to target's successor
            m_alList[iN].iPrev = iE;
        } else {
            // target was the last; now iE is the last
            m_iLast[uState] = iE;
        }
        // link target to iE
        m_alList[iTarget].iNext = iE; 
        // link iE to target's successor
        m_alList[iE].iNext = iN;

    } else {
        // target is NIL: insert iE in front

        // this node becomes the old first node's previous (if it existed)
        if (m_iFirst[uState] != NIL) {
            m_alList[m_iFirst[uState]].iPrev = iE;
        }
        // the previous first node becomes this node's nexxt
        m_alList[iE].iNext = m_iFirst[uState];

        // this node becomes the first in the list
        m_iFirst[uState] = iE;
        if (m_iLast[uState] == NIL) {
            m_iLast[uState] = iE;
        }

    }

    return iE;
}


//----------------------------------------------------------------------------
// findNewPrev
//  find closest previous index with other state.
//  if the distance between a node E and its previous
//  node is greater than one, the node immediately before E
//  is of the other list.
//
int L2List::findNewPrev(int iE) const {
    // find a jump > 1
    int iP = m_alList[iE].iPrev;
    while ((iP != NIL) && (iP == iE -1)) {
        iE = iP;
        if ((iE < 0) || (iE > m_iSize)) {
            printf("FindNewPrev: iE is %d!!!!!!!!!!!!!!\n", iE);
#ifdef L2L_DEBUG
            print_stacktrace(stdout);
#endif
            printf("aborting\n");
            fflush(stdout);
            abort();
        }
        iP = m_alList[iE].iPrev;
    }

    // we found a large jump or reached the beginning of the list
    if (/*(iP != NIL) ||*/ (iE > 0)) {
        // the node immediately before iE is of the other state
        iP = iE-1;
    }
    return iP;
}


//----------------------------------------------------------------------------
// removeElement
//   - unlink from ACTIVE list
//   - find new previous in PASSIVE list
//   - link after new previous in PASSIVE list
//
int L2List::removeElement(int iE) {

    unlink(ACTIVE, iE);
    int iTarget = findNewPrev(iE);
    linkAfter(PASSIVE, iE, iTarget);

    return iTarget;
}


//----------------------------------------------------------------------------
// removeLastElement
//   remove last active element (must not be NULL)
//
int L2List::removeLastElement() {
    int iResult = NIL;
    iResult =  removeElement(m_iLast[ACTIVE]);
    return iResult;
}


//----------------------------------------------------------------------------
// addElement
//   if PASSIVE list not empty:
//   - unlink from PASSIVE list
//   - find new previous in ACTIVE list
//   - link after new previous in ACTIVE list$
//
int L2List::addElement() {

    int iE = m_iFirst[PASSIVE];
    
    if (iE != NIL) {
        // there is at least one free space: remove it from the passive list
        unlink(PASSIVE, iE);
        // find closest previous in ACTIVE list
        int iTarget = findNewPrev(iE);
        // link it
        linkAfter(ACTIVE, iE, iTarget);
        
    }
    return iE;
}


//----------------------------------------------------------------------------
// countOfState
//
int L2List::countOfState(uchar uState) {
    int iCount = 0;
    int iCur = m_iFirst[uState];
    while (iCur != NIL) {
        iCount++;
        iCur =m_alList[iCur].iNext;
    }
    return iCount;
}


//----------------------------------------------------------------------------
// isActve
//
bool L2List::hasState(int iState, int iIndex) {
    bool bSearching = true;
    int iCur    = m_iFirst[iState];
    while ((iCur != NIL) && bSearching) {
        if (iCur == iIndex) {
	     bSearching = false;
	} else {
            iCur = m_alList[iCur].iNext;
	}
    }
    return !bSearching;
}


//----------------------------------------------------------------------------
// collectFragInfo
//   get indexes of all holes which can be filled by trailing actives
//   example
//     0123456789012345678901234567890                              
//     xxoxooxxxxoxooooxoxxxoooxxooooo
//   piHoles:
//      2  4  5 10 12 13
//   piActive
//     35 34 30 29 28 16
//   
//  We start from searching for holes from low indexes up,
//  and searching for actives starting fromn high indexes down.
//  As soon as we run out of holes or actives, or if the arrays are full,
//  searching stops and we decide if the search has to continue or 
//  if the next search has to start from zero.
//  m_iCurHole and m_iCurActive are state variables for the search
//  m_iCurHole   == NIL : start hole search from first
//  m_iCurACTIVE == NIL : start active search from last
//
//  Returns  value
//   iSize     if all indexes fit exactly into arrays(iSize)
//   iSize+1   if arrays are filled, but there is more
//   > 0        number of found holes/actives; no more to be found
//   0         no holes or no actives: nothing to move
// 
uint L2List::collectFragInfo(uint iSize, uint *piHoles, uint *piActive) {
    uint iResult = 0;

    // start search from first or continue where we stopped last time?
    if (m_iCurHole == NIL) {
        m_iCurHole = m_iFirst[PASSIVE];
    }
    // start search from last or continue where we stopped last time?
    if (m_iCurActive == NIL) {
        m_iCurActive = m_iLast[ACTIVE];
    }

    // search holes/actives
    uint iC = 0;

    // we continue searching as long as the array is not full and none of the searches reached the end (NIL)
    // and the hole index is smaller tha the active index 
    while ((iC < iSize) && (m_iCurHole < m_iCurActive) && (m_iCurHole != NIL) && (m_iCurActive != NIL)) {
        // add current indexes to array
        piHoles[iC]  = (uint) m_iCurHole;
        piActive[iC] = (uint) m_iCurActive;

        iC++;
        // move on
        m_iCurHole   = m_alList[m_iCurHole].iNext;
        m_iCurActive = m_alList[m_iCurActive].iPrev;
    }

    // searching is over: decide on return value
    if (iC == 0) {
        // nothing saved in the array
        if ((m_iCurHole == NIL)  && (m_iCurActive != NIL)) {
            // no holes - layer is completely filled
            iResult = 0;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;

        } else if ((m_iCurHole != NIL)  && (m_iCurActive == NIL)) {
            // only holes - layer is completely empty
            iResult = 0;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;
        } else if ((m_iCurHole != NIL)  && (m_iCurActive != NIL)) {
            // probably: m_iCurHole > m_iCurActive
            iResult = 0;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;
        }
    
    } else {
        // "normal" case
        if (iC < iSize) {
            // everything fit into arrays

            iResult = iC;
            // start new collection next time
            m_iCurHole   = NIL;
            m_iCurActive = NIL;
        } else {
            // if array is full, there are two possibilities
            if  (m_iCurHole < m_iCurActive) {
                // there is more
                iResult = iC+1;
            } else {
                // everything fit into array perfectly (nothing more to find)
                iResult = iC;
                // start new collection next time
                m_iCurHole   = NIL;
                m_iCurActive = NIL;
            }
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// defragment
//   move all actives to frontal position
//   With the data from collectFragInfo() this 
//     xxoxooxxxxoxooooxoxxxoooxxooooo
//   results in this pattern:
//     xxxxxxxxxxxxxxooooooooooooooooo
//
int L2List::defragment(uint iSize, uint *piActive) {
 
    for (uint i = 0; i < iSize; i++) {
        removeElement(piActive[i]);
        addElement(); // will fill lowest hole
    }
    return 0;
}


//----------------------------------------------------------------------------
// setState
//   sets the first PASSIVE indexes to ACTIVE, or
//   sets the last ACTIVE indexes to PASSIVE
//
int L2List::setState(uchar uState, uint iNum) {
    if (uState == ACTIVE) {
        for (uint  i = 0; i < iNum; i++) {
            addElement();
        }
    } else {
        for (uint  i = 0; i < iNum; i++) {
            removeLastElement();
        }
    }
    return 0;
}


//----------------------------------------------------------------------------
// findInListForward
//
int L2List::findInListForward(uchar uState, uint iNum) const {
    int iResult = 0;
    int k = m_iFirst[uState];
    // there will be at most m_iSize links; should find an end before that
    for (int i = 0; (i < m_iSize) && (k != NIL) && (k != (int) iNum) ; i++) {
        k = m_alList[k].iNext;
    }
    if (k != (int) iNum) {
        //printf("%d is not contained in %s forward list\n", iNum, uState?"ACTIVE":"PASSIVE");
        iResult = -1;
        
    } else {
    }
    return iResult;
}


//----------------------------------------------------------------------------
// findInListBackward
//
int L2List::findInListBackward(uchar uState, uint iNum) const {
    int iResult = 0;
    int k = m_iLast[uState];
    for (int i = 0; (i < m_iSize) && (k != NIL) && (k != (int)iNum); i++) {
        k = m_alList[k].iPrev;
    }
    if (k != (int)iNum) {
        //printf("%d is not contained in %s backward list\n", iNum, uState?"ACTIVE":"PASSIVE");
        iResult = -1;
    } else {
    }
    return iResult;
}


//----------------------------------------------------------------------------
// error values for checkList
//
//  BAD_FIRST             : Element 0 is neither first active nor first passive
//  BAD_LAST              : element size-1 is neither last active nor last passive
//  BAD_FIRST_VAL_PASSIVE : first passive has value outside [0, size-1]
//  BAD_FIRST_VAL_ACTIVE  : first active has value outside [0, size-1]
//  BAD_LAST_VAL_PASSIVE  : last passive has value outside [0, size-1]
//  BAD_LAST_VAL_ACTIVE   : last active has value outside [0, size-1]
//  BAD_FIRST_PREV_PASSIVE: first passive has non-NIL prev
//  BAD_FIRST_PREV_ACTIVE : first active has non-NIL prev
//  BAD_LAST_NEXT_PASSIVE : last passive has non-NIL next
//  BAD_LAST_NEXT_ACTIVE  : last active has non-NIL next
//  BAD_PREV_VAL          : element has prev val outside [-1,size-1] or equal to self
//  BAD_NEXT_VAL          : element has next val outside [-1,size-1] or equal to self
//  BAD_BALANCE           : not every element is referenced twice
//  BAD_FORWARD_PASSIVE   : the chain from first passive does not lead to last passive
//  BAD_FORWARD_ACTIVE    : the chain from first active does not lead to last active
//  BAD_BACKWARD_PASSIVE  : the chain from last passive does not lead to first passive
//  BAD_BACKWARD_ACTIVE   : the chain from last active does not lead to first active
//  BAD_PREV_NEXT_LINK    : an element's prev's next is not self
//  BAD_NEXT_PREV_LINK    : an element's next's prev is not self
//  BAD_NEXT_MONOTONY     : an elements's next has a smaller index as self
//  BAD_PREV_MONOTONY     : an elements's prev has a larger index as self
//
const uint BAD_FIRST               = 0x00000002;
const uint BAD_LAST                = 0x00000004;
const uint BAD_FIRST_VAL_PASSIVE   = 0x00000010;
const uint BAD_FIRST_VAL_ACTIVE    = 0x00000018;
const uint BAD_LAST_VAL_PASSIVE    = 0x00000040;
const uint BAD_LAST_VAL_ACTIVE     = 0x00000060;
const uint BAD_FIRST_PREV_PASSIVE  = 0x00000100;
const uint BAD_FIRST_PREV_ACTIVE   = 0x00000180;
const uint BAD_LAST_NEXT_PASSIVE   = 0x00000400;
const uint BAD_LAST_NEXT_ACTIVE    = 0x00000600;
const uint BAD_PREV_VAL            = 0x00000800;
const uint BAD_NEXT_VAL            = 0x00001000;
const uint BAD_BALANCE             = 0x00002000;
const uint BAD_FORWARD_PASSIVE     = 0x00010000;
const uint BAD_FORWARD_ACTIVE      = 0x00020000;
const uint BAD_BACKWARD_PASSIVE    = 0x00040000;
const uint BAD_BACKWARD_ACTIVE     = 0x00080000;
const uint BAD_PREV_NEXT_LINK      = 0x00100000;
const uint BAD_NEXT_PREV_LINK      = 0x00200000;
const uint BAD_NEXT_MONOTONY       = 0x00400000;
const uint BAD_PREV_MONOTONY       = 0x00800000;

//----------------------------------------------------------------------------
// checkList
//   check integrity of list:
//     either first[ACTIVE] or first[PASSIVE] must be 0
//     either last[ACTIVE] or last[PASSIVE] must be size-1
//     last[X].next = NIL and first[X] = NIL
//     prev and next values must be in [-1, size-1]
//     every index must appear once as prev and once as next
//
int L2List::checkList() const {
    int iResult = 0;
    if ((m_iFirst[PASSIVE] != 0) && (m_iFirst[ACTIVE] != 0)) {
        iResult |= BAD_FIRST;
        printf("BAD_FIRST (P %d, A %d)\n", m_iFirst[PASSIVE], m_iFirst[ACTIVE]);fflush(stdout);
    }
    if ((m_iLast[PASSIVE] != m_iSize-1) && (m_iLast[ACTIVE] != m_iSize-1)) {
        iResult |= BAD_LAST;
        printf("BAD_LAST (P %d, A %d)\n", m_iLast[PASSIVE], m_iLast[ACTIVE]);fflush(stdout);
    }
    if (m_iFirst[PASSIVE] != NIL) {
        if ((m_iFirst[PASSIVE] < 0) || (m_iFirst[PASSIVE] > m_iSize)) {
	    iResult |= BAD_FIRST_VAL_PASSIVE;	
            printf("BAD_FIRST_VAL_PASSIVE: %d\n", m_iFirst[PASSIVE]);fflush(stdout);
	}
        if (m_alList[m_iFirst[PASSIVE]].iPrev != NIL) {
            iResult |= BAD_FIRST_PREV_PASSIVE;
            printf("BAD_FIRST_PREV_PASSIVE: %d\n", m_alList[m_iFirst[PASSIVE]].iPrev);fflush(stdout);
        }
    }
    if (m_iFirst[ACTIVE] != NIL) {
        if ((m_iFirst[ACTIVE] < 0) || (m_iFirst[ACTIVE] > m_iSize)) {
	    iResult |= BAD_FIRST_VAL_ACTIVE;	
            printf("BAD_FIRST_VAL_ACTIVE: %d\n", m_iFirst[ACTIVE]);fflush(stdout);
	}
        if (m_alList[m_iFirst[ACTIVE]].iPrev != NIL) {
            iResult |= BAD_FIRST_PREV_ACTIVE;
            printf("BAD_FIRST_PREV_ACTIVE: %d\n", m_alList[m_iFirst[ACTIVE]].iPrev);fflush(stdout);
        }
    }
    if (m_iLast[PASSIVE] != NIL) {
        if ((m_iLast[PASSIVE] < 0) || (m_iLast[PASSIVE] > m_iSize)) {
	    iResult |= BAD_LAST_VAL_PASSIVE;	
            printf("BAD_LAST_VAL_PASSIVE: %d\n", m_iLast[PASSIVE]);fflush(stdout);
	}
        if (m_alList[m_iLast[PASSIVE]].iNext != NIL) {
            iResult |= BAD_LAST_NEXT_PASSIVE;
            printf("BAD_LAST_NEXT_PASSIVE: %d\n", m_alList[m_iLast[PASSIVE]].iNext);fflush(stdout);
        }
    }
    if (m_iLast[ACTIVE] != NIL) {
        if ((m_iLast[ACTIVE] < 0) || (m_iLast[ACTIVE] > m_iSize)) {
	    iResult |= BAD_LAST_VAL_ACTIVE;	
            printf("BAD_LAST_VAL_ACTIVE: %d\n", m_iLast[ACTIVE]);fflush(stdout);
	}
        if (m_alList[m_iLast[ACTIVE]].iNext != NIL) {
            iResult |= BAD_LAST_NEXT_ACTIVE;
            printf("BAD_LAST_NEXT_ACTIVE: %d\n", m_alList[m_iLast[ACTIVE]].iNext);fflush(stdout);
        }
    }
    
    
    int *aIndexes = new int[m_iSize];
    memset(aIndexes, 0, m_iSize*sizeof(int));
    for (int i = 0; i < m_iSize;i++) {
        if ((m_alList[i].iPrev < -1) || 
            (m_alList[i].iPrev >= m_iSize) || 
            (m_alList[i].iPrev == i)) {
            iResult |= BAD_PREV_VAL;
            printf("BAD_PREV_VAL %d at %d\n", m_alList[i].iPrev, i);fflush(stdout);
        } else {
            if (m_alList[i].iPrev >= 0) {
                aIndexes[m_alList[i].iPrev]++;
            }
        }

        if ((m_alList[i].iNext < -1) || 
            (m_alList[i].iNext >= m_iSize) || 
            (m_alList[i].iNext == i)) {
            iResult |= BAD_NEXT_VAL;
            printf("BAD_NEXT_VAL %d at %d\n", m_alList[i].iNext, i);fflush(stdout);
        } else {
            if (m_alList[i].iNext >= 0) {
                aIndexes[m_alList[i].iNext]++;
            }
        }
        
        int iP = m_alList[i].iPrev;
        if (iP != NIL) {
            if (m_alList[iP].iNext != i) {
                iResult |= BAD_PREV_NEXT_LINK;
                printf("BAD_PREV_NEXT_LINK at %d: %d.prev=%d, %d.next=%d\n", i, i, iP, iP, m_alList[iP].iNext);fflush(stdout);
            } else {
                // ok
            }
            if (iP >= i) {
                iResult |= BAD_PREV_MONOTONY;
                printf("BAD_PREV_MONOTONY at %d: %d.prev=%d\n", i, i, iP);fflush(stdout);
            }
        } else {
            if ((m_iFirst[PASSIVE] != i) && (m_iFirst[ACTIVE] != i)) {
                iResult |= BAD_PREV_NEXT_LINK;
                printf("BAD_PREV_NEXT_LINK at %d: %d.prev=%d, but %d is not a first\n", i, i, iP, i);fflush(stdout);
            }
        }


        int iN = m_alList[i].iNext;
        if (iN != NIL) {
            if (m_alList[iN].iPrev != i) {
                iResult |= BAD_NEXT_PREV_LINK;
                printf("BAD_NEXT_PREV_LINK at %d: %d.next=%d, %d.prev=%d\n", i, i, iN, iN, m_alList[iN].iPrev);fflush(stdout);
            } else {
                // ok
            }
            if (iN <= i) {
                iResult |= BAD_NEXT_MONOTONY;
                printf("BAD_NEXT_MONOTONY at %d: %d.next=%d\n", i, i, iN);fflush(stdout);
            }

        } else {
            if ((m_iLast[PASSIVE] != i) && (m_iLast[ACTIVE] != i)) {
                iResult |= BAD_NEXT_PREV_LINK;
                printf("BAD_NEXT_PREV_LINK at %d: %d.prev=%d, but %d is not a last\n", i, i, iN, i);fflush(stdout);
            }
        }

    }
    
    if (m_iFirst[PASSIVE] != NIL) {
        aIndexes[m_iFirst[PASSIVE]]++;
    }
    if (m_iFirst[ACTIVE] != NIL) {
        aIndexes[m_iFirst[ACTIVE]]++;
    }
    if (m_iLast[PASSIVE] != NIL) {
        aIndexes[m_iLast[PASSIVE]]++;
    }
    if (m_iLast[ACTIVE] != NIL) {
        aIndexes[m_iLast[ACTIVE]]++;
    }


    for (int i = 0; i < m_iSize;i++) {
        if (aIndexes[i] != 2) {
            printf("BAD_BALANCE at %d: %d\n", i, aIndexes[i]);fflush(stdout);
            iResult |= BAD_BALANCE;
        }
    }

    int iResult2 = findInListForward(PASSIVE, m_iLast[PASSIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_FORWARD_PASSIVE;
    }

    iResult2 = findInListForward(ACTIVE,  m_iLast[ACTIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_FORWARD_ACTIVE;
    }

    iResult2 = findInListBackward(PASSIVE,  m_iFirst[PASSIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_BACKWARD_PASSIVE;
    }

    iResult2 = findInListBackward(ACTIVE,  m_iFirst[ACTIVE]);
    if (iResult2 != 0) {
        iResult |= BAD_BACKWARD_ACTIVE;
    }

    /*
    int k  = -1;
    int k2 = -1;
    k = m_iFirst[PASSIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iNext;
    }
    if (k == NIL) {
        if (k2 != m_iLast[PASSIVE]) {
            printf("end of passive list (%d) does not match m_iLast[PASSIVE] (%d)\n", k2, m_iLast[PASSIVE]);
            iResult |= BAD_FORWARD_PASSIVE;
        }
    } else {
        printf("unfinished forward passive list\n");
        iResult |= BAD_FORWARD_PASSIVE;
    }

    k = m_iFirst[ACTIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iNext;
    }
    if (k == NIL) {
        if (k2 != m_iLast[ACTIVE]) {
            printf("end of active list (%d) does not match m_iLast[ACTIVE] (%d)\n", k2, m_iLast[ACTIVE]);
            iResult |= BAD_FORWARD_ACTIVE;
        }
    } else {
        printf("unfinished forward active list\n");
        iResult |= BAD_FORWARD_ACTIVE;
    }

    k = m_iLast[PASSIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iPrev;
    }
    if (k == NIL) {
        if (k2 != m_iFirst[PASSIVE]) {
            printf("beginning of passive list (%d) does not match m_iFirst[PASSIVE] (%d)\n", k2, m_iFirst[PASSIVE]);
            iResult |= BAD_FORWARD_PASSIVE;
        }
    } else {
        printf("unfinished backward passive list\n");
        iResult |= BAD_FORWARD_PASSIVE;
    }

    k = m_iLast[ACTIVE];
    k2 = k;
    for (int i = 0; (i < m_iSize) && (k != NIL); i++) {
        k2 = k;
        k = m_alList[k].iPrev;
    }
    if (k == NIL) {
        if (k2 != m_iFirst[ACTIVE]) {
            printf("beginning of active list (%d) does not match m_iFirst[ACTIVE] (%d)\n", k2, m_iFirst[ACTIVE]);
            iResult |= BAD_FORWARD_ACTIVE;
        }
    } else {
        printf("unfinished backward active list\n");
        iResult |= BAD_FORWARD_ACTIVE;
    }
    */

    delete[] aIndexes;
    return iResult;
}

//----------------------------------------------------------------------------
// calcHolyness
//
int L2List::calcHolyness(uchar uState, float *pfRatio) const {
    int iPrev =  0;
    int iCur = m_iFirst[uState];
    int iNumHoles = 0;
    int iHoleSize = 0;
    while (iCur != NIL) {
        if (iCur - iPrev > 1) {
            iNumHoles++;
            iHoleSize += iCur - iPrev -1;
        }
        iPrev = iCur;
        iCur = m_alList[iCur].iNext;
    }

    *pfRatio = (float)iHoleSize/(float)m_iSize;
    /*
    printf("Number of holes: %d\n", iNumHoles);
    printf("Holyness: %f%%\n", *pfRatio);
    */
    return iNumHoles;
}

//----------------------------------------------------------------------------
// countActiveRegions
//
int L2List::countActiveRegions() const {
    int iCount = 0;
    int iCur   = m_iFirst[ACTIVE];
    int iPrev  =  iCur;
    while (iCur != NIL) {
        if (iCur - iPrev > 1) {
            iCount++;
        }
        iPrev = iCur;
        iCur = m_alList[iCur].iNext;
    }
    iCount++;

    return iCount;
}


//----------------------------------------------------------------------------
// collectActiveBorders
//
int *L2List::collectActiveBorders(int iCount) const {
    int *pBorders = new int[2*iCount];
    int iPos = 0;

    int iCur   = m_iFirst[ACTIVE];
    int iFirst = iCur;
    int iPrev  =  iCur;
    while (iCur != NIL) {
        if (iCur - iPrev > 1) {
            pBorders[iPos++] = iFirst;
            pBorders[iPos++] = iPrev;
            
            iFirst =   iCur;
        }
        iPrev = iCur;
        iCur = m_alList[iCur].iNext;
    }
    // NIL also ends a region
    pBorders[iPos++] = iFirst;
    pBorders[iPos++] = iPrev;
    
    return pBorders;
}


//----------------------------------------------------------------------------
// createActiveRegions
//
int L2List::createActiveRegions(int *pBorders, int iCount) {
    if (iCount >= 1) {
        int s = pBorders[0];
        int t = pBorders[1];
        if ((iCount == 1) && (s == NIL) && (t == NIL)) {
            // no ACTIVE -> do nothing

        } else if ((iCount == 1) && (s == 0) && (t == m_iSize-1)) {
            // one ACTIVE: entire region
            m_iFirst[ACTIVE] = 0;
            m_iLast[ACTIVE]  = m_iSize-1;
            m_iFirst[PASSIVE] = NIL;
            m_iLast[PASSIVE] = NIL;
        

        } else {
            for (int i = 0; i < iCount; ++i) {
                s = pBorders[2*i];
                t = pBorders[2*i+1];
                
                int sp = m_alList[s].iPrev;
                int tn = m_alList[t].iNext;
                
                // unlink backward
                if (tn != NIL) {
                    m_alList[tn].iPrev = sp;
                }
                
                // unlink forward
                if (sp != NIL) {
                    m_alList[sp].iNext = tn;
                }
                
                // prev.next = id
                if (m_iLast[ACTIVE] != NIL) {
                    m_alList[m_iLast[ACTIVE]].iNext = s;
                }
                
                // settin correct prev
                m_alList[s].iPrev = m_iLast[ACTIVE];
                
                // this will change at every iteration
                m_iLast[ACTIVE] = t;
                // as will this
                m_alList[t].iNext = NIL;
                
                // handle first ACTIVE if required
                if (m_iFirst[ACTIVE] == NIL) {
                    m_iFirst[ACTIVE] = s;
                }
                // handle first PASSIVE if required
                if (sp == NIL) {
                    m_iFirst[PASSIVE] = tn;
                }
                if ((i == iCount-1) && (t != m_iSize-1)) {
                    m_iLast[PASSIVE] = m_iSize-1;
                } else {
                    m_iLast[PASSIVE] = sp;
                }
            }
        }
    }
    return iCount;
}




//----------------------------------------------------------------------------
// display
//
void L2List::display(uchar uState) const {
    printf("[%d][%d]|", m_iFirst[uState], m_iLast[uState]);
    int i = m_iFirst[uState];
    bool bGoOn = true;
    while (bGoOn && (i >= 0) && (i < m_iSize)) {
        
        printf("%d|",i);
        i = m_alList[i].iNext;
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// ddisplay
//
void L2List::ddisplay(uchar uState) const {
    printf("[%d][%d]| ", m_iFirst[uState], m_iLast[uState]);
    int i = m_iFirst[uState];
    bool bGoOn = true;
    while (bGoOn && (i >= 0) && (i < m_iSize)) {
        char sN[16];
        if (m_alList[i].iNext < 0) {
            strcpy(sN, "*");
        } else {
            sprintf(sN, "%d", m_alList[i].iNext);
        }
        char sP[16];
        if (m_alList[i].iPrev < 0) {
            strcpy(sP, "*");
        } else {
            sprintf(sP, "%d", m_alList[i].iPrev);
        }

        printf("%s:(%d):%s | ", sP, i, sN);
        i = m_alList[i].iNext;
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// displayArray
//
void L2List::displayArray(int iFirst,int iLast) const {
    if (iFirst == NIL) {
        iFirst = 0;
    }
    if ((iLast == NIL) || (iLast > m_iSize)) {
        iLast = m_iSize;
    }
        

    int i = iFirst;    
    printf("FP[%d]LP[%d]|", m_iFirst[PASSIVE], m_iLast[PASSIVE]);
    printf("FA[%d]LA[%d]|", m_iFirst[ACTIVE], m_iLast[ACTIVE]);

    while  (i < iLast) {
       char sN[16];
        if (m_alList[i].iNext < 0) {
            strcpy(sN, "*");
        } else {
            sprintf(sN, "%d", m_alList[i].iNext);
        }
        char sP[16];
        if (m_alList[i].iPrev < 0) {
            strcpy(sP, "*");
        } else {
            sprintf(sP, "%d", m_alList[i].iPrev);
        }

        printf("%s:(%d):%s | ",sP, i, sN);

        i++;
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// compare
//
int  L2List::compare(const L2List *pL2L) {
    int iResult = 0;
    if (m_iSize != pL2L->m_iSize) {
        iResult = m_iSize -  pL2L->m_iSize;
    } else {
        iResult =  memcmp(m_alList, pL2L->m_alList, m_iSize*sizeof(L2Node));
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getBufSize
//  this *must* be called before serialize()
//
int L2List::getBufSize(int iDumpMode){
    int iBufSize = -1;
    
    m_iNumRegions = countActiveRegions();
    bool bHoleCond = (m_iSize/(m_iNumRegions+1)) > 10;
    if ((iDumpMode == DUMP_MODE_SMART) ||
        ((iDumpMode == DUMP_MODE_FREE) && bHoleCond)) {

        iBufSize   = (5+2*m_iNumRegions)*sizeof(int);
        m_iSerType = DUMP_MODE_SMART;

    } else if ((iDumpMode == DUMP_MODE_FLAT) ||
        ((iDumpMode == DUMP_MODE_FREE) && !bHoleCond)) {

        iBufSize   = (8+2*m_iSize)*sizeof(int);
        m_iSerType = DUMP_MODE_FLAT;
    } else {
        m_iSerType = DUMP_MODE_NONE;
    }
    
    return iBufSize;
}


//----------------------------------------------------------------------------
// serialize
//
uchar *L2List::serialize(uchar *pBuf) {
    uchar *pCur = putMem(pBuf, &m_iSerType, sizeof(int));

    if (m_iSerType == DUMP_MODE_FLAT) {
        pCur = serializeFlat(pCur);
    } else if (m_iSerType == DUMP_MODE_SMART) {
        pCur = serializeSmart(pCur);
    } else {
        printf("Bad DumpMode: %d\n", m_iSerType); 
    }
    
    return pCur;
}


//----------------------------------------------------------------------------
// serializeFlat
//
uchar *L2List::serializeFlat(uchar *pBuf) {
    uchar *pCur = putMem(pBuf, &m_iSize, sizeof(int));
    pCur = putMem(pCur, m_iFirst, 2*sizeof(int));
    pCur = putMem(pCur, m_iLast, 2*sizeof(int));
    pCur = putMem(pCur, &m_iCurHole, sizeof(int));
    pCur = putMem(pCur, &m_iCurActive, sizeof(int));
    pCur = putMem(pCur, m_alList, 2*m_iSize*sizeof(int));
    return pCur;
}


//----------------------------------------------------------------------------
// serializeSmart
//   before this, getBufSize() must be called (to set m_iNumRegions)
//
uchar *L2List::serializeSmart(uchar *pBuf) {
    int *pRegions = collectActiveBorders(m_iNumRegions);
    uchar *pCur = putMem(pBuf, &m_iSize, sizeof(int));
    pCur = putMem(pCur, &m_iNumRegions, sizeof(int));
    pCur = putMem(pCur, &m_iCurHole, sizeof(int));
    pCur = putMem(pCur, &m_iCurActive, sizeof(int));
    pCur = putMem(pCur, pRegions, 2*m_iNumRegions*sizeof(int));
    delete[] pRegions;
    return pCur;
}


//----------------------------------------------------------------------------
// deserialize
//
uchar *L2List::deserialize(uchar *pBuf) {
    
    uchar *pCur = getMem(&m_iSerType, pBuf, sizeof(int));
    if (m_iSerType == DUMP_MODE_FLAT) {
        pCur = deserializeFlat(pCur);
    } else if (m_iSerType == DUMP_MODE_SMART){
        pCur = deserializeSmart(pCur);
    } else {
        printf("Bad DumpMode: %d\n", m_iSerType); 
    }
    
    return pCur;
}

//----------------------------------------------------------------------------
// deserializeFlat
//
uchar *L2List::deserializeFlat(uchar *pBuf) {
    uchar *pCur = getMem(&m_iSize, pBuf, sizeof(int));
    pCur = getMem(m_iFirst, pCur, 2*sizeof(int));
    pCur = getMem(m_iLast, pCur, 2*sizeof(int));
    pCur = getMem(&m_iCurHole, pCur, sizeof(int));
    pCur = getMem(&m_iCurActive, pCur, sizeof(int));
    pCur = getMem(m_alList, pCur, 2*m_iSize*sizeof(int));
    return pCur;
}

//----------------------------------------------------------------------------
// deserializeSmart
//
uchar *L2List::deserializeSmart(uchar *pBuf) {
    int iSize = -1;
    uchar *pCur = getMem(&iSize, pBuf, sizeof(int));
    if (iSize == m_iSize) {
        pCur = getMem(&m_iNumRegions, pCur, sizeof(int));
        int *pRegions = new int[2*m_iNumRegions];
        pCur = getMem(&m_iCurHole, pCur, sizeof(int));
        pCur = getMem(&m_iCurActive, pCur, sizeof(int));
        pCur = getMem(pRegions, pCur, 2*m_iNumRegions*sizeof(int));
        createActiveRegions(pRegions, m_iNumRegions);
        delete[] pRegions;
    } else {
        pCur = NULL;
    }
    return pCur;
}
