#ifndef __EACHSAMPLING_H__
#define __EACHSAMPLING_H__

#include <vector>
#include <map>
#include "Sampling.h"

class EachSampling : public Sampling {
public:
    EachSampling(int iNumCells);
    virtual ~EachSampling();

protected:
    int m_iNumCells;

}; 


#endif
