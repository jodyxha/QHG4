#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <omp.h>

#include "stdstrutilsT.h"
#include "QDFUtils.h"
#include "AgentCounter.h"

#define SPOP_DT_CELL_ID    "CellID"
#define SPOP_DT_AGENT_ID   "AgentID"

bool s_bVerbose = false;

//----------------------------------------------------------------------------
// constructor
//
AgentCounter::AgentCounter()
    : m_iNumCells(-1),
      m_iNumAgents(-1),
      m_pLongitude(NULL),
      m_pLatitude(NULL),
      m_pAltitude(NULL),
      m_pIceCover(NULL) {

}


//----------------------------------------------------------------------------
// destructor
//
AgentCounter::~AgentCounter() {

    if (m_pAltitude != NULL) {
        delete[] m_pAltitude;
    }

    if (m_pIceCover != NULL) {
        delete[] m_pIceCover;
    }

    if (m_pLongitude != NULL) {
        delete[] m_pLongitude;
    }
    if (m_pLatitude != NULL) {
        delete[] m_pLatitude;
    }

    popcc::iterator itCC;
    for (itCC = m_mpCC.begin(); itCC != m_mpCC.end(); ++itCC) {
        if (itCC->second != NULL) {
            delete[] itCC->second;
        }
    }

    popaginfos::iterator it1;
    for (it1 = m_mpInfos.begin(); it1 != m_mpInfos.end(); ++it1) {
        if (it1->second.second != NULL) {
            delete[] it1->second.second;
        }
    }

    popcounts::iterator it2;
    for (it2 = m_mpCounts.begin(); it2 != m_mpCounts.end(); ++it2) {
        if (it2->second != NULL) {
            delete[] it2->second;
        }
    }
}


//----------------------------------------------------------------------------
// createInstance
//
AgentCounter *AgentCounter::createInstance(const std::string sPop, const std::string sGeo, const std::string sCC) {
    AgentCounter *pAC = new AgentCounter();
    int iResult = pAC->init(sPop, sGeo, sCC);
    if (iResult != 0) {
        delete pAC;
        pAC = NULL;
    }
    return pAC;
}


//----------------------------------------------------------------------------
// init
//
int AgentCounter::init(const std::string sPop, const std::string sGeo, const std::string sCC) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = loadAltIce(sGeo);
    }

    if (iResult == 0) {
        std::string sPopName = qdf_getFirstPopulation(sPop);
        if (!sPopName.empty()) {
            iResult = loadAgentsCell(sPop, sPopName);
        }
    }

    if ((iResult == 0) && (!sCC.empty())) {
        iResult = loadCC(sCC);
    }

    if (iResult == 0) {
        iResult = loadLonLat(sGeo);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// loadEnvArray
//
int loadEnvArray(hid_t hEnvGroup, const std::string sDataSetName, double **ppArray) {
    hid_t hDataSet     = qdf_openDataSet(hEnvGroup, sDataSetName, H5P_DEFAULT);
    hid_t hDataSpace   = H5Dget_space(hDataSet);
    hsize_t dims;
    H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    *ppArray = new double[dims];
    int iResult = 0;
    iResult = qdf_readArray(hEnvGroup, sDataSetName, dims, *ppArray);
    if (iResult == 0) {
        iResult = dims;
    } else {
        delete[] *ppArray;
        *ppArray = NULL;
        iResult = -1;
    }      

    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);

    return iResult;
}


//----------------------------------------------------------------------------
// loadAltIce
//
int AgentCounter::loadAltIce(const std::string sGeo) {
    hid_t hFileGeo     = qdf_openFile(sGeo);
    hid_t hGeoGroup    = qdf_openGroup(hFileGeo, GEOGROUP_NAME);
    int iResult = -1;

    if (s_bVerbose) printf("Loading altitudes\n");
    int iSizeAlt  = loadEnvArray(hGeoGroup, GEO_DS_ALTITUDE, &m_pAltitude);
    if (iSizeAlt > 0) {
        if (s_bVerbose) printf("Successfully read %d altitude items\n", iSizeAlt);

        if (s_bVerbose) printf("Loading icecover\n");
        int iSizeIce  = loadEnvArray(hGeoGroup, GEO_DS_ICE_COVER, &m_pIceCover);
        if (iSizeIce > 0) {
            if (s_bVerbose) printf("Successfully read %d icecover items\n", iSizeIce);

            if (iSizeIce == iSizeAlt) {
                iResult = 0;
                if (m_iNumCells < 0) {
                    m_iNumCells = iSizeAlt;
                } else if (m_iNumCells != iSizeAlt) {
                    printf("established number of cells (%d) does not match number of altitude and ice times (%d)\n", m_iNumCells, iSizeAlt);
                    iResult = -1;
                } else {
                    // ok
                }
            } else {
                printf("altitude size (%d) does not match icecover size (%d)\n", iSizeAlt, iSizeIce);
            }
        } else {
            printf("failed to load icecover\n");
        }
    } else {
        printf("failed to load altitude\n");
    }

    qdf_closeGroup(hGeoGroup);
    qdf_closeFile(hFileGeo);

    return iResult;    

}

