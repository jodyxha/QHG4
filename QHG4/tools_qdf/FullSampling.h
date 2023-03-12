#ifndef __FULLSAMPLING_H__
#define __FULLSAMPLING_H__

#include <vector>
#include <map>
#include "Sampling.h"

class FullSampling : public Sampling {
public:
    FullSampling(int iNumCells);
    virtual ~FullSampling();

protected:
    int m_iNumCells;

}; 


#endif
