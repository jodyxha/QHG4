#ifndef __NAVIGATION_H__
#define __NAVIGATION_H__

#include <omp.h>

#include "types.h"
#include "Environment.h"

// target node => distance
typedef std::map<gridtype, double>   distlist;

// start grid node => distance list
typedef std::map<gridtype, distlist> distancemap;

// for manual bridges
typedef std::pair<gridtype,gridtype> bridgedef;
typedef std::vector<bridgedef>        bridgelist;

class Navigation : public Environment {
public:
    Navigation();
    virtual ~Navigation();
    int setData(const distancemap &mDests, double dSampleDist);
    int setBridges(const bridgelist &vBridges);
    int checkSizes(uint iNumPorts, uint iNumDests, uint iNumDists);
    virtual void resetUpdated() {};

    uint     m_iNumPorts;
    uint     m_iNumDests;
    uint     m_iNumDists;
    double   m_dSampleDist;
    uint     m_iNumBridges;

    distancemap m_mDestinations;
    bridgelist  m_vBridges;
};



#endif
