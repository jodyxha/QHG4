#ifndef __QDFARRAY_CPP_
#define __QDFARRAY_CPP_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <hdf5.h>
#include <string>

#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "QDFArray.h"
#include "QDFArrayT.h"


//----------------------------------------------------------------------------
// create
//
QDFArray *QDFArray::create(const std::string sQDFFile) {
    QDFArray *pQDFA = new QDFArray();
    int iResult = pQDFA->init(sQDFFile);
    if (iResult != 0) {
        delete pQDFA;
        pQDFA = NULL;
    }
    return pQDFA;
}


//----------------------------------------------------------------------------
// create
//
QDFArray *QDFArray::create(hid_t hFile) {
    QDFArray *pQDFA = new QDFArray();
    int iResult = pQDFA->init(hFile);
    if (iResult != 0) {
        delete pQDFA;
        pQDFA = NULL;
    }
    return pQDFA;
}


//----------------------------------------------------------------------------
// constructor
//
QDFArray::QDFArray() 
    : m_hFile(H5P_DEFAULT),
      m_hRoot(H5P_DEFAULT),
      m_hDataSet(H5P_DEFAULT),
      m_hDataSpace(H5P_DEFAULT),
      m_hMemSpace(H5P_DEFAULT),
      m_hDataType(H5P_DEFAULT),
      m_iArraySize(0),
      m_offset(0),
      m_count(0),
      m_stride(0),
      m_block(0),
      m_memsize(0),
      m_iBufSize(0),
      m_bDeleteDataType(true),
      m_bCloseHFile(false) {

    m_vhGroups.clear();
}


//----------------------------------------------------------------------------
// destructor
//
QDFArray::~QDFArray() {

    closeArray();
    if (m_hRoot != H5P_DEFAULT) {
         qdf_closeGroup(m_hRoot);
    }
    if (m_bCloseHFile && (m_hFile != H5P_DEFAULT)) {
         qdf_closeFile(m_hFile);
    }

}


//----------------------------------------------------------------------------
// closeArray
//
void QDFArray::closeArray() {
    if (m_bDeleteDataType && (m_hDataType != H5P_DEFAULT)) {
        qdf_closeDataType(m_hDataType);
        m_hDataType = H5P_DEFAULT;
    }
    if (m_hMemSpace != H5P_DEFAULT) {
        qdf_closeDataSpace(m_hMemSpace);
        m_hMemSpace = H5P_DEFAULT;
    }
    if (m_hDataSpace != H5P_DEFAULT) {
        qdf_closeDataSpace(m_hDataSpace);
        m_hDataSpace = H5P_DEFAULT;
    }
    if (m_hDataSet != H5P_DEFAULT) {
        qdf_closeDataSet(m_hDataSet);
        m_hDataSet = H5P_DEFAULT;
    }
    for (uint i = 0; i < m_vhGroups.size(); i++) {
        if (m_vhGroups[i] != H5P_DEFAULT) {
            qdf_closeGroup(m_vhGroups[i]);
        }
    }
    m_vhGroups.clear();
 
}


