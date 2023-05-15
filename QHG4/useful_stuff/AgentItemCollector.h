#ifndef __AGENTITEMCOLLECTOR_H__
#define __AGENTITEMCOLLECTOR_H__

#include <map>
#include <string>
#include "hdf5.h"

#include "types.h"
#include "QDFUtils.h"

typedef struct sampling_range_t {
    double  m_dLon;
    double  m_dLat;
    double  m_dRad;
    sampling_range_t(double dLon, double dLat, double dRad):m_dLon(dLon),m_dLat(dLat),m_dRad(dRad){};
} sampling_range;

typedef struct bin_context_t {
    std::string  m_sName;
    uint         m_iNumBins;
    double       m_dMinBin;
    double       m_dMaxBin;
    uint        *m_piBins;
    float        m_fMinNonZero;
    float        m_fMaxNonOne;
    uint         m_iNumZeros;
    uint         m_iNumOnes;
    uint         m_iNumAgents;
    uint         m_iNumChecked;
} bin_context;



typedef std::map<std::string, sampling_range_t*> named_sampling_ranges;

// map: cell id => coord pair
typedef std::map<int, std::pair<double, double> >    arrpos_coords;


typedef struct datatype_info_t {
    size_t m_iStructSize;
    size_t m_iOffset;
    hid_t  m_hType;
    datatype_info_t(): m_iStructSize(0),m_iOffset(0),m_hType(H5P_DEFAULT){};
    datatype_info_t(size_t iStructSize, size_t iOffset, hid_t hType): m_iStructSize(iStructSize),m_iOffset(iOffset),m_hType(hType){};
} datatype_info;

typedef std::map<std::string, datatype_info> datatype_infos;

typedef struct aginfo_uchar {
    aginfo_uchar(): m_ulID(0), m_ulCellID(0), m_tItem(0) {};
    aginfo_uchar(idtype ulID, gridtype ulCellID, uchar tItem): m_ulID(ulID), m_ulCellID(ulCellID), m_tItem(tItem) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    uchar    m_tItem;
} aginfo_uchar;
    

typedef struct aginfo_int {
    aginfo_int(): m_ulID(0), m_ulCellID(0), m_tItem(0) {};
    aginfo_int(idtype ulID, gridtype ulCellID, int tItem): m_ulID(ulID), m_ulCellID(ulCellID), m_tItem(tItem) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    int      m_tItem;
} aginfo_int;
    
typedef struct aginfo_long {
    aginfo_long(): m_ulID(0), m_ulCellID(0), m_tItem(0) {};
    aginfo_long(idtype ulID, gridtype ulCellID, long tItem): m_ulID(ulID), m_ulCellID(ulCellID), m_tItem(tItem) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    long     m_tItem;
} aginfo_long;
    

typedef struct aginfo_float {
    aginfo_float(): m_ulID(0), m_ulCellID(0), m_tItem(0) {};
    aginfo_float(idtype ulID, gridtype ulCellID, float tItem): m_ulID(ulID), m_ulCellID(ulCellID), m_tItem(tItem) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_tItem;
} aginfo_float;
    

typedef struct aginfo_double {
    aginfo_double(): m_ulID(0), m_ulCellID(0), m_tItem(0) {};
    aginfo_double(idtype ulID, gridtype ulCellID, double tItem): m_ulID(ulID), m_ulCellID(ulCellID), m_tItem(tItem) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    double   m_tItem;
} aginfo_double;
    

const std::string DTYPE_BYTE   = "byte";
const std::string DTYPE_INT    = "int";
const std::string DTYPE_LONG   = "long";
const std::string DTYPE_FLOAT  = "float";
const std::string DTYPE_DOUBLE = "double";

class AgentItemCollector {
public:
    static AgentItemCollector *createInstance(const std::string sGeoQDF, const std::string sPopQDF, const std::string sSpecies, const std::string sItemName, const std::string sDType);
    ~AgentItemCollector();

    int init(const std::string sGeoQDF, const std::string sPopQDF, const std::string sSpecies, const std::string sItemName, const std::string sDType);
    
    //    int getDistr(double dMin, double dMax, int iNumBins, const char *pItemName);

    
    int analyzeRanges(named_sampling_ranges &mNamedRanges, uint iNumBins, double dMin, double dMax);

    uint  getNumContexts() {return m_iNumContexts;};   
    bin_context *getContext(uint i) { return (i < m_iNumContexts)?m_ppBinContexts[i]:NULL;};
    uint getNumValues() { return m_iNumAgents;};
    double *getValues() { return m_dValArr;};
    uchar *getArray() { return m_pInfos;};

    int    getCurStep()   { return m_iCurStep;};
    float  getStartTime() { return m_fStartTime;};
protected:
    AgentItemCollector();

    int fillCoordMap(const std::string sGeoQDF);
    int loadAgentsCell(const std::string sPopQDF, const std::string sSpecies, const std::string sItemName);

    void  prepareDataTypeInfos();
    hid_t createCompoundDataType(const std::string sItemName);
    uchar *createItemArray();
    int getInfoFor(int i, gridtype *pulCellID, double *pdVal);
    void separateValues();
    int getItemNames(const std::string sQDFPop, const std::string sSpecies);
    int checkItemType(const std::string sPopQDF, const std::string sSpecies, const std::string sItemName, const  std::string sDType);

    int getRootAttributes(hid_t hFile);

    arrpos_coords  m_mCoords;
    uchar         *m_pInfos;
    
    bin_context **m_ppBinContexts;
    uint m_iNumContexts;

    
    uint  m_iNumAgents;
    datatype_infos m_mDataTypeInfos;
    std::string    m_sDataType;
    double        *m_dValArr;
    int           *m_iCellIDArr;
    stringmap      m_mItemNames;

    int            m_iCurStep;
    float          m_fStartTime;
};




#endif
