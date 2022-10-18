#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <hdf5.h>

#include "stdstrutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "SPopulation.h"
#include "AgentItemCollector.h"
#include "HybHDFWriter.h"


//----------------------------------------------------------------------------
//  createInstance
//
HybHDFWriter *HybHDFWriter::createInstance(const std::string sFileName, uint iNumRegions) {
    HybHDFWriter *pHHW = new HybHDFWriter();
    int iResult = pHHW->init(sFileName, iNumRegions);
    if (iResult != 0) {
        delete pHHW;
        pHHW = NULL;
    }
    return pHHW;
}


//----------------------------------------------------------------------------
// constructor
//
HybHDFWriter::HybHDFWriter() 
    : m_hFile(H5P_DEFAULT),
      m_hRoot(H5P_DEFAULT),
      m_bVerbose(false) {
}

//----------------------------------------------------------------------------
// destructor
//
HybHDFWriter::~HybHDFWriter() {
    if (m_hAgentDataType != H5P_DEFAULT) {
        qdf_closeDataType(m_hAgentDataType);
    }
    if (m_hRoot != H5P_DEFAULT) {
        qdf_closeGroup(m_hRoot);
    }
    if (m_hFile != H5P_DEFAULT) {
        qdf_closeFile(m_hFile);
    }
}

//----------------------------------------------------------------------------
// init
//
int HybHDFWriter::init(const std::string sFileName, uint iNumRegions) {
    int iResult = -1;

    bool bNew = false;
    
    stdfprintf(stderr, "[HybHDFWriter::init] using [%s]\n", sFileName); fflush(stderr);
    if (fileExists(sFileName)) {
        stdfprintf(stderr, "[HybHDFWriter::init] opening [%s] with H5F_ACC_RDWR\n", sFileName); fflush(stderr);
        m_hFile = H5Fopen(sFileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    } else {
        stdfprintf(stderr, "[HybHDFWriter::init] [%s] does not exist, create with H5F_ACC_TRUNC\n", sFileName); fflush(stderr);
        m_hFile = H5Fcreate(sFileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        bNew = true;
        
    }
    if (m_hFile != H5P_DEFAULT) {
        m_hRoot = qdf_opencreateGroup(m_hFile, "/", false );
        if (m_hRoot != H5P_DEFAULT) {
            m_hAgentDataType    = createCompoundDataType();
            if (m_hAgentDataType != H5P_DEFAULT) {
                if (bNew) {
                    iResult = qdf_insertAttribute(m_hRoot, "NumRegions", 1, &iNumRegions);
                } else {
                    iResult = 0;
                }
            } else {
                stdfprintf(stderr, "[HybHDFWriter::init] Couldn't create datatype");
            }
        } else {
            stdfprintf(stderr, "[HybHDFWriter::init] Couldn't create root group");
        }
    } else {
        stdfprintf(stderr, "[HybHDFWriter::init] Couldn't create HDF file [%s]", sFileName);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// createCompoundDataType
//   create a compound datattype containing
//      agent id
//      cell  id
//      item 
//
hid_t HybHDFWriter::createCompoundDataType() {
    hid_t hAgentDataType = H5P_DEFAULT;

    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(aginfo_float));
                                
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID.c_str(), HOFFSET(aginfo_float, m_ulID),      H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID.c_str(),  HOFFSET(aginfo_float, m_ulCellID),  H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "Hybridization",          HOFFSET(aginfo_float, m_tItem),     H5T_NATIVE_FLOAT);

    return hAgentDataType;
}


//----------------------------------------------------------------------------
// writeData
//
int HybHDFWriter::writeData(const std::string sSimName, uchar *pData, int iNumVals) {
    int iResult = -1;

    hsize_t dimsm = iNumVals;
    hid_t hDataSpace = H5Screate_simple (1, &dimsm, NULL); 
        
    hid_t hDataSet   = H5Dcreate2(m_hRoot, sSimName.c_str(), m_hAgentDataType, hDataSpace, 
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, m_hAgentDataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    if (status >= 0) {
        iResult = 0;
        if (m_bVerbose) {
             stdfprintf(stderr, "[HybHDFWriter::writeData] Successfully written %d records\n", iNumVals);
        }
    } else {
        stdfprintf(stderr, "[HybHDFWriter::writeData] Couldn't write data\n");
    }

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace); 

    return iResult;
}


//----------------------------------------------------------------------------
// writeDataRegionized
//
int HybHDFWriter::writeDataRegionized(const std::string sSimName, int iStep, float fStartTime, uchar *pData, uint iNumAgs, const loc_cells& mLocCells, loc_landwater &mLandWater) {
    int iResult = -1;
    
   
    if (!qdf_attr_exists(m_hRoot, ROOT_TIME_NAME))  {
        iResult = qdf_insertAttribute(m_hRoot, ROOT_TIME_NAME, 1, &fStartTime);
    }

    stdfprintf(stderr, "[HybHDFWriter::writeDataRegionize] adding group [%s]\n", sSimName); fflush(stderr);
    hid_t hSimGroup = qdf_opencreateGroup(m_hRoot, sSimName);
    
    char sStep[1024];
    sprintf(sStep, "step_%06d", iStep);
    hid_t hStepGroup = qdf_opencreateGroup(hSimGroup, sStep);

    loc_cells::const_iterator it;

    if (pData != NULL) {
        for (it = mLocCells.begin(); it != mLocCells.end(); it++) {
            
            const std::vector<int> &vCells = it->second;

            std::vector<aginfo_float> vSelected;
            aginfo_float ai;
            uchar *p = pData;
            for (uint i = 0; i < iNumAgs; i++) {
                memcpy(&ai, p, sizeof(aginfo_float));
                p += sizeof(aginfo_float);
                int iCell = ai.m_ulCellID;
                std::vector<int>::const_iterator itC = std::find(vCells.begin(), vCells.end(), iCell);
                if (itC != vCells.end()) {
                    vSelected.push_back(ai);
                }
            }
    
  
            // create uchar array to hold enough space
            uchar *pTemp = new uchar[vSelected.size()*sizeof(aginfo_float)];
            uchar *pCur = pTemp;
            for (uint j = 0; j < vSelected.size(); j++) {
                memcpy(pCur, &(vSelected[j]), sizeof(aginfo_float));
                pCur += sizeof(aginfo_float);
            }
            
            hsize_t dimsm = vSelected.size();
            

            hid_t hDataSpace = H5Screate_simple (1, &dimsm, NULL); 
            
            hid_t hDataSet   = H5Dcreate2(hStepGroup, it->first.c_str(), m_hAgentDataType, hDataSpace, 
                                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            herr_t status = H5Dwrite(hDataSet, m_hAgentDataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, pTemp);
            
            if (status >= 0) {
                iResult = 0;
                if (m_bVerbose) {
                    stdfprintf(stderr, "[HybHDFWriter::writeDataRegionize] Successfully written %zd records\n",  vSelected.size());
                }
            } else {
                stdfprintf(stderr, "[HybHDFWriter::writeDataRegionize] Couldn't write data\n");
            }

            int iLandWater[2];
            iLandWater[0] = mLandWater[it->first].first;
            iLandWater[1] = mLandWater[it->first].second;
            qdf_insertAttribute(hDataSet, "LandWater", 2, iLandWater);

            qdf_closeDataSet(hDataSet);
            qdf_closeDataSpace(hDataSpace); 
            delete[] pTemp;
        }
    }
    
    qdf_closeGroup(hStepGroup); 
    qdf_closeGroup(hSimGroup); 

    return iResult;
}

