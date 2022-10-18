#ifndef __CELL_BIRTHS_H__
#define __CELL_BIRTHS_H__

#include <map>
#include <set>

#include <hdf5.h>

#include "types.h"

typedef std::map<float, std::map<gridtype, int>>  birthcounts;

class CellBirths {
public:
    static CellBirths *createInstance(int iNumCells);
    ~CellBirths();    
    void addBirth(float fStep, gridtype iCellID); 

    int writeDataQDF(hid_t hGroup);
    void showAll();

    void reset();
protected:
    CellBirths();
    int init(uint iNumCells);
    void condense();

    uint m_iNumCells;
    uint        *m_pCounts;
    birthcounts m_mBirthCounts;
    bool bCondensed;
};


#endif
