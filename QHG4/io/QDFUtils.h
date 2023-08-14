#ifndef __QDFUTILS_H__
#define __QDFUTILS_H__

#include <hdf5.h>
#include <string>
#include <vector>
#include "types.h"
#include "PolyLine.h"

// this macro is rewuired to calculate offsets of fields in a derived struct
#define qoffsetof(S,M) (ulong(&(S.M))   - ulong(&(S)))

const std::string POPGROUP_NAME   = "Populations";
const std::string GEOGROUP_NAME   = "Geography";
const std::string CLIGROUP_NAME   = "Climate";
const std::string VEGGROUP_NAME   = "Vegetation";
const std::string GRIDGROUP_NAME  = "Grid";
const std::string MSTATGROUP_NAME = "MoveStatistics";
const std::string NAVGROUP_NAME   = "Navigation";
const std::string OCCGROUP_NAME   = "Occupation";
const std::string SUBPOPGROUP_NAME = "SubPopulations";


const std::string AGENT_DATASET_NAME   = "AgentDataSet";
const std::string CELL_DATASET_NAME    = "CellDataSet";
const std::string CTRL_DATASET_NAME    = "Controller";
const std::string DEAD_DATASET_NAME    = "DeadSpaces";
const std::string GENOME_DATASET_NAME  = "Genome";
const std::string PHENOME_DATASET_NAME = "Phenome";


const std::string ROOT_ATTR_NAME = "QHG";
const std::string ROOT_STEP_NAME = "Step";
const std::string ROOT_TIME_NAME = "StartTime";
const std::string ROOT_INFO_NAME = "Info";
    

const std::string GRID_STYPE_ICO  = "ICO";
const std::string GRID_STYPE_IEQ  = "IEQ";
const std::string GRID_STYPE_HEX  = "HEX";
const std::string GRID_STYPE_RECT = "RECT";

const std::string GRID_ATTR_NUM_CELLS   = "NumCells";
const std::string GRID_ATTR_SURF_TYPE   = "SURF_TYPE";
const std::string GRID_ATTR_TYPE        = "GridType";
const std::string GRID_ATTR_FORMAT      = "GridFormat";
const std::string GRID_ATTR_PERIODIC    = "Periodic";

const std::string GRID_DS_CELL_ID       = "CellID";
const std::string GRID_DS_NUM_NEIGH     = "NumNeighbors";
const std::string GRID_DS_NEIGHBORS     = "Neighbors";
 
const std::string GEO_ATTR_NUM_CELLS    = "NumCells";
const std::string GEO_ATTR_MAX_NEIGH    = "MaxNeigh";
const std::string GEO_ATTR_RADIUS       = "Radius";
const std::string GEO_ATTR_SEALEVEL     = "SeaLevel";

const std::string GEO_DS_LONGITUDE      = "Longitude";
const std::string GEO_DS_LATITUDE       = "Latitude";
const std::string GEO_DS_ALTITUDE       = "Altitude";
const std::string GEO_DS_AREA           = "Area";
const std::string GEO_DS_DISTANCES      = "Distances";
const std::string GEO_DS_ICE_COVER      = "IceCover";
const std::string GEO_DS_WATER          = "Water";
const std::string GEO_DS_COASTAL        = "Coastal";
const std::string GEO_DS_ANGLES         = "Angles";
const std::string GEO_DS_DIRS           = "Directions";

const std::string CLI_ATTR_NUM_CELLS    = "NumCells";
const std::string CLI_ATTR_NUM_SEASONS  = "NumSeasons";

const std::string CLI_DS_ACTUAL_TEMPS   = "ActualTemps";
const std::string CLI_DS_ACTUAL_RAINS   = "ActualRains";
const std::string CLI_DS_ANN_MEAN_TEMP  = "AnnualMeanTemp";
const std::string CLI_DS_ANN_TOT_RAIN   = "AnnualRainfall";
const std::string CLI_DS_SEAS_TEMP_DIFF = "SeasonalTempDiff"; 
const std::string CLI_DS_SEAS_RAIN_RAT  = "SeasonalRainRatios";
const std::string CLI_DS_CUR_SEASON     = "CurSeason";

