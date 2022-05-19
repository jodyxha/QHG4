#ifndef __OCCHISTORY_H__
#define __OCCHISTORY_H__

#include <vector>

#include "types.h"

// a uint has 32 bits
const int MAX_POPS  = 8;
const int MSK_CROP  = (0xffffffff>>(32-MAX_POPS));
const int NEG_INF   = ((int)0xffffffff);
const int NEG_INF_C = ((int)(0xffffffff>>MAX_POPS));

typedef std::map<int, uchar> timed_bits;
typedef std::map<int,int>    intintmap;
typedef std::vector<uchar *> ucharpvec;


class OccHistory {
public:
    static OccHistory *createInstance(intvec &vCellIDs, int iNumPops);
    virtual ~OccHistory();
    int setNumPops(int iNumPops);
    int addEntries(int iStep, ucharpvec &pvCounts);
    int addEntry(int iStep, int iCellID, uchar *pCounts);
    int addEntry(int iStep, int iCellIndex, uchar uMask, bool bIgnoreZero);
    const timed_bits* getData() {return m_pTB;};
    const intvec &getCellIDs() {return m_vCellIDs;};
    int getDataSize() { return m_iDataSize;};
    int ID2Idx(int iID);
    uchar*serialize();
    int deserialize(uchar *pBuf);
protected:
    OccHistory(intvec vCellIDs, int iNumPops);
    int  init();
   
    void initializeCounts(float fT);

    intintmap   m_mID2Pos;
    intvec      m_vCellIDs;
    int         m_iNumPops;
    timed_bits *m_pTB;
    int         m_iDataSize;
};
#endif
