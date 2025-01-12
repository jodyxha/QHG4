#include <cstdio>
#include <cstring>
#include <hdf5.h>

#include "xha_strutilsT.h"
#include "PopBase.h"

#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PopWriter.h"


//----------------------------------------------------------------------------
// constructor
//
PopWriter::PopWriter(std::vector<PopBase *> vPops) {
    for (uint i = 0; i < vPops.size(); i++) {
        xha_printf("[PopWriter::PopWriter] adding pop [%s]\n", vPops[i]->getSpeciesName());
        m_mDataTypes[vPops[i]] = vPops[i]->getAgentQDFDataType();
    }
}

//----------------------------------------------------------------------------
// destructor
//
PopWriter::~PopWriter() {
    poptypes::const_iterator it;
    for (it = m_mDataTypes.begin(); it != m_mDataTypes.end(); ++it) {
        qdf_closeDataType(it->second);
    }
}


//----------------------------------------------------------------------------
// write
//  opens (or creates) file for writing
//  writes pop data
//  closes file
//
int PopWriter::write(const std::string sFilename, int iStep, float fStartTime, const std::string sInfoString, const std::string sSub, popwrite_flags iWSpecial, int iDumpMode) {
    int iResult = -1;
    
    m_hFile = qdf_opencreateFile(sFilename, iStep, fStartTime, sInfoString);
    if (m_hFile != H5P_DEFAULT) {
        iResult = write(m_hFile, sSub, iWSpecial, iDumpMode);
    }
    qdf_closeFile(m_hFile); 
    return iResult;
}


//----------------------------------------------------------------------------
// write
//   writes data to the file given by handle
//   (doesn't open or close file)
//
int PopWriter::write(hid_t hFile, const std::string sSub, popwrite_flags iWSpecial, int iDumpMode) {
    int iResult = 0;

    bool bSearching = true;
    poptypes::const_iterator it;

    PopBase *pPop = NULL;
    hid_t   hDataType;
    for (it = m_mDataTypes.begin(); bSearching && (it != m_mDataTypes.end()); ++it) {
        if (it->first->getSpeciesName() == sSub) {
            pPop = it->first;
            hDataType = it->second;
            bSearching = false;
        }
    }

    if (pPop != NULL) {
        herr_t status = H5P_DEFAULT;
        
        m_hFile = hFile;
        if (m_hFile > 0) {
            // make sure Population group exists

            iResult = opencreatePopGroup();
            if (iResult == 0) {

                iResult= opencreateSpeciesGroup(pPop, iDumpMode);
                if (iResult == 0) {
                    
                    if ((iWSpecial & popwrite_flags::PW_AGENTS_ONLY) != 0) {
                    
                        // Create the data space for the dataset.
                        // do not use getNumAgentsTotal()!! -> data space is created too large -> 0-filled data at the end
                        hsize_t dims = 0;
                        if (iDumpMode != -1) {
                            dims = pPop->getNumAgentsMax();
                        } else {
                            dims = pPop->getNumAgentsEffective(); 
                        }
                        
                        // xha_fprintf(stderr, "PopWriter  [%s] numagents total: %lld\n", it->first->getSpeciesName(), dims);
                        hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
                    
                        if (hDataSpace > 0) {
                            
                            // Create the dataset
                            hid_t hDataSet = H5Dcreate2(m_hSpeciesGroup, AGENT_DATASET_NAME.c_str(), hDataType, hDataSpace, 
                                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                            
                            if (hDataSet > 0) {
                            
                                // call the population to write agent data        
                                if (iDumpMode != -1) {
                                    pPop->dumpAgentDataQDF(hDataSpace, hDataSet, hDataType);
                                } else {
                                    pPop->writeAgentDataQDF(hDataSpace, hDataSet, hDataType);
                                }
                                
                                // End access to the dataset and release resources used by it
                                qdf_closeDataSet(hDataSet);
                            }
                            // Terminate access to the data space
                            qdf_closeDataSpace(hDataSpace);
                        }
                    }   
                        
                    // other pop data if needed
                    // (dump dumps everything)
                    if (iDumpMode != -1) {
                        pPop->dumpAdditionalDataQDF(m_hSpeciesGroup);
                    } else {
                        // write additional data  if required
                        if ((iWSpecial & popwrite_flags::PW_ADDITIONAL_ONLY) != 0) {
                            pPop->writeAdditionalDataQDF(m_hSpeciesGroup);
                        }
                    }
                    // close the group
                    qdf_closeGroup(m_hSpeciesGroup);
                } else {
                    // couldn't create species group 
                }
                
                qdf_closeGroup(m_hPopGroup);
            } else {
                // couldn't open or create PopGroup or already exists
            }
            
        } else {
            //couldn't open or create QDF file
            printf("couldn't open or create QDF file\n");
        }

        if (status < 0) {
            iResult = -1;
        }

    } else {
        // this should not happen
        xha_printf("no datatype found for pop [%s]\n", sSub);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// opencreatePopGroup
//   if the group exists, open it
//   otherwise create it
//
int PopWriter::opencreatePopGroup() {
    int iResult = -1;
    
    m_hPopGroup = qdf_openGroup(m_hFile, POPGROUP_NAME);
    if (m_hPopGroup > 0) {
        // already exists
        iResult = 0;
    
    } else {
        // does not exist: create
        m_hPopGroup = qdf_createGroup(m_hFile, POPGROUP_NAME);
        if (m_hPopGroup > 0) {
            // success
            iResult = 0;
        }
    }
    return iResult;
}
    

//----------------------------------------------------------------------------
// opencreateSpeciesGroup
//
int PopWriter::opencreateSpeciesGroup(PopBase *pPB, int iDumpMode) {
    int iResult = 0;

    const std::string sSpeciesName = pPB->getSpeciesName();
    if (qdf_link_exists(m_hPopGroup, sSpeciesName)) {
        xha_printf("SpeciesGroup '%s' exists; deleting\n", sSpeciesName);
        iResult = qdf_deleteGroup(m_hPopGroup, sSpeciesName);
        if (iResult == 0) {
            printf("  deleted\n");
        } else {
            printf("  failed to delete\n");
        }
    } else {
        //        printf("SpeciesGroup '%s' doesn't exist\n", pSpeciesName);
    }

    if (iResult == 0)  {
        // does not exist: create
        m_hSpeciesGroup = qdf_createGroup(m_hPopGroup, sSpeciesName);
        if (m_hSpeciesGroup > 0) {
            // success
            iResult = 0;
            if (iDumpMode == -1) {
                iResult = pPB->writeSpeciesDataQDF(m_hSpeciesGroup);
            } else {
                iResult = pPB->dumpSpeciesDataQDF(m_hSpeciesGroup, iDumpMode);
            }
        }
    }

    return iResult;
}
   
