#ifndef __OCCTRACKER_H__
#define __OCCTRACKER_H__

#include <map>
#include <vector>

#include "types.h"
#include "PopBase.h"
#include "PopLooper.h"
#include "OccHistory.h"



class OccTracker {
public:
    static OccTracker *createInstance(intvec &vCellIDs, PopLooper *pPL);
    virtual ~OccTracker();

    int updateCounts(float fT);
    //not needed    void finalizeCounts();
                                      
    uchar*serialize();
    int deserialize(uchar *pBuf);
    int getOccDataSize() { return m_pOH->getDataSize();}; // for OccWriter
    stringvec &getPopNames() { return m_vPopNames;};
protected:
    OccTracker(intvec vCellIDs);
    int  init(PopLooper *pPL);
    uint calcBitMap(int iCellID);

    intvec       m_vCellIDs;
    popvec       m_vPops;
    stringvec    m_vPopNames;

    OccHistory *m_pOH;
};

 
#endif

