#include <cstdio>
#include <hdf5.h>

#include "types.h"
#include "strutils.h"
//#include "ParamReader.h"

#include "QDFUtils.h"

#include "SCellGrid.h"
#include "Geography.h"

#include "GridGroupReader.h"
#include "GeoGroupReader.h"
#include "WaterFinder.h"


//----------------------------------------------------------------------------
// createInstance
//
WaterFinder *WaterFinder::createInstance(const char *pQDF) {
    WaterFinder *pWF = new WaterFinder();
    int iResult = pWF->init(pQDF);
    if (iResult != 0) {
        delete pWF;
        pWF = NULL;
    }
    return pWF;
}


//----------------------------------------------------------------------------
// destructor
//
WaterFinder::~WaterFinder() {
    if (m_pCG != NULL) {
        delete m_pCG;
    }
}


//----------------------------------------------------------------------------
// constructor
//
WaterFinder::WaterFinder()
: m_iNumNodes(0),
  m_pCG(NULL) {

}
 
//----------------------------------------------------------------------------
// init
//
int WaterFinder::init(const char *pQDF) {
    int iResult = -1;

    iResult = createCellGrid(pQDF);
    if (iResult == 0) {
        printf("CellGrid created\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createCellGrid
//
int WaterFinder::createCellGrid(const char *pQDFFile) {
    m_pCG = NULL;
    int iResult = -1;
    hid_t hFile = qdf_openFile(pQDFFile);
    if (hFile > 0) {
        //        printf("File opened\n");

        GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
        if (pGR != NULL) {
            //int iNumCells=0;
            GridAttributes gridatt;
            iResult = pGR->readAttributes(&gridatt);
            if (iResult == 0) {
                //                printf("num cells: %d\n", iNumCells);
                m_iNumNodes = gridatt.m_iNumCells;
                m_pCG = new SCellGrid(0, m_iNumNodes, gridatt.smData);
                m_pCG->m_aCells = new SCell[m_iNumNodes];
                
                iResult = pGR->readData(m_pCG);
                if (iResult == 0) {
                    GeoGroupReader *pGeoR = GeoGroupReader::createGeoGroupReader(hFile);
                    if (pGeoR != NULL) {
                        GeoAttributes geoatt;
                        iResult = pGeoR->readAttributes(&geoatt);
                        if (iResult == 0) {
                            Geography *pGeo = new Geography(m_iNumNodes, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
                            
                            iResult = pGeoR->readData(pGeo);
                            if (iResult == 0) {
                                m_pCG->setGeography(pGeo);
                            } else {
                                printf("Couldn't read geo data from [%s]\n", pQDFFile);
                            }
                        } else {
                            printf("Couldn't read geo attributes from [%s]\n", pQDFFile);
                        }
                        delete pGeoR;
                    } else {
                        printf("Couldn't create GeoGroupReader for QDF file [%s]\n", pQDFFile);
                    }
                } else {
                    printf("Couldn't read geo attributes from [%s]\n", pQDFFile);
                }
            } else {
                printf("Couldn't get number of cells from [%s]\n", pQDFFile);
            }
            delete pGR;
        } else {
            printf("Couldn't create GridGroupReader for QDF file [%s]\n", pQDFFile);
        }
    } else {
        printf("Couldn't open QDF file [%s]\n", pQDFFile);
    }
    
    if (iResult != 0) {
        delete m_pCG; 
        m_pCG = NULL;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// findWaterNodes
//
int WaterFinder::findWaterNodes(double dLonMin, double dLatMin, double dLonMax, double dLatMax) {
    int iResult = 0;
    m_vWater.clear();
    double *pdWater = m_pCG->getGeography()->m_adWater;
    double *pdLon = m_pCG->getGeography()->getLongitude();
    double *pdLat = m_pCG->getGeography()->getLatitude();
    for (int iIndex =  0; iIndex < m_iNumNodes; iIndex++) {
        if (pdWater[iIndex] > 0) {

            double dLon = pdLon[iIndex];
            double dLat = pdLat[iIndex];
            if ((dLonMin <= dLon) &&
                (dLon < dLonMax) &&
                (dLatMin <= dLat) &&
                (dLat < dLatMax)) {
                m_vWater.push_back(iIndex);
            }
        }
    }
    //printf("found %zd water nodes\n", m_vWater.size());
    return iResult;
}

//----------------------------------------------------------------------------
// findWaterNeighbors
//
int WaterFinder::findWaterNeighbors() {
    int iResult = 0;
    m_vNeigh.clear();
    double *pdWater = m_pCG->getGeography()->m_adWater;

    for (uint i = 0; i < m_vWater.size(); i++) {
        SCell sCell = m_pCG->m_aCells[m_vWater[i]];
        for (int k = 0; k < sCell.m_iNumNeighbors; ++k) {
            int iNeighbor = m_pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID;
            if (pdWater[iNeighbor] <= 0) {
                m_vNeigh.push_back(iNeighbor);
            }
        }   
    }
    //printf("found %zd neighbor nodes\n", m_vNeigh.size());
    return iResult;
}



//----------------------------------------------------------------------------
// collectNodes
//
int WaterFinder::collectNodes(double dLonMin, double dLatMin, double dLonMax, double dLatMax) {
    int iResult = 0;
    
    iResult = findWaterNodes(dLonMin, dLatMin, dLonMax, dLatMax);
    if (iResult == 0) {
        iResult = findWaterNeighbors();
    }
    printf("found %zd water nodes and %zd non-water neighbors\n", m_vWater.size(), m_vNeigh.size());

    return iResult;
}

