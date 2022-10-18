#include <cstdio>
#include <cstring>


#include <map>
#include <set>

#include <hdf5.h>

#include "types.h"
#include "QDFUtils.h"
#include "CellBirths.h"

//-----------------------------------------------------------------------------
// createInstance
//
CellBirths *CellBirths::createInstance(int iNumCells) {
    CellBirths *pCB = new CellBirths();
    int iResult = pCB->init(iNumCells);
    if (iResult != 0) {
        delete pCB;
        pCB = NULL;
    }
    return pCB;
}
        
    
//-----------------------------------------------------------------------------
// addBirth
//
void CellBirths::addBirth(float fStep, gridtype iCellID) {
    m_mBirthCounts[fStep][iCellID]++;
}

//-----------------------------------------------------------------------------
// writeDataQDF
//
int CellBirths::writeDataQDF(hid_t hGroup) {
    int iResult = 0;
    if (!bCondensed) {
        condense();
    }

    // create an array containing the keys
    uint iSize  = m_mBirthCounts.size();
    float *pKeys = new float[iSize];
    std::map<float, std::map<gridtype, int>>::const_iterator it; 
    int i = 0;
    for (it = m_mBirthCounts.begin(); it != m_mBirthCounts.end(); ++it) {
        pKeys[i++] = it->first;
    }
    // write  it
    if (iResult == 0) {
        printf("[CellBirths::writeDataQDF] writing keys[%d]\n", iSize);
        iResult = qdf_writeArray(hGroup, "CellBirth_keys", iSize, pKeys);
    }
    delete[] pKeys;

    if (iResult == 0) {
        printf("[CellBirths::writeDataQDF] writing data[%d]\n", iSize*m_iNumCells);
        iResult = qdf_writeArray(hGroup, "CellBirth_data", iSize*m_iNumCells, m_pCounts);
    }
    
    return iResult;

}


//-----------------------------------------------------------------------------
// condense
//
void CellBirths::condense() {
    if (m_pCounts != NULL) {
        delete[] m_pCounts;
    }
    /*
    std::map<float, std::map<gridtype, int>>::const_iterator ita; 
    for (ita = m_mBirthCounts.begin(); ita != m_mBirthCounts.end(); ++ita) {
        printf("Step [%f]\n", ita->first);
        std::map<gridtype, int>::const_iterator ita2;
        for (ita2 = ita->second.begin(); ita2 != ita->second.end(); ++ita2) {
            printf("  [%d]: %d\n", ita2->first, ita2->second);
        }
    }
    */
    m_pCounts = new uint[m_iNumCells*m_mBirthCounts.size()*sizeof(int)];
    memset(m_pCounts, 0, m_iNumCells*m_mBirthCounts.size()*sizeof(int));
    std::map<float, std::map<gridtype, int>>::const_iterator it; 
    int iLayer = 0;
    for (it = m_mBirthCounts.begin(); it != m_mBirthCounts.end(); ++it) {
        std::map<gridtype, int>::const_iterator it2;
        for (it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            m_pCounts[iLayer+it2->first] = it2->second;
        }
        iLayer += m_iNumCells;
    }
 
    bCondensed = true;
}



//-----------------------------------------------------------------------------
// showAll
//
void CellBirths::showAll() {

    if (!bCondensed) {
        condense();
    }

    int iTot = 0;
    std::map<float, std::map<gridtype, int>>::const_iterator it; 
    uint i = 0;
    for (it = m_mBirthCounts.begin(); it != m_mBirthCounts.end(); ++it) {
        int iSub = 0;
        for (uint j = 0; j < m_iNumCells; j++) {
            iSub += m_pCounts[i+j];
        }

        printf("Step [%f] (%zd nonzeros, subtotal %d)\n", it->first, it->second.size(), iSub);

        iTot += iSub;
        for (uint j = 0; j < m_iNumCells; j++){
            printf("  [%4d]: %4d\n", j, m_pCounts[i+j]);
        }
        i += m_iNumCells;
    }
    printf("Total: %5d\n", iTot);

}


//-----------------------------------------------------------------------------
// 
//
CellBirths::CellBirths() 
    : m_iNumCells(0),
      m_pCounts(NULL),
      bCondensed(false) {
    
    m_mBirthCounts.clear();
}

//-----------------------------------------------------------------------------
// 
//
CellBirths::~CellBirths() {
    
    delete[] m_pCounts;
}


//-----------------------------------------------------------------------------
// init
//
int CellBirths::init(uint iNumCells) {
    int iResult = 0;
    m_iNumCells = iNumCells;

    return iResult;
}


//-----------------------------------------------------------------------------
// reset
//
void CellBirths::reset() {

    m_mBirthCounts.clear();
    bCondensed = false;
}
