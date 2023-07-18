#include <cstdio>
#include <hdf5.h>

#include "LBBase.h"
#include "LayerArrBuf.h"
#include "LBController.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "SequenceIOUtils.h"

//----------------------------------------------------------------------------
// createInstance
//
template<typename T>
SequenceIOUtils<T> *SequenceIOUtils<T>::createInstance(const char *pDataSetName, 
                                                       hid_t hdf_type, 
                                                       LayerArrBuf<T> *paSequence,
                                                       LBController *pSequenceController,
                                                       std::vector<int> *pvDeadList,
                                                       uint iBlockSize) {

    SequenceIOUtils *pSIO = new SequenceIOUtils(pDataSetName, 
                                                hdf_type, 
                                                paSequence, 
                                                pSequenceController,
                                                pvDeadList, 
                                                iBlockSize);
    int iResult = pSIO->init();
    if (iResult != 0) {
        delete pSIO;
        pSIO = NULL;
    }
    return pSIO;
}

//----------------------------------------------------------------------------
// constructor
//
template<typename T>
SequenceIOUtils<T>::SequenceIOUtils(const char *pDataSetName, 
                                    hid_t hdf_type, 
                                    LayerArrBuf<T> *paSequence,
                                    LBController *pSequenceController,
                                    std::vector<int> *pvDeadList,
                                    uint iBlockSize)
    : m_pDataSetName(pDataSetName),
      m_hdf_type(hdf_type),
      m_paSequence(paSequence),
      m_pSequenceController(pSequenceController),
      m_pWriteCopyController(NULL),
      m_iLayerSize(pSequenceController->getLayerSize()),
      m_iBlockSize(iBlockSize),
      m_pvDeadList(pvDeadList) {
}


//----------------------------------------------------------------------------
// destructor
//
template<typename T>
SequenceIOUtils<T>::~SequenceIOUtils() {
    if (m_pWriteCopyController != NULL) {
        delete m_pWriteCopyController;
    }
}


//----------------------------------------------------------------------------
// init
//
template<typename T>
int SequenceIOUtils<T>::init() {
    int iResult = 0;
    
    printf("[SequenceIOUtils<T>::init()] writecopycontroller with real layer size of %d; array size %d\n", m_iLayerSize, m_iBlockSize);

    m_pWriteCopyController = new LBController;

    m_aWriteCopy.init(m_iLayerSize, m_iBlockSize);
    m_pWriteCopyController->init(m_iLayerSize);
    m_pWriteCopyController->addBuffer(static_cast<LBBase *>(&m_aWriteCopy));
    m_pWriteCopyController->addLayer();

    return iResult;
}


