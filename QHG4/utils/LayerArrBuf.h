/*============================================================================
| LayerArrBuf
|  
|  A resizable array derived from LayerBuf, designed to hold arrays instead
|  of simple types. It's sort of an array of arrays
|
|  Allows access to individual arrays with the []-operator.
|
|  Author: Jody Weissmann
\===========================================================================*/

#ifndef __LAYERARRBUF_H__
#define __LAYERARRBUF_H__

#include "types.h"
#include "LayerBuf.h"

template<class T>
class LayerArrBuf : public LayerBuf<T> {
public:
    LayerArrBuf(uint iLayerSize, uint iArraySize, int iStrategy=MB_DESTROY_DELAY1);
    LayerArrBuf();

    void init(uint iLayerSize, uint iArraySize, int iStrategy=MB_DESTROY_DELAY1);
 
    virtual ~LayerArrBuf(){};

#ifndef LBNEW
    virtual inline
    T &operator[](uint i) const { return  LayerBuf<T>::m_vUsedLayers[(i*m_iArraySize)/ LayerBuf<T>::m_iLayerSize][(i*m_iArraySize)%LayerBuf<T>::m_iLayerSize];};
#else
    virtual inline
    T &operator[](uint i) const { return  LayerBuf<T>::m_vUsedLayers[i>>LayerBuf<T>::m_iIndexBits][m_iArraySize*(i & LayerBuf<T>::m_iSubIndexMask)];};
#endif

    virtual size_t size();
    virtual uint getLayerSize() const;

    virtual void elementShift(uint iTo, uint iFrom);
    
    virtual void moveElements(uint iToLayer,   uint iToIndex, 
                              uint iFromLayer, uint iFromIndex, 
                              uint iNum);
        
    virtual int copyBlock(uint iStart, T *pBlock, uint iSize);
    virtual int copyBlock(uint iDest, LayerArrBuf<T> *pBuf, uint iOrig, uint iSize);

    virtual int copyLayer(int iDestLayer, const T *pData);


protected:
    uint m_iArraySize;
};

#endif


