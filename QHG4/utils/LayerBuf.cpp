/*============================================================================
| LayerBuf
|  
|  Resizable arrays of any type, implemented as std::vector of arrays of T,
|  so-called "layers". When more space is needed, new layers are added to the
|  vector.
|
|  Allows element access with the []-operator.
|
|  Author: Jody Weissmann
\===========================================================================*/

#ifndef __LAYERBUF_CPP__
#define __LAYERBUF_CPP__

#include <cstdio>
#include <cstring>
#include <vector>
#include <map>

#include "types.h"
#include "LayerBuf.h"

//---------------------------------------------------------------------------
// constructor
//   - iBlockSize: size of memory blocks
//   - iStrategy: or-ed combination of MB_XXX constants controlling the
//                deletion behaviour
//
template<class T>
LayerBuf<T>::LayerBuf(uint iLayerSize, int iStrategy)
    : m_iLayerSize(iLayerSize),
#ifndef LBOLD
      m_iIndexBits(0),
      m_iSubIndexMask(iLayerSize-1),
#endif
      m_iNumLayers(0),
      m_iStrategy(MB_ZERO_BLOCKS | iStrategy) {

#ifndef LBOLD    
    uint j = m_iLayerSize;
    while (j > 1) {
        j >>= 1;
        m_iIndexBits++;
    }
#endif
}

//---------------------------------------------------------------------------
// constructor
//   - iBlockSize: size of memory blocks
//   - iStrategy: or-ed combination of MB_XXX constants controlling the
//                deletion behaviour
//
template<class T>
LayerBuf<T>::LayerBuf()
    : m_iLayerSize(0),
      m_iIndexBits(0),
      m_iSubIndexMask(0),

      m_iNumLayers(0),
      m_iStrategy(MB_ZERO_BLOCKS) {


}



//---------------------------------------------------------------------------
// destructor
//
template<class T>
LayerBuf<T>::~LayerBuf() {

    // handle used blocks
    for (unsigned int i = 0; i < m_vUsedLayers.size(); i++) {
        if (m_vUsedLayers[i] != NULL) {
            // delete array
            printf("[LayerBuf<T>::createLayer()]deleting layer %d (%p)\n", i,  m_vUsedLayers[i]); fflush(stdout);
            delete[] m_vUsedLayers[i];
        }
    }

    // handle free blocks
    for (unsigned int i = 0; i < m_vFreeLayers.size(); i++) {
        if (m_vFreeLayers[i] != NULL) {
            // delete array
            delete[] m_vFreeLayers[i];
        }
    }
}

//---------------------------------------------------------------------------
// init
//
template<class T>
void LayerBuf<T>::init(uint iLayerSize, int iStrategy) {
    m_iLayerSize = iLayerSize;
    m_iStrategy = MB_ZERO_BLOCKS | iStrategy;


#ifndef LBOLD    
  if (m_iSubIndexMask == 0) {
      m_iSubIndexMask = m_iLayerSize-1;
      m_iIndexBits    = 0;
      
      uint j = m_iLayerSize;
      while (j > 1) {
          j >>= 1;
          m_iIndexBits++;
      }
  }
#endif
}


//---------------------------------------------------------------------------
// size
//  returns capacity 
//
template<class T>
size_t LayerBuf<T>::size() {
    return m_iNumLayers*m_iLayerSize;
}

//---------------------------------------------------------------------------
// createLayer
//   creates new layer or reuses a free one, and returns it
//
template<class T>    
void LayerBuf<T>::createLayer() {
    T *pLayer = NULL;
    //    printf("Creating new layer\n");
    // if free blocks are available ...
    if (m_vFreeLayers.size() > 0) {
        // ... use one of them
        //        printf("Reusing block\n");
        pLayer = m_vFreeLayers.back();
        m_vFreeLayers.pop_back();
    } else {
        // ... otherwise create a new one
        //        printf("Creating block\n");
        pLayer = new T[m_iLayerSize];
        printf("[LayerBuf<T>::createLayer()] created new layer %p\n", pLayer); fflush(stdout);
        // if data is padded there may be uninitialised bytes
        // to prevent valgrind nag: initialize entire layer
        memset(pLayer, 37, m_iLayerSize*sizeof(T));
    }

    if ((m_iStrategy & MB_ZERO_BLOCKS) != 0) {
        memset(pLayer, 0, m_iLayerSize*sizeof(T));
    }
    m_vUsedLayers.push_back(pLayer);
    m_iNumLayers++;
}

//---------------------------------------------------------------------------
// copyLayer
//
template<class T>
int  LayerBuf<T>::copyLayer(int iDestLayer, const T *pData) {
    int iResult = -1;
    if ((iDestLayer >= 0) && (iDestLayer < (int) m_vUsedLayers.size())) {
        memcpy(m_vUsedLayers[iDestLayer], pData, m_iLayerSize*sizeof(T));
        iResult = 0;
    }
    return iResult;
}

//---------------------------------------------------------------------------
// appendLayers
//
template<class T>
int  LayerBuf<T>::appendLayers(LBBase *pLBB) {
    int iResult = -1;
    LayerBuf<T> *pLB = dynamic_cast<LayerBuf<T> *>(pLBB);
    if (pLB != NULL) {
        m_vUsedLayers.insert(m_vUsedLayers.begin(), pLB->m_vUsedLayers.begin(), pLB->m_vUsedLayers.end());
        iResult = 0;
    } else{
        // expected a LayerBuf<T>
    }
    return iResult;
}


//---------------------------------------------------------------------------
// freeAllLayers
//  removes all used layers from vector of used layers.
//  depending on strategy, layers are destroyed or saved for reuse in vector 
//  of free layers
//
template<class T>
void LayerBuf<T>::freeAllLayers() {
    for (unsigned int i = 0; i < m_vUsedLayers.size(); i++) {
        freeLayer(i);
    }
}


