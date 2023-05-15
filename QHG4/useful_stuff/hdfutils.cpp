#include <hdf5.h>
#include <cstring>

#include "stdstrutils.h"
#include "stdstrutilsT.h"

#include "hdfutils.h"

//----------------------------------------------------------------------------
// link_exists
//
bool link_exists(hid_t hGroup, const std::string sName) {
    return (H5Lexists(hGroup, sName.c_str(), H5P_DEFAULT));
}

//----------------------------------------------------------------------------
// attr_exists
//
bool attr_exists(hid_t hGroup, const std::string sName) {
    return (H5Aexists(hGroup, sName.c_str()));
}

//----------------------------------------------------------------------------
// extract_numeric_attribute
//    generic
//
int extract_numeric_attribute(hid_t hLoc, const std::string sName, uint iNum, void *vValue, hid_t hType) {
    int iResult = -1;
    hsize_t dims[2];

    if (attr_exists(hLoc, sName)) {
        hid_t hAttribute = H5Aopen_name(hLoc, sName.c_str());
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

        if (((rank == 1) && (dims[0] == iNum)) ||
            ((rank == 0) && (iNum == 1))) {
        
            herr_t status = H5Aread(hAttribute, hType, vValue);
        
            if (status == 0) {
                iResult = 0;
            } else {
                stdprintf("read attribute err\n");
            } 
        } else {
            stdprintf("Bad Rank (%d) or bad size %llu (!= %d)\n", rank, dims[0], iNum);
        }
        H5Sclose(hAttrSpace);
        H5Aclose(hAttribute);

    } else {
        stdprintf("Attribute [%s] does not exist\n", sName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// extract_string_attribute
//    string
//
std::string extract_string_attribute(hid_t hLoc, const std::string sName) {
    stdprintf("[extract_string_attribute] name [%s]\n", sName);
    std::string sValue = "";    
    if (attr_exists(hLoc, sName)) {
        hid_t hAttribute = H5Aopen_by_name(hLoc, ".", sName.c_str(), H5P_DEFAULT, H5P_DEFAULT);
        hid_t atype  = H5Aget_type(hAttribute);
        hsize_t size = H5Tget_size (atype);
        stdprintf("[extract_string_attribute] size is %d\n", size);
        char *pString = new char[size+1];
        memset(pString, 0, size+1);
        herr_t status = H5Aread(hAttribute, atype, pString);
        if (status >= 0) {
            sValue = pString;
        }
        H5Aclose(hAttribute);
        delete[] pString;
    } else {
        stdprintf("Attribute [%s] does not exist\n", sName);
    }
    return std::string(sValue);
}



//----------------------------------------------------------------------------
// createCompoundDataType
//
hid_t createCompoundDataType(uint iSize, fieldvec &vFieldData) {
    hid_t hAgentDataType = H5P_DEFAULT;
           
    hAgentDataType = H5Tcreate(H5T_COMPOUND, iSize);

    // reading compound data: the second argument to H5Tinsert must be the name of an element in the structure 
    // used in the HDF file
    // if the name dopes not exist, noerror is produced and the  results are undefined
    // Here we must use the names of the elements of OoANavSHybYchMTDPop written to QDF 
    
    for (uint i = 0; i < vFieldData.size(); i++) {
        H5Tinsert(hAgentDataType, vFieldData[i].sName.c_str(),   vFieldData[i].iOffset,  vFieldData[i].hType);
    }    
    return hAgentDataType;
}
