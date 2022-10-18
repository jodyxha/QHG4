#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <omp.h>
#include <hdf5.h>

#include "types.h"
#include "stdstrutilsT.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "SequenceProvider.h"
#include "QDFSequenceExtractor.h"
#include "IDSampler2.h"

#include "LineReader.h"

#define BUFSIZE_SEQUENCES 1000000


//----------------------------------------------------------------------------
// constructor
//
template<typename T>
QDFSequenceExtractor<T>::QDFSequenceExtractor(WELL512 *pWELL, bool bCartesian)
    : m_sPopName(""),
      m_sQDFGeoFile(""),
      m_sSequenceDataSetName(""),
      m_iNumSelected(0),
      m_iNumRefSelected(0),
      m_pCurSample(NULL),
      m_pRefSample(NULL),
      m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_iSequenceSize(0),
      m_iNumBlocks(0),
      m_iPloidy(0),
      m_bCartesian(bCartesian),
      m_pWELL(pWELL),
      m_bVerbose(false) {

    m_vQDFPopFiles.clear();
    m_sSelected.clear();
    m_mSelected.clear(); 
    m_sRefSelected.clear();
    m_mRefSelected.clear();
    m_mLocData.clear();
    m_mSequences.clear();
}


//----------------------------------------------------------------------------
// destructor
//
template<typename T>
QDFSequenceExtractor<T>::~QDFSequenceExtractor() {
    qdf_closeFile(m_hFile);
    qdf_closeGroup(m_hPopulation);
    qdf_closeGroup(m_hSpecies);

    // ... probaly more
    
    if (m_pCurSample != NULL) {
        delete m_pCurSample;
    }
    
    if (m_pRefSample != NULL) {
        delete m_pRefSample;
    }
    typename  
    QDFSequenceExtractor<T>::sequencemap::const_iterator it;
    for (it =  m_mSequences.begin(); it != m_mSequences.end(); ++it) {
        delete[] it->second;
    }
}


//----------------------------------------------------------------------------
// getSequenceSize
//   (from SequenceProvider)
//
template<typename T>
int QDFSequenceExtractor<T>::getSequenceSize() { 
    return m_iSequenceSize;
}

//----------------------------------------------------------------------------
// getSequence
//   (from SequenceProvider)
//
template<typename T>
const T *QDFSequenceExtractor<T>::getSequence(idtype iID) {
    const T *pSequence = NULL;
    typename QDFSequenceExtractor<T>::sequencemap::const_iterator itm = m_mSequences.find(iID);
    if (itm != m_mSequences.end()) {
        pSequence = itm->second;
    }
    return pSequence;
}

