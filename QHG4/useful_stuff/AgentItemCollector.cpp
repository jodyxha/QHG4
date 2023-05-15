#include <cstdio>
#include <cstring>
#include <omp.h>
// for stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "hdf5.h"

#include "strutils.h"
#include "stdstrutilsT.h"
#include "geomutils.h"
#include "SPopulation.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"
#include "AgentItemCollector.h"

stringmap c_mTypeTranslation = {
    {"byte",   "int_8"},
    {"int",    "int_32"},
    {"long",   "int_64"},
    {"float",  "float_32"},
    {"double", "float_64"},
};
stringmap c_mInvTypeTranslation = {
    {"int_8",    "byte",  },
    {"int_32",   "int",   },
    {"int_64",   "long",  },
    {"float_32", "float", },
    {"float_64", "double",},
};

//----------------------------------------------------------------------------
// createInstance
//
AgentItemCollector *AgentItemCollector::createInstance(const std::string sGeoQDF, const std::string sPopQDF, const std::string sSpecies, const std::string sItemName, const std::string sDType) {
    AgentItemCollector *pAIC = new AgentItemCollector();
    int iResult = pAIC->init(sGeoQDF, sPopQDF, sSpecies, sItemName, sDType);
    if (iResult != 0) {
        delete pAIC;
        pAIC = NULL;
           }
    return pAIC;
}


//----------------------------------------------------------------------------
// constructor
//
AgentItemCollector::AgentItemCollector() 
    : m_pInfos(NULL),
      m_ppBinContexts(NULL),
      m_iNumContexts(0),
      m_sDataType(""),
      m_dValArr(NULL),
      m_iCellIDArr(NULL) {
}
  

//----------------------------------------------------------------------------
// destructor
//
AgentItemCollector::~AgentItemCollector() {
    if (m_pInfos != NULL) {
        delete[] m_pInfos;
    }

    if (m_ppBinContexts != NULL) {
        for (uint i = 0; i < m_iNumContexts; i++) {
            delete[] m_ppBinContexts[i]->m_piBins;
            delete m_ppBinContexts[i];
        }
        delete[] m_ppBinContexts;
    }

    if (m_dValArr != NULL) {
        delete[] m_dValArr;
    }
    if (m_iCellIDArr != NULL) {
        delete[] m_iCellIDArr;
    }
}


