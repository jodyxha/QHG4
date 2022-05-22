

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include <hdf5.h>

#include "types.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"

#include "QDFUtils.h"
#include "MedianHybMaker.h"



const std::string DATASET_MEDIANS = "Medians";

//----------------------------------------------------------------------------
// 
//

const std::string SPOP_DT_CELL_ID    = "CellID";

//----------------------------------------------------------------------------
//  create_instance
//
template<typename T>
MedianHybMaker<T> *MedianHybMaker<T>::create_instance(const std::string sQDFFile, const std::string sSpecies) {

    MedianHybMaker<T> *pMM = new MedianHybMaker();
    int iResult = pMM->init(sQDFFile, sSpecies);
    if (iResult != 0) {
        delete pMM;
        pMM = NULL;
    }
    return pMM;
}

//----------------------------------------------------------------------------
//  constructor
//
template<typename T>
MedianHybMaker<T>::MedianHybMaker()
    : m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_iNumThreads(0),
      m_avHybValues(NULL),
      m_pAgents(NULL) {
    
}


//----------------------------------------------------------------------------
//  destructor
//
template<typename T>
MedianHybMaker<T>::~MedianHybMaker() {

    if (m_avHybValues != NULL) {
        delete[] m_avHybValues;
    }

    if (m_pAgents != NULL) {
        delete[] m_pAgents;
    }


    if (m_hSpecies != H5P_DEFAULT) {
        qdf_closeGroup(m_hSpecies);
    }

    if (m_hPopulation != H5P_DEFAULT) {
        qdf_closeGroup(m_hPopulation);
    }

    if (m_hFile != H5P_DEFAULT) {
        qdf_closeFile(m_hFile);
    }
}
    
    
//----------------------------------------------------------------------------
//  init
//  - open file
//  - open pop-group
//  - open spc group
//  - get num cells
//  - get num threads
//  - allocate m_avHybValues
//
template<typename T>
int MedianHybMaker<T>::init(const std::string sQDFFile, const std::string sSpecies) {
    int iResult = 0;

    m_hFile     = qdf_openFile(sQDFFile, true);

    if (m_hFile != H5P_DEFAULT) {
        stdprintf("opened file [%s]\n", sQDFFile);
        m_hPopulation  = qdf_openGroup(m_hFile, POPGROUP_NAME);
        if (m_hPopulation != H5P_DEFAULT) {
            stdprintf("opened population group [%s]\n", POPGROUP_NAME);
            m_hSpecies  = qdf_openGroup(m_hPopulation, sSpecies);
            if (m_hSpecies != H5P_DEFAULT) {
                stdprintf("opened species group [%s]\n", sSpecies);
                iResult = qdf_extractAttribute(m_hSpecies, SPOP_ATTR_NUM_CELL, 1, &m_iNumCells);
                if (iResult == 0) {
                    stdprintf("[MedianHybMaker::init] found attribute [%s]: %d\n", SPOP_ATTR_NUM_CELL, m_iNumCells);
                } else {   
                    stdprintf("[MedianHybMaker::init] couldn't read attribute [%s] in group [%s]\n", SPOP_ATTR_NUM_CELL, sSpecies);
                    iResult = -1;
                }
            } else {
                stdprintf("[MedianHybMaker::init] couldn't open group [%s] in [%s/%s]\n", sSpecies, sQDFFile, POPGROUP_NAME);
                iResult = -1;
            }
            
        } else {
            stdprintf("[MedianHybMaker::init] couldn't open group [%s] in [%s]\n", POPGROUP_NAME, sQDFFile);
            iResult = -1;
        }
    } else {
        stdprintf("[MedianHybMaker::init] couldn't open [%s] as qdf file\n", sQDFFile);
        iResult = -1;
    }


    if (iResult == 0) {
        m_iNumThreads = 1;//omp_get_max_threads();

        m_avHybValues = new  std::vector<float>[m_iNumCells];
        for (int j = 0; j < m_iNumCells;  ++j) {
            m_avHybValues[j].clear();
        }
        
    }

    return iResult;
}



//----------------------------------------------------------------------------
// createCompoundDataType
//   create a compound datattype containing
//      cell  id
//      item 
//
template<typename T>
hid_t MedianHybMaker<T>::create_compound_data_type() {
    hid_t hAgentDataType = H5P_DEFAULT;
           

    // reading compound data: the second argument to H5Tinsert must be the name of an element in the structure 
    // used in the HDF file
    // if the name dopes not exist, noerror is produced and the  results are undefined
    // Here we must use the names of the elements of OoANavSHybYchMTDPop written to QDF 

    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(T));
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID.c_str(),   HOFFSET(T, m_ulCellID),  H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "PheneticHyb",             HOFFSET(T, m_fHybridization),     H5T_NATIVE_FLOAT);

    return hAgentDataType;
}



