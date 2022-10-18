#ifndef __MOVESTATS_OLD_H__
#define __MOVESTATS_OLD_H__

#include <hdf5.h>
#include <omp.h>

#include <set>
#include "types.h"

/*
typedef struct statentry {
    double m_dDist;
    int    m_iHops;
    double m_dTime;
    statentry() : m_dDist(0), m_iHops(-1), m_dTime(-1) {};
    statentry(double dDist, int iHops, double dTime) : m_dDist(dDist), m_iHops(iHops), m_dTime(dTime) {};
} statentry;

typedef std::map<int, statentry> newstats;
*/

class MoveStats_old {
public:
    MoveStats_old();
    MoveStats_old(uint iNumCells);
    virtual ~MoveStats_old();

    int init();

    int addStatCandidates(int iCell, double dDist, int iHops, double dTime);
    int insertNewStats(double fCurTime);

    int readDataQDF(hid_t hGroup);
    int writeDataQDF(hid_t hGroup);
    
    uint m_iNumCells;
    

    int    *m_aiHops;
    double *m_adDist;
    double *m_adTime;

    int    *m_aiHopsTemp;
    double *m_adDistTemp;
    intset *m_asChanged;
  
};

#endif