const std::string VEG_ATTR_NUM_CELLS    = "NumCells";
const std::string VEG_ATTR_NUM_SPECIES  = "NumSpecies";

const std::string VEG_DS_MASS           = "Mass";
const std::string VEG_DS_BASE_NPP       = "BaseNPP";
const std::string VEG_DS_NPP            = "NPP";

const std::string MSTAT_ATTR_NUM_CELLS   = "NumCells";
const std::string MSTAT_DS_HOPS          = "Hops";
const std::string MSTAT_DS_DIST          = "Dist";
const std::string MSTAT_DS_TIME          = "Time";

const std::string NAV_ATTR_NUM_PORTS     = "NumPorts";
const std::string NAV_ATTR_NUM_DESTS     = "NumDests";
const std::string NAV_ATTR_NUM_DISTS     = "NumDists";
const std::string NAV_ATTR_SAMPLE_DIST   = "SampleDist";
const std::string NAV_ATTR_NUM_BRIDGES   = "NumBridges";
const std::string NAV_DS_MULTIPLICITIES  = "Multiplicities";
const std::string NAV_DS_DEST_IDS        = "DestIDs";
const std::string NAV_DS_DISTANCES       = "Distances";
const std::string NAV_DS_BRIDGES         = "Bridges";

const std::string OCC_ATTR_POP_NAMES     = "PopNames";
const std::string OCC_DS_OCCTRACK        = "OccTrack";

const std::string SPOP_QDF_VERSION       = "QDFVersion";
const std::string SPOP_ATTR_CLASS_NAME   = "ClassName";
const std::string SPOP_ATTR_SPECIES_NAME = "SpeciesName";
const std::string SPOP_ATTR_NUM_CELL     = "NumCells";
const std::string SPOP_ATTR_PRIO_INFO    = "PrioInfo";
const std::string SPOP_ATTR_INIT_SEED    = "InitialSeed";
const std::string SPOP_DS_CAP            = "Capacities";
const std::string SPOP_DS_HOPS           = "Hops";
const std::string SPOP_DS_DIST           = "Dist";
const std::string SPOP_DS_TIME           = "Time";

// optional
const std::string SPOP_ATTR_NUM_STATES   = "WELLNumStates";
const std::string SPOP_ATTR_CUR_INDEX    = "WELLCurIndex";
const std::string SPOP_ATTR_FINAL_WELL   = "WELLFinalState";
const std::string SPOP_ATTR_IDGEN_STATE  = "IDGenState";

const int DS_TYPE_NONE    =  0;
const int DS_TYPE_CHAR    =  1;
const int DS_TYPE_SHORT   =  2;
const int DS_TYPE_INT     =  3;
const int DS_TYPE_LONG    =  4;
const int DS_TYPE_LLONG   =  5;
const int DS_TYPE_UCHAR   =  6;
const int DS_TYPE_USHORT  =  7;
const int DS_TYPE_UINT    =  8;
const int DS_TYPE_ULONG   =  9;
const int DS_TYPE_ULLONG  = 10;
const int DS_TYPE_FLOAT   = 11;
const int DS_TYPE_DOUBLE  = 12;
const int DS_TYPE_LDOUBLE = 13;

const int CMP_ERR_NONE      =  0;
const int CMP_ERR_UNKNOWN   = -1;
const int CMP_ERR_NUM_ELEMS = -2;
const int CMP_ERR_OFFSET    = -3;
const int CMP_ERR_CLASS     = -4;
const int CMP_ERR_TYPE      = -5;
const int CMP_ERR_NAME      = -6;


hid_t qdf_createFile(const std::string sFileName, int fStep, float fStartTime, const std::string sInfoString);
hid_t qdf_openFile(const std::string sFileName, bool bRW=false);
hid_t qdf_opencreateFile(const std::string sFileName, int fStep, float fStartTime, const std::string sInfoString);
void  qdf_closeFile(hid_t hFile);