//----------------------------------------------------------------------------
// writeSequenceDataQDF
//
template<typename T>
int SequenceIOUtils<T>::writeSequenceDataQDF(hid_t hSpeciesGroup, uint iNumArrays) {

    //    int iResult = 0;
    fflush(stdout); fflush(stderr);
    printf("[SequenceIOUtils<T>::writeSequenceDataQDF]\n");
    herr_t status=-1;
    int iTotalKill = 0;
    uint iNumWritten = 0;
    uint iWrittenLongs = 0;

    // amount of elements we have to save 
    hsize_t dims = iNumArrays * m_iBlockSize;
    printf("  num agents total: %lu = %d * %d\n", dims, iNumArrays, m_iBlockSize);
    fflush(stdout); fflush(stderr);

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        
        // Create the dataset

        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, m_pDataSetName, m_hdf_type, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        
        if (hDataSet > 0) {
            hsize_t dimsm = m_iLayerSize*(ulong)m_iBlockSize;
            printf("  memspace size: %lu=%d*%d\n", dimsm, m_iLayerSize, m_iBlockSize);
            fflush(stdout); fflush(stderr);
            hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

            hsize_t iOffset = 0;
            hsize_t iCount  = 0;  
            status = 0;

            uint iD = 0;

            // does the array contain sequences?
            if (m_pSequenceController->getNumUsed() > 0) {
                // loop over BufLayers
                for (uint j = 0; j < m_paSequence->getNumUsedLayers(); j++) {
                    // avoid empty layers
                    if (m_pSequenceController->getNumUsed(j) > 0) {
                    
                        // write agents of layer j as hyperslab
                        const T* pSlab0 = m_paSequence->getLayer(j);
                        m_aWriteCopy.copyLayer(0, pSlab0);
                        m_pWriteCopyController->setL2List(m_pSequenceController->getL2List(j), 0);

                        int iNumKilled = 0;
                        // remove dead in slab (the dead list has been filled in writeAgentQDFSafe())
                        while ((iD < m_pvDeadList->size()) && (m_pvDeadList->at(iD) < (int)((j+1)*m_iLayerSize))) {

                            m_pWriteCopyController->deleteElement(m_pvDeadList->at(iD) - j*m_iLayerSize);

                            iD++;
                            iNumKilled++;
                        }
                        iTotalKill += iNumKilled;

                        // here it is important not to have an empty slab
                        m_pWriteCopyController->compactData();
                    
                        const T* pSlab = m_aWriteCopy.getLayer(0);
                        // printf("layer %u: %d genomes\n", j, m_pWriteCopyController->getNumUsed(0));
                        iNumWritten +=  m_pWriteCopyController->getNumUsed(0);
                        // adapt memspace if size of slab must change
                        iCount =  m_pWriteCopyController->getNumUsed(0)*(ulong)m_iBlockSize;
                        if (iCount != dimsm) {
                            qdf_closeDataSpace(hMemSpace); 
                            dimsm = iCount;
                            hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
                            printf("  memspace resize: %lu=%d*%d\n", dimsm, m_pWriteCopyController->getNumUsed(0), m_iBlockSize);
                        }
			printf("Now we slab offs %lu, count %lu\n", iOffset, iCount);
                        fflush(stdout); fflush(stderr);
                        
                        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                                     &iOffset, NULL, &iCount, NULL);
			printf("res %d; Now we write from %p\n", status, pSlab);
                        fflush(stdout); fflush(stderr);
                        
                        status = H5Dwrite(hDataSet, m_hdf_type, hMemSpace,
                                          hDataSpace, H5P_DEFAULT, pSlab);

			printf("res %d; Now we have written\n", status);
                        fflush(stdout); fflush(stderr);
                        iOffset += iCount;
                        iWrittenLongs += iCount;
                    } else {
                        printf("[SequenceIOUtils<T>::writeSequenceDataQDF] ignored layer %d because it's empty\n", j);
                    }
                }
                
            }
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        } else {
            printf("[SequenceIOUtils<T>::writeSequenceDataQDF] couldn't create dataset\n");
	}
        qdf_closeDataSpace(hDataSpace);
    } else {
        printf("[SequenceIOUtils<T>::writeSequenceDataQDF] couldn't create dataspace\n");

    }

    printf("[SequenceIOUtils<T>::writeSequenceDataQDF] written %u items (%u longs), killed %d\n", iNumWritten, iWrittenLongs, iTotalKill); fflush(stdout);
    printf("[SequenceIOUtils<T>::writeSequenceDataQDF] end with status %d\n", status); fflush(stdout);
    fprintf(stderr, "[SequenceIOUtils<T>::writeSequenceDataQDF] written %u items (%u longs), killed %d\n", iNumWritten, iWrittenLongs, iTotalKill); fflush(stderr);
    fprintf(stderr, "[SequenceIOUtils<T>::writeSequenceDataQDF] end with status %d\n", status); fflush(stderr);

    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// readSequenceDataQDF
