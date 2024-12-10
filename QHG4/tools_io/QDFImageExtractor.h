#ifndef __QDFIMAGEEXTRACTOR_H__
#define __QDFIMAGEEXTRACTOR_H__

#include <map>
#include <vector>
#include <string>

#include "qhg_consts.h"
#include "types.h"
#include "QDFUtils.h"

class SurfaceGrid;
class AlphaComposer;
class TextRenderer;
class LookUp;


const int ARR_CODE_NONE   = -1;
const int ARR_CODE_LON    =  1;
const int ARR_CODE_LAT    =  2;
const int ARR_CODE_ALT    =  3;
const int ARR_CODE_ICE    =  4;
const int ARR_CODE_WATER  =  5;
const int ARR_CODE_COAST  =  6;
const int ARR_CODE_TEMP   =  7;
const int ARR_CODE_RAIN   =  8;
const int ARR_CODE_NPP_B  =  9;
const int ARR_CODE_NPP    = 10;
const int ARR_CODE_DIST   = 11;
const int ARR_CODE_TIME   = 12;
const int ARR_CODE_POP    = 20;
const int ARR_CODE_AG     = 21;

const int DS_TYPE_POP  = 14;
const int DS_TYPE_AG   = 15; 

const std::string DEF_POS    = "BL";
const std::string DEF_OFFSET = "5";

// data set info: location, name, type
typedef struct ds_info{
    std::string sGroup;
    std::string sSubGroup;
    std::string sDataSet;
    std::string sElement;
    int         iDataType;
    ds_info() 
        : sGroup(""), sSubGroup(""),sDataSet(""), sElement(""), iDataType(DS_TYPE_NONE) {};
    ds_info(std::string sGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(""), sDataSet(sDataSet1), sElement(""), iDataType(iDataType1) {};
    ds_info(std::string sGroup1, std::string sSubGroup1, std::string sDataSet1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(sSubGroup1), sDataSet(sDataSet1), sElement(""), iDataType(iDataType1) {};
    ds_info(std::string sGroup1, std::string sSubGroup1, std::string sDataSet1, std::string sElement1, int iDataType1)
        : sGroup(sGroup1), sSubGroup(sSubGroup1), sDataSet(sDataSet1), sElement(sElement1), iDataType(iDataType1) {};
} ds_info;

typedef  std::pair<std::string, int>       arrind;
typedef  std::map<arrind, ds_info>         array_data;
typedef  std::map<arrind, LookUp *>        lookup_data; 
typedef  std::map<arrind, int>             which_data; 
typedef  std::vector<arrind>               data_order;

// image properties: size, longitude shift
typedef struct img_prop {
    int iW;
    int iH;
    double dWV;
    double dHV;
    double dOLon;
    double dOLat;
    //double dLonRoll;
    img_prop(int iW0, int iH0, double dLR0):iW(iW0),iH(iH0),dWV(dNaN),dHV(dNaN),dOLon(dLR0),dOLat(dNaN){};
    img_prop(int iW0, int iH0, double dWV0, double dHV0, double dOLon0, double dOLat0):iW(iW0),iH(iH0),dWV(dWV0),dHV(dHV0),dOLon(dOLon0),dOLat(dOLat0){};
} img_prop;

class QDFImageExtractor {
public:
    
    static QDFImageExtractor *createInstance(SurfaceGrid *pSG, 
                                             const std::string sQDFGrid, 
                                             stringvec &vQDFs, 
                                             const std::string sArrayData, 
                                             img_prop    &ip, 
                                             bool         bVerbose);
    ~QDFImageExtractor();
    
    int extractAll(const std::string sOutPat, const std::string sCompOp, const std::string sText);
protected:
    QDFImageExtractor();
    int init(SurfaceGrid *pSG, 
             const std::string sQDFGrid, 
             stringvec &vQDFs, 
             const std::string sArrayDatad, 
             img_prop    &ip, 
             bool         bVerbose);

    int checkArrayName(arrind &aiNameIndex);
    int splitArraySpec(const std::string sArrayData);
    int splitArrayColors(const std::string sArrayData);
    int checkConsistent();
    bool checkGroup(hid_t hFile, const ds_info &rdInfo);
    bool checkGroupNew(const std::string sQDF, const ds_info &rdInfo);
    double *extractData(const std::string sQDF, const ds_info &pGroupDS);
    
    LookUp *createLookUp(std::string sLUName, stringvec &vParams);

    int LoopLayers(const std::string sOutPat, const std::string sCompOp);
    int addTimeLayer(float fTime);
    int addTextLayer(std::string sText);
    
    int writePNGFile(const std::string sPat, const std::string sReplace);
    bool extractColors(const std::string sColorDesc, uchar *pR, uchar *pG, uchar *pB, uchar *pA);

    double **createDataMatrix(double *pData);

    void deleteMatrix(double **pData);
    void cleanUpLookUps();

    int loadAgentsCell(const std::string sPop, const std::string sPopName, const std::string sItemName, double *pdArrN, double *pdArr);
    int calcTextPos(TextRenderer *pTR, std::string sText, std::string sPos, std::string sOffs,  int &iWText, int &iHText);

    SurfaceGrid    *m_pSG;
    int    m_iW;
    int    m_iH;
    double m_dWV;
    double m_dHV;
    double m_dOLon;
    double m_dOLat;
    double m_dLonRoll; // same as m_dOLon 
    int    m_iNumLayers;

    int    m_iOX;
    int    m_iOY;
    stringvec       m_vQDFs;
    array_data      m_mvArrayData;
    lookup_data     m_mvLookUpData;
    data_order      m_vOrder;
    which_data      m_mvWhich;
    int             m_iNumCells;

    AlphaComposer  *m_pAC;

    bool            m_bVerbose;
};

#endif
