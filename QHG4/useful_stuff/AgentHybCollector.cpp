#include <cstdio>
#include <cstring>
#include <omp.h>

#include "hdf5.h"

#include "geomutils.h"
#include "SPopulation.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"
#include "AgentHybCollector.h"


//----------------------------------------------------------------------------
// createInstance
//
AgentHybCollector *AgentHybCollector::createInstance(const char *pGeoQDF, const char *pPopQDF, const char *pSpecies, const char *pItemName) {
    AgentHybCollector *pAIC = new AgentHybCollector();
    int iResult = pAIC->init(pGeoQDF, pPopQDF, pSpecies, pItemName);
    if (iResult != 0) {
        delete pAIC;
        pAIC = NULL;
    }
    return pAIC;
}


//----------------------------------------------------------------------------
// constructor
//
AgentHybCollector::AgentHybCollector() 
    : m_pInfos(NULL),
      m_ppBinContexts(NULL),
      m_iNumContexts(0) {
}
  

//----------------------------------------------------------------------------
// destructor
//
AgentHybCollector::~AgentHybCollector() {
    if (m_pInfos != NULL) {
        delete[] m_pInfos;
    }

    if (m_ppBinContexts != NULL) {
        for (uint i = 0; i < m_iNumContexts; i++) {
            delete[] m_ppBinContexts[i]->m_piBins;
            delete m_ppBinContexts[i];
        }
        delete[] m_ppBinContexts;
    }
}


