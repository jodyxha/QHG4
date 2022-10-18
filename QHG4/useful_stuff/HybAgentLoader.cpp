#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <omp.h>


#include "stdstrutilsT.h"
#include "QDFUtils.h"
#include "HybAgentLoader.h"


#define SPOP_DT_CELL_ID     "CellID"
#define SPOP_DT_AGENT_ID    "AgentID"
#define SPOP_DT_HYBR_ID     "Hybridization"
#define SPOP_DT_PHEN_HYB_ID "PheneticHyb"

//----------------------------------------------------------------------------
// constructor
//
HybAgentLoader::HybAgentLoader()
    : m_iNumAgents(-1),
      m_pInfos(NULL) {

}


//----------------------------------------------------------------------------
// destructor
//
HybAgentLoader::~HybAgentLoader() {


    if (m_pInfos != NULL) {
        delete[] m_pInfos;
    }
 
}


//----------------------------------------------------------------------------
// createInstance
//
HybAgentLoader *HybAgentLoader::createInstance(const std::string sPop) {
    HybAgentLoader *pHAL = new HybAgentLoader();
    int iResult = pHAL->init(sPop);
    if (iResult != 0) {
        delete pHAL;
        pHAL = NULL;
    }
    return pHAL;
}


//----------------------------------------------------------------------------
// init
//
int HybAgentLoader::init(const std::string sPop) {
    int iResult = 0;


    if (iResult == 0) {
        std::string sPopName = qdf_getFirstPopulation(sPop);
        if (sPopName.empty()) {
            iResult = loadAgentsCell(sPop, sPopName);
        } else {
            stdfprintf(stderr, "Couldn't open [%s] as QDF file\n", sPop);
            iResult = -1;
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// loadAgentsCell
//
int HybAgentLoader::loadAgentsCell(const std::string sPop, const std::string sPopName) {
    hid_t hFilePop     = qdf_openFile(sPop);
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    fprintf(stderr, "[loadAgentsCell] pop %s,popname %s\n", sPop, sPopName);
    // findat this point m_iNumCells should be known (loadNPP() loadAltIce() already called)
    if (sPopName.empty()) {
        getPopulationNames(hPopulation);
    } else {
        m_vNames.push_back(sPopName);
    }

    int iResult = 0;

    std::vector<std::string>::iterator it;
    for (it = m_vNames.begin(); (iResult == 0) && (it != m_vNames.end()); ++it) {

        hid_t hSpecies     = qdf_openGroup(hPopulation, *it);
        hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
        hid_t hDataSpace   = H5Dget_space(hDataSet);
    
        hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof (aginfo));

        H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     HOFFSET(aginfo, m_ulCellID), H5T_NATIVE_INT);
        H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    HOFFSET(aginfo, m_ulID),     H5T_NATIVE_LONG);
        H5Tinsert(hAgentDataType, SPOP_DT_PHEN_HYB_ID, HOFFSET(aginfo, m_fPhenHyb), H5T_NATIVE_FLOAT);
        //        H5Tinsert(hAgentDataType, SPOP_DT_HYBR_ID,    HOFFSET(aginfo, m_fHybridization), H5T_NATIVE_FLOAT);
        
        hsize_t dims;
        H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        m_iNumAgents = dims;
        m_pInfos = new aginfo[dims];

        hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
        herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pInfos);
        if (status >= 0) {
            stdfprintf(stderr, "pop %s: %llu\n", *it, dims);
        } else {
            stdfprintf(stderr, "bad status for pop %s\n", *it);

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
// popAllInfoUtil
//  callback function for iteration in getPopulationNames()
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
int HybAgentLoader::getPopulationNames(hid_t hPopGroup) {
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


