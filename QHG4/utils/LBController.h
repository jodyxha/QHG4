/*============================================================================
| LBController
|  
|  Uses an L2List to keep track of used and unused indexes in a collection of 
|  LBBase objects.
|  The LBBase objects must have the same layer size.
|  
|  Author: Jody Weissmann
\===========================================================================*/

#ifndef __LBCONTROLLER_H__
#define __LBCONTROLLER_H__

#include "types.h"
#include "L2List.h"

class LBBase;


class LBController {
public:
    LBController(uint iLayerSize);
    LBController();
    ~LBController();

    int init(int iLayerSize);

    int addBuffer(LBBase *pLB);
    int removeBuffer(LBBase *pLB);

    // layer managenent
    int addLayer();
    int removeLayer(uint iLayer);

    // element management
    uint getFreeIndex();
    int  deleteElement(uint lIndex);

    int compactData();
    void clear();
    uint reserveSpace2(uint iNum);

    // iterating
    int getFirstIndex(uchar uState) const;
    int getNextIndex(uchar uState, int iCur) const;
    int getLastIndex(uchar uState) const;

    // info
    uint getLayerSize() const {return m_iLayerSize;};
    uint getNumLayers() const {return (uint) m_vpL2L.size();};
    uint getNumUsed() const   {return m_iNumUsed;};
    uint getNumFree() const   {return (uint)m_vpL2L.size()*m_iLayerSize-m_iNumUsed;};

    // info for hdf5 
    uint getNumUsed(int i) const   {return m_vpL2L[i]->countOfState(L2List::ACTIVE);};
    uint getNumUnused(int i) const   {return m_vpL2L[i]->countOfState(L2List::PASSIVE);};

    // for debugging
    int checkLists(bool bDisplayArray=false);
    void calcHolyness();
    void displayArray(uint iLayer, int iFirst, int iLast);
    bool hasState(int iState, int iIndex);

    const L2List *getL2List(uint iLayer);
    int setL2List(const L2List *pL2List, uint iLayer);

    int    getBufSize(int iDumpMode);
    uchar *serialize(uchar *pBuf);
    int    deserialize(uchar *pBuf);
    
    static const int NIL       = L2List::NIL;
    static const uchar PASSIVE = L2List::PASSIVE;
    static const uchar ACTIVE  = L2List::ACTIVE;

    static const int DUMP_MODE_NONE  = L2List::DUMP_MODE_NONE;
    static const int DUMP_MODE_FLAT  = L2List::DUMP_MODE_FLAT;
    static const int DUMP_MODE_SMART = L2List::DUMP_MODE_SMART;
    static const int DUMP_MODE_FREE  = L2List::DUMP_MODE_FREE;


protected:
    uint m_iLayerSize;
    int compactLayers();

    uint m_iNumUsed;

    std::vector<L2List *> m_vpL2L;
    std::vector<LBBase *> m_vpLB;
};

#endif
