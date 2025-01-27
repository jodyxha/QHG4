/*****************************************************************************
 * SCellGrid manages a fixed size array of SCell structs
 *
\****************************************************************************/ 

#ifndef __SCELLGRID_H__
#define __SCELLGRID_H__

class OccTracker;

#include <cstdlib>
#include <unistd.h>
#include <string>
#include <map>

#include <hdf5.h>

#include "icoutil.h"
#include "SCell.h"

#include "Environment.h"
#include "Geography.h"
#include "Climate.h"
#include "Vegetation.h"
#include "Navigation.h"
#include "Navigation2.h"
//#include "OccTracker.h"
#include "GraphDesc.h"

#include "surfconsts.h"


const int GRID_TYPE_NONE  =  -1;
const int GRID_TYPE_FLAT4 =   0;
const int GRID_TYPE_FLAT6 =   1;
const int GRID_TYPE_ICO   =   2;
const int GRID_TYPE_IEQ   =   3;
const int GRID_TYPE_GRAPH =   4;



class SCellGrid {
public:
    static SCellGrid *createInstance(GraphDesc *pGD);

    SCellGrid(int iID, uint iNumCells, const stringmap &smSurfaceData);    
    ~SCellGrid();

    int         m_iID;
    uint        m_iNumCells;
    SCell       *m_aCells;
    int         m_iMaxNeighbors;

    // general grid info
    stringmap m_smSurfaceData;
    int       m_iType;
    int       m_iConnectivity;

    // map ID -> index in m_apCells
    std::map<gridtype, int>   m_mIDIndexes;

    // geographical data
    Geography  *m_pGeography;
    // climate data
    Climate    *m_pClimate;
    //vegetation stuff
    Vegetation *m_pVegetation;
    // navigation
    Navigation *m_pNavigation;
    // occupation tracking
    Navigation2 *m_pNavigation2;
    // occupation tracking
    OccTracker *m_pOccTracker;


    std::map<std::string, Environment*> m_mEnvironments;

    void setGeography(Geography* pGeo);
    void setClimate(Climate* pClim);
    void setVegetation(Vegetation* pVeg);
    void setNavigation(Navigation* pNav);
    void setNavigation2(Navigation2* pNav);
    void setOccTracker(OccTracker* pOcc);
 
    void delGeography();
    void delClimate();
    void delVegetation();
    void delNavigation();
    void delNavigation2();
    void delOckTracker();
 
    bool isCartesian();   


    // (may be removed later)
    void display();
};

#endif
