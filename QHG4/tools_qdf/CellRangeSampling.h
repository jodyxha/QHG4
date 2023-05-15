#ifndef __CELLRANGESAMPLING_H__
#define __CELLRANGESAMPLING_H__

#include <vector>
#include <map>
#include "Sampling.h"

typedef std::vector<std::pair<int, double>> cellrad;

class CellRangeSampling : public Sampling {
public:
    CellRangeSampling(int iNumCells, double **apCoords, double dScale, bool bSpher);
    virtual ~CellRangeSampling();

    virtual int setRangeDescription(void *pDescr);

protected:
    int m_iNumCells;
    double **m_apCoords;
    double (*m_distFunc)(double, double, double, double, double);
    double m_dScale;
    bool    m_bSpher;
}; 


#endif
