#include "string.h"

#include "QDFImageExtractor.h"

#include "utils.h"
#include "strutils.h"
#include "stdstrutilsT.h"
#include "Vec3D.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "QDFArray.h"
#include "QDFArrayT.h" 
#include "SCellGrid.h"

#include "LookUp.h"
#include "RainbowLookUp.h"
#include "Rainbow2LookUp.h"
#include "GeoLookUp.h"
#include "Geo2LookUp.h"
#include "TwoToneLookUp.h"
#include "FadeOutLookUp.h"
#include "FadeToLookUp.h"
#include "PNGImage.h"

#include "IcoFace.h"
#include "Surface.h"
#include "SurfaceGrid.h"
#include "TextRenderer.h"
#include "AlphaComposer.h"

typedef struct aginfo_t {
    gridtype m_ulCellID;
    float    m_fHybridization;
} aginfo;
#define SPOP_DT_CELL_ID    "CellID"


//----------------------------------------------------------------------------
// createInstance
//
QDFImageExtractor *QDFImageExtractor::createInstance(SurfaceGrid *pSG, 
                                                     const std::string sQDFGrid, 
                                                     stringvec &vQDFs, 
                                                     const std::string sArrayData, 
                                                     img_prop    &ip, 
                                                     bool         bVerbose) {
    QDFImageExtractor *pQIE = new QDFImageExtractor();
    int iResult = pQIE->init(pSG, sQDFGrid, vQDFs, sArrayData, ip, bVerbose);
    if (iResult != 0) {
        delete pQIE;
        pQIE = NULL;
    }
    return pQIE;
}


//----------------------------------------------------------------------------
// constructor
//
QDFImageExtractor::QDFImageExtractor() 
    : m_iW(0),
      m_iH(0),
      m_dWV(360.0),
      m_dHV(180.0),
      m_dOLon(0),
      m_dOLat(-90.0),
      m_dLonRoll(0),
      m_iNumLayers(0),
      m_iOX(0),
      m_iOY(0),
      m_iNumCells(0),
      m_pAC(NULL),
      m_bVerbose(false) {


}

//----------------------------------------------------------------------------
// destructor
//
QDFImageExtractor::~QDFImageExtractor() {
    
    if (m_pAC != NULL) {
        delete m_pAC;
    }

    cleanUpLookUps();
}

    
//----------------------------------------------------------------------------
// init
//
int QDFImageExtractor::init(SurfaceGrid *pSG, 
                            const std::string sQDFGrid, 
                            stringvec &vQDFs, 
                            const std::string sArrayData, 
                            img_prop    &ip,
                            bool         bVerbose) {
    int iResult = 0;

    m_pSG   = pSG;
    m_vQDFs = vQDFs;
    m_iNumLayers = m_vQDFs.size();

    m_iW = ip.iW;
    m_iH = ip.iH;
    //m_dLonRoll = ip.dLonRoll;
    m_dWV   = std::isnan(ip.dWV)?m_dWV:ip.dWV;
    m_dHV   = std::isnan(ip.dHV)?m_dHV:ip.dHV;
    m_dOLon = std::isnan(ip.dOLon)?m_dOLon:ip.dOLon;
    m_dOLat = std::isnan(ip.dOLat)?m_dOLat:ip.dOLat;
    stdprintf("Win: %f %f %f %f\n", m_dWV, m_dHV, m_dOLon, m_dOLat);
    m_bVerbose = bVerbose;
    iResult = splitArrayColors(sArrayData);
    if (iResult == 0) {
        iResult = checkConsistent();
        if (iResult == 0) {

            // we know this will work
            hid_t hFile = qdf_openFile(sQDFGrid);
            hid_t hGrid = qdf_openGroup(hFile, GRIDGROUP_NAME);
            iResult = qdf_extractAttribute(hGrid, GRID_ATTR_NUM_CELLS, 1, &m_iNumCells);
            qdf_closeGroup(hGrid);
            qdf_closeFile(hFile);
            
            if (iResult == 0) {
                m_pAC = AlphaComposer::createInstance(m_iW, m_iH);
                if (m_pAC != NULL) {
                } else {
                    stdprintf("Couldn't create AlphaComposer\n");
                    iResult = -1;
                }
            } else {
                stdprintf("couldn't extrac attribute [%s] from [%s]\n", GRID_ATTR_NUM_CELLS, sQDFGrid);
            }
        } else {
            stdprintf("checkConsistent failed\n");
            // error already printed in checkConsistent
        }
    } else {
        stdprintf("splitArrayColors failed\n");
        // error already printed in splitArrayColors
    }

    return iResult;
}