//
template<typename T>
int SequenceIOUtils<T>::readSequenceDataQDF(hid_t hSpeciesGroup, uint iNumReadItems) {
    int iResult = -1;
    printf("[SequenceIOUtils<T>::readSequenceDataQDF]\n");
    
    if (qdf_link_exists(hSpeciesGroup, m_pDataSetName)) {
        iResult = 0;
        
        // the buffer must hold a mulPhenetics<T>::init()tiple of the array length
        uint iReadBufSize = iNumReadItems*m_iBlockSize;
        T *aBuf = new T[iReadBufSize];

        // open the data set
        hid_t hDataSet = H5Dopen2(hSpeciesGroup, m_pDataSetName, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        // get toal number of elements in dataset
        hsize_t dims;;
        herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        printf("Dataspace extent: %lu\n", dims);

        // initialize some counters and indexes
        int iFirstIndex = 0;
        int iTotalToDo = dims;
        hsize_t iCount;
        hsize_t iOffset = 0;
    
        // loop until all elements have been read
        while ((iResult == 0) && (dims > 0)) {
            // can we get a full load of the buffer?
            if (dims > iReadBufSize) {
                iCount = iReadBufSize;
            } else {
                iCount = dims;
            }
            //@@            printf("Slabbing offset %lld, count %lld\n", iOffset,iCount);
        
            // read a slab
            hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
            status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                         &iOffset, NULL, &iCount, NULL);
            status = H5Dread(hDataSet, m_hdf_type, hMemSpace,
                             hDataSpace, H5P_DEFAULT, aBuf);
        
        
            if (status >= 0) {
                // distribute the data 
                int iArrOffset = 0;
                int iNumThisPass = iCount/m_iBlockSize;
                int iNumToCopy = iNumThisPass;
                //@@                printf("starting inner loop: iTotalToDo %d, iNumThisPass %d\n", iTotalToDo, iNumThisPass);
                // while we still have genomes to fill and data from the slab not used up
                while ((iTotalToDo > 0) && (iNumThisPass > 0)) {
                    
                    // creation of agents has already created space in the genome buffer
                    // copy from the current point in the buffer to the current index
                    T *pCur = aBuf+iArrOffset;
                    // the positions in the LayerBuf are already have the correct L2List  links
                    // because reserveSpace2() has been called by readAgentDataQDF()
                    m_paSequence->copyBlock(iFirstIndex, (T *) pCur, iNumToCopy);

                    // update counters and indexes
                    iFirstIndex += iNumToCopy;
                    iTotalToDo  -= iNumToCopy*m_iBlockSize;
                    iArrOffset  += iNumToCopy*m_iBlockSize;
                    //@@                    printf("after copyblock: firstindex %d, numforcell: %d, totaltodo %d\n", iFirstIndex, iNumForCell,iTotalToDo);
                    iNumThisPass -= iNumToCopy;
                    iNumToCopy    = iNumThisPass;
                }

            } else {
                printf("Error during slab reading\n");
                iResult = -1;
            }
        
            dims    -= iCount;
            iOffset += iCount;
            
        }
        

        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSet(hDataSet);
        
        delete[] aBuf;
    } else {
        printf("WARNING: no dataset [%s] found\n", m_pDataSetName);
    }
    return iResult;

}

