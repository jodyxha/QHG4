#ifndef __CELLREGIONS_H__
#define __CELLREGIONS_H__

#include "types.h"
#include "SCellGrid.h"

class CellRegions {
public:
    static CellRegions *createInstance(SCellGrid *pCG, double dRadius);
    int findInside(double dLon, double dLat, double dDist, intset &sCells);
    virtual ~CellRegions();
protected:
    CellRegions(SCellGrid *pCG, double dRadius);
    int init();

    SCellGrid *m_pCG;
    double      m_dRadius;
 
    intset *m_asCandidates;
};


#endif
