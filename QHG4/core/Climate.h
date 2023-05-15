#ifndef __CLIMATE_H__
#define __CLIMATE_H__

#include "types.h"
#include "Observable.h"
#include "Environment.h"

class SCellGrid;

typedef uchar   climatecount;
typedef double  climatenumber;

class SCellGrid;
class Geography;

#define EVT_CLIMATE_CHANGE 1001

const std::string ENV_CLI     = "climate";

class Climate : public Environment, public Observable {
public:
    Climate(SCellGrid *pCG, uint iNumCells, climatecount iNumSeasons);

    virtual ~Climate();

    void prepareArrays();
    // void initSeasons(SeasonProvider *pspTemps, SeasonProvider *pspRains);

    void setSeason(climatecount iSeason=0);  
    
    inline void resetUpdated() { m_bUpdated = false; };

    bool         m_bUpdated;
    uint         m_iNumCells;        // number of cells
    climatecount m_iNumSeasons;      // number of seasons

    climatecount m_iSeasonMonths;    // season size in months (1,2,3,4,6,12)
    climatecount m_iCurSeason;       // current season 0,...m_iSeasonStep-1

    climatenumber  *m_adActualTemps;  // actual temperatures
    climatenumber  *m_adActualRains;  // actual seasonal rainfall

    
    climatenumber  *m_adAnnualMeanTemp;  // current annual mean temperature
    climatenumber  *m_adAnnualRainfall;  // current annual total rainfall
    climatenumber **m_aadSeasonalTempD;  // differences of seasonal temperature to mean annual temperature
    climatenumber **m_aadSeasonalRainR;  // ratios of seasonal rainfall to total annual rainfall

    climatenumber *m_adSeasTempDiff;
    climatenumber *m_adSeasRainRatio;
    // season

};

#endif
