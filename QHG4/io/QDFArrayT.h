#ifndef __QDFARRAYT_H__
#define __QDFARRAYT_H__

#include <hdf5.h>
#include <string>
#include <vector>
#include "types.h"
#include "QDFArray.h"

//----------------------------------------------------------------------------
// getFirstSlab
//
template<typename T>
int QDFArray::getFirstSlab(T *pBuffer, const uint iSize, const std::string sFieldName) {
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

#endif
