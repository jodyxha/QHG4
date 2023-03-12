#ifndef __QDFUTILS_CPP__
#define __QDFUTILS_CPP__


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <hdf5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "stdstrutilsT.h"
#include "PolyLine.h"
#include "QDFUtilsT.h"
#include "QDFUtils.h"



#define ROOT_ATTR_VALUE  "A QHG4 Data File"

static std::map<hid_t, std::string> msTypeNames = {
    {H5T_NATIVE_CHAR, "char"},
    {H5T_NATIVE_SCHAR, "signed char"},
    {H5T_NATIVE_UCHAR, "unsigned char"},
    {H5T_NATIVE_SHORT, "short"},
    {H5T_NATIVE_USHORT, "unsigned short"},
    {H5T_NATIVE_INT, "int"},
    {H5T_NATIVE_INT32, "int"},
    {H5T_NATIVE_UINT, "unsigned"},
    {H5T_NATIVE_UINT32, "unsigned"},
    {H5T_NATIVE_LONG, "long"},
    {H5T_NATIVE_ULONG, "unsigned long"},
    {H5T_NATIVE_LLONG, "long long"},
    {H5T_NATIVE_ULLONG, "unsigned long long"},
    {H5T_NATIVE_FLOAT, "float"},
    {H5T_NATIVE_DOUBLE, "double"}, 
    {H5T_NATIVE_LDOUBLE, "long double"},
    {H5T_NATIVE_HSIZE, "hsize_t"},
    {H5T_NATIVE_HSSIZE, "hssize_t"}, 
    {H5T_NATIVE_HERR, "herr_t"},
    {H5T_NATIVE_HBOOL, "hbool_t"}
};




//----------------------------------------------------------------------------
// close
//
void qdf_closeFile(hid_t hFile) {
    if (hFile > 0) {
        H5Fclose(hFile);
    }
}


