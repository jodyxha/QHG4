#include <cstdio>

#include <vector>
#include <hdf5.h>

#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PopBase.h"
#include "PopReader.h"

//----------------------------------------------------------------------------
// constructor
//
PopReader::PopReader()
    : m_hFile(H5P_DEFAULT),
      m_hPopGroup(H5P_DEFAULT),
      m_hSpeciesGroup(H5P_DEFAULT),
      m_bOpenedFile(false) {
}

//----------------------------------------------------------------------------
// create
//
PopReader *PopReader::create(const std::string sFilename) {
    PopReader *pPR = new PopReader();
    int iResult = pPR->open(sFilename);
    if (iResult != 0) {
        delete pPR;
        pPR = NULL;
    }
    return pPR;
}

//----------------------------------------------------------------------------
// create
//
PopReader *PopReader::create(hid_t hFile) {
    PopReader *pPR = new PopReader();
    int iResult = pPR->open(hFile);
    if (iResult != 0) {
        delete pPR;
        pPR = NULL;
    }
    return pPR;
}

//----------------------------------------------------------------------------
// destructor
//
PopReader::~PopReader() {
    if (m_bOpenedFile) {
        if (m_hPopGroup != H5P_DEFAULT) {
            qdf_closeGroup(m_hPopGroup);
        }
        if (m_hFile != H5P_DEFAULT) {
            qdf_closeFile(m_hFile);
        }
    }
}

//----------------------------------------------------------------------------
// popInfo
//   call back for open()
//
herr_t popInfo(hid_t loc_id, const char *pName, const H5L_info_t*pInfo, void *opdata) {
    H5G_stat_t statbuf;
    int iResult = -1;
    H5Gget_objinfo(loc_id, pName, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        hid_t hSpecies = qdf_openGroup(loc_id, pName); 
        iResult = 0;
        popinfo spi;
        if (iResult == 0) {
            iResult = qdf_extractSAttribute(hSpecies, SPOP_ATTR_CLASS_NAME, spi.m_sClassName);
        }
        if (iResult == 0) {
            iResult = qdf_extractSAttribute(hSpecies, SPOP_ATTR_SPECIES_NAME, spi.m_sSpeciesName);
        }        
        if (iResult == 0) {
            iResult = qdf_extractAttribute(hSpecies, SPOP_ATTR_NUM_CELL, 1, &spi.m_iNumCells);
        }
        popinfolist *pPIList = (popinfolist *)opdata;
        if (pPIList != NULL) {
            pPIList->push_back(spi);
        }
        //  delete[] pType;
        qdf_closeGroup(hSpecies);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// open
//   open the file and go to the pop group
//   otherwise create it
//
int PopReader::open(const std::string sFileName) {
    int iResult = -1;
    m_bOpenedFile = true;
    hid_t hFile = qdf_openFile(sFileName);
    if (hFile != H5P_DEFAULT) {
        iResult = open(hFile);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// open
//   open the file and go to the pop group
//   otherwise create it
//
int PopReader::open(hid_t hFile) {
    int iResult = -1;
    m_hFile = hFile;
    if (m_hFile != H5P_DEFAULT) {
       
        m_hPopGroup = qdf_openGroup(m_hFile, POPGROUP_NAME);
        if (m_hPopGroup > 0) {
            // already exists
            H5Literate(m_hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popInfo, &m_vPopList);
            iResult = 0;
        } else {
            iResult = POP_READER_ERR_NO_POP_GROUP;
        }
    }
    return iResult;
}
    
//----------------------------------------------------------------------------
// read
//
int PopReader::read(PopBase *pPB, const std::string sSpeciesName, int iNumCells, bool bRestore) {
    int iResult = -1;    

    //    printf("reading data for [%s]\n", pSpeciesName);
    m_hSpeciesGroup = qdf_openGroup(m_hPopGroup, sSpeciesName);
    // read attributes
    if (m_hSpeciesGroup > 0) {
        //  printf("species group open: reading species data\n");
        if (bRestore) {
            iResult = pPB->restoreSpeciesDataQDF(m_hSpeciesGroup);
        } else {
            iResult = pPB->readSpeciesDataQDF(m_hSpeciesGroup);
        }
        if (iResult == 0) {
            if (pPB->getNumCells() == iNumCells) {
                // set the handles
                hid_t hAgentType = pPB->getAgentQDFDataType();
                hid_t hDataSet = H5Dopen2(m_hSpeciesGroup, AGENT_DATASET_NAME.c_str(), H5P_DEFAULT);
                hid_t hDataSpace = H5Dget_space(hDataSet);
                

                // now load the data
                if (bRestore) {
                    iResult = pPB->restoreAgentDataQDF(hDataSpace, hDataSet, hAgentType);
                    if (iResult == 0) {
                        iResult = pPB->restoreAdditionalDataQDF(m_hSpeciesGroup);
                    } 
                
                } else {
                    iResult = pPB->readAgentDataQDF(hDataSpace, hDataSet, hAgentType);
                    if (iResult == 0) {
                        iResult = pPB->readAdditionalDataQDF(m_hSpeciesGroup);
                    }
                }

                
                qdf_closeDataSpace(hDataSpace);
                qdf_closeDataSet(hDataSet);
            } else {
                iResult = POP_READER_ERR_CELL_MISMATCH;
                   }
        } else {
            iResult = POP_READER_ERR_READ_SPECIES_DATA;
        }
        qdf_closeGroup(m_hSpeciesGroup);
    } else {
        iResult = POP_READER_ERR_NO_SPECIES_GROUP;
    }
    return iResult;
}
