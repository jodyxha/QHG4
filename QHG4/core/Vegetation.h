#ifndef __VEGETATION_H__
#define __VEGETATION_H__

#include <string>
#include "Environment.h"

class Geography;
class Climate;
class SCellGrid;

typedef double         veginumber;

const std::string ENV_VEG     = "vegetation";


class Vegetation : public Environment {
 public:
    Vegetation(SCellGrid *pCG, uint iNumCells, int iNumVegSpecies);
    virtual ~Vegetation();
    
    uint m_iNumCells;
    int m_iNumVegSpecies;
    
    double *m_adBaseANPP;
    double *m_adTotalANPP;
    
    int update(float fTime);
    int climateUpdate(float fTime);
    void resetUpdated() {};
 protected:
    double m_fPreviousTime;    

};




#endif