//---------------------------------------------------------------------------
// detachAllLayers
//  simply clears the layers, but does not delete them
//
template<class T>
void LayerBuf<T>::detachAllLayers() {
    m_vUsedLayers.clear();
}


//---------------------------------------------------------------------------
// freeLayer
//  removes layer from vector of used layerss.
//  depending on strategy, layer is destroyed or saved for reuse in vector 
//  of free layer
//
template<class T>
void LayerBuf<T>::freeLayer(uint iIndex) {
    if (iIndex < m_vUsedLayers.size()) {
        T* pLayer = m_vUsedLayers[iIndex];
        //        printf("freeing u %p\n", pLayer); 
        
        bool bDestroy = false;
        unsigned int iDelay = (m_iStrategy & MB_DESTROY_DELAY_MASK);
        if ((iDelay !=  MB_DESTROY_LAZY) && (m_vFreeLayers.size() >= iDelay)) {
            bDestroy = true;
        }

        if (bDestroy) {
            //            printf("RealDestroy\n");
            delete[] pLayer;
        } else {
            m_vFreeLayers.push_back(pLayer);
        }
        m_vUsedLayers.erase(m_vUsedLayers.begin() + iIndex);
        m_iNumLayers--;
    }
}

//---------------------------------------------------------------------------
// elementShift
//
template<class T>
void LayerBuf<T>::elementShift(uint iTo, uint iFrom) {

    size_t iLayerTo   = iTo / m_iLayerSize;
    size_t iLayerFrom = iFrom / m_iLayerSize;
    size_t iIndexTo   = iTo % m_iLayerSize;
    size_t iIndexFrom = iFrom % m_iLayerSize;

    memcpy(&(m_vUsedLayers[iLayerTo][iIndexTo]), 
           &(m_vUsedLayers[iLayerFrom][iIndexFrom]), 
           sizeof(T));
}


//---------------------------------------------------------------------------
// moveElements
//
template<class T>
void LayerBuf<T>::moveElements(uint iToLayer,   uint iToIndex, 
                               uint iFromLayer, uint iFromIndex, 
                               uint iNum) {

    memcpy(&(m_vUsedLayers[iToLayer][iToIndex]), 
           &(m_vUsedLayers[iFromLayer][iFromIndex]), 
          iNum* sizeof(T));

}


//---------------------------------------------------------------------------
// copyBlock
//   copying from an array
//
template<class T>
int LayerBuf<T>::copyBlock(uint iStart, T *pBlock, uint iSize) {
    int iResult = 0;
    T *pCur = pBlock;

    uint iLayer = iStart / m_iLayerSize;
    uint iPos   = iStart % m_iLayerSize;

    uint iNum = m_iLayerSize-iPos;

    if (iSize < iNum) {
        iNum = iSize;
    }
    iSize -= iNum;

    //    printf("First Layer (%d) has %d free spaces\n", iLayer, m_iLayerSize-iPos);
    // do the first few
    //    printf("copy %u items of size %zd from %p (%p+%d) to layer %u pos %u (size %u)\n", iNum, sizeof(T), pCur, pBlock, 0, iLayer, iPos, m_iLayerSize);
    memcpy(&(m_vUsedLayers[iLayer][iPos]), pCur, iNum*sizeof(T));
    
    while (iSize > 0) {
        pCur += iNum;
        iLayer++;
        iNum = (iSize < m_iLayerSize) ? iSize : m_iLayerSize;
        iSize -= iNum;
        //        printf("copy %u items of size %zd from %p (%p+%ld) to layer %u/%u pos 0 (size %ld)\n", iNum, sizeof(T), pCur, pBlock, pCur-pBlock, iLayer, getNumLayers(), m_iLayerSize);fflush(stdout);
        memcpy(&(m_vUsedLayers[iLayer][0]), pCur, iNum*sizeof(T));
    }
    return iResult;
}


//---------------------------------------------------------------------------
// copyBlock
//   copying from an LayerBuf
//
template<class T>
int LayerBuf<T>::copyBlock(uint iDest, LayerBuf<T> *pBuf, uint iOrig, uint iSize) {
    T* pTemp = new T[iSize];
    T* pCur  = pTemp;

    uint iNum = iSize;
    uint iToDo = iSize;
    uint iLayer = iOrig/m_iLayerSize;
    while (iToDo > 0) {
        if (iToDo > m_iLayerSize) {
            iNum = m_iLayerSize - iOrig;
        } else {
            iNum = iToDo;
        }
        memcpy(pCur, pBuf->getLayer(iLayer)+iOrig, iNum*sizeof(T));
        iOrig = 0;
        iToDo -= iNum;
        pCur += iNum;
        iLayer++;
            
    }
    // we must explicitly reference LayerBuf, 
    // because copyBlock is virtual in LayerBuf amd LayerArrBuf
    LayerBuf<T>::copyBlock(iDest, pTemp, iSize);
    delete[] pTemp;
    return 0;
}


//---------------------------------------------------------------------------
// showUsedLayers
//
template<class T>
void LayerBuf<T>::showUsedLayers() {
    printf("Used:");
    for (unsigned int i = 0; i < m_vUsedLayers.size(); i++) {
        printf("%p ", m_vUsedLayers[i]);
    }
    printf("\n");

}

//---------------------------------------------------------------------------
// showFreeLayers
//
template<class T>
void LayerBuf<T>::showFreeLayers() {
    printf("Free:");
    for (unsigned int i = 0; i < m_vFreeLayers.size(); i++) {
        printf("%p ", m_vFreeLayers[i]);
    }
    printf("\n");
}

#endif


