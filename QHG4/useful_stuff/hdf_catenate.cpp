#include <cstdio>
#include <cstring>

#include <vector>
#include <hdf5.h>

#include "strutils.h"
#include "stdstrutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"

class aginfo_float {
public:
    aginfo_float(): m_ulID(0), m_ulCellID(0), m_tItem(0) {};
    aginfo_float(idtype ulID, gridtype ulCellID, float tItem): m_ulID(ulID), m_ulCellID(ulCellID), m_tItem(tItem) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_tItem;
};

#define MAX_BUF 1000000
#define SPOP_DT_AGENT_ID "AgentID"
#define SPOP_DT_CELL_ID  "CellID"

//----------------------------------------------------------------------------
// get_group_name
//   callback for h5Literate
//      collects group names
//
herr_t get_group_name(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status = 0;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        std::vector<std::string> *pv =(std::vector<std::string>*)(opdata) ;
        pv->push_back(name);
    }
    return status;
}


//----------------------------------------------------------------------------
// get_dataset_name
//   callback for h5Literate
//      collects dataset names
//
herr_t get_dataset_name(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status = 0;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_DATASET) {
        std::vector<std::string> *pv = (std::vector<std::string>*)(opdata) ;
        pv->push_back(name);
    }
    return status;
}


//----------------------------------------------------------------------------
// copyDataSet
//
int copyDataSet(hid_t hStepOut, hid_t hStepIn, const std::string sDataSetName, hid_t hAgentDataType, aginfo_float **ppBuf, hsize_t *iCurBufSize, bool bVerbose) {
    int iResult = 0;
    hid_t hDataSetIn = qdf_openDataSet(hStepIn, sDataSetName);
    hid_t hDataSpaceIn = H5Dget_space(hDataSetIn);
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpaceIn, &dims, NULL); 
    if (status >= 0) {
        hid_t hDataSpaceOut = H5Screate_simple (1, &dims, NULL); 
        if (*iCurBufSize > dims) {
            *iCurBufSize = dims;
            if (*ppBuf != NULL) {
                delete[] *ppBuf;
            }
            *ppBuf = new aginfo_float[*iCurBufSize];
        }
        if (bVerbose) stdprintf("Reading %llu from data set %s\n", dims, sDataSetName);fflush(stdout);
        hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
        status = H5Dread(hDataSetIn, hAgentDataType, hMemSpace,
                     hDataSpaceIn, H5P_DEFAULT, *ppBuf);
    
        if (status >= 0) {
            if (bVerbose) stdprintf("Writing %llu to data set %s stepout %ld, hDSout %ld\n", dims, sDataSetName, hStepOut, hDataSpaceOut);fflush(stdout);
            
            hid_t hDataSetOut = H5Dcreate2(hStepOut, sDataSetName, hAgentDataType, hDataSpaceOut, 
                                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            status = H5Dwrite(hDataSetOut, hAgentDataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, *ppBuf);
            if (status >= 0) {
                int iLandWater[2];
                qdf_extractAttribute(hDataSetIn, "LandWater", 2, iLandWater);
                if (bVerbose) stdprintf("Have attrs (%d, %d)\n", iLandWater[0], iLandWater[1]);fflush(stdout);
                qdf_insertAttribute(hDataSetOut, "LandWater", 2, iLandWater);
            } else {
                iResult = -1;
                stdfprintf(stderr, "Couldn't write data to [%s] \n", sDataSetName);fflush(stdout);
            }
            qdf_closeDataSet(hDataSetOut);

        } else {
            iResult = -1;
            stdfprintf(stderr, "Couldn't read data from [%s] \n", sDataSetName);fflush(stdout);
        }
    } else {
        iResult = -1;
        stdfprintf(stderr, "Couldn't get extents for [%s] \n", sDataSetName);fflush(stdout);
    }
    qdf_closeDataSet(hDataSetIn);

    return iResult;
}


//----------------------------------------------------------------------------
// createCompundDataType
//   create a compund datattype containing
//      agent id
//      cell  id
//      item 
//
hid_t createCompundDataType() {
    hid_t hAgentDataType = H5P_DEFAULT;

    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(aginfo_float));
                                
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    HOFFSET(aginfo_float, m_ulID),      H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     HOFFSET(aginfo_float, m_ulCellID),  H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "Hybridization",     HOFFSET(aginfo_float, m_tItem),     H5T_NATIVE_FLOAT);

    return hAgentDataType;
}


//----------------------------------------------------------------------------
// popInfo
//  callback function for iteration in getFirstGroup()
//
herr_t grpInfoUtil(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status = 0;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        strcpy((char *)opdata, name);
        status = 1;
    }
    return status;
}