//----------------------------------------------------------------------------
// init
//
int AgentHybCollector::init(const char *pQDFGeo, const char *pPopQDF, const char *pSpecies, const char *pItemName) {
    int iResult = 0;
    float f0 = omp_get_wtime();
    iResult = fillCoordMap(pQDFGeo);
    float f1 = omp_get_wtime();

    if (iResult == 0) {
        fprintf(stderr, "Successfully read coords (%fs)\n", f1 - f0);
        iResult = loadAgentsCell(pPopQDF, pSpecies, pItemName);
        float f2 = omp_get_wtime();
        if (iResult == 0) {
            fprintf(stderr, "Successfully read agent items (%fs)\n", f2 - f1);
        } else {
            fprintf(stderr, "Couldn't get agent items\n");
        }
    } else {
        fprintf(stderr, "Couldn't load coords\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//
int AgentHybCollector::fillCoordMap(const char *pQDFGeoGrid) {
    int iResult = -1;
    uint iNumCells = 0;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
    char sPath[512];
    
    QDFArray *pQA = QDFArray::create(pQDFGeoGrid);
    if (pQA != NULL) {

        // get the cell IDs
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            iNumCells = pQA->getSize();
            pCellIDs = new int[iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, iNumCells, GRID_DS_CELL_ID);
            if (iCount == iNumCells) {
                //                fprintf(stderr, "Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                fprintf(stderr, "Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            fprintf(stderr, "Couldn't open QDF array for [%s:%s/%s]\n", pQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME);
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
                        fprintf(stderr, "Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        fprintf(stderr, "Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells);
                    }
                } else {
                    iResult = -1;
                    fprintf(stderr, "Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE);
                }
                pQA->closeArray();
            }
        }

        // get the cell Latitudes
        if (iResult == 0) {
            sprintf(sPath, "%s/%s", GEOGROUP_NAME, GEO_DS_LATITUDE);
            iResult = pQA->openArray(sPath);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLat = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, iNumCells);
                    if (iCount == iNumCells) {
                        fprintf(stderr, "Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        fprintf(stderr, "Couldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL,iNumCells);
                    }
                } else {
                    iResult = -1;
                    fprintf(stderr, "Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, pQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        fprintf(stderr, "Couldn't create QDFArray\n");
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
// loadAgentsCell
//
int AgentHybCollector::loadAgentsCell(const char *pPop, const char *pPopName, const char *pItemName) {
    hid_t hFilePop     = qdf_openFile(pPop);
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    fprintf(stderr, "[loadAgentsCell] pop %s,popname %s\n", pPop, pPopName);
    // at this point m_iNumCells should be known (loadNPP() loadAltIce() already called)

    int iResult = 0;

    hid_t hSpecies     = qdf_openGroup(hPopulation, pPopName);
    hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace   = H5Dget_space(hDataSet);

    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof (aginfo));


    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     HOFFSET(aginfo, m_ulCellID),       H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    HOFFSET(aginfo, m_ulID),           H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, pItemName,           HOFFSET(aginfo, m_fHybridization), H5T_NATIVE_FLOAT);
        
    hsize_t dims;
    H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    m_iNumAgents = dims;
    m_pInfos = new aginfo[dims];

    hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
    herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pInfos);
    if (status >= 0) {
        fprintf(stderr, "pop %s: %llu\n", pPopName, dims);
    } else {
        fprintf(stderr, "bad status for pop %s\n", pPopName);
        
        delete[] m_pInfos;
        m_pInfos = NULL;
        iResult = -1;
    }
    
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
    qdf_closeGroup(hSpecies);
   
    qdf_closeGroup(hPopulation);
    qdf_closeFile(hFilePop);
   
    return iResult;
}

#define EPS 0.000001

//----------------------------------------------------------------------------
// analyzeRanges
//
int AgentHybCollector::analyzeRanges(named_ranges &mNamedRanges, uint iNumBins) {
    int iResult = 0;

    std::vector<std::string> vNames;
    named_ranges::const_iterator it;                            
    for(it = mNamedRanges.begin(); it != mNamedRanges.end(); ++it) {
        vNames.push_back(it->first);
    }

    if (m_ppBinContexts != NULL) {
        for (uint i = 0; i < m_iNumContexts; i++) {
            delete[] m_ppBinContexts[i]->m_piBins;
            delete m_ppBinContexts[i];
        }
        delete[] m_ppBinContexts;
    }

    m_iNumContexts = vNames.size();
    m_ppBinContexts = new bin_context*[m_iNumContexts];

#pragma omp parallel for 
    for (uint k = 0; k < m_iNumContexts; k++) {
        bin_context  *pCurContext = new bin_context();

        pCurContext->m_sName = vNames[k];

        pCurContext->m_iNumBins = iNumBins;
        pCurContext->m_piBins = new uint[iNumBins];
        memset(pCurContext->m_piBins, 0, iNumBins*sizeof(uint));

        pCurContext->m_fMinNonZero = 1;
        pCurContext->m_fMaxNonOne  = 1;
        pCurContext->m_iNumZeros   = 0;
        pCurContext->m_iNumOnes    = 0;
        pCurContext->m_iNumAgents  = m_iNumAgents;
        pCurContext->m_iNumChecked = 0;

        range *pRange = mNamedRanges[vNames[k]];
        
        for (uint i = 0; i < m_iNumAgents; i++)  {
            bool bUse = true;
            if (pRange != NULL) {
                double dLon0 = m_mCoords[m_pInfos[i].m_ulCellID].first;
                double dLat0 = m_mCoords[m_pInfos[i].m_ulCellID].second;
                bUse = (spherdistDeg(dLon0, dLat0, pRange->m_dLon, pRange->m_dLat, RADIUS_EARTH_KM) < pRange->m_dRad);
            }

            if (bUse) {
                float v = m_pInfos[i].m_fHybridization;
                //printf("v %f\n", v);
            
                uint iBin =  v * iNumBins;
                if (iBin >= iNumBins) {
                    iBin = iNumBins - 1;
                }
                pCurContext->m_piBins[iBin]++;
                
                double d = 1 - v;
                if (d <  pCurContext->m_fMaxNonOne) {
                    if ((d > 0.0) && (d < 1.0)) {
                        pCurContext->m_fMaxNonOne = d;
                    }
                }
                if (v <  pCurContext->m_fMinNonZero) {
                    if ((v > 0.0) && (v < 1.0)) {
                         pCurContext->m_fMinNonZero = v;
                    }
                }
                if (v <= 0+EPS) {
                     pCurContext->m_iNumZeros++;
                }
                if (v >= 1-EPS) {
                     pCurContext->m_iNumOnes++;
                }
                pCurContext->m_iNumChecked++;
            }
        } 
        m_ppBinContexts[k] = pCurContext;

    }
 
    return iResult;
}
