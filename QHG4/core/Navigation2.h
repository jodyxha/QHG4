#ifndef __NAVIGATION2_H__
#define __NAVIGATION2_H__

#include <omp.h>

#include "types.h"
#include "Environment.h"

const std::string ENV_NAV2 = "navigation2";

class CellGrid;
// target node => distance
typedef std::map<gridtype, double>   destdistmap;

// start grid node => distance list
typedef std::map<gridtype, destdistmap> waterwaymap;

// for manual bridges
typedef std::pair<gridtype,gridtype> bridgedef;
typedef std::vector<bridgedef>       bridgelist;

class Navigation2 : public Environment {
public:
    Navigation2(SCellGrid *pCG);
    virtual ~Navigation2();
    int setData(const waterwaymap &mWaterWays/*, uint iNumWaterWays*/, double dSampleDist);
    int setBridges(const bridgelist &vBridges);
    //int checkSizes(uint iNumPorts, uint iNumDests, uint iNumDists);
    virtual void resetUpdated() {};

    uint     m_iNumCells;
    uint     m_iNumWaterWays;
    double   m_dSampleDist;
    uint     m_iNumBridges;

    waterwaymap   m_mWaterWays;
    bridgelist    m_vBridges;
};



#endif
