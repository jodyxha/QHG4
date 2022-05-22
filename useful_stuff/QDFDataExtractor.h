#ifndef __QDFDATAEXTRACTOR_H__
#define __QDFDATAEEXTRACTOR_H__

#include <map>
#include <vector>
#include <string>

#include "EQsahedron.h"
#include "QDFUtils.h"
#include "PseudoPopManager.h"


const int ARR_CODE_NONE  = -1;
const int ARR_CODE_LON   =  1;
const int ARR_CODE_LAT   =  2;
const int ARR_CODE_ALT   =  3;
const int ARR_CODE_ICE   =  4;
const int ARR_CODE_WATER =  5;
const int ARR_CODE_COAST =  6;
const int ARR_CODE_TEMP  =  7;
const int ARR_CODE_RAIN  =  8;
const int ARR_CODE_NPP_B =  9;
const int ARR_CODE_NPP   = 10;
const int ARR_CODE_DIST  = 11;
const int ARR_CODE_TIME  = 12;
const int ARR_CODE_POP   = 20;

const int DS_TYPE_POP    = 14;

// data set info: location, name, type
typedef struct ds_info{
    std::string sGroup;
    std::string sSubGroup;
    std::string sDataSet;
    int         iDataType;
    ds_info() 
        : sGroup(""), sSubGroup(""),sDataSet(""), iDataType(DS_TYPE_NONE) {};
    ds_info(std::string sGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(""), sDataSet(sDataSet1), iDataType(iDataType1) {};
    ds_info(std::string sGroup1, std::string sSubGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(sSubGroup1), sDataSet(sDataSet1), iDataType(iDataType1) {};
} ds_info;

typedef  std::map<std::string, ds_info>    array_data;
typedef  std::vector<std::string>          string_list;
typedef std::map<int, std::string> patmap;



class QDFDataExtractor {
public:
    static patmap mPseudoArrPatterns;

    static QDFDataExtractor *createInstance(const std::string sQDFGrid, 
                                            const std::string sQDFAdditional, 
                                            const std::string sArrayPath, 
                                            int                iWidth, 
                                            int                iHeight);

    static QDFDataExtractor *createInstance(const std::string sQDFGrid, 
                                            const std::string sQDFAdditional, 
                                            const std::string sArrayPath, 
                                            float             fScale);
    ~QDFDataExtractor();
    
    double **extractDataTable();
    double **extractDataList();
    static const patmap &getPseudoPats() { return mPseudoArrPatterns;};
protected:
    QDFDataExtractor();
    int init(const std::string sQDFGrid, 
             const std::string sQDFAdditional, 
             const std::string sArrayPath, 
             float        fScaleX,
             float        fScaleY);

    int findArraySource();
    int getArraySource(const std::string sQDF);
    int handlePseudoArrays();
    const std::string containsPath(const std::string sPath);
    int getGridAttrs();
    bool isCharArray();


    int    m_iW;
    int    m_iH;
    double m_dDeltaLon;
    double m_dDeltaLat;
    double *m_pdData;

    std::string m_sQDFGrid;
    std::string m_sQDFAdditional;
    std::string m_sArrayPath;
    int             m_iNumCells;
    int             m_iSubDivs;
    EQsahedron *m_pEQ;

    PseudoPopManager *m_pPPM;
};

#endif
