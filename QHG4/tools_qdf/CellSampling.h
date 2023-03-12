#ifndef __CELLSAMPLING_H__
#define __CELLSAMPLING_H__

#include <vector>
#include <map>
#include "Sampling.h"

class CellSampling : public Sampling {
public:
    CellSampling(int iNumCells);
    virtual ~CellSampling();

protected:
    int m_iNumCells;

}; 


#endif