//-----------------------------------------------------------------------------
// dumpAdditionalDataQDF
//
template<typename T>
int SequenceIOUtils<T>::dumpSequenceDataQDF(hid_t hSpeciesGroup) {
    int iResult = -1;
    
    printf("[SequenceIOUtils<T>::dumpSequenceDataQDF]\n");
    fflush(stdout);

    hsize_t dims = m_paSequence->getNumLayers()*m_iLayerSize*m_iBlockSize;
    printf("[SequenceIOUtils<T>::dumpSequenceDataQDF] dim = %lu = %d * %d * %d\n", dims, m_paSequence->getNumLayers(), m_iLayerSize, m_iBlockSize );
    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);


    if (hDataSpace > 0) {
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, m_pDataSetName, m_hdf_type, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
       
        if (hDataSet > 0) {
 
            iResult = 0;
            hsize_t dimsm = m_iLayerSize*m_iBlockSize;
            hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
            
            hsize_t offset = 0;
            hsize_t count  = dimsm;  
            
            // step size when going through data (e.g. stride = 2: use every second element)
            //hsize_t stride = 1;
            //hsize_t block  = 1;
            
            // we don't remove dead spaces
            
            herr_t status = H5P_DEFAULT;
            
            for (uint j = 0; (iResult == 0) && (j < m_paSequence->getNumLayers()); j++) {
                // write agents of layer j as hyperslab
                const T *pSlab = m_paSequence->getLayer(j);

                status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                             &offset, NULL, &count, NULL);
                status = H5Dwrite(hDataSet, m_hdf_type, hMemSpace,
                                  hDataSpace, H5P_DEFAULT, pSlab);
                offset += count;
            }
            qdf_closeDataSpace(hMemSpace); 
            qdf_closeDataSet(hDataSet);

            printf("[SequenceIOUtils<T>::dumpSequenceDataQDF] status after loop): %d\n", status);
            //iResult =  (status >= 0)?iResult:-1;
        } else {
            printf("[SequenceIOUtils<T>::dumpSequenceDataQDF] couldn't create data set\n");
        }
        qdf_closeDataSpace(hDataSpace);
    } else {
        printf("[SequenceIOUtils<T>::dumpSequenceDataQDF] couldn't create data space\n");
    }
    
    printf("[SequenceIOUtils<T>::dumpSequenceDataQDF] written %d layers of size %d\n",  m_paSequence->getNumLayers(), m_pWriteCopyController->getLayerSize());

    return iResult;
}


//-----------------------------------------------------------------------------
// restoreAdditionalDataQDF
//
template<typename T>
int SequenceIOUtils<T>::restoreSequenceDataQDF(hid_t hSpeciesGroup) {
    int iResult = -1;
    
    printf("[SequenceIOUtils<T>::restoreSequenceDataQDF]\n");
    printf("[SequenceIOUtils<T>::restoreSequenceDataQDF] species group %ld\n", hSpeciesGroup);
    fflush(stdout);

    hsize_t dims;

    hid_t hDataSet = H5Dopen2(hSpeciesGroup, m_pDataSetName, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    if (hDataSpace > 0) {
        iResult = 0;

        herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        printf("[SequenceIOUtils<T>::restoreSequenceDataQDF] dims:%lu\n", dims); fflush(stdout);
        hsize_t dimsm = ((long)m_paSequence->getLayerSize())*m_iBlockSize;
        printf("[SequenceIOUtils<T>::restoreSequenceDataQDF] slabsize:%lu\n", dimsm); fflush(stdout);
        printf("[SequenceIOUtils<T>::restoreSequenceDataQDF] total size:%lu\n", dimsm*m_paSequence->getNumLayers()); fflush(stdout);
        T *pSlab  = new T[dimsm];
        hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

        hsize_t offset = 0;
        hsize_t count  = dimsm;  
    
        // step size when going through data (e.g. stride = 2: use every second element)
        hsize_t stride = 1;
        hsize_t block  = 1;
            
        // we don't remove dead spaces
            
        status = H5P_DEFAULT;
            
        for (uint j = 0; (iResult == 0) && (j < m_paSequence->getNumLayers()); j++) {
            // write agents of layer j as hyperslab
            printf("[Genetics::restoreAdditionalDataQDF] loop %d: sofar %lu, getting %lun", j, offset, count); fflush(stdout);
            status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                         &offset, &stride, &count, &block);
            
            status = H5Dread(hDataSet, m_hdf_type, hMemSpace,
                                 hDataSpace, H5P_DEFAULT, pSlab);
        
            m_paSequence->copyLayer(j, pSlab);
            offset += count;
            iResult =  (status >= 0)?iResult:-1;
        }
        qdf_closeDataSpace(hMemSpace); 
        delete[] pSlab;
        
        
    } else {
        printf("[SequenceIOUtils<T>::restoreSequenceDataQDF] couldn't create data space\n");
    }
 
    return iResult;
 }

