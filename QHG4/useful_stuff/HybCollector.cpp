
#include <cstring>
#include <string>
#include <hdf5.h>

#include "types.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "hdfutils.h"
#include "HybCollector.h"

const std::string ATTR_NUM_REGIONS = "NumRegions";

//-----------------------------------------------------------------------------
//  
//


//----------------------------------------------------------------------------
// read_agent_data
//    
//




//-----------------------------------------------------------------------------
//  constructor
//
HybCollector::HybCollector(uint iNumBins) 
    : m_iNumBins(iNumBins),
      m_iNumSteps(0),
      m_iNumRegions(0),
      m_aiHisto(NULL),
      m_hFile(H5P_DEFAULT),
      m_hSim(H5P_DEFAULT),
      m_hAgentType(H5P_DEFAULT),
      m_fMin(0),
      m_fMax(1) {


    std::vector<field_data> vFHyb = {
        { "AgentID",       HOFFSET(aginfo, m_ulID),      H5T_NATIVE_LONG},
        { "CellID",        HOFFSET(aginfo, m_ulCellID),  H5T_NATIVE_INT}, 
        { "Hybridization", HOFFSET(aginfo, m_fHybridization),     H5T_NATIVE_FLOAT}
    };
    std::vector<field_data> vFYMT = {

        { "CellID",   HOFFSET(aginfo_ymt, m_ulCellID),  H5T_NATIVE_INT},
        { "AgentID",  HOFFSET(aginfo_ymt, m_ulID),      H5T_NATIVE_LONG},
        { "Gender",    HOFFSET(aginfo_ymt, m_iGender),   H5T_NATIVE_UCHAR},
        { "PheneticHyb",             HOFFSET(aginfo_ymt, m_fHybridization),     H5T_NATIVE_FLOAT},
        { "Ychr",                    HOFFSET(aginfo_ymt, m_iYchr),     H5T_NATIVE_UCHAR},
        { "mtDNA",                   HOFFSET(aginfo_ymt, m_imtDNA),    H5T_NATIVE_UCHAR},
    };

    m_hAgentType = createCompoundDataType(sizeof(aginfo), vFHyb);
}
    

//-----------------------------------------------------------------------------
//  destructor
//
HybCollector::~HybCollector() {
    if (m_aiHisto != NULL) {
        delete[] m_aiHisto;
    }

    if (m_hFile != H5P_DEFAULT) {
        H5Fclose(m_hFile);
    }
    if (m_hSim != H5P_DEFAULT) {
        H5Gclose(m_hSim);
    }
}


//-----------------------------------------------------------------------------
//  create_instance
//
HybCollector  *HybCollector::create_instance(const std::string sFile, uint iNumBins) {
    HybCollector *pHC = new HybCollector(iNumBins);
    int iResult = pHC->init(sFile);
    if (iResult != 0) {
        delete pHC;
        pHC = NULL;
    }
    return pHC;
}