//----------------------------------------------------------------------------
// qdf_getFirstGroup
//  create and return name for first population found in QDF file
//  the returned string must be delete[]d by the caller
//
char *qdf_getFirstGroup(hid_t hFile) {
    char *pGrp = NULL;

    char s[64];
    *s = '\0';
    H5Literate(hFile, H5_INDEX_NAME, H5_ITER_INC, 0, grpInfoUtil, s);
    if (*s != '\0') {
        pGrp = new char[strlen(s)+1];
        strcpy(pGrp, s);
    } else {
        stdprintf("No groups found\n");
    }
    return pGrp;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    int iOffs = 0;
    bool bVerbose = false;
    if (iArgC > 1) {
        if (strcmp(apArgV[1], "-v") == 0) {
            bVerbose = true;
            iOffs = 1;
        }
    }
    if (iArgC > 2+iOffs) {

        stdprintf("opening output file %s\n", apArgV[iOffs+1]); fflush(stdout);
        hid_t hDest  = H5Fcreate(apArgV[1+iOffs], H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        std::vector<std::string> vGroups;

        std::vector<hid_t> v;
        stdprintf("opening input files %d - %d\n", iOffs+2, iArgC);
        for (int i = iOffs+2; (iResult == 0) && (i < iArgC); i++) {
            if (bVerbose) stdprintf("Trying to open %s ...", apArgV[i]); fflush(stdout);
            hid_t hIn = H5Fopen(apArgV[i], H5F_ACC_RDONLY, H5P_DEFAULT);
            if (hIn != H5P_DEFAULT) {
                v.push_back(hIn);
                vGroups.push_back(qdf_getFirstGroup(hIn));
                if (bVerbose) stdprintf("OK %ld != %ld!\n", hIn, H5P_DEFAULT);
            } else {
                iResult = -1;
                if (bVerbose) stdprintf("Failed\n"); else stdprintf("Failed to open [%s]\n", apArgV[i+iOffs]);
            }
        }

        // copy some root attributes from first input
        float fStartTime = 0;
        iResult = qdf_extractAttribute(v[0], ROOT_TIME_NAME, 1, &fStartTime);
        if (iResult == 0) {
            iResult = qdf_insertAttribute(hDest, ROOT_TIME_NAME, 1, &fStartTime);
        }
        if (iResult == 0) {
            uint iNumRegions = 0;
            iResult = qdf_extractAttribute(v[0], "NumRegions", 1, &iNumRegions);
            if (iResult == 0) {
                iResult = qdf_insertAttribute(hDest, "NumRegions", 1, &iNumRegions);
            }
        }
        

        // createa hdf data type for agent ID, Cell ID and hybridization
        hid_t hAgentDataType = createCompundDataType();
        hsize_t iCurSize = 0;
        aginfo_float *pBuf = NULL;

        // loop through input files
        for (uint i = 0; i < v.size(); i++) {
            stdprintf("Cppying data from [%s]\n", apArgV[i+iOffs+2]);
            hid_t hSimOut = qdf_createGroup(hDest, vGroups[i]);
            if (bVerbose) stdprintf("created top-level group %s\n", vGroups[i]);
            // collect subgroup names (should be "step_XXX")
            std::vector<std::string> vSteps;
            vSteps.clear();
            hid_t hSimIn = qdf_openGroup(v[i], vGroups[i]);
            if (bVerbose) stdprintf("finding steps for sim %u %s  (%ld) %ld\n", i, vGroups[i], hSimIn, v[i]);fflush(stdout);
            H5Literate(hSimIn, H5_INDEX_NAME, H5_ITER_INC, 0, get_group_name, &vSteps);
            
            // loop through subgroups
            for (uint k = 0; k < vSteps.size(); k++) {
                if (bVerbose) stdprintf("Have group %s in %ld \n", vSteps[k], hSimOut);
                hid_t hStepOut = qdf_createGroup(hSimOut, pStep);
                if (bVerbose) stdprintf("-> %ld\n", hStepOut);
                // now t hrough datasets
                hid_t hStepIn = qdf_openGroup(hSimIn, pStep);

                // collect data set names
                std::vector<std::string> vRegions;
                vRegions.clear();
                H5Literate(hStepIn, H5_INDEX_NAME, H5_ITER_INC, 0, get_dataset_name, &vRegions);
                for (uint j = 0; j < vRegions.size(); j++) {
                    const char *pDataSet = vRegions[j].c_str();
                    //printf("Have dataset %s\n", pDataSet);
                    
                    // do the copy
                    iResult = copyDataSet(hStepOut, hStepIn, pDataSet, hAgentDataType, &pBuf, &iCurSize, bVerbose);
                    

                }
                qdf_closeGroup(hStepIn);
                qdf_closeGroup(hStepOut);
            }
            qdf_closeGroup(hSimIn);
            qdf_closeGroup(hSimOut);

        }
        // clean_up
        delete[] pBuf;
        qdf_closeDataType(hAgentDataType);

        qdf_closeFile(hDest);
      

        for (uint i = 0; i < v.size(); i++) {
            qdf_closeFile(v[i]);
        }
    } else {
        stdprintf("usage: %s <hdf_out> <hdf_in>*\n", apArgV[0]);
    }
    return iResult;
}
