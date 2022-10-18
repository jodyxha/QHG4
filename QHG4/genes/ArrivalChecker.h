#ifndef __ARRIVALCHECKER_H__
#define __ARRIVALCHECKER_H__

#include <set>
#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "AnalysisUtils.h"

struct celldistnum {
    int    m_iCellID;
    double m_dDist;
    int    m_iNumAgents;

    celldistnum(int iCellID, double dDist, int iNumAgents):m_iCellID(iCellID),m_dDist(dDist),m_iNumAgents(iNumAgents){};
};

typedef std::pair<int,double>                         celldist;
typedef std::map<std::string, std::vector<celldist> > loccelldists;                 
typedef std::map<std::string, celldistnum *>          loccelldist;                 

typedef struct aginfo_t {
    idtype   m_ulID;
    gridtype m_ulCellID;
} aginfo;

typedef std::map<std::string, std::pair<size_t, aginfo*>> popaginfos;
typedef std::map<std::string, int *> popcounts;

const int ARR_NONE  =  0;
const int ARR_NAME  =  1;
const int ARR_LON   =  2;
const int ARR_LAT   =  4;
const int ARR_TIME  =  8;
const int ARR_COUNT = 16;
const int ARR_ALL   = 255;


class ArrivalChecker {
public:
    static ArrivalChecker *createInstance(const std::string sDFGrid, const std::string sQDFStats, const std::string sLocFile, double dDistance, FILE *fOut);

    ~ArrivalChecker();
    int init(const std::string sQDFGrid, const std::string sQDFStats, const std::string sLocFile, double dDistance);
    int findClosestCandidates(bool bSpherical);
    void showTable(bool bFormat, bool bSort);
    void showCSV(bool bHead, int iWhat);
private:
    ArrivalChecker(FILE *fOut);
    int fillCoordMap(const std::string sQDFGeoGrid);
    int readStats(const std::string sQDFStats, const std::string sSpecies);
    int calcSphericalDistances();
    int calcCartesianDistances();
    int loadAgentsCell(const std::string sPopFile, const std::string sSpecies);
    int getCellAgentCounts();
    int getCountsForCells(std::vector<celldist> &v);
    void deleteArrays();
    
    uint m_iNumCells;
    arrpos_coords m_mCoords;
    int    *m_pCellIDs;
    loc_data m_mLocData;
    double *m_pTravelTimes;
    double *m_pTravelDists;

    loccelldists *m_pmCandidates;
    loccelldist   m_mFinalCellDist; 
    stringvec    m_vNames;
    
    int          m_iNumAgents;
    int          m_iTotal;
    aginfo      *m_pInfos;
    int         *m_pCounts;

    bool         m_bEmptyOutput;
    FILE        *m_fOut;
};

#endif
