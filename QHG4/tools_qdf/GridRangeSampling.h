#ifndef __GRIDRANGESAMPLING_H__
#define __GRIDRANGESAMPLING_H__

#include <vector>
#include <map>
#include "Sampling.h"
#include "CellRangeSampling.h"

class griddata {
public:
    griddata(double dXMin, double dXMax, double dXStep, double dYMin, double dYMax, double dYStep, double dRange) 
        : m_dXMin(dXMin),
          m_dXMax(dXMax),
          m_dXStep(dXStep),
          m_dYMin(dYMin),
          m_dYMax(dYMax),
          m_dYStep(dYStep),
          m_dRange(dRange){};

    double m_dXMin;
    double m_dXMax;
    double m_dXStep;
    double m_dYMin;
    double m_dYMax;
    double m_dYStep;
    double m_dRange;
};


typedef std::vector<griddata> griddef;

class GridRangeSampling : public CellRangeSampling {
public:
    GridRangeSampling(int iNumCells, double **apCoords, double dScale, bool bSpher);
    virtual ~GridRangeSampling();

    virtual int setRangeDescription(void *pDescr);

protected:
  

}; 



#endif