//----------------------------------------------------------------------------
// checkArrayName
//
int QDFImageExtractor::checkArrayName(arrind &aiNameIndex) {
    int iCode = ARR_CODE_NONE;
    const std::string &sName = aiNameIndex.first;
    if (sName == "lon")  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_LONGITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_LON;
    } else if (sName == "lat")  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_LATITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_LAT;
    } else if (sName == "alt")  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_ALTITUDE, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_ALT;
    } else if (sName == "ice")  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_ICE_COVER, DS_TYPE_CHAR);
        iCode = ARR_CODE_ICE;
    } else if (sName == "water")  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_WATER, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_WATER;
    } else if (sName == "coastal")  {
        m_mvArrayData[aiNameIndex] = ds_info(GEOGROUP_NAME, GEO_DS_COASTAL, DS_TYPE_CHAR);
        iCode = ARR_CODE_COAST;
    } else if (sName == "temp")  {
        m_mvArrayData[aiNameIndex] = ds_info(CLIGROUP_NAME, CLI_DS_ANN_MEAN_TEMP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_TEMP;
    } else if (sName == "rain")  {
        m_mvArrayData[aiNameIndex] = ds_info(CLIGROUP_NAME, CLI_DS_ANN_TOT_RAIN, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_RAIN;
    } else if (sName == "npp")  {
        m_mvArrayData[aiNameIndex] = ds_info(VEGGROUP_NAME, VEG_DS_NPP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_NPP;
    } else if (sName == "npp_b")  {
        m_mvArrayData[aiNameIndex] = ds_info(VEGGROUP_NAME, VEG_DS_BASE_NPP, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_NPP;
    } else if (startsWith(sName, "dist_"))  {
        const std::string s1 = sName.substr(5);
        m_mvArrayData[aiNameIndex] = ds_info(POPGROUP_NAME, s1, SPOP_DS_DIST, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_DIST;
    } else if (startsWith(sName, "time_"))  {
        const std::string s1 = sName.substr(5);
        m_mvArrayData[aiNameIndex] = ds_info(POPGROUP_NAME, s1, SPOP_DS_TIME, DS_TYPE_DOUBLE);
        iCode = ARR_CODE_TIME;
    } else if (startsWith(sName, "pop_"))  {
        const std::string s1 = sName.substr(4);
        m_mvArrayData[aiNameIndex] = ds_info(POPGROUP_NAME, s1, AGENT_DATASET_NAME, DS_TYPE_POP);
        if(m_bVerbose) {
            stdprintf("Adding arrayData(%s, %s, %s, %d)\n", POPGROUP_NAME, s1, AGENT_DATASET_NAME, DS_TYPE_POP);
        }
        //mvArrayData[pName] = ds_info(sGroup, AGENT_DATASET_NAME, DS_TYPE_POP);
        iCode = ARR_CODE_POP;
        
    } else if (startsWith(sName, "agent_"))  {
        const std::string sData = sName.substr(6);

        bool bOK = false;
        
        size_t iPos1 = sData.find("[");
        if (iPos1 != std::string::npos) {
            const std::string sPop = sData.substr(0, iPos1);
            size_t iPos2 = sData.find("]");
            if (iPos2 != std::string::npos) {
                std::string sItem = sData.substr(iPos1+1, iPos2-iPos1+1);
                
                m_mvArrayData[aiNameIndex] = ds_info(POPGROUP_NAME, sPop, AGENT_DATASET_NAME, sItem, DS_TYPE_AG);
                bOK = true;
                if(true ||m_bVerbose) {
                    stdprintf("Adding arrayData(%s, %s, %s, %s, %d)\n", POPGROUP_NAME, sPop, AGENT_DATASET_NAME, sItem, DS_TYPE_AG);
                }
            }
        }

        if (!bOK) {
            stdprintf("Bad agent item format\n");
        } else {
            //mvArrayData[pName] = ds_info(sGroup, AGENT_DATASET_NAME, DS_TYPE_POP);
            iCode = ARR_CODE_AG;
        }
    } else {
        iCode = ARR_CODE_NONE;
    }
    

    if (iCode != ARR_CODE_NONE) {
        if (m_bVerbose) {
            stdprintf("added ARR [%s@%d] -> Group [%s%s%s], dataset [%s], type %d\n", 
                   aiNameIndex.first, 
                   aiNameIndex.second, 
                   m_mvArrayData[aiNameIndex].sGroup, 
                   (m_mvArrayData[aiNameIndex].sSubGroup != "")?"/":"",
                   m_mvArrayData[aiNameIndex].sSubGroup,  
                   m_mvArrayData[aiNameIndex].sDataSet, 
                   m_mvArrayData[aiNameIndex].iDataType);
        }
    }
    return iCode;
}


//----------------------------------------------------------------------------
// splitArraySpec
//  format is 
//    array_spec ::= <array_name>[@<index>]|<lookup>
//
int QDFImageExtractor::splitArraySpec(const std::string sArraySpec) {
    int iResult = -1;

    std::string sArrName;
    size_t iPosIndex   = sArraySpec.find("@");
    size_t iPosLookUp  = sArraySpec.find("|");
    int iIndex = -1;
    if (iPosLookUp != std::string::npos) {
        if (iPosIndex != std::string::npos) {
            sArrName = sArraySpec.substr(0, iPosIndex);
            std::string sIndex = sArraySpec.substr(iPosIndex+1, iPosLookUp - iPosIndex - 1);
            if (strToNum(sIndex, &iIndex)) {
                iResult = 0;
            } else {
                iResult = -1;
            }
        } else {
            // no index - doesn't matter
            sArrName = sArraySpec.substr(0, iPosLookUp);
            iResult = 0;
        }
        
        // do lookup
        size_t iPosParams = sArraySpec.find(":", iPosLookUp + 1);
        std::string  sLUName = sArraySpec.substr(iPosLookUp + 1, iPosParams - iPosLookUp - 1);
        if (!sLUName.empty()) {
            arrind aiNameIndex(sArrName, iIndex);
            int iArrCode = checkArrayName(aiNameIndex);
            if (iArrCode >= 0) {

                std::string sParams = sArraySpec.substr(iPosParams+1);
                stringvec vLUParams;
                uint iNum = splitString(sParams, vLUParams, ":");
                stdprintf("Found %u params for %s\n", iNum, sLUName);
                
                LookUp *pLU = createLookUp(sLUName, vLUParams);
                m_mvLookUpData[aiNameIndex] = pLU;
                if (m_bVerbose) {
                    stdprintf("added LU  [%s@%d] -> [%p]\n", aiNameIndex.first, aiNameIndex.second, pLU);
                }
                
                if (pLU != NULL) {
                    m_vOrder.push_back(aiNameIndex);
                    iResult = 0;
                } else {
                    iResult =-1;
                }
                
            } else {
                stdprintf("Unknown array name [%s]\n", sArrName);
                iResult = -1;
            }

        } else {
            stdprintf("Expected LookUp name after '|': [%s]\n", sArraySpec);
            iResult = -1;
        }
    

    } else {
        stdprintf("No lookup given: [%s]\n", sArraySpec);
        iResult = -1;
    }
    
    return iResult; 
}


//----------------------------------------------------------------------------
// splitArrayColors
//
int QDFImageExtractor::splitArrayColors(const std::string sArrayData) {
    int iResult = 0;

    stringvec vSpecs;
    uint iNum = splitString(sArrayData, vSpecs, ",");
    for (uint i = 0; (iResult == 0) && (i < iNum); i++) {
        iResult = splitArraySpec(vSpecs[i]);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// checkConsistent
//
int QDFImageExtractor::checkConsistent() {
    int iResult = 0;
        
    hid_t  hQDFGeoGrid = qdf_openFile(m_vQDFs[0]);
    if (hQDFGeoGrid != H5P_DEFAULT) {
        hid_t hGrid = qdf_openGroup(hQDFGeoGrid, GRIDGROUP_NAME);
        if (hGrid != H5P_DEFAULT) {
            qdf_closeGroup(hGrid);
        } else {
            stdprintf("No grid group in [%s]\n", m_vQDFs[0]);
            iResult =-1;
        }
        qdf_closeFile(hQDFGeoGrid);

        array_data::const_iterator it;
        if (m_bVerbose) {
            stdprintf("Have %zd ArrayData elements\n", m_mvArrayData.size());
        }
        for (it = m_mvArrayData.begin(); (iResult == 0) && (it != m_mvArrayData.end()); ++it) {
            bool bSearching = true;
            if (m_bVerbose) {
                stdprintf("Have %zd QDFs\n", m_vQDFs.size());
            }
            for (uint i = 0; bSearching && (i < m_vQDFs.size()); i++) {
                if (m_bVerbose) {
                    stdprintf("checking [%s]:%s(%s)\n", m_vQDFs[i], it->second.sGroup, it->second.sSubGroup);
                }
                if (checkGroupNew(m_vQDFs[i], it->second)) {
                    if ((it->first.second < 0) || (it->first.second == (int)i)) {
                        if (true || m_bVerbose) {
                            stdprintf("%s: using Group [%s%s%s], Dataset [%s] in [%s]\n", 
                                   it->first.first, 
                                   it->second.sGroup,  
                                   (it->second.sSubGroup != "")?"/":"",
                                   it->second.sSubGroup,  
                                   it->second.sDataSet, 
                                   m_vQDFs[i]);
                        }
                        bSearching = false;
                        m_mvWhich[it->first] = i;
                    } 
                }
            } 
                if (bSearching) {
                    m_mvWhich[it->first] = -1;
                    iResult = -1;
                    stdprintf("Group [%s%s%s], Dataset [%s] not found in any of the QDF files\n", 
                           it->second.sGroup,  
                           (it->second.sSubGroup != "")?"/":"",
                           it->second.sSubGroup,  
                           it->second.sDataSet);
                } else {                
                    iResult = 0;
                }

        }
    } else {
        stdprintf("Couldn't open grid group in [%s]\n", m_vQDFs[0]);
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// checkGroupNew
//
bool QDFImageExtractor::checkGroupNew(const std::string sQDF, const ds_info &rdInfo) {
    bool bOK = false; 
 
    std::string sSub("");
    if (rdInfo.sSubGroup != "") {
        sSub = stdsprintf("%s/", rdInfo.sSubGroup);
    }
    std::string sFull = stdsprintf("%s/%s%s", rdInfo.sGroup, sSub,  rdInfo.sDataSet);
    if (m_bVerbose) {
        stdprintf("Checking [%s] for path [%s]\n", sQDF, sFull);
    }
    if (qdf_checkPathExists(sQDF, sFull) == 0) {
        bOK = true;
    }
    return bOK;
}


//----------------------------------------------------------------------------
// checkGroup
//
bool QDFImageExtractor::checkGroup(hid_t hFile, const ds_info &rdInfo) {
    bool bOK = false; 

    hid_t hGroup = qdf_openGroup(hFile, rdInfo.sGroup, true);
           
    if (hGroup != H5P_DEFAULT) {
        hid_t hSubGroup = H5P_DEFAULT;
        hid_t hTestGroup = hGroup;
        if (rdInfo.sSubGroup != "") {
            hSubGroup = qdf_openGroup(hGroup, rdInfo.sSubGroup, true);
            hTestGroup = hSubGroup;
        } 
        if (hTestGroup != H5P_DEFAULT) {
            stdprintf("in group [%s] checking for [%s]\n", rdInfo.sGroup, rdInfo.sSubGroup);
            if (qdf_link_exists(hTestGroup, rdInfo.sDataSet)) {
                bOK = true;
            }
        }
        if (hSubGroup != H5P_DEFAULT) {
            qdf_closeGroup(hSubGroup);
        }
        qdf_closeGroup(hGroup);
    }
    return bOK;
}


//----------------------------------------------------------------------------
// extractData
//
double *QDFImageExtractor::extractData(const std::string sQDF, const ds_info &pGroupDS) {
    int iResult = -1;
    double *pdArr  = NULL;
    char   *pdArr1 = NULL;
 
    // get number of elements
    hid_t hFile = qdf_openFile(sQDF);

    // we know this will work, too
    hid_t hGroup = qdf_openGroup(hFile, pGroupDS.sGroup);
        
    pdArr = new double[m_iNumCells];
        
    switch (pGroupDS.iDataType) {
    case DS_TYPE_DOUBLE: 
        iResult = qdf_readArray(hGroup, pGroupDS.sDataSet, m_iNumCells, (double *)pdArr);
        break;

    case DS_TYPE_CHAR:
        pdArr1 = new char[m_iNumCells];
        iResult = qdf_readArray(hGroup, pGroupDS.sDataSet, m_iNumCells, (char *)pdArr1);
        if (iResult == 0) {
            for (int i = 0; i < m_iNumCells; i++) {
                pdArr[i] = pdArr1[i];
            }
        }
        delete[] pdArr1;
        break;

    case DS_TYPE_POP: {
        if (m_bVerbose) stdprintf("extracting pop [%s]\n", pGroupDS.sSubGroup);
        memset(pdArr, 0, m_iNumCells*sizeof(double));
        QDFArray *pQA = QDFArray::create(sQDF);
        if (pQA != NULL) {
            iResult = pQA->openArray(POPGROUP_NAME, pGroupDS.sSubGroup,  pGroupDS.sDataSet);
            if (iResult == 0) {
                
                uint iNumAgents = pQA->getSize();
                if (m_bVerbose) stdprintf("found %d agents\n", iNumAgents);
                int *pdArr2 = new int[iNumAgents];
                uint iCount = pQA->getFirstSlab(pdArr2, iNumAgents, "CellID");
                if (iCount != iNumAgents) {
                    stdprintf("Got %d cell IDs instead of %d\n", iCount, iNumAgents);
                    iResult = -1;
                } else {
                    if (m_bVerbose) stdprintf("counting them per cell\n");
                    for (uint i = 0; i < iNumAgents; ++i) {
                        pdArr[pdArr2[i]]++;
                    }
                }
                delete[] pdArr2;
            }
            delete pQA;
        }
    }
    break;
    case DS_TYPE_AG: {
        if (m_bVerbose) stdprintf("extracting item [%s] from pop [%s]\n", pGroupDS.sElement, pGroupDS.sSubGroup);
      
        memset(pdArr, 0, m_iNumCells*sizeof(double));
        double *pdArrN = new double[m_iNumCells];
        memset(pdArrN, 0, m_iNumCells*sizeof(float));

        QDFArray *pQA = QDFArray::create(sQDF);
        if (pQA != NULL) {
            iResult = pQA->openArray(POPGROUP_NAME, pGroupDS.sSubGroup,  pGroupDS.sDataSet);
            if (iResult == 0) {
                
                uint iNumAgents = pQA->getSize();
                if (m_bVerbose) stdprintf("found %d agents\n", iNumAgents);
                int *pdArr2 = new int[iNumAgents];
                uint iCount = pQA->getFirstSlab(pdArr2, iNumAgents, "CellID");
                if (iCount != iNumAgents) {
                    stdprintf("Got %d cell IDs instead of %d\n", iCount, iNumAgents);
                    iResult = -1;
                } else {
                    if (m_bVerbose) stdprintf("counting them per cell\n");
                    for (uint i = 0; i < iNumAgents; ++i) {
                        pdArrN[pdArr2[i]]++;
                    }
                }
                delete[] pdArr2;
            }
            delete pQA;



        }


        iResult = loadAgentsCell(sQDF,  pGroupDS.sSubGroup, pGroupDS.sElement, pdArrN, pdArr);
        

        
    }
        break;
    default:
        stdprintf("Can't process data of type %d\n", pGroupDS.iDataType);
    }

    qdf_closeGroup(hGroup);
        
    if (iResult == 0) {
        int iMin = -1;
        int iMax = -1;
        double dMin = 1e100;
        double dMax = -1e100;
        for (int i = 0; i < m_iNumCells; i++) {
            if (pdArr[i] < dMin) {
                dMin = pdArr[i];
                iMin = i;
            }
            if (pdArr[i] > dMax) {
                dMax = pdArr[i];
                iMax = i;
            }
        }
        if (m_bVerbose) {
            stdprintf("%s: min: %f @ %d; max %f @ %d\n", pGroupDS.sDataSet, dMin, iMin, dMax, iMax);
        }
    }
    if (iResult != 0) {
        delete[] pdArr;
        pdArr = NULL;
    }

    qdf_closeFile(hFile);
    return pdArr;
}

//----------------------------------------------------------------------------
// loadAgentsCell
//
int QDFImageExtractor::loadAgentsCell(const std::string sPop, const std::string sPopName, const std::string sItemName, double *pdArrN, double *pdArr) {
    hid_t hFilePop     = qdf_openFile(sPop);
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    stdfprintf(stderr, "[loadAgentsCell] pop %s,popname %s\n", sPop, sPopName);
    // at this point m_iNumCells should be known (loadNPP() loadAltIce() already called)

    int iResult = 0;

    hid_t hSpecies     = qdf_openGroup(hPopulation, sPopName);
    hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace   = H5Dget_space(hDataSet);

    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof (aginfo));


    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     HOFFSET(aginfo, m_ulCellID),       H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, sItemName.c_str(),   HOFFSET(aginfo, m_fHybridization), H5T_NATIVE_FLOAT);
        
    hsize_t dims;
    H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    int iNumAgents = dims;
    aginfo *pInfos = new aginfo[dims];

    hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
    herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, pInfos);

    for (int i = 0; i < iNumAgents; i++) {
        pdArr[pInfos[i].m_ulCellID]+= pInfos[i].m_fHybridization;
    }
    for (int i = 0; i < m_iNumCells; i++) {
        if (pdArrN[i] > 0) {
            pdArr[i] /= pdArrN[i];
        } else {
            pdArr[i] = dNaN;
        }
    }

    if (status >= 0) {
        stdfprintf(stderr, "pop %s: %llu\n", sPopName, dims);
    } else {
        stdfprintf(stderr, "bad status for pop %s\n", sPopName);
        iResult = -1;
    }
    delete[] pInfos;
       
    
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
    qdf_closeGroup(hSpecies);
   
    qdf_closeGroup(hPopulation);
    qdf_closeFile(hFilePop);
   
    return iResult;
}

//----------------------------------------------------------------------------
// createLookUp
//
LookUp *QDFImageExtractor::createLookUp(std::string sLUName, stringvec &vParams) {
    LookUp *pLU = NULL;

    double dMin = 0;
    double dMax = 0;
    double dSea = 0;
    uchar uR1  = 0;
    uchar uG1  = 0;
    uchar uB1  = 0;
    uchar uA1  = 0;
    uchar uR2  = 0;
    uchar uG2  = 0;
    uchar uB2  = 0;
    uchar uA2  = 0;
    int iResult = 0;
    int iBadIndex = -1;

    if (sLUName == "rainbow") {
        if (vParams.size() > 1) {
            if (strToNum(vParams[0], &dMin)) {
                if (strToNum(vParams[1], &dMax)) {
                    pLU = new RainbowLookUp(dMin, dMax);
                } else {
                    iBadIndex = 1;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iBadIndex = 2;
            iResult = -3;
        }
    } else if (sLUName == "rainbow2") {
        if (vParams.size() > 1) {
            if (strToNum(vParams[0], &dMin)) {
                if (strToNum(vParams[1], &dMax)) {
                    pLU = new Rainbow2LookUp(dMin, dMax);
                } else {
                    iBadIndex = 1;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iBadIndex = 2;
            iResult = -3;
        }
    } else if (sLUName == "geo") {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0], &dMin)) {
                if (strToNum(vParams[1], &dSea)) {
                    if (strToNum(vParams[2], &dMax)) {
                        pLU = new GeoLookUp(dMin, dSea, dMax);
                    } else {
                        iBadIndex = 2;
                        iResult = -2;
                    }
                } else {
                    iBadIndex = 2;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iBadIndex = 3;
            iResult = -3;
        }

    } else if (sLUName == "geo2") {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0], &dMin)) {
                if (strToNum(vParams[1], &dSea)) {
                    if (strToNum(vParams[2], &dMax)) {
                        pLU = new Geo2LookUp(dMin, dSea, dMax);
                    } else {
                        iBadIndex = 2;
                        iResult = -2;
                    }
                } else {
                    iBadIndex = 2;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iBadIndex = 3;
            iResult = -3;
        }

    } else if (sLUName == "twotone") {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0], &dMax)) {
                if (extractColors(vParams[1], &uR1, &uG1, &uB1, &uA1)) {
                    if (extractColors(vParams[2], &uR2, &uG2, &uB2, &uA2)) {
                        pLU = new TwoToneLookUp(dMax, 
                                                uR1/255.0, uG1/255.0, uB1/255.0, uA1/255.0, 
                                                uR2/255.0, uG2/255.0, uB2/255.0, uA2/255.0);
                    } else {
                        iBadIndex = 2;
                        iResult = -4;
                    }
                } else {
                    iBadIndex = 1;
                    iResult = -4;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iBadIndex = 3;
            iResult = -3;
        }
        
    } else if (sLUName == "fadeout") {
        if (vParams.size() > 2) {
            if (strToNum(vParams[0], &dMin)) {
                if (strToNum(vParams[1], &dMax)) {
                    if (extractColors(vParams[2], &uR1, &uG1, &uB1, &uA1)) {
                        pLU = new FadeOutLookUp(dMin, dMax,
                                                uR1/255.0, uG1/255.0, uB1/255.0, uA1/255.0);
                    } else {
                        iBadIndex = 2;
                        iResult = -4;
                    }
                } else {
                    iBadIndex = 1;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iBadIndex = 3;
            iResult = -3;
        }

    } else if (sLUName == "fadeto") {
        if (vParams.size() > 3) {
            if (strToNum(vParams[0], &dMin)) {
                if (strToNum(vParams[1], &dMax)) {
                    if (extractColors(vParams[2], &uR1, &uG1, &uB1, &uA1)) {
                        if (extractColors(vParams[3], &uR2, &uG2, &uB2, &uA2)) {
                            pLU = new FadeToLookUp(dMin, dMax,
                                                   uR1/255.0, uG1/255.0, uB1/255.0, uA1/255.0,
                                                   uR2/255.0, uG2/255.0, uB2/255.0, uA2/255.0);
                        } else {
                            iBadIndex = 3;
                            iResult = -4;
                        }
                    } else {
                        iBadIndex = 2;
                        iResult = -4;
                    }
                } else {
                    iBadIndex = 1;
                    iResult = -2;
                }
            } else {
                iBadIndex = 0;
                iResult = -2;
            }
        } else {
            iBadIndex = 4;
            iResult = -3;
        }

    } else {
        iResult = -1;
    }
    
    switch (iResult) {
    case 0:
        break;
    case -1:
        stdprintf("Unknown lookup name [%s]\n", sLUName);
        break;
    case -2:
        stdprintf("Expected number for parameter of [%s]: [%d]\n", sLUName, iBadIndex);
        break;
    case -3:
        stdprintf("Not enough parameters for [%s]\n", sLUName);
        break;
    case -4:
        stdprintf("Not a valid color description ('#RRGGBBAA'): [%s]\n", vParams[iBadIndex]);
        break;
    default:
        stdprintf("Unknown error %d\n", iResult);
    }

    return pLU;

}


//----------------------------------------------------------------------------
// createLookUp
//
int QDFImageExtractor::addTimeLayer(float fTime) {
    int iResult = 0;

    std::string sText = stdsprintf("%05.1fky", fTime);
    iResult = addTextLayer(sText);

    return iResult;

    /*
    uchar **ppTimeData = NULL;
    if (fTime >= 0) {
        TextRenderer *pTR = TextRenderer::createInstance(m_iW, m_iH);
        if (pTR != NULL) {
            pTR->setFontSize(24);
            pTR->setColor(1, 1, 1, 1);
            char sText[512];
            sprintf(sText, "%05.1fky", fTime);
            pTR->addText(sText, 5, m_iH-5);
            
            ppTimeData = pTR->createData();

            m_pAC->addPNGDataPar(ppTimeData);
            //m_pAC->addPNGData(ppTimeData);
            
            //pTR->writeToPNG("glomp.png");
            pTR->deleteArray(ppTimeData);
            
            delete pTR;
        } else {
            stdprintf("Couldn't create TextRenderer\n");
            iResult = -1;
        }
    }
    return iResult;
    */
}

//----------------------------------------------------------------------------
// calcTextPos
//
int QDFImageExtractor::calcTextPos(TextRenderer *pTR, std::string sText, std::string sPos, std::string sOffs, int &iWText, int &iHText) {

    int iResult = 0;
    int iOffs   = 0;
    if (strToNum(sOffs, &iOffs)) {

        m_iOX = iOffs;
        m_iOY = m_iH-iOffs;
        
        if (sPos.length() == 2) {
            pTR->setFontSize(24);
            pTR->getExt(sText.c_str(), iWText, iHText);
            stdprintf("Extents for [%s]: %d x %d\n", sText, iWText, iHText); 

            switch (sPos[0]) {
            case 'B':
                m_iOY = m_iH - iOffs;
                break;
        
            case 'C':
                m_iOY = m_iH/2 - iHText/2;
                break;
        
            case 'U':
                m_iOY = iOffs + iHText;
                break;
            default:
                stdprintf("Unknown y-pos character [%c] - only know 'B', 'C', or 'U'\n", sPos[1]);
                iResult = -1;
        
            }
    
        
            switch (sPos[1]) {
            case 'L':
                m_iOX = iOffs;
                break;
        
            case 'C':
                m_iOX = m_iW/2 - iWText/2;
                break;
        
            case 'R':
                m_iOX = m_iW - iWText -iOffs;
                break;
            default:
                stdprintf("Unknown x-pos character [%c] - only know 'L', 'C', or 'R'\n", sPos[1]);
                iResult = -1;
        
            }
        } else {
            stdprintf("Expected 2 pos chars but got [%s]\n", sPos);
            iResult = -1;
        }
    } else {
        stdprintf("Couldn't convert offset [%s] to integer\n", sOffs);
        iResult = -1;
    }
    return iResult;
}
    
//----------------------------------------------------------------------------
// createLookUp
//
int QDFImageExtractor::addTextLayer(std::string sText) {
    int iResult = 0;

    uchar **ppTimeData = NULL;
    if (!sText.empty()) {
        
        TextRenderer *pTR = TextRenderer::createInstance(m_iW, m_iH);
        if (pTR != NULL) {

 
            stringvec vParts;
            uint iNum = splitString(sText, vParts, ":");

            switch (iNum) {
            case 1:
                vParts.push_back(DEF_POS);
                vParts.push_back(DEF_OFFSET);
                break;
            case 2:
                vParts.push_back(DEF_OFFSET);
                break;
            case 3:
                // nothing to add
                break;
            default:
                stdprintf("Bad number of text pos details : %d (expected <text>[:<pos>[:><offs>]])\n");
                iResult = -1;
            }
            if (iResult == 0) {
                int iWText = 0;
                int iHText = 0;

                iResult = calcTextPos(pTR, vParts[0], vParts[1], vParts[2], iWText, iHText);
                

                if (iResult == 0) {
                    stdprintf("for [%s]: iOX = %d iOY=%d\n", vParts[1], m_iOX, m_iOY);
                    pTR->setColor(1, 1, 1, 1);
                    pTR->addText(vParts[0].c_str(), m_iOX, m_iOY);
                    
                    ppTimeData = pTR->createData();
                    
                    m_pAC->addPNGDataPar(ppTimeData);
                    //m_pAC->addPNGData(ppTimeData);
                    
                    //pTR->writeToPNG("glomp.png");
                    pTR->deleteArray(ppTimeData);
                }
 
            }

            delete pTR;
                        
        } else {
            stdprintf("Couldn't create TextRenderer\n");
            iResult = -1;
        }

    }
    return iResult;
}


//----------------------------------------------------------------------------
// extractAll
//
int QDFImageExtractor::extractAll(const std::string sOutPat, const std::string sCompOp, const std::string sText) {
    int iResult = -1;
    
    LoopLayers(sOutPat, sCompOp);
    if (!sCompOp.empty()) {
        //addTimeLayer(fTime);
        addTextLayer(sText);
        
        // save the composite
        iResult = writePNGFile(sOutPat, "merge");
                                    
        if (iResult == 0) {
            if (m_bVerbose) {
                stdprintf("success for merge\n");
            }
        } else {
            stdprintf("Image creation failed\n");
        }
    }
    return iResult;                                
}


//----------------------------------------------------------------------------
// LoopLayers
//
int QDFImageExtractor::LoopLayers(const std::string sOutPat, const std::string sCompOp) {
    int iResult = 0;

    for (uint i = 0; (iResult == 0) && (i < m_vOrder.size()); i++) {
        // extract data
        arrind aiNameIndex   = m_vOrder[i];
        const std::string sName    = aiNameIndex.first;
        const ds_info dsCur  = m_mvArrayData[aiNameIndex];
        double *pData = NULL;
        if ((m_mvWhich[aiNameIndex] >= 0) && (m_mvWhich[aiNameIndex] < m_iNumLayers))  {
            pData = extractData(m_vQDFs[m_mvWhich[aiNameIndex]], dsCur);
        } else {
            stdprintf("What? whichvalue %d  -- this should not happen!\n", m_mvWhich[aiNameIndex]);
            iResult = -1;
        }


        if (pData != NULL) {
                        
            double **pdDataMatrix = createDataMatrix(pData);
            
            m_pAC->addMatrix(pdDataMatrix, m_mvLookUpData[aiNameIndex]);
            if (sCompOp.empty()) {
                            
                // no compositing: create png data and write to file
                std::string sFullName = stdsprintf("%s_%d", sName, m_mvWhich[aiNameIndex]);
                
                iResult = writePNGFile(sOutPat, sFullName);
                            
                if (iResult == 0) {
                    if (m_bVerbose) {
                        stdprintf("success for [%s]\n", sName);
                    }
                } else {
                    stdprintf("Image creation failed\n");
                }
                m_pAC->clear();
            }
                        
            deleteMatrix(pdDataMatrix);
            delete[] pData;
        } else {
            // unknown data type or so
            iResult = -1;
        }
    } // end loop


    
    //std::string sOutName = replacePat(sOutPat, "merge");
    
    // save the composite
    iResult = writePNGFile(sOutPat, "merge");
            
    if (iResult == 0) {
        printf("success for merge\n");
    } else {
        printf("Image creation failed\n");
    }
    

    return iResult;
}


//---------------------------------------------------------------------------
// hex2Val
//
bool hex2Val(const std::string sHex, uchar *puVal) {
    bool bOK = false;
    uchar iVal = 0;
    if (strToHex(sHex, &iVal)) {
        *puVal = (uchar) (iVal & 0xff);
        bOK = true;
    }
    return bOK;
}


//---------------------------------------------------------------------------
// extractColors
//
bool QDFImageExtractor::extractColors(const std::string sColorDesc, uchar *pR, uchar *pG, uchar *pB, uchar *pA) {
    bool bOK = false;
    std::string sHex;

    if ((sColorDesc.length() == 9) && (sColorDesc.front() == '#')) {
        sHex = sColorDesc.substr(1,2);
        if  (hex2Val(sHex, pR)) {
            sHex = sColorDesc.substr(3,2);
            if  (hex2Val(sHex, pG)) {
                sHex = sColorDesc.substr(5,2);
                if  (hex2Val(sHex, pB)) {
                    sHex = sColorDesc.substr(7,2);
                    if  (hex2Val(sHex, pA)) {
                        bOK = true;
                    }
                }
            }
        }
    } // wrong length or doesn't start with '#'
    
    return bOK;
}


//----------------------------------------------------------------------------
// writePNGFile
//
int QDFImageExtractor::writePNGFile(std::string sPat, const std::string sReplace) {
    int iResult = -1;
    
    std::size_t iPos = sPat.find("###");
    if (iPos != std::string::npos) {
        sPat.replace(iPos, 3, sReplace);
    }

    PNGImage *pPI = new PNGImage(m_iW, m_iH);
    if (pPI != NULL) {
        if (m_bVerbose) {
            stdprintf("Writing data to [%s]\n", sPat);
        }
        bool bOK = pPI->createPNGFromData(m_pAC->getData(), sPat);
        if (bOK) {
            iResult = 0;
        } else {
            iResult = -1;
        }

        delete pPI;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// createDataMatrix
//
double **QDFImageExtractor::createDataMatrix(double *pData) {
    double **ppdData = NULL;
    
    Surface *pSurf = m_pSG->getSurface();
    SCellGrid *pCG = m_pSG->getCellGrid();

    // allocate
    ppdData = new double*[m_iH];
    for (int i = 0; i < m_iH; i++) {
        ppdData[i] = new double[m_iW];
    }

    double dDeltaLon = m_dWV/(m_iW+1);
    double dDeltaLat = m_dHV/(m_iH+1);
        
    stdprintf("start at %f\n", m_dOLon+m_dHV);
    double dCurLat = (m_dOLat+m_dHV) - dDeltaLat;
    for (int i = 0; i < m_iH; i++) {
        double dCurLon = dDeltaLon+m_dOLon;
        for (int j = 0; j < m_iW; j++) {
            IcoFace *pF = dynamic_cast<IcoFace*>(pSurf->findFace(dCurLon*M_PI/180, dCurLat*M_PI/180));
           
            idtype ids[3];
            int    idx[3];
            double val[3];
            for (uint k = 0; k < 3; k++) {
                ids[k] = pF->getVertexID(k);
                idx[k] = pCG->m_mIDIndexes[ids[k]];
                val[k] = pData[idx[k]];
            }
            
            double d0;
            double d1;

            double dX1 = cos(dCurLon*M_PI/180)*cos(dCurLat*M_PI/180);    
            double dY1 = sin(dCurLon*M_PI/180)*cos(dCurLat*M_PI/180);    
            double dZ1 = sin(dCurLat*M_PI/180);    
            Vec3D P(dX1, dY1, dZ1);
            

            pF->calcBary(&P, &d0, &d1);
            double d2 = 1 - d0 -d1;

            ppdData[i][j] = d0*val[0] + d1*val[1] + d2*val[2];

            dCurLon += dDeltaLon;
        }
        dCurLat -= dDeltaLat;
    }
    return ppdData;
}


//----------------------------------------------------------------------------
// cleanUpLookUps
//
void QDFImageExtractor::cleanUpLookUps() {
    lookup_data::const_iterator it;
    for (it = m_mvLookUpData.begin(); it != m_mvLookUpData.end(); ++it) {
        delete it->second;
    }
}

//----------------------------------------------------------------------------
// deleteMatrix
//
void QDFImageExtractor::deleteMatrix(double **pData) {
    if (pData != NULL) {
        for (int i = 0; i < m_iH; i++) {
            delete[] pData[i];
        }
        delete[] pData;
    }
}
