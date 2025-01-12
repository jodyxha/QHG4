#ifndef __COORDRANGESAMPLING_H__
#define __COORDRANGESAMPLING_H__

#include <vector>
#include <map>

#include "strutils.h"
#include "xha_strutilsT.h"

#include "geomutils.h"
#include "Sampling.h"
#include "CellRangeSampling.h"

class coorddata {
public:
    coorddata(double dX, double dY, double dRange)
        :m_dX(dX), m_dY(dY), m_dRange(dRange) {};

    double m_dX;
    double m_dY;
    double m_dRange;
};

typedef std::vector<coorddata> coordrad;

class CoordRangeSampling : public CellRangeSampling {
public:
    CoordRangeSampling(int iNumCells, double **apCoords, double dScale, bool bSpher);
    virtual ~CoordRangeSampling();
   
    virtual int setRangeDescription(void *pDescr);

    
protected:

}; 


#endif
