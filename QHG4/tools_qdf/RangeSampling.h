#ifndef __RANGESAMPLING_H__
#define __RANGESAMPLING_H__

#include <vector>
#include <map>
#include "Sampling.h"

typedef std::vector<std::pair<int, double>> pointrad;

class RangeSampling : public Sampling {
public:
    RangeSampling(int iNumCells, pointrad &vDisks, double *dLon, double *dLat, double dScale, bool bSpher);
    virtual ~RangeSampling();

protected:
    int m_iNumCells;
    double *m_dLon;
    double *m_dLat;
    bool    m_bSpher;
    double (*m_distFunc)(double, double, double, double, double);
}; 


#endif
