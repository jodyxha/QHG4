#ifndef __QDFUTILS_H__
#define __QDFUTILS_H__

#include <vector>
#include <string>
#include <hdf5.h>

typedef std::vector<std::string> stringvec;

// this macro is rewuired to calculate offsets of fields in a derived struct
#define qoffsetof(S,M) (ulong(&(S.M))   - ulong(&(S)))

#define POPGROUP_NAME    "Populations"
#define GEOGROUP_NAME    "Geography"
#define CLIGROUP_NAME    "Climate"
#define VEGGROUP_NAME    "Vegetation"
#define GRIDGROUP_NAME   "Grid"
#define MSTATGROUP_NAME  "MoveStatistics"
#define NAVGROUP_NAME    "Navigation"
#define SUBPOPGROUP_NAME "SubPopulations"
#define PIEGROUP_NAME    "PiePlots"
#define VECGROUP_NAME    "VectorData"

#define AGENT_DATASET_NAME "AgentDataSet"
#define CELL_DATASET_NAME  "CellDataSet"
#define PIE_DATASET_NAME   "PieDataSet"
#define VEC_DATASET_NAME   "VectorDataSet"

#define ROOT_ATTR_NAME "QHG"
#define ROOT_TIME_NAME "Time"
    

#define GRID_STYPE_ICO  "ICO"
#define GRID_STYPE_HEX  "HEX"
#define GRID_STYPE_RECT "RECT"

#define GRID_ATTR_NUM_CELLS   "NumCells"
#define GRID_ATTR_TYPE        "GridType"
#define GRID_ATTR_FORMAT      "GridFormat"
#define GRID_ATTR_PERIODIC    "Periodic"

#define GRID_DS_CELL_ID       "CellID"
#define GRID_DS_NUM_NEIGH     "NumNeighbors"
#define GRID_DS_NEIGHBORS     "Neighbors"
 
#define GEO_ATTR_NUM_CELLS    "NumCells"
#define GEO_ATTR_MAX_NEIGH    "MaxNeigh"
#define GEO_ATTR_RADIUS       "Radius"
#define GEO_ATTR_SEALEVEL     "SeaLevel"

#define GEO_DS_LONGITUDE      "Longitude"
#define GEO_DS_LATITUDE       "Latitude"
#define GEO_DS_ALTITUDE       "Altitude"
#define GEO_DS_AREA           "Area"
#define GEO_DS_DISTANCES      "Distances"
#define GEO_DS_ICE_COVER      "IceCover"
#define GEO_DS_WATER          "Water"
#define GEO_DS_COASTAL        "Coastal"
#define GEO_DS_ANGLES         "Angles"
#define GEO_DS_DIRS           "Directions"

#define CLI_ATTR_NUM_CELLS    "NumCells"
#define CLI_ATTR_NUM_SEASONS  "NumSeasons"
#define CLI_ATTR_DYNAMIC      "Dynamic"

#define CLI_DS_ACTUAL_TEMPS   "ActualTemps"
#define CLI_DS_ACTUAL_RAINS   "ActualRains"
#define CLI_DS_ANN_MEAN_TEMP  "AnnualMeanTemp"
#define CLI_DS_ANN_TOT_RAIN   "AnnualRainfall"
#define CLI_DS_SEAS_TEMP_DIFF "SeasonalTempDiff" 
#define CLI_DS_SEAS_RAIN_RAT  "SeasonalRainRatios"
#define CLI_DS_CUR_SEASON     "CurSeason"

#define VEG_ATTR_NUM_CELLS    "NumCells"
#define VEG_ATTR_NUM_SPECIES  "NumSpecies"
#define VEG_ATTR_DYNAMIC      "Dynamic"

#define VEG_DS_MASS           "Mass"
#define VEG_DS_BASE_NPP       "BaseNPP"
#define VEG_DS_NPP            "NPP"


#define MSTAT_ATTR_NUM_CELLS   "NumCells"
#define MSTAT_DS_HOPS          "Hops"
#define MSTAT_DS_DIST          "Dist"
#define MSTAT_DS_TIME          "Time"

#define NAV_ATTR_NUM_PORTS     "NumPorts"
#define NAV_ATTR_NUM_DESTS     "NumDests"
#define NAV_ATTR_NUM_DISTS     "NumDists"
#define NAV_ATTR_SAMPLE_DIST   "SampleDist"
#define NAV_ATTR_NUM_BRIDGES   "NumBridges"
#define NAV_DS_MULTIPLICITIES  "Multiplicities"
#define NAV_DS_DEST_IDS        "DestIDs"
#define NAV_DS_DISTANCES       "Distances"
#define NAV_DS_BRIDGES         "Bridges"

#define SPOP_ATTR_CLASS_ID     "ClassID"
#define SPOP_ATTR_CLASS_NAME   "ClassName"
#define SPOP_ATTR_SPECIES_ID   "SpeciesID"
#define SPOP_ATTR_SPECIES_NAME "SpeciesName"
#define SPOP_ATTR_SENS_DIST    "SensingDistance"
#define SPOP_ATTR_NUM_CELL     "NumCells"
#define SPOP_ATTR_PRIO_INFO    "PrioInfo"

#define POP_DS_CAPACITY        "Capacities"
#define POP_DS_HOPS            "Hops"
#define POP_DS_DIST            "Dist"
#define POP_DS_TIME            "Time"
#define POP_DS_HYBR            "Hybridization"
#define POP_DS_MEDIANS         "Medians"

#define PIE_ATTR_NUM_PIES      "NumPies"
#define PIE_ATTR_NUM_VALS      "NumVals"
#define PIE_ATTR_NUM_DIMS      "NumDims"
#define PIE_ATTR_VAL_NAMES     "ValNames"
#define PIE_DS_DATA            "PieData"

#define VEC_ATTR_NUM_VECS      "NumVecs"


hid_t qdf_openFile(const char *pFileName);
void  qdf_closeFile(hid_t hFile);

hid_t qdf_openGroup(hid_t hGroup, const char *pGroupName, bool bForceCheck=true);
void  qdf_closeGroup(hid_t hGroup);

void  qdf_closeDataSet(hid_t hDataSet);
void  qdf_closeDataSpace(hid_t hDataSpace);
void  qdf_closeDataType(hid_t hDataType);
void  qdf_closeAttribute(hid_t hDataSpace);

bool qdf_link_exists(hid_t hGroup, const char *pName);
bool qdf_attr_exists(hid_t hGroup, const char *pName);
 
int qdf_extractSAttribute(hid_t hLoc, const char *pName, uint iNum, char   *sValue); 

int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, char   *cValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, int    *iValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, uint   *iValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, long   *lValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, float  *fValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, double *dValue); 


int qdf_readArray(hid_t hGroup, const char *pName, int iNum, double *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, float *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, int *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, char *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, ushort *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData);

int qdf_readArrays(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, unsigned char *pData);

int qdf_readArraySlabT(hid_t hGroup, const char *pName, int iSize, int iOffset, int iStride, int iBlock, double *pData);


int qdf_listGroupContents(hid_t loc_id, const char *pName, const char *pIndent);
int qdf_listAttributes(hid_t loc_id, const char *pName, const char *pIndent);


int collectNumericDataSets(hid_t hPopGroup, stringvec &vNames);
int collectSubGroups(hid_t hPopGroup, stringvec &vNames);
int qdf_extractSAttribute2(hid_t hLoc, const std::string sName, std::string &sValue);

#endif
