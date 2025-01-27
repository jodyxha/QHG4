#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <string>

#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GroupReader.h"


//----------------------------------------------------------------------------
// constructor
//
template<class T, typename U>
GroupReader<T,U>::GroupReader() 
    :  m_hFile(H5P_DEFAULT),
       m_hGroup(H5P_DEFAULT),
       m_iNumCells(0),
       m_pAttributes(0),
       m_bCloseHFile(false) {
}

//----------------------------------------------------------------------------
// destructor
//
template<class T, typename U>
GroupReader<T,U>::~GroupReader() {
    if (m_bCloseHFile) {
        qdf_closeGroup(m_hGroup);
        qdf_closeFile(m_hFile);
    }
}


//----------------------------------------------------------------------------
// init
//
template<class T, typename U>
int GroupReader<T,U>::init(const std::string sFileName, const std::string sGroupName) {
    int iResult = -1;
    
    hid_t hFile = qdf_openFile(sFileName);
    if (hFile > 0) {
        m_bCloseHFile = true;
        iResult = init(hFile, sGroupName);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// init
//
template<class T, typename U>
int GroupReader<T,U>::init(hid_t hFile, const std::string sGroupName) {
    int iResult = -1;
    m_hGroup = qdf_openGroup(hFile, sGroupName);
    if (m_hGroup > 0) {
        m_hFile = hFile;
        iResult = 0;
    }
        
    return iResult;
}

//----------------------------------------------------------------------------
// readAttributes
//
template<class T, typename U>
int GroupReader<T,U>::readAttributes(U *pAttributes) {
    int iResult = tryReadAttributes(pAttributes);
    
    if (iResult == 0) {
        m_pAttributes = pAttributes;
    }                
    
    return iResult;
}

//----------------------------------------------------------------------------
// readAttributes
//
template<class T, typename U>
int GroupReader<T,U>::tryReadAttributes(U *pAttributes) {
    int iResult = 0;
    
    if ((iResult == 0)) {
        iResult = qdf_extractAttribute(m_hGroup, GRID_ATTR_NUM_CELLS, 1, &(pAttributes->m_iNumCells)); 
       
    }                
    
    return iResult;
}


