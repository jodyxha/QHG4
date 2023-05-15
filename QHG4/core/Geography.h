#ifndef __GEOGRAPHY_H__
#define __GEOGRAPHY_H__

#include <string>

#include "Environment.h"

class SCellGrid;

typedef double geonumber;

const std::string ENV_GEO     = "geography";

class Geography : public Environment {
public:
    Geography(SCellGrid *pCG, uint iNumCells, uint iMaxNeighbors, geonumber dRadius, geonumber dSeaLevel=0);
    Geography(SCellGrid *pCG);
    Geography();
    virtual ~Geography();

    int init(uint iNumCells, uint iMaxNeighbors, geonumber dRadius, geonumber dSeaLevel=0);


    //global values
    bool m_bUpdated;
    uint m_iNumCells;
    uint m_iMaxNeighbors;
    geonumber   m_dRadius;      // "real" earth radius 
    geonumber   m_dSeaLevel;    // sea level (may change, depending on climate)
    // cell specific values
    geonumber  *m_adLatitude;   // "real" latitude of cell center  (radians) 
    geonumber  *m_adLongitude;  // "real" longitude of cell center (radians) 
    geonumber  *m_adAltitude;   // "real" altitude of cell center  
    geonumber  *m_adDistances;  // distances to neighbors: distances from cell k at m_aadDistances[N*k, N*k+N-1], where N=max #neighbors
    geonumber  *m_adArea;       // area of cell
    bool       *m_abIce;        // ice (1) or no ice (0)
    geonumber  *m_adWater;      // presence of water (rivers in cell)
    bool       *m_abCoastal;    // coast in neighborhood
    geonumber  *m_adAngles;     // orientation of direction to neighbors: 0->east, pi/2->north, pi->west, 3pi/2 south
                                // this must be calculated
  
    // additionals
    void calcAngles();
	inline void resetUpdated() { m_bUpdated = false; };
};

#endif
 
