/*============================================================================
| Permutator
| 
|  Permute an array (or part of an array)
|
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __PERMUTATOR_H__
#define __PERMUTATOR_H__

#include <cstring>
#include "types.h"
#include "WELL512.h"

class WELL512;

class Permutator {
public:
    static Permutator *createInstance(uint iInitSize);
    ~Permutator();
    const uint *permute(uint iNumTot, uint iNumSel, WELL512 *pWELL);
    const uint *permute(uint iNumTot, uint iNumSel);

    template<typename T>
    T *permuteBuf(uint iNumTot, uint iNumSel, WELL512 *pWELL, T *Buf);

    template<typename T>
    T *permuteBuf(uint iNumTot, uint iNumSel, T *Buf);


    uint getSize()        { return m_iSize;};
    uint getPrevTot()     { return m_iPrevTot;};
    const uint* getPerm() { return m_aiPerm;};

    void setSize(uint iSize)       { resize(iSize);};
    void setPrevTot(uint iPrevTot) { m_iPrevTot = iPrevTot;};
    void setPerm(uint *aiPerm) { memcpy(m_aiPerm, aiPerm, m_iSize*sizeof(uint));};


protected:
    Permutator(uint iInitSize);
    int init();
    void resize(uint iNewSize);

    uint  m_iSize;
    uint *m_aiPerm;
    uint m_iPrevTot;
};


//-----------------------------------------------------------------------------
// permuteBuf
//   fills the first iNumSel places of pBuf with a random selection
//   picked from the first iNumTot elements.
//   If iNumTot == iNumSel, m_aiPerm is filled with a random permutation of
//   all elements the integers from [0, iNumTot-1]
//
template<typename T>
T *Permutator::permuteBuf(uint iNumTot, uint iNumSel, WELL512 *pWELL, T *pBuf) {
    T *pPerm = NULL;
    // check if we have to resize
    if (iNumTot > m_iSize) {
        resize(iNumTot);
    }
    
    if (iNumSel <= iNumTot) {

        // now exchange the first iNumSel elements with other random elements
        for (uint i = 0; i < iNumSel; ++i) {
            uint k = pWELL->wrandi(i, iNumTot);
            std::swap(pBuf[i], pBuf[k]);
        }
        pPerm = pBuf;
    }
    return pPerm;
}

//-----------------------------------------------------------------------------
// permuteBuf
//   fills the first iNumSel places of pBuf with a random selection
//   picked from the first iNumTot elements.
//   If iNumTot == iNumSel, m_aiPerm is filled with a random permutation of
//   all elements the integers from [0, iNumTot-1]
//
template<typename T>
T *Permutator::permuteBuf(uint iNumTot, uint iNumSel, T *pBuf) {
    T *pPerm = NULL;
    // check if we have to resize
    if (iNumTot > m_iSize) {
        resize(iNumTot);
    }
    
    if (iNumSel <= iNumTot) {

        // now exchange the first iNumSel elements with other random elements
        for (uint i = 0; i < iNumSel; ++i) {
            uint k = i + (int)(((iNumTot -1)*1.0*rand())/(1.0/RAND_MAX));
            std::swap(pBuf[i], pBuf[k]);
        }
        pPerm = pBuf;
    }
    return pPerm;
}






#endif