//----------------------------------------------------------------------------
// collect_agent_hybs
//
template<typename T>
int  MedianHybMaker<T>::collect_agent_hybs() {
    stdfprintf(stderr, "[MedianHybMaker::collect_agent_hybs]\n");
    int iResult = 0;

    hid_t hDataSet   = qdf_openDataSet(m_hSpecies, AGENT_DATASET_NAME);
    hid_t hDataSpace = H5Dget_space(hDataSet);


    hid_t hAgentDataType = create_compound_data_type();
    hsize_t dims = 0;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    hsize_t iOffset = 0;
    hsize_t iCount  = 0;
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;
    int iNumAgents = dims;
    uint iBufSize = iNumAgents*sizeof(T);
    
    m_pAgents = new uchar[iBufSize];
    memset(m_pAgents, 0, iBufSize);

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
        status = H5Dread(hDataSet, hAgentDataType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, m_pAgents);
        if (status >= 0) {

            //process buffer
            for (uint k = 0; k < iCount; k++)  {
                
                int   iC = ((T*)m_pAgents)[k].m_ulCellID;
                float fH = ((T*)m_pAgents)[k].m_fHybridization;

                m_avHybValues[iC].push_back(fH);
            }
            
                      
            // update counts
            dims -= iCount;
            iOffset += iCount;
            
        } else {
            iResult = -1;
        }
        iRound++;
    }
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);

    stdfprintf(stderr, "[MedianHybMaker::collect_agent_hybs] done\n");
 
    return iResult;
}


//----------------------------------------------------------------------------
// merge_vectors
//
template<typename T>
int MedianHybMaker<T>::merge_vectors() {
    stdfprintf(stderr, "[MedianHybMaker::merge_vectors]\n");

    int iResult = 0;

    for (uint iC = 0; iC < m_iNumCells; ++iC) {
        /*
        for (int iT = 1; (iResult == 0) && (iT <m_iNumThreads); ++iT) {
            m_avHybValues[0][iC].insert(m_avHybValues[0][iC].end(),  m_avHybValues[iT][iC].begin(),  m_avHybValues[iT][iC].end());
        }
        */
        std::sort(m_avHybValues[iC].begin(),m_avHybValues[iC].end());
    }

    stdfprintf(stderr, "[MedianHybMaker::merge_vectors done]\n");
    return iResult;
}


//----------------------------------------------------------------------------
// calc_medians
//
template<typename T>
int MedianHybMaker<T>::calc_medians() {
    int iResult = 0;
    iResult = collect_agent_hybs();
    if (iResult == 0) {

        m_pMedians = new float[m_iNumCells];
        memset(m_pMedians, 0, m_iNumCells*sizeof(float));
        
        for (int iC = 0; iC < m_iNumCells; ++iC) {
            std::sort(m_avHybValues[iC].begin(),m_avHybValues[iC].end());
            
            float fVal = 0;
            uint iSize = m_avHybValues[iC].size(); 
            
            if (iSize > 0) {
                if ((iSize%2) == 0) {
                    fVal = (m_avHybValues[iC][iSize/2 - 1] + m_avHybValues[iC][iSize/2])/2.0;
                } else  {
                    fVal = m_avHybValues[iC][iSize/2];
                }
                m_pMedians[iC] = fVal;
            } else {
                m_pMedians[iC] = -1;
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// write_medians
//
template<typename T>
int MedianHybMaker<T>::write_medians() {
    int iResult = -1;

    if (m_pMedians != NULL) {
        if (qdf_link_exists(m_hSpecies, DATASET_MEDIANS)) {
            herr_t status = H5Ldelete(m_hSpecies, DATASET_MEDIANS.c_str(), H5P_DEFAULT);
            iResult = (status >= 0)?0:-1;
        }

        
        hsize_t dim = m_iNumCells;
        hid_t hDataSpace = H5Screate_simple(1, &dim, NULL);
        hid_t hDataSet   = H5Dcreate2(m_hSpecies, DATASET_MEDIANS.c_str(), H5T_NATIVE_FLOAT, hDataSpace, 
                                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, m_pMedians);

        qdf_closeDataSet(hDataSet);
        qdf_closeDataSpace(hDataSpace);


    //        hid_t hDataSet = qdf_writeArray(m_hSpecies, DATASET_MEDIANS, m_iNumCells, m_pMedians);
        if (hDataSet != H5P_DEFAULT) {
            stdprintf("[MedianHybMaker<T>::write_medians] successfully written medians\n");
            
        } else { 
            stdprintf("[MedianHybMaker<T>::write_medians] couldn't create dataset [%s]\n", DATASET_MEDIANS);
        }
        
    } else {
        stdprintf("[MedianHybMaker<T>::write_medians] can't write NULL medians\n");
    }

    return iResult;
}
