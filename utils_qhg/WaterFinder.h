#ifndef __WATERFINDER_H__
#define __WATERFINDER_H__

#include <vector>

#include "SCellGrid.h"

typedef std::vector<int> nodelist;

class WaterFinder {
public:
    
    static WaterFinder *createInstance(const char *pQDF);

    int collectNodes(double dLonMin, double dLatMin, double dLonMax, double dLatMax);
    virtual ~WaterFinder();
    const nodelist &getWaterNodes() {return m_vWater;};
    const nodelist &getNeighNodes() {return m_vNeigh;};

protected:
    WaterFinder();
    int init(const char *pQDF);
    int createCellGrid(const char *pQDFFile);
    int findWaterNodes(double dLonMin, double dLatMin, double dLonMax, double dLatMax);
    int findWaterNeighbors();
    
    int        m_iNumNodes;
    SCellGrid *m_pCG;
    double    *m_adWater;
    nodelist   m_vWater;
    nodelist   m_vNeigh;

};

#endif
