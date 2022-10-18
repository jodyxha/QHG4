#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <string>

#include "stdstrutilsT.h"
#include "Geography.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "GeoGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
GeoGroupReader::GeoGroupReader() {
}


//----------------------------------------------------------------------------
// createGeoGroupReader
//
GeoGroupReader *GeoGroupReader::createGeoGroupReader(const std::string sFileName) {
    GeoGroupReader *pGR = new GeoGroupReader();
    int iResult = pGR->init(sFileName, GEOGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createGeoGroupReader
//
GeoGroupReader *GeoGroupReader::createGeoGroupReader(hid_t hFile) {
    GeoGroupReader *pGR = new GeoGroupReader();
    int iResult = pGR->init(hFile, GEOGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}



//----------------------------------------------------------------------------
// tryReadAttributes
//
int GeoGroupReader::tryReadAttributes(GeoAttributes *pAttributes) {
    int iResult = GroupReader<Geography, GeoAttributes>::tryReadAttributes(pAttributes);

    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, GEO_ATTR_MAX_NEIGH, 1, &(pAttributes->m_iMaxNeighbors)); 
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, GEO_ATTR_RADIUS, 1, &(pAttributes->m_dRadius)); 
    }                
    if (iResult == 0) {
        iResult = qdf_extractAttribute(m_hGroup, GEO_ATTR_SEALEVEL, 1, &(pAttributes->m_dSeaLevel)); 
    }                


    return iResult;
}



//-----------------------------------------------------------------------------
// readArray
//
int GeoGroupReader::readArray(Geography *pGG, const std::string sArrayName) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        ((m_pAttributes->m_iNumCells == pGG->m_iNumCells) && 
         (m_pAttributes->m_iMaxNeighbors == pGG->m_iMaxNeighbors))) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            stdprintf("Number of cells or max neighbors do not correspond:\n");
            if (m_pAttributes->m_iNumCells != pGG->m_iNumCells) {
               stdprintf("  GeoGroupReader::m_iNumCells: %d; Geography::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pGG->m_iNumCells);
            }
            if (m_pAttributes->m_iMaxNeighbors != pGG->m_iMaxNeighbors) {
                stdprintf("  GeoGroupReader::m_iMaxNeighbors: %d; Geography::m_iMaxNeighbors: %d\n", m_pAttributes->m_iMaxNeighbors,  pGG->m_iMaxNeighbors);
            }
        }
    }

    if (iResult == 0) {
        if (sArrayName == GEO_DS_LONGITUDE) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_LONGITUDE, pGG->m_iNumCells, pGG->m_adLongitude);
        } else if (sArrayName == GEO_DS_LATITUDE) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_LATITUDE,  pGG->m_iNumCells, pGG->m_adLatitude);
        } else if (sArrayName == GEO_DS_ALTITUDE) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_ALTITUDE,  pGG->m_iNumCells, pGG->m_adAltitude);
        } else if (sArrayName == GEO_DS_AREA) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_AREA,      pGG->m_iNumCells, pGG->m_adArea);
        } else if (sArrayName == GEO_DS_DISTANCES) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_DISTANCES, pGG->m_iNumCells*pGG->m_iMaxNeighbors, pGG->m_adDistances);
        } else if (sArrayName == GEO_DS_ICE_COVER) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_ICE_COVER, pGG->m_iNumCells, (char *)pGG->m_abIce);
        } else if (sArrayName == GEO_DS_WATER) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_WATER,     pGG->m_iNumCells, pGG->m_adWater);
        } else if (sArrayName == GEO_DS_COASTAL) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_COASTAL,   pGG->m_iNumCells, (char *)pGG->m_abCoastal);
        } else {
            stdprintf("Unknown array [%s]\n", sArrayName);
            iResult = -1;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readData
//
int GeoGroupReader::readData(Geography *pGG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        ((m_pAttributes->m_iNumCells == pGG->m_iNumCells) && 
         (m_pAttributes->m_iMaxNeighbors == pGG->m_iMaxNeighbors))) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            stdprintf("Number of cells or max neighbors do not correspond:\n");
            if (m_pAttributes->m_iNumCells != pGG->m_iNumCells) {
               stdprintf("  GeoGroupReader::m_iNumCells: %d; Geography::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pGG->m_iNumCells);
            }
            if (m_pAttributes->m_iMaxNeighbors != pGG->m_iMaxNeighbors) {
                stdprintf("  GeoGroupReader::m_iMaxNeighbors: %d; Geography::m_iMaxNeighbors: %d\n", m_pAttributes->m_iMaxNeighbors,  pGG->m_iMaxNeighbors);
            }
        }
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_LONGITUDE, pGG->m_iNumCells, pGG->m_adLongitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_LATITUDE, pGG->m_iNumCells, pGG->m_adLatitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_ALTITUDE, pGG->m_iNumCells, pGG->m_adAltitude);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_AREA,     pGG->m_iNumCells, pGG->m_adArea);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_DISTANCES, pGG->m_iNumCells*pGG->m_iMaxNeighbors, pGG->m_adDistances);
    }
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGroup, GEO_DS_ICE_COVER, pGG->m_iNumCells, (char *)pGG->m_abIce);
        /*
           stdprintf("Read ice    : ");
            for (int i = 0; i < 32; i++) {
               stdprintf(" %d", pGG->m_abIce[i]);
            }
           stdprintf(" ...\n");
        */
    }
    if (iResult == 0) {
        if (qdf_link_exists(m_hGroup, GEO_DS_WATER)) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_WATER, pGG->m_iNumCells, pGG->m_adWater);
        }
    }
    if (iResult == 0) {
        if (qdf_link_exists(m_hGroup, GEO_DS_COASTAL)) {
            iResult = qdf_readArray(m_hGroup, GEO_DS_COASTAL, pGG->m_iNumCells,  (char *)pGG->m_abCoastal);
            /*
           stdprintf("Read coastal: ");
            for (int i = 0; i < 32; i++) {
               stdprintf(" %d", pGG->m_abCoastal[i]);
            }
           stdprintf(" ...\n");
            */
        }
    }

    pGG->m_bUpdated = true;

    return iResult;


}