//----------------------------------------------------------------------------
// create
//
hid_t qdf_createFile(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString) {
    int iResult = -1;
    hid_t hFile = H5P_DEFAULT;

    H5E_auto2_t  hOldFunc;
    void *old_client_data;
    hid_t hErrorStack = H5Eget_current_stack();

    // remember previous settings
    H5Eget_auto(hErrorStack, &hOldFunc, &old_client_data);

    // Turn off error handling */
    H5Eset_auto(hErrorStack, NULL, NULL);

    hFile = H5Fcreate(sFileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (hFile > 0) {
        //@@        printf("file opened\n");
        // prepare datatypace for attribute
        hid_t hRoot=qdf_opencreateGroup(hFile, "/", false);
    
        if (hRoot > 0) {
            //@@            printf("root ok\n");
            iResult = qdf_insertSAttribute(hRoot, ROOT_ATTR_NAME, ROOT_ATTR_VALUE);
            if (iResult == 0) {
                iResult = qdf_insertSAttribute(hRoot, ROOT_STEP_NAME, iStep);
            }
            if (iResult == 0) {
                iResult = qdf_insertSAttribute(hRoot, ROOT_TIME_NAME, fStartTime);
            }
            if (iResult == 0) {
                if (!sInfoString.empty()) {
                    iResult = qdf_insertSAttribute(hRoot, ROOT_INFO_NAME, sInfoString);
                }
            } 
            qdf_closeGroup(hRoot);
        }
        if (iResult != 0) {
            qdf_closeFile(hFile);
            hFile = H5P_DEFAULT;
            printf("pflarz\n");
        }
    } else {
        stdprintf("Couldn't create file [%s]\n", sFileName);
    }
    // Restore previous error handler
    H5Eset_auto(hErrorStack, hOldFunc, old_client_data);
  
    return hFile;
}


//----------------------------------------------------------------------------
// open
//
hid_t qdf_openFile(const std::string sFileName, bool bRW) {
    int iResult = -1;

    H5E_auto2_t  hOldFunc;
    void *old_client_data;
    hid_t hErrorStack = H5E_DEFAULT;

    // remember previous settings
    H5Eget_auto(hErrorStack, &hOldFunc, &old_client_data);

    // Turn off error handling */
    H5Eset_auto(hErrorStack, NULL, NULL);

    hid_t hFile = H5P_DEFAULT;
    struct stat buf;
    iResult = stat(sFileName.c_str(), &buf);
    if (iResult == 0) {
        hFile = H5Fopen(sFileName.c_str(), bRW?H5F_ACC_RDWR:H5F_ACC_RDONLY, H5P_DEFAULT);
        if (hFile > 0) {
            if (true || qdf_attr_exists(hFile, ROOT_ATTR_NAME)) {
                std::string sValue = qdf_extractSAttribute(hFile, ROOT_ATTR_NAME);
                if (sValue.empty()) {
                    qdf_closeFile(hFile);
                    hFile = H5P_DEFAULT;
                    iResult = -1;
                    stdfprintf(stderr, "qdf_openFile: [%s] couldn't extract attr [%s]\n", sFileName, ROOT_ATTR_NAME);
                }
            } else {
                hFile = H5P_DEFAULT;
                stdfprintf(stderr, "qdf_openFile: [%s] does not have attr [%s]\n", sFileName, ROOT_ATTR_NAME);
            }
        } else { 
            hFile = H5P_DEFAULT;
            stdfprintf(stderr, "qdf_openFile: couldn't open [%s] in %s mode\n", sFileName, bRW?"RDWR":"RDONLY");
        }
    } else {
        hFile = H5P_DEFAULT;
        stdfprintf(stderr, "qdf_openFile: [%s] doesn't exist\n", sFileName);
    }
    // Restore previous error handler
    H5Eset_auto(hErrorStack, hOldFunc, old_client_data);

    return hFile;
}


//----------------------------------------------------------------------------
// opencreate
//
hid_t qdf_opencreateFile(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString) {
    hid_t hFile = qdf_openFile(sFileName);
    if (hFile == H5P_DEFAULT) {
        hFile = qdf_createFile(sFileName, iStep, fStartTime, sInfoString);
    }
    return hFile;
}


//----------------------------------------------------------------------------
// qdf_createGroup
//
hid_t qdf_createGroup(hid_t hFile, const std::string sGroupName) {
    hid_t hGroup = H5Gcreate(hFile, sGroupName.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    return hGroup;
}


//----------------------------------------------------------------------------
// qdf_openGroup
//
hid_t qdf_openGroup(hid_t hFile, const std::string sGroupName, bool bForceCheck) {
    hid_t hGroup = H5P_DEFAULT;
    if ((!bForceCheck) || qdf_link_exists(hFile, sGroupName)) {
        hGroup = H5Gopen(hFile, sGroupName.c_str(), H5P_DEFAULT);
    }
    return hGroup;
}


//----------------------------------------------------------------------------
// qdf_opencreateGroup
//
hid_t qdf_opencreateGroup(hid_t hFile, const std::string sGroupName, bool bForceCheck) {
     hid_t hGroup = qdf_openGroup(hFile, sGroupName, bForceCheck);
     if (hGroup <= 0) {
        // does not exist: create
        hGroup = qdf_createGroup(hFile, sGroupName);
        if (hGroup > 0) {
            // success
            //@@            printf("group [%s] didn't exist: created\n", pGroupName);
        }
    }
    return hGroup;
}


//----------------------------------------------------------------------------
// qdf_closeGroup
//
void  qdf_closeGroup(hid_t hGroup) {
    if (hGroup > 0) {
        H5Gclose(hGroup);
    }
}

//----------------------------------------------------------------------------
// qdf_deleteGroup
//
int qdf_deleteGroup(hid_t hGroup, std::string sSubName) {
    int iResult = -1;
    herr_t status = H5Ldelete(hGroup, sSubName.c_str(), H5P_DEFAULT);
    if (status >= 0) {
        iResult = 0;
    } else {
        iResult = -1;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// qdf_openDataSet
//
hid_t qdf_openDataSet(hid_t hGroup, const std::string sDataSet, bool bForceCheck) {
    hid_t hDataSet = H5P_DEFAULT;
    if ((!bForceCheck) || qdf_link_exists(hGroup, sDataSet)) {
        hDataSet = H5Dopen(hGroup, sDataSet.c_str(), H5P_DEFAULT);
    }
    return hDataSet;
}


//----------------------------------------------------------------------------
// qdf_closeDataSet
//
void  qdf_closeDataSet(hid_t hDataSet) {
    if (hDataSet > 0) {
        H5Dclose(hDataSet);
    }
}
 

//----------------------------------------------------------------------------
// qdf_closeDataSpace
//
void  qdf_closeDataSpace(hid_t hDataSpace) {
    if (hDataSpace > 0) {
        H5Sclose(hDataSpace);
    }
}


//----------------------------------------------------------------------------
// qdf_closeDataType
//
void  qdf_closeDataType(hid_t hDataType) {
    if (hDataType > 0) {
        H5Tclose(hDataType);
    }
}


//----------------------------------------------------------------------------
// qdf_closeAttribute
//
void  qdf_closeAttribute(hid_t hAttribute) {
    if (hAttribute > 0) {
        H5Aclose(hAttribute);
    }
}


//----------------------------------------------------------------------------
// qdf_link_exists
//
bool qdf_link_exists(hid_t hGroup, const std::string sName) {
    return (H5Lexists(hGroup, sName.c_str(), H5P_DEFAULT));
}

//----------------------------------------------------------------------------
// qdf_attr_exists
//
bool qdf_attr_exists(hid_t hGroup, const std::string sName) {
    return (H5Aexists(hGroup, sName.c_str()));
}


//----------------------------------------------------------------------------
// insertSAttribute
//    strings
//
int qdf_insertSAttribute(hid_t hLoc, const std::string sName, const std::string sValue) {
    return qdf_insertSAttribute(hLoc, sName, sValue.c_str());
}


//----------------------------------------------------------------------------
// insertSAttribute
//    strings
//
int qdf_insertSAttribute(hid_t hLoc, const std::string sName, const char  *pValue) {
    int iResult = -1;
    hid_t hAttrMemType = H5Tcopy (H5T_C_S1);
    herr_t status = H5Tset_size (hAttrMemType, strlen(pValue)+1);
    hsize_t dims = 1;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, sName.c_str(), 
                                 hAttrMemType, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);
 
    status = H5Awrite (hAttribute, hAttrMemType, pValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        fprintf(stderr, "Couldn't create root attribute\n");
    } 
    qdf_closeDataType(hAttrMemType);
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}


//----------------------------------------------------------------------------
// qdf_insertAttribute
//    generic
//
int qdf_insertAttribute(hid_t hLoc, const std::string sName, const uint iNum, void *vValue, const hid_t hType) {
    int iResult = -1;
    hsize_t dims = iNum;
    hid_t hAttrSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttribute = H5Acreate(hLoc, sName.c_str(), 
                                 hType, hAttrSpace, 
                                 H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite (hAttribute, hType, vValue);
    
    if (status == 0) {
        iResult = 0;
    } else {
        stdprintf("write %s attribute err\n", msTypeNames[hType]);
    } 
    qdf_closeDataSpace(hAttrSpace);
    qdf_closeAttribute(hAttribute);

    return iResult;
}


//----------------------------------------------------------------------------
// qdf_extractAttribute
//    generic
//
int qdf_extractAttribute(hid_t hLoc, const std::string sName, const uint iNum, void *vValue, const hid_t hType) {
    int iResult = -1;
    hsize_t dims[2];

    if (qdf_attr_exists(hLoc, sName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, sName.c_str());
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
        
            herr_t status = H5Aread(hAttribute, hType, vValue);
        
            if (status == 0) {
                iResult = 0;
            } else {
                stdprintf("read %s attribute err\n", msTypeNames[hType]);
            } 
        } else {
            stdprintf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
    } else {
        stdprintf("Attribute [%s] does not exist\n", sName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// qdf_extractSAttribute
//    string as return value
//
std::string qdf_extractSAttribute(hid_t hLoc, const std::string sName) {

    std::string sValue = "";    
    if (qdf_attr_exists(hLoc, sName)) {
        hid_t hAttribute = H5Aopen_by_name(hLoc, ".", sName.c_str(), H5P_DEFAULT, H5P_DEFAULT);
        hid_t atype  = H5Aget_type(hAttribute);
        hsize_t size = H5Tget_size (atype);
        char *pString = new char[size+1];
        memset(pString, 0, size+1);
        herr_t status = H5Aread(hAttribute, atype, pString);
        if (status >= 0) {
	    sValue = pString;
        }
        qdf_closeAttribute(hAttribute);
        delete[] pString;
    } else {
        stdprintf("Attribute [%s] does not exist\n", sName);
    }
    return std::string(sValue);
}

//----------------------------------------------------------------------------
// qdf_extractSAttribute
//    string as reference
//
int qdf_extractSAttribute(hid_t hLoc, const std::string sName, std::string &sValue) {
    int iResult = -1;

    sValue = "";    
    if (qdf_attr_exists(hLoc, sName)) {
        hid_t hAttribute = H5Aopen_by_name(hLoc, ".", sName.c_str(), H5P_DEFAULT, H5P_DEFAULT);
        hid_t atype  = H5Aget_type(hAttribute);
        hsize_t size = H5Tget_size (atype);
        char *pString = new char[size+1];
        memset(pString, 0, size+1);
        herr_t status = H5Aread(hAttribute, atype, pString);
        if (status >= 0) {
	    sValue = pString;
            iResult = 0;
        }
        qdf_closeAttribute(hAttribute);
        delete[] pString;
    } else {
        stdprintf("Attribute [%s] does not exist\n", sName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// qdf_readArray
//   generic
//
int qdf_readArray(hid_t hGroup, const std::string sName, const uint iNum, void *vData, const hid_t hType) {
    herr_t status = H5P_DEFAULT;
    if (qdf_link_exists(hGroup, sName)) {
        hid_t hDataSet   = qdf_openDataSet(hGroup, sName);
        status = H5Dread(hDataSet, hType, H5S_ALL, H5S_ALL, H5P_DEFAULT, vData);
        qdf_closeDataSet(hDataSet);
    } else {
        stdprintf("Dataset [%s] does not exist in group\n", sName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_writeArray
//   generic
//
int qdf_writeArray(hid_t hGroup, const std::string sName, const uint iNum, void *vData, const hid_t hType) {
    hsize_t dim = iNum;
    hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, sName.c_str(), hType, hDataSpace, 
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    herr_t status = H5Dwrite(hDataSet, hType, H5S_ALL, H5S_ALL, H5P_DEFAULT, vData);

    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_replaceArray
//   generic
//
int qdf_replaceArray(hid_t hGroup, const std::string sName, const uint iNum, void *vData, const hid_t hType) {
    herr_t status = H5P_DEFAULT;
    if (!qdf_link_exists(hGroup, sName)) {
        status = qdf_writeArray(hGroup, sName, iNum, vData, hType);
    } else {
        hid_t hDataSet   = qdf_openDataSet(hGroup, sName);
        status = H5Dwrite(hDataSet, hType, H5S_ALL, H5S_ALL, H5P_DEFAULT, vData);

        qdf_closeDataSet(hDataSet);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_readArrays
//   double**
//
int qdf_readArrays(hid_t hGroup, const std::string sName, const uint iNumArr, const uint iSize, double **pData) {
    herr_t status = H5P_DEFAULT;
    
    if (qdf_link_exists(hGroup, sName)) {
        hsize_t iCount[2];
        iCount[0] = iSize;  
        iCount[1] = 1;

        hsize_t iOffset[2];
        iOffset[0] = 0;
        iOffset[1] = 0;
        hsize_t iStride[2] = {1, 1};
        hsize_t iBlock[2]  = {1, 1};
   
        hsize_t dims = iSize;

        hid_t hMemSpace  = H5Screate_simple (1, &dims, NULL); 
        hid_t hDataSet   = qdf_openDataSet(hGroup, sName);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        for(uint i = 0; i < iNumArr; i++) {
            status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                         iOffset, iStride, iCount, iBlock);
            status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                             hDataSpace, H5P_DEFAULT, pData[i]);


            iOffset[1]++;
        }


        qdf_closeDataSet(hDataSet);
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSpace(hMemSpace);
    } else {
        stdprintf("Dataset [%s] does not exist in group\n", sName);
    }
    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// qdf_writeArrays
//   double**
//
int qdf_writeArrays(hid_t hGroup, const std::string sName, const uint iNumArr, const uint iSize, const double **pData) {
    herr_t status   = H5P_DEFAULT;

    hsize_t iCount[2];
    iCount[0] = iSize;
    iCount[1] = 1;
  
    hsize_t iOffset[2];
    iOffset[0] = 0;
    iOffset[1] = 0;

    hsize_t iStride[2] = {1, 1};
    hsize_t iBlock[2]  = {1, 1};

    hsize_t dims[2];
    dims[0] = iSize;
    dims[1] = iNumArr;

    hid_t hDataSpace = H5Screate_simple(2, dims, NULL);
    hid_t hDataSet   = H5Dcreate2(hGroup, sName.c_str(),  H5T_NATIVE_DOUBLE, hDataSpace, 
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    hid_t hMemSpace  = H5Screate_simple (1, iCount, NULL); 

    for(uint i = 0; i < iNumArr; i++) {
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     iOffset, iStride, iCount, iBlock);
        status = H5Dwrite(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData[i]);

        iOffset[1]++;
    }
    qdf_closeDataSet(hDataSet);
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSpace(hMemSpace);
    return (status >= 0)?0:-1;
}


//-----------------------------------------------------------------------------
// group_info
//  callback for H5Giterate2
//
static herr_t group_info(hid_t loc_id, const char *name, void *opdata) {
    herr_t ret=0;
    printf("%s%s\n", (const char*) opdata, name);
    return ret;
}


//----------------------------------------------------------------------------
// listDataSets
//
int qdf_listGroupContents(hid_t loc_id, const std::string sName, const char *pIndent) {
    int iResult = 0;
    //   printf("  %sA(", pIndent);
    printf("%s  DataSets\n", pIndent);
    char sIndent[strlen(pIndent)+5];
    sprintf(sIndent, "    %s", pIndent);
    int idx=0;
    herr_t status = H5Giterate(loc_id, sName.c_str(), &idx, group_info, sIndent);
    
    iResult = (status >= 0)?0:-1;
    return iResult;        
}


//----------------------------------------------------------------------------
// qdf_getDataType
//
int qdf_getDataType(hid_t hDataSet) {
    int iType = DS_TYPE_NONE;
    
    hid_t hType = H5Dget_type(hDataSet);
    if (H5Tequal(hType, H5T_NATIVE_CHAR) > 0)  {
        iType = DS_TYPE_CHAR;
    } else if (H5Tequal(hType, H5T_NATIVE_SHORT) > 0)  {
        iType = DS_TYPE_SHORT;
    } else if (H5Tequal(hType, H5T_NATIVE_INT) > 0)  {
        iType = DS_TYPE_INT;
    } else if (H5Tequal(hType, H5T_NATIVE_LONG) > 0)  {
        iType = DS_TYPE_LONG;
    } else if (H5Tequal(hType, H5T_NATIVE_LLONG) > 0)  {
        iType = DS_TYPE_LLONG;
    } else if (H5Tequal(hType, H5T_NATIVE_UCHAR) > 0)  {
        iType = DS_TYPE_UCHAR;
    } else if (H5Tequal(hType, H5T_NATIVE_USHORT) > 0)  {
        iType = DS_TYPE_USHORT;
    } else if (H5Tequal(hType, H5T_NATIVE_UINT) > 0)  {
        iType = DS_TYPE_UINT;
    } else if (H5Tequal(hType, H5T_NATIVE_ULONG) > 0)  {
        iType = DS_TYPE_ULONG;
    } else if (H5Tequal(hType, H5T_NATIVE_ULLONG) > 0)  {
        iType = DS_TYPE_ULLONG;
    } else if (H5Tequal(hType, H5T_NATIVE_FLOAT) > 0)  {
        iType = DS_TYPE_FLOAT;
    } else if (H5Tequal(hType, H5T_NATIVE_DOUBLE) > 0)  {
        iType = DS_TYPE_DOUBLE;
    } else if (H5Tequal(hType, H5T_NATIVE_LDOUBLE) > 0)  {
        iType = DS_TYPE_LDOUBLE;
    } else {
        iType = DS_TYPE_NONE;
    }
    H5Tclose(hType);
    return iType;
}



//----------------------------------------------------------------------------
// qdf_getDataExtents 
//
int qdf_getDataExtents(hid_t hGroup, const std::string sName, std::vector<hsize_t> &vSizes) {
    int iResult = -1;
    
    hid_t hDataSet   = qdf_openDataSet(hGroup, sName);
    if (hDataSet != H5P_DEFAULT) {
        hid_t hDataSpace = H5Dget_space (hDataSet);
        if (hDataSpace != H5P_DEFAULT) {
            int iNumDims     = H5Sget_simple_extent_ndims(hDataSpace);
            
            hsize_t *pSizes = new hsize_t[iNumDims];
            memset(pSizes, 0, iNumDims*sizeof(hsize_t));
            iNumDims = H5Sget_simple_extent_dims (hDataSpace, pSizes, NULL);
            if (iNumDims >= 0) {
                vSizes.clear();
                vSizes.insert(vSizes.begin(), pSizes, pSizes+iNumDims);
                iResult = 0;
            }
            delete[] pSizes;
            
        } else {
            stdprintf("Couldn't get data space for data set\n");
        }
    } else {
        stdprintf("Dataset [%s] does not exist in group\n", sName);
        
    }
    return iResult;
}

//----------------------------------------------------------------------------
// popInfo
//  callback function for iteration in getFirstPopulation()
//
herr_t popInfoUtil(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
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
// qdf_getFirstPopulation
//  create and return name for first population found in QDF file
//  the returned string must be delete[]d by the caller
//
const std::string qdf_getFirstPopulation(const std::string sPopQDF) {
    std::string sPopName = "";

    hid_t hFile = qdf_openFile(sPopQDF);
    if (hFile > 0) {
        if (qdf_link_exists(hFile, POPGROUP_NAME)) {
            hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
            if (hPopGroup > 0) {
                char s[256];
                *s = '\0';
                H5Literate(hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popInfoUtil, s);
                
                if (*s != '\0') {
                    sPopName = s;
                
                } else {
                    stdprintf("No Population subgroup in file [%s]\n", sPopQDF);
                }
                qdf_closeGroup(hPopGroup);
            }
        } else {
            stdprintf("No Population Group in file [%s]\n", sPopQDF);
        }
        qdf_closeFile(hFile);
    } else {
        stdprintf("Not a QDF file [%s]\n", sPopQDF);
    }
    return sPopName;
}


//----------------------------------------------------------------------------
// qdf_checkForPop
//  create and return name of population if it appears in QDF file
//  the returned string must be delete[]d by the caller
//
const std::string qdf_checkForPop(const std::string sPopQDF, const std::string sSpeciesName) {
    std::string sPop = "";

    hid_t hFile = qdf_openFile(sPopQDF);
    if (hFile > 0) {
        if (qdf_link_exists(hFile, POPGROUP_NAME)) {
            hid_t hPopGroup = qdf_openGroup(hFile, POPGROUP_NAME);
            if (hPopGroup > 0) {
                if (qdf_link_exists(hPopGroup, sSpeciesName)) {
                    sPop = sSpeciesName;
                }
                qdf_closeGroup(hPopGroup);
            }
            qdf_closeFile(hFile);
        } else {
            stdprintf("No Population Group in file [%s]\n", sPopQDF);
        }
    } else {
        stdprintf("Not a QDF file [%s]\n", sPopQDF);
    }
    return std::string(sPop);
}


//----------------------------------------------------------------------------
// qdf_hasGeo
//  check if geo and grid sections are present
//
bool qdf_hasGeo(const std::string sQDF) {
    bool bResult = false;

    hid_t hFile = qdf_openFile(sQDF);
    if (hFile > 0) {
        if (qdf_link_exists(hFile, GRIDGROUP_NAME)) {
            if (qdf_link_exists(hFile, GEOGROUP_NAME)) {
                bResult = true;
            }
        }
        qdf_closeFile(hFile);
    } else {
        stdprintf("Not a QDF file [%s]\n", sQDF);
    }
    return bResult;
}


//-----------------------------------------------------------------------------
// qdf_createPolyLine
//
PolyLine *qdf_createPolyLine(hid_t hSpeciesGroup, const std::string sPLParName) {
    stdprintf("qdf_createPoly will work on %s\n", sPLParName);
    PolyLine *pPL = NULL;

    hid_t hAttr = H5Aopen(hSpeciesGroup, sPLParName.c_str(), H5P_DEFAULT);

    hid_t hSpace = H5Aget_space(hAttr);
    hsize_t dims = 3;
    hvl_t       aPLData[3];
    int ndims = H5Sget_simple_extent_dims(hSpace, &dims, NULL);
    if (ndims == 1) {
        hid_t hMemType = H5Tvlen_create (H5T_NATIVE_DOUBLE);

        herr_t status = H5Aread(hAttr, hMemType, aPLData);
            
        if (status >= 0) {
            if (aPLData[0].len > 0) {
                pPL = new PolyLine((uint)aPLData[0].len-1);
                memcpy(pPL->m_afX, aPLData[0].p, (pPL->m_iNumSegments+1)*sizeof(double));
                memcpy(pPL->m_afV, aPLData[1].p, (pPL->m_iNumSegments+1)*sizeof(double));
                memcpy(pPL->m_afA, aPLData[2].p, pPL->m_iNumSegments*sizeof(double));
            } else {
                pPL = new PolyLine((uint)0);
            }
        } else {
            stdprintf("Couldn't read %s attribute\n", sPLParName);
            
        }
        status = H5Dvlen_reclaim (hMemType, hSpace, H5P_DEFAULT, aPLData);
       
        qdf_closeAttribute(hAttr);
        qdf_closeDataType(hMemType);
        qdf_closeDataSpace(hSpace);
    } else {
        printf("Bad number of dimensions: %d\n", ndims);
    }

    return pPL;

}


//-----------------------------------------------------------------------------
// qdf_writePolyLine
//
int qdf_writePolyLine(hid_t hSpeciesGroup, PolyLine *pPL, const std::string sPLParName) {
    int iResult = 0;
    
    hvl_t       aPLData[3];
    if ((pPL != NULL) && (pPL->m_iNumSegments > 0)) {
        int iNumPoints = pPL->m_iNumSegments+1;
        aPLData[0].len = iNumPoints;
        aPLData[0].p   = (void *) malloc(iNumPoints*sizeof(double));
        memcpy(aPLData[0].p, pPL->m_afX, iNumPoints*sizeof(double));
        aPLData[1].len = iNumPoints;
        aPLData[1].p   = (void *) malloc(iNumPoints*sizeof(double));
        memcpy(aPLData[1].p, pPL->m_afV, iNumPoints*sizeof(double));
        aPLData[2].len = pPL->m_iNumSegments;
        aPLData[2].p   = (void *) malloc(pPL->m_iNumSegments*sizeof(double));
        memcpy(aPLData[2].p, pPL->m_afA, pPL->m_iNumSegments*sizeof(double));
    } else {
        aPLData[0].len = 0;
        aPLData[0].p   = NULL;
        aPLData[1].len = 0;
        aPLData[1].p   = NULL;
        aPLData[2].len = 0;
        aPLData[2].p   = NULL;
    }

    hid_t hMemType = H5Tvlen_create (H5T_NATIVE_DOUBLE);
    hsize_t dims = 3;
    hid_t hSpace = H5Screate_simple (1, &dims, NULL);
    hid_t hAttr = H5Acreate (hSpeciesGroup, sPLParName.c_str(), hMemType, hSpace, H5P_DEFAULT,
                             H5P_DEFAULT);
    herr_t status = H5Awrite (hAttr, hMemType, aPLData);


    status = H5Dvlen_reclaim (hMemType, hSpace, H5P_DEFAULT, aPLData);
    qdf_closeAttribute (hAttr);
    iResult = (status >= 0)?0:-1;

    return iResult;
}


/* original version bad because H5Tget_member_type() return strange values

//-----------------------------------------------------------------------------
// qdf_compareDataTypes
//    we assume the datatype is composed of predefined H5T types only
//    (i.e. no recursion)
//    (otherwise use get_nmber_class 
bool qdf_compareDataTypes(hid_t t1, hid_t t2)  {
    bool bEqual = true;
    
    int n1 = H5Tget_nmembers(t1);
    int n2 = H5Tget_nmembers(t2);
    if (n1 == n2) {
        for (int i = 0; bEqual && (i < n1); i++) {
            // same offset?
            if (H5Tget_member_offset(t1, i) == H5Tget_member_offset(t2, i)) {
                // same type class?
                if (H5Tget_member_class(t1, i) == H5Tget_member_class(t2, i)) {
                    if (H5Tget_member_class(t1, i) == H5T_COMPOUND) {
                        bEqual = qdf_compareDataTypes(H5Tget_member_type(t1, i),H5Tget_member_type(t2, i));
                    } else {
                        if (H5Tget_member_type(t1, i) == H5Tget_member_type(t2, i)) {
                            if (strcmp(H5Tget_member_name(t1, i), H5Tget_member_name(t2, i)) == 0) {
                                bEqual = true;
                            } else {
                                printf("different names for element %d: %s - %s\n", i, H5Tget_member_name(t1, i), H5Tget_member_name(t2, i));
                                bEqual = false; // different names
                            }
                        } else {
                            printf("different types for element %d: %ld - %ld\n", i, H5Tget_member_type(t1, i), H5Tget_member_type(t2, i));
                            bEqual = false; // different types
                        }
                    }
                } else {
                    printf("different classes for element %d\n", i);
                    bEqual = false; // differen type classes
                }
            } else {
                printf("different offsets of element %d: %zd - %zd\n", i, H5Tget_member_offset(t1, i), H5Tget_member_offset(t2, i));
                bEqual = false; // different offsets
            }
        }
    } else {
        printf("different nmembers\n");
        bEqual = false;
    }

    return bEqual;
    
}
*/

//-----------------------------------------------------------------------------
// qdf_compareDataTypes
//    we assume the datatype is composed of predefined H5T types only
//    returns 0 if datatypes are equal, a negative number if different:
//    -2: different number of members
//    -3: different offsets
//    -4: different class
//    -5: different type
//    -6: different name
//    
int qdf_compareDataTypes(hid_t t1, hid_t t2)  {
    int iResult = CMP_ERR_NONE;
    
    int n1 = H5Tget_nmembers(t1);
    int n2 = H5Tget_nmembers(t2);
    if (n1 == n2) {
        for (int i = 0; (iResult == 0) && (i < n1); i++) {
            // same offset?
            if (H5Tget_member_offset(t1, i) == H5Tget_member_offset(t2, i)) {
                // same type class?
                if (H5Tget_member_class(t1, i) == H5Tget_member_class(t2, i)) {
                    if (H5Tget_member_class(t1, i) == H5T_COMPOUND) {
                        iResult = qdf_compareDataTypes(H5Tget_member_type(t1, i),H5Tget_member_type(t2, i));
                    } else {
                        if (true || (H5Tget_member_type(t1, i) == H5Tget_member_type(t2, i))) {
                            if (strcmp(H5Tget_member_name(t1, i), H5Tget_member_name(t2, i)) == 0) {
                                iResult = CMP_ERR_NONE;
                            } else {
                                iResult = CMP_ERR_NAME; // different names
                            }
                        } else {
                            iResult = CMP_ERR_TYPE; // different types
                        }
                    }
                } else {
                    iResult = CMP_ERR_CLASS; // differen type classes
                }
            } else {
                iResult = CMP_ERR_OFFSET; // different offsets
            }
        }
    } else {
        iResult = CMP_ERR_NUM_ELEMS;
    }

    return iResult;
    
}


//-----------------------------------------------------------------------------
// qdf_checkPathExists
//
int qdf_checkPathExists(const std::string sQDF, const std::string sPath) {
    int iResult = -1;

    std::string sComb = "";

    bool bSearching = true;

    stringvec vParts;
    uint iNum = splitString(sPath, vParts, "/");
    if (iNum > 0) {
        
        hid_t hFile = qdf_openFile(sQDF);

        for (uint i = 0; bSearching && (i < iNum); ++i) {
            sComb += "/" + vParts[i];
            if (qdf_link_exists(hFile, sComb)) {
                // nothing to be done
            } else {
                bSearching = false;
            }
        }

        if (bSearching) {
            iResult = 0;
        }
        qdf_closeFile(hFile);
    } else {
        // empty path
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// qdf_getSurfType
//
int qdf_getSurfType(const std::string sQDF, std::string &sSurfType) {
    int iResult = -1;
    
   hid_t hFile = qdf_openFile(sQDF);
   if (hFile > 0) {
       if (qdf_link_exists(hFile, GRIDGROUP_NAME)) {
           hid_t hGrid = qdf_openGroup(hFile, GRIDGROUP_NAME);
           sSurfType = qdf_extractSAttribute(hGrid, GRID_ATTR_SURF_TYPE); 
           if (!sSurfType.empty()) {
               iResult = 0;
           }
           qdf_closeGroup(hGrid);
       } else {
           stdprintf("QDF file [%s] has no grid group\n", sQDF);
       }
       qdf_closeFile(hFile);
   } else {
       stdprintf("Couldn't open [%s] as QDF file\n", sQDF);
   }
   return iResult;
}
#endif
