#ifndef __OCCTRACKER_H__
#define __OCCTRACKER_H__

#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "PopBase.h"
#include "PopLooper.h"
#include "Environment.h"
#include "OccHistory.h"

const std::string ENV_OCC = "occtracker";

class OccTracker : public Environment {
public:
    static OccTracker *createInstance(SCellGrid *pCG, intvec &vCellIDs, PopLooper *pPL);
    virtual ~OccTracker();

    int updateCounts(float fT);
    //not needed    void finalizeCounts();
                                      
    uchar*serialize();
    int deserialize(uchar *pBuf);
    int getOccDataSize() { return m_pOH->getDataSize();}; // for OccWriter
    stringvec &getPopNames() { return m_vPopNames;};

    // for Environment
    virtual void resetUpdated(){};

protected:
    OccTracker(SCellGrid *pCG, intvec vCellIDs);
    int  init(PopLooper *pPL);
    uint calcBitMap(int iCellID);

    intvec       m_vCellIDs;
    popvec       m_vPops;
    stringvec    m_vPopNames;

    OccHistory *m_pOH;
};

 
#endif

