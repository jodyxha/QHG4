#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
#include <string>
#include <omp.h>

#include "qhg_consts.h"
#include "stdstrutilsT.h"
#include "geomutils.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"
#include "AnalysisUtils.h"
#include "CellSampler.h"


//----------------------------------------------------------------------------
// createInstance
//
CellSampler *CellSampler::createInstance(const char *pGeoQDF, const char *pLocFile, double dDistance) {
    CellSampler *pCS = new CellSampler();
    int iResult = pCS->init(pGeoQDF, pLocFile, dDistance);
    if (iResult != 0) {
        delete pCS;
        pCS = NULL;
    }
    return pCS;
}


//----------------------------------------------------------------------------
// constructor
//
CellSampler::CellSampler() {
}

//----------------------------------------------------------------------------
// destructor
//
CellSampler::~CellSampler() {
}

//----------------------------------------------------------------------------
// init
//
int CellSampler::init(const char *pGeoQDF, const char *pLocFile, double dDistance) {
    int iResult = 0;
    
    //  float f0 = omp_get_wtime();
    iResult = fillCoordMap(pGeoQDF);
    //    float f1 = omp_get_wtime();
    if (iResult == 0) {
        stdfprintf(stderr, "[CellSampler::init] have coords\n");
        locspec locSpec(pLocFile);
        m_vNames.clear();
        
        iResult = fillLocData(&locSpec, m_mLocData, &m_vNames);
        if (iResult == 0) {
            iResult = selectCells(m_mvSelected);
            if (iResult == 0) {
                
            } else {
                stdfprintf(stderr, "[CellSampler::init] Couldn't select cellsfill loc data\n");
            }
        } else {
            stdfprintf(stderr, "[CellSampler::init] Couldn't fill loc data\n");
        }

    } else {
        stdfprintf(stderr, "[CellSampler::init] Couldn't load coords\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//  get longitude and latitude arrays form the geogrid
//
int CellSampler::fillCoordMap(const char *pQDFGeoGrid) {
    int iResult = -1;
    uint iNumCells = 0;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
   
    
    QDFArray *pQA = QDFArray::create(pQDFGeoGrid);
    if (pQA != NULL) {

        // get the cell IDs
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            iNumCells = pQA->getSize();
            pCellIDs = new int[iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, iNumCells, GRID_DS_CELL_ID);
            if (iCount == iNumCells) {
                //                stdfprintf(stderr, "Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                stdfprintf(stderr, "[CellSampler::fillCoordMap] Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            stdfprintf(stderr, "[CellSampler::fillCoordMap] Couldn't open QDF array for [%s:%s/%s]\n", pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME);
        }

        // get the cell Longitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLon = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, iNumCells);
                    if (iCount == iNumCells) {
                        stdfprintf(stderr, "[CellSampler::fillCoordMap] Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "[CellSampler::fillCoordMap] Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "[CellSampler::fillCoordMap] Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE);
                }
                pQA->closeArray();
            }
        }

        // get the cell Latitudes
        if (iResult == 0) {
            std::string sPath = stdsprintf("%s/%s", GEOGROUP_NAME, GEO_DS_LATITUDE);
            iResult = pQA->openArray(sPath);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLat = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, iNumCells);
                    if (iCount == iNumCells) {
                        stdfprintf(stderr, "[CellSampler::fillCoordMap] Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "[CellSampler::fillCoordMap] Couldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL, iNumCells);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "[CellSampler::fillCoordMap] Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        stdfprintf(stderr, "[CellSampler::fillCoordMap] Couldn't create QDFArray\n");
    }
     
    // put everything into a map CellID => (lon, lat)
    if (iResult == 0) {
        // save coordinate values
        for (uint i = 0; i < iNumCells; i++) {
            m_mCoords[pCellIDs[i]] = std::pair<double, double>(pdLon[i], pdLat[i]);
        }
    }

    if (pCellIDs != NULL) {
        delete[] pCellIDs;
    }
    if (pdLon != NULL) {
        delete[] pdLon;
    }
    if (pdLat != NULL) {
        delete[] pdLat;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// getCandidatesNew1
//
int CellSampler::selectCells(loc_cells &mvCandidates) {
    int iResult = 0;

    stdfprintf(stderr, "[CellSampler::selectCells] CellSampler selecting cells\n");
    // each agent should only belong to one region
    loc_data::const_iterator it;
    loc_cells *amvCandidatesPar = new loc_cells[omp_get_max_threads()];

    for (it = m_mLocData.begin(); (iResult == 0)  && (it != m_mLocData.end()); ++it) {
        double dLon0 = it->second.dLon;
        double dLat0 = it->second.dLat;

#pragma omp parallel for
        for (uint iC = 0; iC < m_mCoords.size(); iC++) {
            double dLon1 = m_mCoords[iC].first;
            double dLat1 = m_mCoords[iC].second;
        
            //            double d = spherdist(dLon0*Q_PI/180, dLat0*Q_PI/180, dLon1*Q_PI/180, dLat1*Q_PI/180, RADIUS);
            double d = spherdistDeg(dLon0, dLat0, dLon1, dLat1, RADIUS_EARTH_KM);
            

            if (d < it->second.dDist) {
                amvCandidatesPar[omp_get_thread_num()][it->first].push_back(iC);
            }
        }
    }
    
    for (int i = 0; i < omp_get_max_threads(); i++) {
        loc_cells::const_iterator it2;
        for (it2 = amvCandidatesPar[i].begin(); it2 != amvCandidatesPar[i].end(); ++it2) {
            mvCandidates[it2->first].insert(mvCandidates[it2->first].end(), it2->second.begin(), it2->second.end());
        }
    }
    delete[] amvCandidatesPar;
    return iResult;
}

//----------------------------------------------------------------------------
// showSelected
//
void CellSampler::showSelected(FILE *fOut, uint iLim) {
    loc_cells::const_iterator it2;
    for (it2 = m_mvSelected.begin(); it2 != m_mvSelected.end(); ++it2) {
        stdfprintf(fOut, "%s:\n  ", it2->first.c_str());
        const std::vector<int> &v = it2->second;
        uint iHi = v.size();
        if ((iLim <  iHi)) {
            iHi = iLim;
        }
        for (uint j = 0; j < iHi; ++j) {
            stdfprintf(fOut, "%d  ", v[j]);
        }
        if (iLim <  v.size()) { 
           stdfprintf(fOut, "...");
        }
        stdfprintf(fOut, "\n");
    }
}