//----------------------------------------------------------------------------
// setRange
//   (from SequenceProvider)
//
template<typename T>
int QDFSequenceExtractor<T>::setRange(int iFirstBlock, int iShiftBlocks) {
    int iResult = -1;
    if ((iFirstBlock >= 0) && ((uint)(iFirstBlock + iShiftBlocks) < m_iNumBlocks)) {
        
        // loop through sequences and memmove blocks to create shorter x-ploid sequences
        typename QDFSequenceExtractor<T>::sequencemap::iterator itm;
        for (itm =  m_mSequences.begin(); itm != m_mSequences.end(); ++itm) {
            T *pSequence = itm->second;
            
            memmove(pSequence, pSequence+iFirstBlock, iShiftBlocks*sizeof(T));
            if (m_iPloidy > 1) {
                T *p = pSequence + iShiftBlocks;
                memmove(p, pSequence+(m_iNumBlocks+iFirstBlock), iShiftBlocks*sizeof(T));
            }
            
        }
        int iLast = iFirstBlock + iShiftBlocks;
        if (iLast == iShiftBlocks-1) {
            int iLeftOver = m_iSequenceSize % calcNumItemsInBlock();
            m_iSequenceSize = (iShiftBlocks-1)*calcNumItemsInBlock() + iLeftOver;
        } else {
            m_iSequenceSize = iShiftBlocks*calcNumItemsInBlock();
        }
        m_iNumBlocks = iShiftBlocks;
        printf("successfully set range to %d, %d\n", iFirstBlock, iShiftBlocks);
        printf("new numblocks: %d\n", m_iNumBlocks);
        printf("new seqlen:    %d\n", m_iSequenceSize);
        iResult = 0;
    } else {
        printf("Bad range: %d + %d >= %d\n", iFirstBlock, iShiftBlocks, m_iNumBlocks);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// init
//
template<typename T>
int QDFSequenceExtractor<T>::init(const std::string sQDFGeoFile, 
                                  const std::string sQDFPopFile, 
                                  const std::string sSpeciesName, 
                                  const std::string sAttrSequenceSize,
                                  const std::string sSequenceDataSetName) {


    int iResult = -1;

    if (!sAttrSequenceSize.empty()) {

        m_sPopName = "";
        if (sSpeciesName.empty()) {
            m_sPopName = qdf_getFirstPopulation(sQDFPopFile);
        } else {
            m_sPopName = qdf_checkForPop(sQDFPopFile, sSpeciesName);
        }
        if (!m_sPopName.empty()) {
            if (qdf_hasGeo(sQDFGeoFile)) {
                m_sSequenceDataSetName = sSequenceDataSetName;
                
                if (m_bVerbose) stdprintf("using population [%s]\n", m_sPopName);
                
                m_sQDFGeoFile = sQDFGeoFile;
                
                m_vQDFPopFiles.push_back(sQDFPopFile);
                // open qdf and get species
                // we know file, pop group and species group exist
                m_hFile        = qdf_openFile(sQDFPopFile);
                m_hPopulation  = qdf_openGroup(m_hFile, POPGROUP_NAME);
                m_hSpecies     = qdf_openGroup(m_hPopulation, m_sPopName);
                
                   
                iResult = qdf_extractAttribute(m_hSpecies, sAttrSequenceSize, 1, &m_iSequenceSize);
                if (iResult == 0) {
                    stdprintf("Sequence size %d\n", m_iSequenceSize);
                    iResult = extractAdditionalAttributes();

                    if (iResult == 0) {
                        m_iNumBlocks = calcNumBlocks();
                     }

                    if (m_iPloidy == 0) {
                        iResult = -1; 
                        stdfprintf(stderr, "The Ploidy of the sequence should be set in extractAdditionalAttributes()\n");
                    }
                } else {
		    stdfprintf(stderr, "Couldn't extract Attribute [%s] for sequence size\n", sAttrSequenceSize);
                }
            } else {
 	        stdfprintf(stderr, "No Grid & Geo in [%s]\n", sQDFGeoFile);
            }
        }
    } else {
        fprintf(stderr, "The name for the sequence size attribute must not be NULL\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getSelectedGenesDense
//  this algorithm is better if the samples are dense
//
template<typename T>
int QDFSequenceExtractor<T>::getSelectedSequencesDense(const std::string sSequenceDataSetName, int iNumPerBuf) {
    int iResult = 0;

    printf("getSelectedSequences (dense)\n");

    // read buffer
    ulong iReadBufSize = ((iNumPerBuf<=0)?BUFSIZE_SEQUENCES:iNumPerBuf)*m_iPloidy*m_iNumBlocks;
    T *aBuf = new T[iReadBufSize];
   
    // open the DataSet and data space
    if (m_bVerbose) { stdprintf("Opening data set [%s] in species\n", sSequenceDataSetName); fflush(stdout); }
    hid_t hDataSet = qdf_openDataSet(m_hSpecies, sSequenceDataSetName);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    if (m_bVerbose) stdprintf("Dataspace extent: %lld\n", dims);

    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    int iGlobalOffset = 0;

    arrpos_ids mAllSelected(m_mSelected);
    mAllSelected.insert(m_mRefSelected.begin(), m_mRefSelected.end());
    //    indexids::const_iterator itIdxId = m_mSelected.begin();
    arrpos_ids::const_iterator itIdxId = mAllSelected.begin();
    
    if (m_bVerbose) stdprintf("Trying to extract %zd genomes\n", mAllSelected.size());
    // loop until all elements have been read
    while ((iResult == 0) && (dims > 0) && (itIdxId != mAllSelected.end())) {
        // can we get a full load of the buffer?
        if (dims > iReadBufSize) {
            iCount = iReadBufSize;
        } else {
            iCount = dims;
        }
        
        // read a slab
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, NULL, &iCount, NULL);
        status = readQDFSequenceSlab(hDataSet, hMemSpace, hDataSpace, aBuf);
        
        if (status >= 0) {
            // distribute the data 
            int iNumThisPass = iCount/(m_iPloidy*m_iNumBlocks);
            for (int i = 0; (iResult == 0) && (i < iNumThisPass) && (itIdxId != mAllSelected.end()); i++) {
                int j = i+iGlobalOffset;
                
                if (j == itIdxId->first) {
                    T *pBlock = new T[m_iPloidy*m_iNumBlocks];
                    memcpy(pBlock, aBuf+i*m_iPloidy*m_iNumBlocks, m_iPloidy*m_iNumBlocks*sizeof(T));
                    m_mSequences[itIdxId->second] = pBlock;

                    ++itIdxId;
                }
            }
            
            iGlobalOffset += iNumThisPass;
            dims          -= iCount;
            iOffset       += iCount;
            
        } else {
	    stdfprintf(stderr, "Error during slab reading\n");
	    iResult = -1;
        }
    }
    

    if (m_mSequences.size() == mAllSelected.size()) {
        stdprintf("Successfully read all required %zd sequences\n", m_mSequences.size());
    } else {
        stdfprintf(stderr, "Error: read only %zd of %zd required sequences\n", m_mSequences.size(), m_mSelected.size());
        iResult = -1;
    }

    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
        
    delete[] aBuf;
    
    return iResult;

}


//----------------------------------------------------------------------------
// getSelectedGenesSparse
//  this algorithm is better if the samples are sparse
//
template<typename T>
int QDFSequenceExtractor<T>::getSelectedSequencesSparse(const std::string sSequenceDataSetName, int iNumPerBuf) {
    int iResult = 0;
    m_bVerbose = true;

    // read buffer
    ulong iReadBufSize = m_iPloidy*m_iNumBlocks;
    T *aBuf = new T[iReadBufSize];
    
    printf("getSelectedGenes (sparse)\n");
    printf("m_iNumBlocks: %d\n", m_iNumBlocks);
    // open the DataSet and data space
    if (m_bVerbose) {stdprintf("Opening data set [%s] in species\n", sSequenceDataSetName); fflush(stdout);}
    hid_t hDataSet = qdf_openDataSet(m_hSpecies, sSequenceDataSetName);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    if (m_bVerbose) printf("Dataspace extent: %lld\n", dims);

    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    arrpos_ids mAllSelected(m_mSelected);
    mAllSelected.insert(m_mRefSelected.begin(), m_mRefSelected.end());

    iCount  = m_iPloidy*m_iNumBlocks;
    arrpos_ids::const_iterator itIdxId;
    for (itIdxId= mAllSelected.begin(); itIdxId != mAllSelected.end(); ++itIdxId) {
        iOffset = m_iPloidy*m_iNumBlocks*itIdxId->first;

        if (iOffset < dims-iCount+1) {
            // read a slab
            hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
            status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                         &iOffset, NULL, &iCount, NULL);
            status = readQDFSequenceSlab(hDataSet, hMemSpace, hDataSpace, aBuf);
            
            if (status >= 0) {
                T *pBlock = new T[m_iPloidy*m_iNumBlocks];
                memcpy(pBlock, aBuf, m_iPloidy*m_iNumBlocks*sizeof(T));
               m_mSequences[itIdxId->second] = pBlock;
                
            } else {
                fprintf(stderr, "Error during slab reading\n");
                iResult = -1;
            }
        } else {
            printf("Index too big: (%lld+1)*%d*numblocks > %lld\n", iOffset, m_iPloidy, dims);
            printf("iOffset:    %lld\n", iOffset);
            printf("iCount:     %lld\n", iCount);
            printf("numblocks:  %d\n", m_iNumBlocks);
            printf("IdxID:      %d\n", itIdxId->first);
            iResult = -1;
        }
    }

    if (m_mSequences.size() == mAllSelected.size()) {
        stdprintf("Successfully read all required %zd sequences\n", m_mSequences.size());
    } else {
        stdfprintf(stderr, "Error: read only %zd of %zd required sequences\n", m_mSequences.size(), m_mSelected.size());
        iResult = -1;
    }

    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
        
    delete[] aBuf;
    
    return iResult;

}


//----------------------------------------------------------------------------
// createSelection
//   if all reads have been successful, the number of agents is returned.
//   uses number and sampling distance from file
//   otherwise -1 on error
//
template<typename T>
int QDFSequenceExtractor<T>::createSelection(const std::string sLocFile, const std::string sRefFile, bool bDense, int iNumPerBuf, const std::string sSampIn, const std::string sSampOut) {
    // "0, 0" use num samp and dist from file
    int iResult = createSelection(sLocFile, 0, 0, sRefFile, bDense, iNumPerBuf, sSampIn, sSampOut);
    return iResult;
}


//----------------------------------------------------------------------------
// createSelection
//   if all reads have been successful, the number of agents is returned.
//   otherwise -1 on error
//
template<typename T>
int QDFSequenceExtractor<T>::createSelection(const std::string sLocFile, int iNumSamp, double dSampDist, const std::string sRefFile, bool bDense, int iNumPerBuf, const std::string sSampIn, const std::string sSampOut) {
    int iResult = -1;

    if (m_pCurSample != NULL) {
        delete m_pCurSample;
        m_pCurSample = NULL;
    }
    m_sSelected.clear();
    m_mSelected.clear();

    locspec ls(sLocFile, dSampDist, iNumSamp);
    locspec rs(sRefFile, dSampDist, iNumSamp);

    if (m_bVerbose) stdprintf("--- creating IDSampler from [%s] for locations [%s] (pSampIn:[%s])\n", m_sQDFGeoFile, sLocFile, sSampIn);
    if (sSampIn.empty()) {
        IDSampler2 *pIS = IDSampler2::createInstance(m_sQDFGeoFile, m_pWELL, m_bCartesian);
        if (pIS != NULL) {
            iResult = 0;
            if (m_bVerbose && false) {
                printf("--- getting samples from [");
                for (uint i = 0; i < m_vQDFPopFiles.size(); ++i) {
                    if (i > 0) {
                        printf(", ");
                    }
                    printf("%s", m_vQDFPopFiles[i].c_str());
                }
                printf("]\n");
            }

            m_pCurSample = pIS->getSamples(m_vQDFPopFiles, m_sPopName, &ls, m_mLocData, &rs);
            m_pRefSample = pIS->getRefSample();

            if ((iResult == 0) && (!sSampOut.empty())) {
                stdprintf("writing samples to [%s]\n", sSampOut);
                m_pCurSample->write(sSampOut);
            }
            

            delete pIS;

        } else {
            iResult = -1;
            stdfprintf(stderr, "Couldn't create IDSampler for Grid [%s]\n", m_sQDFGeoFile);
        }
    } else {
        stdprintf("Getting samples from [%s]\n", sSampIn);
        m_pCurSample = new IDSample();
        m_pRefSample = new IDSample();
        iResult = m_pCurSample->read(sSampIn);
        fillLocData(&ls, m_mLocData);
        if (iResult == 0)  {
            if (!sSampOut.empty()) {
                stdprintf("writing samples to [%s]\n", sSampOut);
                m_pCurSample->write(sSampOut);
            }
        } else {
            stdfprintf(stderr, "Couldn't read IDSample from [%s]\n", sSampIn);

            delete m_pCurSample;
            m_pCurSample = NULL;
            delete m_pRefSample;
            m_pRefSample = NULL;
        }
    }

    if (m_pCurSample != NULL) {
        //        m_pCurSample->display();
        iResult = 0;
        if (m_pCurSample != NULL) {
            m_sSelected.clear();
            m_pCurSample->getFullIDSet(m_sSelected);
            m_pCurSample->getFullIndexIDMap(m_mSelected);
            m_pRefSample->getFullIDSet(m_sRefSelected);
             m_pRefSample->getFullIndexIDMap(m_mRefSelected);
            //           pIS->getLocationIDSet();

            m_iNumSelected = m_sSelected.size();
            if (m_bVerbose) { stdprintf("Sampled: Total %d id%s\n", m_iNumSelected, (m_iNumSelected!=1)?"s":"");fflush(stdout);}
            
            double dT0 = omp_get_wtime();
            if (bDense) {
                iResult = getSelectedSequencesDense(m_sSequenceDataSetName, iNumPerBuf);
            } else {
                iResult = getSelectedSequencesSparse(m_sSequenceDataSetName, iNumPerBuf);
            }
            double dT1 = omp_get_wtime();
            if (m_bVerbose) { stdprintf("Got sequences: [%p] R[%d]\n", m_sSequenceDataSetName, iResult);fflush(stdout);}
            if (m_bVerbose) { stdprintf("QDFSequenceExtractor: time to get genomes: %f\n", dT1- dT0);}
        } else {
	    stdfprintf(stderr, "Couldn't get samples\n");
        }
        
    }
    return iResult;
}

