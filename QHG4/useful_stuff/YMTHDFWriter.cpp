#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <hdf5.h>

#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "SPopulation.h"
#include "AgentYMTCollector.h"
#include "YMTHDFWriter.h"


//----------------------------------------------------------------------------
//  createInstance
//
YMTHDFWriter *YMTHDFWriter::createInstance(const std::string sFileName, uint iNumRegions) {
    YMTHDFWriter *pHHW = new YMTHDFWriter();
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
YMTHDFWriter::YMTHDFWriter() 
    : m_hFile(H5P_DEFAULT),
      m_hRoot(H5P_DEFAULT),
      m_bVerbose(false) {
}

//----------------------------------------------------------------------------
// destructor
//
YMTHDFWriter::~YMTHDFWriter() {
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
int YMTHDFWriter::init(const std::string sFileName, uint iNumRegions) {
    int iResult = -1;

    bool bNew = false;
    
    xha_fprintf(stderr, "[YMTHDFWriter::init] using [%s]\n", sFileName); fflush(stderr);
    if (fileExists(sFileName)) {
        xha_fprintf(stderr, "[YMTHDFWriter::init] opening [%s] with H5F_ACC_RDWR\n", sFileName); fflush(stderr);
        m_hFile = H5Fopen(sFileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    } else {
        xha_fprintf(stderr, "[YMTHDFWriter::init] [%s] does not exist, create with H5F_ACC_TRUNC\n", sFileName); fflush(stderr);
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
                xha_fprintf(stderr, "[YMTHDFWriter::init] Couldn't create datatype");
            }
        } else {
            xha_fprintf(stderr, "[YMTHDFWriter::init] Couldn't create root group");
        }
    } else {
        xha_fprintf(stderr, "[YMTHDFWriter::init] Couldn't create HDF file [%s]", sFileName);
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
hid_t YMTHDFWriter::createCompoundDataType() {
    hid_t hAgentDataType = H5P_DEFAULT;

    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(aginfo_ymt));

    // writing compound data: the name passed  as second argument to H5Tinsert will be used in the HDF file to be written
                              
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID.c_str(), HOFFSET(aginfo_ymt, m_ulID),      H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID.c_str(),  HOFFSET(aginfo_ymt, m_ulCellID),  H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER.c_str(),   HOFFSET(aginfo_ymt, m_iGender),   H5T_NATIVE_UCHAR);
    H5Tinsert(hAgentDataType, "Hybridization",          HOFFSET(aginfo_ymt, m_fHybridization),     H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "Ychr",                   HOFFSET(aginfo_ymt, m_iYchr),     H5T_NATIVE_UCHAR);
    H5Tinsert(hAgentDataType, "mtDNA",                  HOFFSET(aginfo_ymt, m_imtDNA),    H5T_NATIVE_UCHAR);

    return hAgentDataType;
}


//----------------------------------------------------------------------------
// writeData
//
int YMTHDFWriter::writeData(const std::string sSimName, uchar *pData, int iNumVals) {
    int iResult = -1;

    hsize_t dimsm = iNumVals;
    hid_t hDataSpace = H5Screate_simple (1, &dimsm, NULL); 
        
    hid_t hDataSet   = H5Dcreate2(m_hRoot, sSimName.c_str(), m_hAgentDataType, hDataSpace, 
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, m_hAgentDataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, pData);

    if (status >= 0) {
        iResult = 0;
        if (m_bVerbose) {
             xha_fprintf(stderr, "[YMTHDFWriter::writeData] Successfully written %d records\n", iNumVals);
        }
    } else {
        xha_fprintf(stderr, "[YMTHDFWriter::writeData] Couldn't write data\n");
    }

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace); 

    return iResult;
}


//----------------------------------------------------------------------------
// writeDataRegionized
//
int YMTHDFWriter::writeDataRegionized(const std::string sSimName, int iStep, float fStartTime, uchar *pData, uint iNumAgs, const loc_cells& mLocCells, loc_landwater &mLandWater) {
    int iResult = -1;
    /*
    xha_fprintf(stderr, "YMTHDFWriter: some values before writing\n"); 
    for (uint i = 0; i < 5; i++) {
        aginfo_ymt *pai = (aginfo_ymt  *)pData;
        xha_fprintf(stderr, "%d: agent id %u, cell id %d, hyb %f\n", i, pai[i].m_ulID, pai[i].m_ulCellID, pai[i].m_fHybridization);
        int j = iNumAgs-i-1;
        xha_fprintf(stderr, "%d: agent id %u, cell id %d, hyb %f\n", j, pai[j].m_ulID, pai[j].m_ulCellID, pai[j].m_fHybridization);
    }
    */
   
    if (!qdf_attr_exists(m_hRoot, ROOT_TIME_NAME))  {
        iResult = qdf_insertAttribute(m_hRoot, ROOT_TIME_NAME, 1, &fStartTime);
    }

    xha_fprintf(stderr, "[YMTHDFWriter::writeDataRegionize] adding group [%s]\n", sSimName); fflush(stderr);
    hid_t hSimGroup = qdf_opencreateGroup(m_hRoot, sSimName);
    
    char sStep[1024];
    sprintf(sStep, "step_%06d", iStep);
    hid_t hStepGroup = qdf_opencreateGroup(hSimGroup, sStep);

    loc_cells::const_iterator it;

    if (pData != NULL) {
        for (it = mLocCells.begin(); it != mLocCells.end(); it++) {
            
            const std::vector<int> &vCells = it->second;

            std::vector<aginfo_ymt> vSelected;
            aginfo_ymt ay;
            uchar *p = pData;
            for (uint i = 0; i < iNumAgs; i++) {
                memcpy(&ay, p, sizeof(aginfo_ymt));
                p += sizeof(aginfo_ymt);
                int iCell = ay.m_ulCellID;
                std::vector<int>::const_iterator itC = std::find(vCells.begin(), vCells.end(), iCell);
                if (itC != vCells.end()) {
                    vSelected.push_back(ay);
                }
            }
    
  
            // create uchar array to hold enough space
            uchar *pTemp = new uchar[vSelected.size()*sizeof(aginfo_ymt)];
            uchar *pCur = pTemp;
            for (uint j = 0; j < vSelected.size(); j++) {
                memcpy(pCur, &(vSelected[j]), sizeof(aginfo_ymt));
                pCur += sizeof(aginfo_ymt);
            }
            
            hsize_t dimsm = vSelected.size();
            

            hid_t hDataSpace = H5Screate_simple (1, &dimsm, NULL); 
            
            hid_t hDataSet   = H5Dcreate2(hStepGroup, it->first.c_str(), m_hAgentDataType, hDataSpace, 
                                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            herr_t status = H5Dwrite(hDataSet, m_hAgentDataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, pTemp);
            
            if (status >= 0) {
                iResult = 0;
                if (m_bVerbose) {
                    xha_fprintf(stderr, "[YMTHDFWriter::writeDataRegionize] Successfully written %zd records\n",  vSelected.size());
                }
            } else {
                xha_fprintf(stderr, "[YMTHDFWriter::writeDataRegionize] Couldn't write data\n");
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