//-----------------------------------------------------------------------------
//  init
//
int HybCollector::init(const std::string sFile) { 
    int iResult = -1;
    //    xha_fprintf(stderr, "[init] file [%s]\n", sFile);
    m_hFile = H5Fopen(sFile.c_str(), H5F_ACC_RDONLY,  H5P_DEFAULT);
    if (m_hFile != H5P_DEFAULT) {
        int iNumRegs = 0;
        iResult = extract_numeric_attribute(m_hFile, ATTR_NUM_REGIONS, 1, &iNumRegs, H5T_NATIVE_INT32);
        if (iResult == 0) {

            // first hierarchy level: simulation groups (usually just one)
            iResult = collect_sim_names();
            if (iResult == 0) {
                
                if (m_vSimNames.size() > 0) {
                    m_hSim = H5Gopen(m_hFile, m_vSimNames[0].c_str(), H5P_DEFAULT);
                    // second hierarchy level: time step groups
                    iResult = collect_step_names();
                    if (iResult == 0) {

                        hid_t hCurStep  = H5Gopen(m_hSim, m_vStepNames[0].c_str(), H5P_DEFAULT);
                        // third hierarchy level: region data sets
                        iResult = collect_region_names(hCurStep);

                        if (iResult == 0) {
                            // fourth hierarchy level: hybridization values
                            m_aiHisto = new int[m_vStepNames.size()*m_vRegionNames.size()*m_iNumBins];
                            memset(m_aiHisto, 0, m_vStepNames.size()*m_vRegionNames.size()*m_iNumBins*sizeof(int));
                        

 

                        } else {
                            xha_fprintf(stderr, "Couldn't collect region names\n");
                        }
                        H5Gclose(hCurStep);
                    
                    } else {
                        xha_fprintf(stderr, "Couldn't collect step names\n");
                    }
                } else {
                    xha_fprintf(stderr, "found no sim names\n");
		    iResult = -1;
                }
                
            } else {
                xha_fprintf(stderr, "Couldn't collect sim names\n");
            }
        } else {
            xha_fprintf(stderr, "Couldn't find attribute [%s]\n", ATTR_NUM_REGIONS);
        }
        
        
    } else {
        xha_fprintf(stderr, "Couldn't open file [%s]\n", sFile);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  name_grabber
//    callback for H5Litertate
//    op_data must be a pointer to a string vector
//
herr_t  name_grabber(hid_t g_id, const char *name, const H5L_info_t *info, void *op_data) {
    stringvec *pvNames = (stringvec*)op_data;
    pvNames->push_back(name);
    return 0;   
}


//-----------------------------------------------------------------------------
//  collect_sim_names
//
int HybCollector::collect_sim_names() {
    int iResult = 0;
    
    herr_t status = H5Literate(m_hFile, H5_INDEX_NAME, H5_ITER_INC, 0, name_grabber, &m_vSimNames);
    if (status >= 0) {
        iResult = 0;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  collect_step_names
//
int HybCollector::collect_step_names() {
    int iResult = 0;
    
    herr_t status = H5Literate(m_hSim, H5_INDEX_NAME, H5_ITER_INC, 0, name_grabber, &m_vStepNames);
    if (status >= 0) {
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  collect_region_names
//
int HybCollector::collect_region_names(hid_t hStep) {
    int iResult = 0;

    herr_t status = H5Literate(hStep, H5_INDEX_NAME, H5_ITER_INC, 0, name_grabber, &m_vRegionNames);
    if (status >= 0) {
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  show_names
//
void HybCollector::show_names() {
    xha_printf("SimNames (%zd)\n", m_vSimNames.size());
    for (uint i = 0; i < m_vSimNames.size(); i++) {
        xha_printf("  %s\n", m_vSimNames[i]);
    }

    xha_printf("StepNames (%zd)\n", m_vStepNames.size());
    for (uint i = 0; i < m_vStepNames.size(); i++) {
        xha_printf("  %s\n", m_vStepNames[i]);
    }

    xha_printf("RegionNames (%zd)\n", m_vRegionNames.size());
    for (uint i = 0; i < m_vRegionNames.size(); i++) {
        xha_printf("  %s\n", m_vRegionNames[i]);
    }
}


//-----------------------------------------------------------------------------
//  show_data
//
void HybCollector::show_data() {
    for (uint i = 0; i < m_vStepNames.size(); i++) {
        xha_printf("[%s]\n", m_vStepNames[i]);
        for (uint j = 0; j < m_vRegionNames.size(); j++) {
            xha_printf("  [%s](%d)\n    ", m_vRegionNames[j], i*m_vRegionNames.size()*m_iNumBins+j*m_iNumBins);
            for (uint k = 0; k < m_iNumBins; k++) {
                printf(" %d", m_aiHisto[i*m_vRegionNames.size()*m_iNumBins+j*m_iNumBins + k]);
            }
            xha_printf("\n");
        }
    }
}


//-----------------------------------------------------------------------------
//  collect_hybridisations
//
int HybCollector::collect_hybridisations() {
    int iResult = 0;

    for (uint i = 0; i < m_vStepNames.size(); i++) {
        
        hid_t hStep = H5Gopen(m_hSim, m_vStepNames[i].c_str(), H5P_DEFAULT);

        for (uint j = 0; j < m_vRegionNames.size(); j++) {
            hid_t hRegion = H5Dopen(hStep, m_vRegionNames[j].c_str(), H5P_DEFAULT);

            int iSize = 10000;
            aginfo pBuffer[iSize];
            
            //            xha_fprintf(stderr, "collecting for %s/%s\n", m_vStepNames[i],  m_vRegionNames[j]);
            
            iResult = readAgentDataHDF(i, j, hRegion,  pBuffer, iSize);

            H5Dclose(hRegion); 
        }


        H5Gclose(hStep);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readAgentDataQDF
//  read attributes to species data
//
int  HybCollector::readAgentDataHDF(uint iStep, uint iRegion, hid_t hDataSet, void *pBuffer, uint iBufSize) {
    //    xha_fprintf(stderr, "[HybCollector::readAgentDataHDF]\n");
    int iResult = 0;

    hid_t hDataSpace = H5Dget_space(hDataSet);

    hsize_t dims = 0;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    hsize_t iOffset = 0;
    hsize_t iCount  = 0;
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;

    memset(pBuffer, 0, iBufSize*sizeof(aginfo));
    // compacting seems ok, since we're changing the occupancy anyway
    int iRound = 0;
    while ((iResult == 0) && (dims > 0)) {
        if (dims > iBufSize) {
            iCount = iBufSize;
        } else {
            iCount = dims;
        }

        // read a buffer full
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, &iStride, &iCount, &iBlock);
        status = H5Dread(hDataSet, m_hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, pBuffer);
        if (status >= 0) {

            //process buffer
           
            uint iPosBase = iStep*m_vRegionNames.size()*m_iNumBins + iRegion*m_iNumBins;
            for (uint k = 0; k < iCount; k++) {
                float h = ((aginfo*)pBuffer)[k].m_fHybridization;
                // get bin for h
                uint iBin = m_iNumBins*(h - m_fMin)/(m_fMax - m_fMin);
                if (iBin <= 0) {
                    iBin = 0;
                } else if (iBin >= m_iNumBins) {
                    iBin = m_iNumBins - 1;
                }

                // find pos of bin
                uint iPos = iPosBase + iBin;
                // increase 
                m_aiHisto[iPos]++;
            }
            
                       
            // update counts
            dims -= iCount;
            iOffset += iCount;
            
        } else {
            iResult = -1;
        }
        iRound++;
    }
    H5Sclose(hDataSpace);
 
    return iResult;
}


//-----------------------------------------------------------------------------
//  
//

//-----------------------------------------------------------------------------
//  
//