//----------------------------------------------------------------------------
// loadLonLat
//
int AgentCounter::loadLonLat(const std::string sGeo) {
    hid_t hFileGeo     = qdf_openFile(sGeo);
    hid_t hGeoGroup    = qdf_openGroup(hFileGeo, GEOGROUP_NAME);
    int iResult = -1;

    if (s_bVerbose) printf("Loading altitudes\n");
    int iSizeLon  = loadEnvArray(hGeoGroup, GEO_DS_LONGITUDE, &m_pLongitude);
    if (iSizeLon > 0) {
        if (s_bVerbose) printf("Successfully read %d longitude items\n", iSizeLon);

        if (s_bVerbose) printf("Loading icecover\n");
        int iSizeLat  = loadEnvArray(hGeoGroup, GEO_DS_LATITUDE, &m_pLatitude);
        if (iSizeLat > 0) {
            if (s_bVerbose) printf("Successfully read %d latitude items\n", iSizeLat);

            if (iSizeLon == iSizeLat) {
                iResult = 0;
                if (m_iNumCells < 0) {
                    m_iNumCells = iSizeLon;
                } else if (m_iNumCells != iSizeLon) {
                    printf("established number of cells (%d) does not match number of longitude and longitude items (%d)\n", m_iNumCells, iSizeLon);
                    iResult = -1;
                } else {
                    // ok
                }
            } else {
                printf("longitude size (%d) does not match latitude size (%d)\n", iSizeLon, iSizeLat);
            }
        } else {
            printf("failed to load icecover\n");
        }
    } else {
        printf("failed to load altitude\n");
    }

    qdf_closeGroup(hGeoGroup);
    qdf_closeFile(hFileGeo);

    return iResult;    

}

//----------------------------------------------------------------------------
// loadCC
//
int AgentCounter::loadCC(const std::string sCC) {
    int iResult = 0;
    hid_t hFileCC     = qdf_openFile(sCC);
    hid_t hCCGroup     = H5P_DEFAULT;
    hid_t hPopGroup    = H5P_DEFAULT;
   
    if (s_bVerbose) stdprintf("loading CCs for %zd name\n", m_vNames.size());
    if (qdf_link_exists(hFileCC, POPGROUP_NAME)) {
        hPopGroup = qdf_openGroup(hFileCC, POPGROUP_NAME);
        if (s_bVerbose) stdprintf("opened group [%s]\n", POPGROUP_NAME);
        
        for (uint i = 0; (iResult == 0) && (i < m_vNames.size()); i++) {
            const std::string &sName = m_vNames[i];
            if (qdf_link_exists(hPopGroup, sName)) {
                hCCGroup = qdf_openGroup(hPopGroup, sName);
                
                if (s_bVerbose) stdprintf("opened subgroup [%s]\n", sName);
                double *pCC;
                int iSizeCC  = loadEnvArray(hCCGroup, SPOP_DS_CAP, &pCC);
                if (iSizeCC > 0) {
                    if (s_bVerbose) stdprintf("Successfully read %d cc items\n", iSizeCC);
                    iResult = 0;
                    if (m_iNumCells < 0) {
                        m_iNumCells = iSizeCC;
                    } else if (m_iNumCells != iSizeCC) {
                        stdprintf("established number of cells (%d) does not match number of npp times (%d)\n", m_iNumCells, iSizeCC);
                        iResult = -1;
                    } else {
                        // ok
                        stdprintf("Adding CCs to collection\n");
                        m_mpCC[sName] = pCC;
                    }
                } else {
                    printf("failed to load npp\n");
                    iResult = -1;
                }
            } else {
                stdprintf("Cuodn0t find species group [%s] in group [%s] exist in [%s]\n", m_vNames[i], POPGROUP_NAME, sCC);
                iResult = -1;
            }
            qdf_closeGroup(hCCGroup);
        }
    } else {
        stdprintf("group [%s] exist in [%s]\n", POPGROUP_NAME, sCC);
        iResult = -1;
    }

    qdf_closeGroup(hPopGroup);
    qdf_closeFile(hFileCC);

    return iResult;

}


