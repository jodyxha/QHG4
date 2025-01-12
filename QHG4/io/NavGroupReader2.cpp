#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <string>

#include "xha_strutilsT.h"
#include "Navigation.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "NavGroupReader2.h"

//----------------------------------------------------------------------------
// constructor
//
NavGroupReader2::NavGroupReader2() {
}


//----------------------------------------------------------------------------
// createNavGroupReader
//
NavGroupReader2 *NavGroupReader2::createNavGroupReader2(const std::string sFileName) {
    NavGroupReader2 *pNR2 = new NavGroupReader2();
    int iResult = pNR2->init(sFileName, NAV2GROUP_NAME);
    if (iResult != 0) {
        delete pNR2;
        pNR2 = NULL;
    }
    return pNR2;
}

//----------------------------------------------------------------------------
// createNavGroupReader
//
NavGroupReader2 *NavGroupReader2::createNavGroupReader2(hid_t hFile) {
    NavGroupReader2 *pNR2 = new NavGroupReader2();
    int iResult = pNR2->init(hFile, NAV2GROUP_NAME);
    if (iResult != 0) {
        delete pNR2;
        pNR2 = NULL;
    }
    return pNR2;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int NavGroupReader2::tryReadAttributes(Nav2Attributes *pAttributes) {
    // we do *not* call the super class method, because navigation has no numcells
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV2_ATTR_NUM_CELLS,     1, &(pAttributes->m_iNumCells)); 
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV2_ATTR_NUM_WATERWAYS, 1, &(pAttributes->m_iNumWaterWays)); 
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV2_ATTR_SAMPLE_DIST,   1, &(pAttributes->m_dSampleDist)); 
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, NAV2_ATTR_NUM_BRIDGES,   1, &(pAttributes->m_iNumBridges)); 
        if (iResult != 0) {
            //doesn´t matter
            iResult = 0;
        }
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// readData
//
int NavGroupReader2::readData(Navigation2 *pNG) {
    int iResult = 0;
    // no check for number of cells required 
    
    uint iNumWaterWays = m_pAttributes->m_iNumWaterWays;
    double dSampleDist = m_pAttributes->m_dSampleDist;
   
   //create temporary arrays
    gridtype *piStarts = new gridtype[iNumWaterWays];
    gridtype *piDests  = new gridtype[iNumWaterWays];
    double   *pdDists  = new double[iNumWaterWays];
    

    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, NAV2_DS_START_NODES, iNumWaterWays, piStarts);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, NAV2_DS_DEST_NODES,  iNumWaterWays, piDests);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, NAV2_DS_DISTANCES,   iNumWaterWays, pdDists);
    }

    if (iResult == 0) {
        waterwaymap mWaterWays;

        
        for (uint i = 0; i < iNumWaterWays; i++)  {
            gridtype iStart = piStarts[i];
            gridtype iDest  = piDests[i];
            double dDist = pdDists[i];
            
            waterwaymap::iterator itWW = mWaterWays.find(iStart);
            if (itWW != mWaterWays.end()) {
                itWW->second[iDest] = dDist;
            } else {
                destdistmap mDD;
                mDD[iDest] = dDist;
                itWW->second = mDD;
            }

        }
        pNG->setData(mWaterWays, dSampleDist);
       
    }


    delete[] piStarts;
    delete[] piDests;
    delete[] pdDists;

    return iResult;
}


//-----------------------------------------------------------------------------
// readBridges
//
int NavGroupReader2::readBridges(Navigation2*pNG) {
    int iResult = 0;
    // no check for number of cells required 

    uint iNumBridges = m_pAttributes->m_iNumBridges;

    //create temporary arrays
    double *pBridges       = new double[2*iNumBridges];
    

    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, NAV2_DS_BRIDGES, 2*iNumBridges, pBridges);
    }
  
    bridgelist vBridges;
    if (iResult == 0) {
        for (uint i = 0; i < iNumBridges; ++i) {
            vBridges.push_back(bridgedef(pBridges[2*i], pBridges[2*i+1]));
        }   
    } else {
        vBridges.clear();
    }
    pNG->setBridges(vBridges);

    delete[] pBridges;
 
    return iResult;
}