//----------------------------------------------------------------------------
// init
//
int AgentItemCollector::init(const std::string sGeoQDF, const std::string sPopQDF, const std::string sSpecies, const std::string sItemName, const std::string sDType) {
    int iResult = 0;

    iResult = checkItemType(sPopQDF, sSpecies, sItemName, sDType);
    if (iResult == 0) {
        m_sDataType = sDType;
        float f0 = omp_get_wtime();
        iResult = fillCoordMap(sGeoQDF);
        float f1 = omp_get_wtime();
        if (iResult == 0) {
            
            stdfprintf(stderr, "Successfully read coords from [%s] (%fs)\n", sGeoQDF, f1 - f0);
            iResult = loadAgentsCell(sPopQDF, sSpecies, sItemName);
            float f2 = omp_get_wtime();
            if (iResult == 0) {
                separateValues();

                stdfprintf(stderr, "Successfully read agent items from [%s] (%fs)\n", sPopQDF, f2 - f1);
            } else {
                stdfprintf(stderr, "Couldn't get agent items\n");
            }
        } else {
            // error message already displayed in checkItemType
        }
    } else {
        stdfprintf(stderr, "Couldn't load coords\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// checkItemType
//  check if the item exists in the AgentDataSet, and that its type matches
//
int AgentItemCollector::checkItemType(const std::string sPopQDF, const std::string sSpecies, const std::string sItemName, const std::string sDType) {
    int iResult = -1;
    iResult = getItemNames(sPopQDF, sSpecies);
    if (iResult == 0) {
        prepareDataTypeInfos(); 
        iResult = -1;
        stringmap::const_iterator its = m_mItemNames.find(sItemName);
        if (its != m_mItemNames.end()) {
            stringmap::const_iterator its2 = c_mTypeTranslation.find(sDType);
            if (its2 != c_mTypeTranslation.end()) {
                if (its2->second == its->second) {
                    iResult = 0;
                } else {
                    stdfprintf(stderr, "Wrong type: item [%s] has type [%s]\n", sItemName, c_mInvTypeTranslation[its->second]);
                }
            } else {
                stdfprintf(stderr, "Unknown or unsupported type: [%s]\n", sDType);
            }
        } else {
            stdfprintf(stderr, "unknown item name [%s]\n", sItemName);
            stdfprintf(stderr, "the current dataset contains these items:\n");
            for (its = m_mItemNames.begin(); its != m_mItemNames.end(); ++its) {
                stdfprintf(stderr, "  %s (%s)\n", its->first, c_mInvTypeTranslation[its->second]);
            }
        }
    } else {
        stdfprintf(stderr, "Couldn't collect item names\n");
    }
        
    return iResult;
}

//----------------------------------------------------------------------------
// getRootAttributes
//  extract the attribtes of the root group
//
int AgentItemCollector::getRootAttributes(hid_t hFile) {
    int iResult = 0;

    std::string sValue = "";
    iResult = qdf_extractSAttribute(hFile, ROOT_STEP_NAME, sValue);
    if (iResult == 0) {
        if (strToNum(sValue, &m_iCurStep)) {
            iResult = qdf_extractSAttribute(hFile, ROOT_TIME_NAME, sValue);
            if (iResult == 0) {
                if (strToNum(sValue, &m_fStartTime)) {
                    iResult = 0;
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "Couldn't convert attribute [%s] (%s) to number\n", ROOT_TIME_NAME, sValue);
                }
            } else {
                iResult = -1;
                stdfprintf(stderr, "Couldn't get attribute [%s]\n", ROOT_TIME_NAME);
            }
        } else {
            iResult = -1;
            stdfprintf(stderr, "Couldn't convert attribute [%s] (%s) to number\n", ROOT_STEP_NAME, sValue);
        }
    } else {
        iResult = -1;
        stdfprintf(stderr, "Couldn't get attribute [%s]\n", ROOT_STEP_NAME);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getItemNames
//  extract the name of the items in the AgentDataSet
//
int AgentItemCollector::getItemNames(const std::string sPopQDF, const std::string sSpecies) {
    int iResult = -1; 

    hid_t hFile  = qdf_openFile(sPopQDF, "r");
    if (hFile != H5P_DEFAULT) {
        iResult = getRootAttributes(hFile);
        if (iResult == 0) {
            std::string sGroupName = stdsprintf("%s/%s", POPGROUP_NAME, sSpecies);
        hid_t hGroup = qdf_openGroup(hFile, sGroupName);
        if (hGroup != H5P_DEFAULT) {
            hid_t hDSet  = qdf_openDataSet(hGroup, AGENT_DATASET_NAME);
            if (hDSet != H5P_DEFAULT) {
                hid_t hType  = H5Dget_type(hDSet);
                if (hType > H5P_DEFAULT) {
                    int n = H5Tget_nmembers(hType);
                    for (int i = 0; i < n; i++) {
                        hid_t st = H5Tget_member_type(hType, i);
                        H5T_class_t c = H5Tget_member_class(hType, i);
                        size_t ss = 8*H5Tget_size(st);
                        char sc[1024];

                        sprintf(sc, "unsupported class");
                        if ((c == H5T_INTEGER) || (c==H5T_FLOAT)) {
                            if (c == H5T_INTEGER) {
                                sprintf(sc,"int_%zd", ss);
                            } else { 
                                sprintf(sc,"float_%zd", ss);
                            }
                        }
                        m_mItemNames[H5Tget_member_name(hType,i)] = sc;
                    }
                    iResult = 0;
                    
                    H5Tclose(hType);
                } else {
                    stdfprintf(stderr, "Couldn't open get data type for data set [%s]\n", AGENT_DATASET_NAME);
                }
                qdf_closeDataSet(hDSet);
            } else {
                stdfprintf(stderr, "Couldn't open data set [%s]\n", AGENT_DATASET_NAME);
            }
            qdf_closeGroup(hGroup);
        } else {
            stdfprintf(stderr, "Couldn't open group [%s]\n", sGroupName);
        }

        } else {
            stdfprintf(stderr, "Couldn't get root attributesof [%s]\n", sPopQDF);
        }
        qdf_closeFile(hFile);

    } else {
        stdfprintf(stderr, "Couldn't open [%s] for reading\n", sPopQDF);
    }
    return iResult;

}


//----------------------------------------------------------------------------
// fillCoordMap
//  get longitude and latitude arrays form the geogrid
//
int AgentItemCollector::fillCoordMap(const std::string sQDFGeoGrid) {
    int iResult = -1;
    uint iNumCells = 0;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
     
    QDFArray *pQA = QDFArray::create(sQDFGeoGrid);
    if (pQA != NULL) {

        // get the cell IDs
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            iNumCells = pQA->getSize();
            pCellIDs = new int[iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, iNumCells, GRID_DS_CELL_ID);
            if (iCount == iNumCells) {
                //                fprintf(stderr, "Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                stdfprintf(stderr, "Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            stdfprintf(stderr, "Couldn't open QDF array for [%s:%s/%s]\n", sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME);
        }

        // get the cell Longitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLon = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, iNumCells);
                    if (iCount == iNumCells) {
                        stdfprintf(stderr, "Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE);
                }
                pQA->closeArray();
            }
        }

        // get the cell Latitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LATITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLat = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, iNumCells);
                    if (iCount == iNumCells) {
                        stdfprintf(stderr, "Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "Couldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL,iNumCells);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        stdfprintf(stderr, "Couldn't create QDFArray\n");
    }
     
    // put everything into a map CellID => (lon, lat)
    if (iResult == 0) {
        // save coordinate values
        for (uint i = 0; i < iNumCells; i++) {
            m_mCoords[pCellIDs[i]] = std::pair<double, double>(pdLon[i], pdLat[i]);
        }
    }

    if (pCellIDs != NULL) {
        delete[] pCellIDs;
    }
    if (pdLon != NULL) {
        delete[] pdLon;
    }
    if (pdLat != NULL) {
        delete[] pdLat;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// prepareDataTypeInfos
//  prepare HDF5 data types vor int, long, float, double
//
void AgentItemCollector::prepareDataTypeInfos() {
    m_mDataTypeInfos[DTYPE_BYTE]   =  datatype_info(sizeof (aginfo_uchar),  HOFFSET(aginfo_uchar,  m_tItem), H5T_NATIVE_UCHAR);
    m_mDataTypeInfos[DTYPE_INT]    =  datatype_info(sizeof (aginfo_int),    HOFFSET(aginfo_int,    m_tItem), H5T_NATIVE_INT);
    m_mDataTypeInfos[DTYPE_LONG]   =  datatype_info(sizeof (aginfo_long),   HOFFSET(aginfo_long,   m_tItem), H5T_NATIVE_LONG);
    m_mDataTypeInfos[DTYPE_FLOAT]  =  datatype_info(sizeof (aginfo_float),  HOFFSET(aginfo_float,  m_tItem), H5T_NATIVE_FLOAT);
    m_mDataTypeInfos[DTYPE_DOUBLE] =  datatype_info(sizeof (aginfo_double), HOFFSET(aginfo_double, m_tItem), H5T_NATIVE_DOUBLE);

}


//----------------------------------------------------------------------------
// createCompoundDataType
//   create a compound datattype containing
//      agent id
//      cell  id
//      item 
//
hid_t AgentItemCollector::createCompoundDataType(const std::string sItemName) {
    hid_t hAgentDataType = H5P_DEFAULT;

    datatype_infos::const_iterator it = m_mDataTypeInfos.find(m_sDataType);
    if (it != m_mDataTypeInfos.end()) {
        hAgentDataType = H5Tcreate (H5T_COMPOUND, it->second.m_iStructSize);


        H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID.c_str(),     HOFFSET(aginfo_int,  m_ulCellID),  H5T_NATIVE_INT);
        H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID.c_str(),    HOFFSET(aginfo_long, m_ulID),      H5T_NATIVE_LONG);
        
        H5Tinsert(hAgentDataType, sItemName.c_str(),   it->second.m_iOffset,              it->second.m_hType);
    } else {
        stdfprintf(stderr, "[createCompoundDataType] unknown data type: [%s]\n", m_sDataType);
    }
    return hAgentDataType;
}


//----------------------------------------------------------------------------
// createItemArray
//   create the array to hold the compound data read from the AgentDataSet
//
uchar *AgentItemCollector::createItemArray() {
    uchar *pArray = NULL;
    if (DTYPE_BYTE == m_sDataType) { 
        pArray = new uchar[m_iNumAgents*sizeof(aginfo_uchar)];
    } else     if (DTYPE_INT == m_sDataType) { 
        pArray = new uchar[m_iNumAgents*sizeof(aginfo_int)];
    } else     if (DTYPE_LONG == m_sDataType) { 
        pArray = new uchar[m_iNumAgents*sizeof(aginfo_long)];
    } else     if (DTYPE_FLOAT == m_sDataType) { 
        pArray = new uchar[m_iNumAgents*sizeof(aginfo_float)];
    } else     if (DTYPE_DOUBLE == m_sDataType) { 
        pArray = new uchar[m_iNumAgents*sizeof(aginfo_double)];
    } else  {
        stdfprintf(stderr, "[createItemArray] unknown data type: [%s]\n", m_sDataType);
    }
    return pArray;
}


//----------------------------------------------------------------------------
// loadAgentsCell
//  read the compound data from the AgentDataSet
//
int AgentItemCollector::loadAgentsCell(const std::string sPop, const std::string sPopName, const std::string sItemName) {
    hid_t hFilePop     = qdf_openFile(sPop);
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    stdfprintf(stderr, "[loadAgentsCell] pop %s,popname %s\n", sPop, sPopName);
    // at this point m_iNumCells should be known (loadNPP() loadAltIce() already called)

    int iResult = 0;

    hid_t hSpecies     = qdf_openGroup(hPopulation, sPopName);
    hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace   = H5Dget_space(hDataSet);

    hid_t hAgentDataType    = createCompoundDataType(sItemName);
    if (hAgentDataType != H5P_DEFAULT) {

        hsize_t dims;
        H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        m_iNumAgents = dims;
        m_pInfos = createItemArray();
        
        hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
        herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pInfos);
        if (status >= 0) {
            stdfprintf(stderr, "pop %s: %llu\n", sPopName, dims);
        } else {
            stdfprintf(stderr, "bad status for pop %s\n", sPopName);
            
            delete[] m_pInfos;
            m_pInfos = NULL;
            iResult = -1;
        }
        qdf_closeDataSpace(hMemSpace);
    }

    qdf_closeDataType(hAgentDataType);
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
    qdf_closeGroup(hSpecies);
   
    qdf_closeGroup(hPopulation);
    qdf_closeFile(hFilePop);
   
    return iResult;
}


//----------------------------------------------------------------------------
// separateValues
//   create separate arrays from the array of compound data
//   for easier handling in parallelized loop
//
void AgentItemCollector::separateValues() {
    m_dValArr = new double[m_iNumAgents];
    memset(m_dValArr, 0, m_iNumAgents*sizeof(double));
    m_iCellIDArr = new int[m_iNumAgents];
    memset(m_iCellIDArr, 0, m_iNumAgents*sizeof(int));

    if (DTYPE_BYTE == m_sDataType) { 
#pragma omp parallel for
        for (uint i = 0; i < m_iNumAgents; i++) {
            m_dValArr[i] = (double)((aginfo_uchar*)m_pInfos)[i].m_tItem;
            m_iCellIDArr[i] = (int)((aginfo_uchar*)m_pInfos)[i].m_ulCellID;
        }
    } else     if (DTYPE_INT == m_sDataType) { 
#pragma omp parallel for
        for (uint i = 0; i < m_iNumAgents; i++) {
            m_dValArr[i] = (double)((aginfo_int*)m_pInfos)[i].m_tItem;
            m_iCellIDArr[i] = (int)((aginfo_int*)m_pInfos)[i].m_ulCellID;
        }
    } else     if (DTYPE_LONG == m_sDataType) {     
#pragma omp parallel for
        for (uint i = 0; i < m_iNumAgents; i++) {
            m_dValArr[i] = (double)((aginfo_long*)m_pInfos)[i].m_tItem;
            m_iCellIDArr[i] = (int)((aginfo_long*)m_pInfos)[i].m_ulCellID;
        }
    } else     if (DTYPE_FLOAT == m_sDataType) { 
#pragma omp parallel for
        for (uint i = 0; i < m_iNumAgents; i++) {
            m_dValArr[i] = (double)((aginfo_float*)m_pInfos)[i].m_tItem;
            m_iCellIDArr[i] = (int)((aginfo_float*)m_pInfos)[i].m_ulCellID;
        }
    } else     if (DTYPE_DOUBLE == m_sDataType) { 
#pragma omp parallel for
        for (uint i = 0; i < m_iNumAgents; i++) {
            m_dValArr[i] = (double)((aginfo_double*)m_pInfos)[i].m_tItem;
            m_iCellIDArr[i] = (int)((aginfo_double*)m_pInfos)[i].m_ulCellID;
            /*
              if (m_dValArr[i] < 0) {
              fprintf(stderr, "negneg1\n"); fflush(stderr);
              }    
            */
        }
    } else {
        stdfprintf(stderr, "[separateValues] unknown datatype [%s]\n", m_sDataType);fflush(stderr);
    }
    /*
    fprintf(stderr, "valarrcheck (%p)\n", m_dValArr);fflush(stderr);
    for (int i = 0; i < m_iNumAgents; i++) {
        //        fprintf(stderr, "%d\n", i); fflush(stderr);
        if (m_dValArr[i] < 0) {
            fprintf(stderr, "negneg\n"); fflush(stderr);
        }
    }
    */
}


//----------------------------------------------------------------------------
// getInfoFor
//
int AgentItemCollector::getInfoFor(int i, gridtype *pulCellID, double *pdVal) {
    int iResult = 0;
    if (DTYPE_INT == m_sDataType) { 
        aginfo_int *pai = (aginfo_int*)m_pInfos;
        *pulCellID = pai[i].m_ulCellID;
        //        *pdVal     = (double)pai[i].m_tItem;
    } else     if (DTYPE_LONG == m_sDataType) { 
        aginfo_long *pai = (aginfo_long*)m_pInfos;
        *pulCellID = pai[i].m_ulCellID;
        //        *pdVal     = (double)pai[i].m_tItem;

    } else     if (DTYPE_FLOAT == m_sDataType) { 
        aginfo_float *pai = (aginfo_float*)m_pInfos;
        *pulCellID = pai[i].m_ulCellID;
        //        *pdVal     = (double)pai[i].m_tItem;

    } else     if (DTYPE_DOUBLE == m_sDataType) { 
        aginfo_double *pai = (aginfo_double*)m_pInfos;
        *pulCellID = pai[i].m_ulCellID;
        //        *pdVal     = (double)pai[i].m_tItem;

    } else  {
        stdfprintf(stderr, "[getInfoFor] unknown data type: [%s]\n", m_sDataType);
    }
                     
    //*pulCellID = ((aginfo_int*)m_pInfos)[i].m_ulCellID;
    return iResult;
} 


#define EPS 0.000001

//----------------------------------------------------------------------------
// analyzeRanges
//   for every sampling region, loop through all agents and increase
//   counts in bins as appropriate
//
int AgentItemCollector::analyzeRanges(named_sampling_ranges &mNamedSamplingRanges, uint iNumBins, double dMin, double dMax) {
    int iResult = 0;

    // want vector of names
    std::vector<std::string> vNames;
    named_sampling_ranges::const_iterator it;                            
    for(it = mNamedSamplingRanges.begin(); it != mNamedSamplingRanges.end(); ++it) {
        vNames.push_back(it->first);
    }
    
    // get the cellIDs and values into arrays

    // delete the contexts (including the bins) if they do exist
    if (m_ppBinContexts != NULL) {
        for (uint i = 0; i < m_iNumContexts; i++) {
            delete[] m_ppBinContexts[i]->m_piBins;
            delete m_ppBinContexts[i];
        }
        delete[] m_ppBinContexts;
    }
    fprintf(stderr, "[analyzeRanges] after del contexts: %p\n", m_dValArr); fflush(stderr);

    m_iNumContexts = vNames.size();
    m_ppBinContexts = new bin_context*[m_iNumContexts];

    // loop through contexts
#pragma omp parallel for 
    for (uint k = 0; k < m_iNumContexts; k++) {
        //fprintf(stderr, "[analyzeRanges] in loop with %d: %p\n", k, m_dValArr); fflush(stderr);
        bin_context  *pCurContext = new bin_context();
        //fprintf(stderr, "\r                                                          ");
        //fprintf(stderr, "\r%d processing %s", omp_get_thread_num(), vNames[k]);
        pCurContext->m_sName = vNames[k];

        pCurContext->m_iNumBins = iNumBins;
        pCurContext->m_dMinBin  = dMin;
        pCurContext->m_dMaxBin  = dMax;
        pCurContext->m_piBins = new uint[iNumBins];
        memset(pCurContext->m_piBins, 0, iNumBins*sizeof(uint));

        pCurContext->m_fMinNonZero = 1;
        pCurContext->m_fMaxNonOne  = 1;
        pCurContext->m_iNumZeros   = 0;
        pCurContext->m_iNumOnes    = 0;
        pCurContext->m_iNumAgents  = m_iNumAgents;
        pCurContext->m_iNumChecked = 0;

        sampling_range *pSamplingRange = mNamedSamplingRanges[vNames[k]];

        // loop through agents
        for (uint i = 0; i < m_iNumAgents; i++)  {
            bool bUse = true;
            // ulCellID is at the same position in all aginfo_*
            gridtype ulCellID = m_iCellIDArr[i]; //((aginfo_int*)m_pInfos)[i].m_ulCellID;;
            
            //fprintf(stderr, "[analyzeRanges] ag %d (T%d) cell id: %d\n", i, omp_get_thread_num(), ulCellID); fflush(stderr);

            // if a range is specified, check if the agent is within the radius (no range: use all agents)
            if (pSamplingRange != NULL) {
                double dLon0 = m_mCoords[ulCellID].first;
                double dLat0 = m_mCoords[ulCellID].second;
                bUse = (spherdistDeg(dLon0, dLat0, pSamplingRange->m_dLon, pSamplingRange->m_dLat, RADIUS_EARTH_KM) < pSamplingRange->m_dRad);
            }

            if (bUse) {
                // get the value
                //fprintf(stderr, "doing agent %d/%ds, valarr %p\n", i, m_iNumAgents, m_dValArr); fflush(stderr);
                double dVal = m_dValArr[i];
                
                /*
                if ((dVal > 0.1) && (dVal < 0.9)) {
                         fprintf(stderr, "dVal[%d]@%d  %f\n", i, ulCellID, dVal); fflush(stderr);
                }
                */                    

                // find the bin
                uint iBin =  iNumBins * dVal/(dMax - dMin);
                if (iBin >= iNumBins) {
                    iBin = iNumBins - 1;
                }

                pCurContext->m_piBins[iBin]++;
                
                // get other characteristis
                double d = dMax - dVal;
                // value closest to (but not equal to) maximum
                if (d <  pCurContext->m_fMaxNonOne) {
                    pCurContext->m_fMaxNonOne = dVal;
                }

                d = dVal - dMin;
                // value closest to (but not equal to) minimum
                if (d <  pCurContext->m_fMinNonZero) {
                    pCurContext->m_fMinNonZero = dVal;
                }
                
                // count 'zeros' (closer to minimum than EPS)
                if (dVal <= dMin+EPS) {
                     pCurContext->m_iNumZeros++;
                }
                // count 'ones' (closer to maximum than EPS)
                if (dVal >= dMax-EPS) {
                     pCurContext->m_iNumOnes++;
                }
                pCurContext->m_iNumChecked++;
            }
        } 
        m_ppBinContexts[k] = pCurContext;

    }
    return iResult;
}
