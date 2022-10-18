#ifndef __OCCANALYZER___
#define __OCCANALYZER___

#include "xPopLooper.h" 
#include "OccHistory.h"

class OccAnalyzer {
public:
    OccAnalyzer(OccHistory *pOH, const stringvec &vPopNames);
    ~OccAnalyzer();

    void showBitOrder();
    void showHistory(int iCellID);
    void showHistory(const timed_bits &tb);
    uint occupationAtTimeIndex(float fT, int iCellIndex);
    uint *occupationAtTime(float fT, uint *pVals);
    uint occupationAtTime(float fT, int iCellD);
    int  occupationOfRegion(intvec  &vCells, timed_bits &tb);
    uint *occupationOfRegionAtTime(intvec &vCells);

    int translateIDs(intvec &vCellIDs, intvec &vCellIndexes);
    timed_bits *removeZeroStates();
    timed_bits *mergeRegionStates(intvec &vRegionIDs, bool bIgnoreZeros);
    const intvec &getCellIDs() {return m_pOH->getCellIDs();};
protected:

    OccHistory *m_pOH;
    const stringvec m_vPopNames;
};


#endif