//----------------------------------------------------------------------------
// loadAgentsCell
//
int AgentCounter::loadAgentsCell(const std::string sPop, const std::string sPopName) {
    hid_t hFilePop     = qdf_openFile(sPop);
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    if (s_bVerbose) stdprintf("[loadAgentsCell] pop %s,popname %s\n", sPop, sPopName);
    // at this point m_iNumCells should be known (loadNPP() loadAltIce() already called)
    getPopulationNames(hPopulation);
    for (uint i = 0; i < m_vNames.size(); i++) {
        m_mpCounts[m_vNames[i]] = new int[m_iNumCells];
    }

    int iResult = 0;


    std::vector<std::string>::iterator it;
    for (it = m_vNames.begin(); (iResult == 0) && (it != m_vNames.end()); ++it) {

        hid_t hSpecies     = qdf_openGroup(hPopulation, *it);
        hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
        hid_t hDataSpace   = H5Dget_space(hDataSet);



    
        hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof (aginfo));


        H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     HOFFSET(aginfo, m_ulCellID),   H5T_NATIVE_INT);
        H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    HOFFSET(aginfo, m_ulID),       H5T_NATIVE_LONG);
        
        hsize_t dims;
        H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        m_iNumAgents = dims;
        m_pInfos = new aginfo[dims];

        hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
        herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pInfos);
        if (status >= 0) {
            stdprintf("pop %s: %llu\n", *it, dims);
            m_mpInfos[*it] = std::pair<size_t, aginfo*>(dims, m_pInfos);
        } else {
            stdprintf("bad status for pop %s\n", *it);

            delete[] m_pInfos;
	    m_pInfos = NULL;
            iResult = -1;
        }
        
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSet(hDataSet);
        qdf_closeGroup(hSpecies);
    }

    qdf_closeGroup(hPopulation);
    qdf_closeFile(hFilePop);

    return iResult;
}


//----------------------------------------------------------------------------
// countAgentsInCells
//
int AgentCounter::countAgentsInCells() {
    int iNumThreads = omp_get_max_threads();
    if (s_bVerbose) printf("[countAgentsInCells] starting\n");
    // create arrays for threads
    int **pCountsT = new int*[iNumThreads];
    for (int i = 0; i < iNumThreads; i++) {
        pCountsT[i] = new int[m_iNumCells];
        memset(pCountsT[i], 0, m_iNumCells*sizeof(int));
    }

    m_iTotal = 0;
    int iTempTotal = 0;
    
    popcounts::iterator it;
    for (it = m_mpCounts.begin(); it != m_mpCounts.end(); ++it) {

        
        // create arrays for threads
        for (int i = 0; i < iNumThreads; i++) {
            memset(pCountsT[i], 0, m_iNumCells*sizeof(int));
        }


        size_t iNumAgents = m_mpInfos[it->first].first;
        aginfo * &pInfos  = m_mpInfos[it->first].second;

#pragma omp parallel for
        for (uint i = 0; i < iNumAgents; i++) {
            int iCell   = pInfos[i].m_ulCellID;
            pCountsT[omp_get_thread_num()][iCell]++;
        }
        
        int *pCounts = m_mpCounts[it->first];
        memset(pCounts, 0, m_iNumCells*sizeof(int));
        if (s_bVerbose) printf("collating now\n");
        // collate counts
#pragma omp parallel for
        for (int j = 0; j < m_iNumCells; j++) {
            for (int i = 0; i < iNumThreads; i++) {
                pCounts[j] += pCountsT[i][j];
            }
        }
#pragma omp parallel for reduction(+:iTempTotal)
        for (int j = 0; j < m_iNumCells; j++) {
            iTempTotal += pCounts[j];
        }

    }

    m_iTotal = iTempTotal;
    // delete thread arrays
    for (int i = 0; i < iNumThreads; i++) {
        delete[] pCountsT[i];
    }
    delete[] pCountsT;

     return m_iTotal;
}


//----------------------------------------------------------------------------
// popInfo
//  callback function for iteration in getFirstPopulation()
//
herr_t popAllInfoUtil(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    herr_t status = 0;
    H5G_stat_t statbuf;
    H5Gget_objinfo(loc_id, name, false, &statbuf);
    //pInfo->type???
    if (statbuf.type == H5G_GROUP) {
        ((std::vector<std::string> *)(opdata))->push_back(name);
    }
    return status;
}


//----------------------------------------------------------------------------
// getPopulationNames
//
int AgentCounter::getPopulationNames(hid_t hPopGroup) {
    uint iResult = -1;
    m_vNames.clear();
    if (hPopGroup > 0) {
        herr_t status = H5Literate(hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popAllInfoUtil, &m_vNames);
        if (status >= 0) {
            iResult = m_vNames.size();
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getCC
//
double *AgentCounter::getCC(const std::string sSpecies) {
    double *pCC = NULL;
    popcc::const_iterator it;
    if (sSpecies == "") {
        it =  m_mpCC.begin();
    } else {
         it = m_mpCC.find(sSpecies);
    }
    if (it != m_mpCC.end()) { 
        pCC = it->second;
    }
    return pCC;
} 


//----------------------------------------------------------------------------
// getPopCounts
//
int *AgentCounter::getPopCounts(const std::string sSpecies) {
    int *pCount = NULL;
    popcounts::const_iterator it;
    if (sSpecies == "") {
        it =  m_mpCounts.begin();
    } else {
         it = m_mpCounts.find(sSpecies);
    }
    if (it != m_mpCounts.end()) { 
        pCount = it->second;
    }
    return pCount;
} 