hid_t qdf_createGroup(hid_t hGroup, const std::string sGroupName);
hid_t qdf_openGroup(hid_t hGroup, const std::string sGroupName, bool bForceCheck=true);
hid_t qdf_opencreateGroup(hid_t hGroup, const std::string sGroupName, bool bForceCheck=true);
void  qdf_closeGroup(hid_t hGroup);
int   qdf_deleteGroup(hid_t hGroup, std::string sSubName);

hid_t qdf_openDataSet(hid_t hGroup, const std::string sGroupName, bool bForceCheck=true);
void  qdf_closeDataSet(hid_t hDataSet);
void  qdf_closeDataSpace(hid_t hDataSpace);
void  qdf_closeDataType(hid_t hDataType);
void  qdf_closeAttribute(hid_t hAttribute);

bool qdf_link_exists(hid_t hGroup, const std::string sName);
bool qdf_attr_exists(hid_t hGroup, const std::string sName);

int qdf_insertSAttribute(hid_t hLoc, const std::string sName, const std::string sValue); 
int qdf_insertSAttribute(hid_t hLoc, const std::string sName, const char  *pValue); 
template<typename T>
int qdf_insertSAttribute(hid_t hLoc, const std::string sName, const T &tValue);

std::string qdf_extractSAttribute(hid_t hLoc, const std::string sName); 
int qdf_extractSAttribute(hid_t hLoc, const std::string sName, std::string &sValue); 


int qdf_insertAttribute(hid_t hLoc, const std::string sName, const uint iNum, void *vValue, const hid_t hType);
template<typename T>
int qdf_insertAttribute(hid_t hLoc, const std::string sName, const uint iNum, T *tValue);


int qdf_extractAttribute(hid_t hLoc, const std::string sName, const uint iNum, void *vValue, const hid_t hType);
template<typename T>
int qdf_extractAttribute(hid_t hLoc, const std::string sName, const uint iNum, T *tValue);

template<typename T>
int qdf_readArray(hid_t hGroup, const std::string sName, const uint iNum, T *pData);

template<typename T>
int qdf_writeArray(hid_t hGroup, const std::string sName, const uint iNum, T *pData);

template<typename T>
int qdf_replaceArray(hid_t hGroup, const std::string sName, const uint iNum, T *pData);

int qdf_readArray(hid_t hGroup,    const std::string sName, const uint iNum, void *vData, const hid_t hType);
int qdf_replaceArray(hid_t hGroup, const std::string sName, const uint iNum, void *vData, const hid_t hType);
int qdf_writeArray(hid_t hGroup,   const std::string sName, const uint iNum, void *vData, const hid_t hType);

int qdf_readArrays(hid_t hGroup,  const std::string sName, const uint iNumArr, const uint iSize, double **pData);
int qdf_writeArrays(hid_t hGroup, const std::string sName, const uint iNumArr, const uint iSize, const double **pData);

int qdf_listGroupContents(hid_t loc_id, const std::string sName, char *pIndent);

int qdf_getDataType(hid_t hDataSet);
int qdf_getDataExtents(hid_t hGroup, const std::string sName, std::vector<hsize_t> &vSizes);


const std::string qdf_getFirstPopulation(const std::string sPopQDF);
const std::string qdf_checkForPop(const std::string sPopQDF, const std::string sSpeciesName);
bool  qdf_hasGeo(const std::string sQDF);

PolyLine *qdf_createPolyLine(hid_t hSpeciesGroup, const std::string sPLParName);
int qdf_writePolyLine(hid_t hSpeciesGroup, PolyLine *pPL, const std::string sPLParName);
int qdf_compareDataTypes(hid_t t1, hid_t t2);

int qdf_checkPathExists(const std::string sQDF, const std::string sPath);
int qdf_getSurfType(const std::string sQDF, std::string &sSurfType);

#endif