//----------------------------------------------------------------------------
// init
//
int QDFArray::init(const std::string sQDFFile) {
    int iResult = -1;
    
    hid_t hFile = qdf_openFile(sQDFFile);
    if (hFile > 0) {
        m_bCloseHFile = true;
        iResult = init(hFile);
    } else {
        xha_printf("QDFArray couldn't open [%s]\n", sQDFFile);
        m_hFile = H5P_DEFAULT;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// init
//
int QDFArray::init(hid_t hFile) {
    int iResult = 0;
    m_hFile = hFile;

    std::string sValue = qdf_extractSAttribute(m_hFile, ROOT_STEP_NAME);
    if (iResult == 0) {
        if (strToNum(sValue, &m_iStep)) {
            iResult = 0;
        } else{
            xha_printf("Time step is not a float [%s]\n", sValue);
        }
    } else {
        xha_printf("No time step found\n");
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// openArray
//  open dataset in group
//
int QDFArray::openArray(const std::string sGroup, const std::string sDataset) {
    int iResult = 0;
    stringvec vGroups;
    vGroups.push_back(sGroup);
    iResult = openArray(vGroups, sDataset);
    return iResult;   
}


//----------------------------------------------------------------------------
// openArray
//  open dataset in subgroup group2 of group1
//
int QDFArray::openArray(const std::string sGroup1, const std::string sGroup2, const std::string sDataset) {
    int iResult = 0;
    stringvec vGroups;
    vGroups.push_back(sGroup1);
    vGroups.push_back(sGroup2);
    iResult = openArray(vGroups, sDataset);
    return iResult;   
}


//----------------------------------------------------------------------------
// openArray
//  open dataset given in path ("group1/.../groupN/dataset")
//
int QDFArray::openArray(const std::string sPathToDataset) {
    int iResult = -1;


    stringvec vParts;
    uint iNum = splitString(sPathToDataset, vParts, "/");
    if (iNum > 0) {
        stringvec vGroups;
        for (uint i = 0; i < iNum; ++i) {
            vGroups.push_back(vParts[i]);
        }
        

        const std::string sDataSet = vGroups.back();
        vGroups.pop_back();
        iResult = openArray(vGroups, sDataSet);
    } else {
        xha_printf("Empty path provided\n");
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// openArray
//  open dataset in "vGroups[0]/.../vGroups[N]"
//
int QDFArray::openArray(stringvec &vGroups, const std::string sDataSet) {
    int iResult = 0;

    // open the groups (last element is the dataset)
    hid_t hPrev = m_hFile;
    for (uint i = 0; (iResult == 0) && (i < vGroups.size()); i++) {
        //@@        printf("opening group [%s]...\n", vGroups[i]);
        hid_t hGroup = qdf_openGroup(hPrev, vGroups[i]);
        if (hGroup > 0) {
            m_vhGroups.push_back(hGroup);
            hPrev = hGroup;
            //@@printf("OK\n");fflush(stdout);
        } else {
            iResult = -1;
            xha_printf("failed\n");fflush(stdout);
        }
    }
    
    // open the dataset
    if (iResult == 0) {
        //@@        printf("Opening data set [%s]\n", pDataSet);
        m_hDataSet = qdf_openDataSet(m_vhGroups.back(), sDataSet);
        if (m_hDataSet > 0) {
            m_hDataSpace = H5Dget_space(m_hDataSet);
            if (m_hDataSpace >= 0) {
                hsize_t sdim[3];
                int iDims = H5Sget_simple_extent_dims(m_hDataSpace, sdim, NULL);
                if (iDims == 1) {
                    m_iArraySize = (uint)sdim[0];
                    iResult = 0;
                    
                } else {
                    xha_printf("Not a one-dimensinal data set [%s]\n", sDataSet);
                    iResult = -1;
                }
            } else {
                xha_printf("Couldn't get sata space for data set [%s]\n", sDataSet);
                iResult = -1;
            }
        } else {
            xha_printf("Couldn't open dataset [%s] in group [%s]\n", vGroups.back(), sDataSet);
            iResult = -1;
        }
    }
    
    if (iResult != 0) {
        closeArray();
    }

    return iResult;
}

/*
//----------------------------------------------------------------------------
// getFirstSlab
//
int QDFArray::getFirstSlab(int *pBuffer, uint iSize, const std::string sFieldName) {
    int iResult = -1;

    iResult = getFirstSlab(pBuffer, iSize,/ * H5T_NATIVE_INT,* / sFieldName);
    
    return iResult;
}


//----------------------------------------------------------------------------
// getFirstSlab
//
int QDFArray::getFirstSlab(double *pBuffer, const uint iSize, const std::string sFieldName) {
    int iResult = -1;
  
    iResult = getFirstSlab(pBuffer, iSize,/ * H5T_NATIVE_DOUBLE,* / sFieldName);

    return iResult;
}

//----------------------------------------------------------------------------
// getFirstSlab
//
int QDFArray::getFirstSlab(long *pBuffer, const uint iSize, const std::string sFieldName) {
    int iResult = -1;
  
    iResult = getFirstSlab(pBuffer, iSize,/ * H5T_NATIVE_LONG,* / sFieldName);

    return iResult;
}


//----------------------------------------------------------------------------
// getFirstSlab
//
int QDFArray::getFirstSlab(ulong *pBuffer, const uint iSize, const std::string sFieldName) {
    int iResult = -1;
  
    iResult = getFirstSlab(pBuffer, iSize,/ * H5T_NATIVE_ULONG,* / sFieldName);

    return iResult;
}
*/
/*
//----------------------------------------------------------------------------
// getFirstSlab
//
template<typename T>
int QDFArray::getFirstSlab(T *pBuffer, const uint iSize,/ * hid_t hBaseType,* / const std::string sFieldName) {
    int iResult = -1;
    
    m_memsize = iSize;
    if (m_iArraySize < m_memsize) {
        m_memsize = m_iArraySize;
    }
    m_hMemSpace = H5Screate_simple (1, &m_memsize, NULL); 
                    
    //@@    printf("Dataspace num elements: %lld\n",  H5Sget_simple_extent_npoints(m_hDataSpace));
    m_offset = 0;
    m_count  = m_memsize;  
                    
                    // step size when going through data (stride =2: use every second element)
    m_stride = 1;
    m_block  = 1;
    hid_t hBaseType = qdf_typeSelector<T>();
    iResult = setDataType(sFieldName, hBaseType, sizeof(T));
    if (iResult == 0) {
        iResult = readSlab(pBuffer);
    } 
    
    return iResult;
}


//----------------------------------------------------------------------------
// getNextSlab
//
template<typename T>
int QDFArray::getNextSlab(T *pBuffer, const uint iSize) {
    int iResult = 0;
    
    hsize_t remaining = m_iArraySize - m_offset;
    if (remaining < m_memsize) {
        qdf_closeDataSpace(m_hMemSpace);
        m_memsize = remaining;
        m_hMemSpace = H5Screate_simple (1, &m_memsize, NULL); 
    }

    m_count = m_memsize;

    if (iResult == 0) {
        iResult = readSlab(pBuffer);
    } 
    return iResult;
}


//----------------------------------------------------------------------------
// readSlab
//
template<typename T>
int QDFArray::readSlab(T *pBuffer) {
    int iResult = 0;

    herr_t status = H5Sselect_hyperslab(m_hDataSpace, H5S_SELECT_SET, 
                                        &m_offset, &m_stride, &m_count, &m_block);
        
    status = H5Dread(m_hDataSet, m_hDataType, m_hMemSpace,
                     m_hDataSpace, H5P_DEFAULT, pBuffer);
    if (status >= 0) {
        m_offset += m_count;
        iResult = (int)m_count;
    } else {
        printf("Error during read\n");
        iResult = -1;
    }
     
    return iResult;
}

*/
//----------------------------------------------------------------------------
// setDataType
//
int QDFArray::setDataType(const std::string sFieldName, const hid_t hBaseType, const uint iSize) {
    int iResult = -1;

    herr_t status = H5P_DEFAULT;
    if (!sFieldName.empty()) {
        hid_t hType = H5Dget_type(m_hDataSet); 
        int iR = H5Tget_member_index(hType, sFieldName.c_str());
        if (iR >=  0) {
            m_hDataType = H5Tcreate(H5T_COMPOUND, iSize);
            status = H5Tinsert(m_hDataType, sFieldName.c_str(), 0, hBaseType);
            if (status >= 0) {
                iResult = 0;
            }
            m_bDeleteDataType = true;
        } else {
            xha_printf("The field [%s]  does not exist in this Dataset\n", sFieldName);
        }
    } else {
        m_hDataType = hBaseType;
        m_bDeleteDataType = false;
        iResult = 0;
    }
    return iResult;
}


#endif
