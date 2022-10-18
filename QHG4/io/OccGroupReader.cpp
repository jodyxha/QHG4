#include <cstdio>
#include <cstring>
#include <hdf5.h>

#include "stdstrutilsT.h"
#include "OccHistory.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "OccGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
OccGroupReader::OccGroupReader() {
}


//----------------------------------------------------------------------------
// createOccGroupReader
//
OccGroupReader *OccGroupReader::createOccGroupReader(const std::string sFileName) {
    OccGroupReader *pOR = new OccGroupReader();
    int iResult = pOR->init(sFileName, OCCGROUP_NAME);
    if (iResult != 0) {
        delete pOR;
        pOR = NULL;
    }
    return pOR;
}

//----------------------------------------------------------------------------
// createOccGroupReader
//
OccGroupReader *OccGroupReader::createOccGroupReader(hid_t hFile) {
    OccGroupReader *pOR = new OccGroupReader();
    int iResult = pOR->init(hFile, OCCGROUP_NAME);
    if (iResult != 0) {
        delete pOR;
        pOR = NULL;
    }
    return pOR;
}


//----------------------------------------------------------------------------
// tryReadAttributes
//
int OccGroupReader::tryReadAttributes(OccAttributes *pAttributes) {
    // we do *not* call the super class method, because occhistory has no numcells
    int iResult = -1;

    herr_t status;
    hsize_t dims[1];
    hid_t hAttr = H5Aopen_name(m_hGroup, OCC_ATTR_POP_NAMES.c_str());
    if (hAttr >= 0) {
        hid_t hDataType  = H5Aget_type(hAttr);
        hid_t hSpace = H5Aget_space(hAttr);
        int rank = H5Sget_simple_extent_ndims(hSpace);
        status = H5Sget_simple_extent_dims(hSpace, dims, NULL);
        if (status >= 0) {
            size_t size = H5Tget_size (hDataType);
            char *pBuf = new char[size*dims[0]];
            stdprintf("rank %d, dom[0] %llu, size %lu\n", rank, dims[0], size);
            status = H5Aread(hAttr, hDataType, pBuf);
            if (status >= 0) {
                // split strings and put innto vector
                
                char *p = pBuf;
                for (uint i = 0; i <dims[0]; i++) {
                    pAttributes->m_vPopNames.push_back(p);
                    stdprintf("  [%s]\n", p);
                    p+= size;
                }
                iResult = 0;
            } else {
                stdprintf("couldn't read attribute [%s]\n", OCC_ATTR_POP_NAMES);
            }
        } else {
            stdprintf("couldn't get extents for attribute [%s]\n", OCC_ATTR_POP_NAMES);
        }
        qdf_closeAttribute(hAttr);
    } else {
        stdprintf("couldn't open attribute [%s]\n", OCC_ATTR_POP_NAMES);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readArray
//
int OccGroupReader::readArray(OccHistory *pOcc, const std::string sArrayName) {
    int iResult = -1;

    return iResult;
}


//-----------------------------------------------------------------------------
// readData
//
int OccGroupReader::readData(OccHistory *pOcc) {
    int iResult = -1;
    if (qdf_link_exists(m_hGroup, OCC_DS_OCCTRACK)) {
        hid_t hDataSet   = H5Dopen(m_hGroup, OCC_DS_OCCTRACK.c_str(), H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);
        hsize_t dims[2];
        H5Sget_simple_extent_dims(hDataSpace, dims, NULL);
        uchar *pBuf = new uchar[dims[0]];
        
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSet(hDataSet);

        iResult = qdf_readArray(m_hGroup, OCC_DS_OCCTRACK, dims[0], pBuf);
        if (iResult == 0) {
            iResult = pOcc->deserialize(pBuf);
            if (iResult == 0) {
                stdprintf("successfully extracted data\n");
            } else {
                stdprintf("couldn't deserialize buffer\n");
            }
        } else {
            stdprintf("couldn't read array\n");
        }
        delete[] pBuf;
    } else {
        stdprintf("Dataset [%s] does not exist in group\n", OCC_DS_OCCTRACK);
    }
    return iResult;
}
