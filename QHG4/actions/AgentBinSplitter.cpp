#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>

#include <openssl/sha.h>
#include <hdf5.h>

#include <vector>
#include <string>
#include <algorithm>

#include "MessLoggerT.h"
#include "strutils.h"
#include "stdstrutilsT.h"
#include "clsutils.h"

#include "BinomialDist.h"
#include "ParamProvider2.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"

#include "AgentBinSplitter.h"

template<typename T>
const char *AgentBinSplitter<T>::asNames[] = {
    ATTR_ABS_BIN_MIN_NAME,
    ATTR_ABS_BIN_MAX_NAME,
    ATTR_ABS_NUM_BINS_NAME,
    ATTR_ABS_VAR_FIELD_NAME};

const std::string SUB_POP_GROUP_NAME = "SubPopGroups";
//const std::string AGENT_DATASET_NAME = "AgentDataSet";

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
AgentBinSplitter<T>::AgentBinSplitter(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, LBController *pAgentController)
    : Action<T>(pPop, pCG, ATTR_ABS_NAME,sID),
      m_iNumThreads(omp_get_max_threads()),
      m_pAgentController(pAgentController),
      m_dBinMin(0),
      m_dBinMax(0),
      m_iNumBins(0),
      m_sVarField(""),
      m_iVarOffset(0) {

    // prepare the vecarrays
    m_pvLBCs = new std::vector<LBController *>[m_iNumThreads];
    m_pvLBs  = new std::vector<LayerBuf<T>>[m_iNumThreads];
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
AgentBinSplitter<T>::~AgentBinSplitter() {
    if (m_pvLBCs != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            stdprintf("[AgentBinSplitter<T>::destrucotr] i=%d: %zd elements\n", i, m_pvLBCs[i].size()); fflush(stdout);
            for (uint j = 0; j < m_pvLBCs[i].size(); j++) {
                if (m_pvLBCs[i][j] != NULL) {
                    stdprintf("[AgentBinSplitter<T>::destrucotr] deleting [%d][%u]: %p\n", i, j, m_pvLBCs[i][j]); fflush(stdout);
                     delete m_pvLBCs[i][j];
                }
            }
        }
        delete[] m_pvLBCs;
    }

    if (m_pvLBs != NULL) {
        delete[] m_pvLBs;
    }
}


//-----------------------------------------------------------------------------
// getVariableOffset
//
template<typename T>
int AgentBinSplitter<T>::getVariableOffset() {
    
    int iResult = -1;

    stdprintf("[AgentBinSplitter<T>::getVariableOffset] start\n"); fflush(stdout);
    hid_t hAgentType = this->m_pPop->getAgentQDFDataType(); 
    stdprintf("[AgentBinSplitter<T>::getVariableOffset] agenttyype %lx, def %lx\n", hAgentType, H5P_DEFAULT); fflush(stdout);
    if (hAgentType != H5P_DEFAULT) {
        int iVarIndex =  H5Tget_member_index(hAgentType, m_sVarField.c_str());  
        stdprintf("[AgentBinSplitter<T>::getVariableOffset] VarIndex %d\n", iVarIndex); fflush(stdout);

        for (int i = 0; i < H5Tget_nmembers(hAgentType); i++) {
            char *pt = H5Tget_member_name(hAgentType, i);
            stdprintf("[AgentBinSplitter<T>::getVariableOffset] index %d (%03d): [%s]\n", i, H5Tget_member_offset(hAgentType, i), pt); fflush(stdout);
            H5free_memory(pt);
        }
        if (iVarIndex >= 0)  {
            //int iNumMembers = H5Tget_nmembers(hAgentType);
            m_iVarOffset =  H5Tget_member_offset(hAgentType, iVarIndex);
            stdprintf("[AgentBinSplitter<T>::getVariableOffset] VarIndex %d; VarOffset %d\n", iVarIndex, m_iVarOffset); fflush(stdout);
            m_hVarType = H5Tget_member_type(hAgentType, iVarIndex);

            iResult = 0;
        } else {
            // couldn't dind field with specifiedname
            stdprintf("[AgentBinSplitter<T>::getVariableOffset] Error: couldn't find index for [%s]\n", m_sVarField.c_str()); fflush(stdout);
        }
       
    } else {
        // got invalid agent type
        stdprintf("[AgentBinSplitter<T>::getVariableOffset] Error: got invalid agent type\n"); fflush(stdout);
    }
    stdprintf("[AgentBinSplitter<T>::getVariableOffset] end\n"); fflush(stdout);
    return iResult;
    
}

//-----------------------------------------------------------------------------
// getDVal
//
template<typename T>
double AgentBinSplitter<T>::getDVal(T &ag) {

    double dResult = fNaN;
    stdprintf("[AgentBinSplitter<T>::getDVal] start\n"); fflush(stdout);
    char *p = (char*)(&ag);
     
    if (H5Tequal(m_hVarType,  H5T_NATIVE_CHAR)) {
        char j;
        memcpy(&j, p + m_iVarOffset, sizeof(char));
        printf("retrieved char: %d\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_UCHAR)) {
        uchar j;
        memcpy(&j, p + m_iVarOffset, sizeof(char));
        printf("retrieved uchar: %d\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_SHORT)) {
        short int j;
        memcpy(&j, p + m_iVarOffset, sizeof(short int));
        printf("retrieved short: %d\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_USHORT)) {
        ushort j;
        memcpy(&j, p + m_iVarOffset, sizeof(ushort));
        printf("retrieved ushort: %d\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_INT32)) {
        int j;
        memcpy(&j, p + m_iVarOffset, sizeof(int));
        printf("retrieved int: %d\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_UINT32)) {
        uint j;
        memcpy(&j, p + m_iVarOffset, sizeof(uint));
        printf("retrieved uint: %d\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_LONG)) {
        long j;
        memcpy(&j, p + m_iVarOffset, sizeof(long));
        printf("retrieved long: %ld\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_ULONG)) {
        ulong j;
        memcpy(&j, p + m_iVarOffset, sizeof(ulong));
        printf("retrieved ulong: %ld\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_FLOAT)) {
        float j;
        memcpy(&j, p + m_iVarOffset, sizeof(float));
        printf("retrieved float: %f\n", j);
        dResult = (double)(j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_DOUBLE)) {
        double j;
        memcpy(&j, p + m_iVarOffset, sizeof(double));
        printf("retrieved double: %f\n", j);
    } else if (H5Tequal(m_hVarType,  H5T_NATIVE_DOUBLE)) {
        double j;
        memcpy(&j, p + m_iVarOffset, sizeof(double));
        printf("retrieved double: %f\n", j);
    } else {
        printf("not many left to do this is one of them\n");
    }                       

    stdprintf("[AgentBinSplitter<T>::getDVal] end\n"); fflush(stdout);

    return dResult;
}




//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int AgentBinSplitter<T>::preLoop() {
    
    stdprintf("[AgentBinSplitter<T>::preLoop] start\n"); fflush(stdout);
    int iLayerSize = m_pAgentController->getLayerSize();
    stdprintf("[AgentBinSplitter<T>::preLoop] LayerSize %d, numthreads %d, numbins %d\n", iLayerSize, m_iNumThreads, m_iNumBins); fflush(stdout);
    for (int iT = 0; iT < m_iNumThreads; iT++) {
        m_pvLBs[iT].resize(m_iNumBins);
        m_pvLBCs[iT].resize(m_iNumBins);
        for (int j = 0; j < m_iNumBins; j++) {
            
            m_pvLBs[iT][j].init(iLayerSize);

            m_pvLBCs[iT][j] = new LBController(iLayerSize);
            stdprintf("[AgentBinSplitter<T>::preLoop] created [%d][%u]: %p\n", iT, j, m_pvLBCs[iT][j]); fflush(stdout);
            m_pvLBCs[iT][j]->addBuffer(static_cast<LBBase *>(&(m_pvLBs[iT][j])));
            m_pvLBCs[iT][j]->addLayer();
        }
    }
    stdprintf("[AgentBinSplitter<T>::preLoop] end\n"); fflush(stdout);
    return 0;
    
}


//-----------------------------------------------------------------------------
// preWrite
//
template<typename T>
int AgentBinSplitter<T>::preWrite(float fTime) {

    int iResult = 0;
    stdprintf("[AgentBinSplitter<T>::preWrite] start\n"); fflush(stdout);
    iResult = getVariableOffset();
    
    if (iResult == 0) { 
        for (int i = 0; i < m_iNumBins; i++) {
            m_pvLBCs[0][i]->clear();
        }
        // loop over all agents
        //#pragma omp parallel for
        for (int iA = this->m_pPop->getFirstAgentIndex(); iA <=  this->m_pPop->getLastAgentIndex(); iA++) {
            int iT = omp_get_thread_num();

            double d = getDVal( this->m_pPop->m_aAgents[iA]);
            double dBin = m_iNumBins*(d - m_dBinMin)/(m_dBinMax - m_dBinMin);
            if (dBin < 0) {
                dBin = 0;
            } else if (dBin >= m_iNumBins) {
                dBin = m_iNumBins-1;
            }
            int iBin = (int)dBin;
            stdprintf("[AgentBinSplitter<T>::preWrite] agent @%d: val %f, bin %d\n", iA, d, iBin); fflush(stdout);
            uint iStart = m_pvLBCs[iT][iBin]->reserveSpace2(1);
            // copy block
            m_pvLBs[iT][iBin].copyBlock(iStart, &( this->m_pPop->m_aAgents[iA]), 1);

        }
        
        // here we'd need to collate the bufs
        for (int i = 0; i < m_iNumBins; i++) {
            stdprintf("[AgentBinSplitter<T>::preWrite] buffer %d has %d entries\n", i,  m_pvLBCs[0][i]->getNumUsed()); fflush(stdout);
        }

    } else {
        stdprintf("[AgentBinSplitter<T>::preWrite] getVariableOffset failed\n"); fflush(stdout);
    }
    // clear & fill al LBControllers
    // cumulate over threads
    stdprintf("[AgentBinSplitter<T>::preWrite] end\n"); fflush(stdout);
    return iResult;

}

//-----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
template<typename T>
int AgentBinSplitter<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = -1;

    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] start\n"); fflush(stdout);
    // create subgroup "SubPopulations"
    hid_t hSubPopGroup = qdf_createGroup(hSpeciesGroup, SUB_POP_GROUP_NAME);
    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] hSubPopGroup is %lx\n", hSubPopGroup); fflush(stdout);
    if (hSubPopGroup != H5P_DEFAULT)  {
        // add some attributes
        iResult = 0;
        // in "SubPopulations" loop throughLBContollers
        stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] loop through %zd controllers\n", m_pvLBCs[0].size()); fflush(stdout);
        for (unsigned i = 0; (i < m_pvLBCs[0].size()) && (iResult == 0);i++) {
            char sName[256];
            sprintf(sName, "SubPop_%03d", i);
            
            //    create subgroup "SubPop_<i>"
            stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] creating subgroup %s\n", sName); fflush(stdout);
            hid_t hSubSubGroup = qdf_createGroup(hSubPopGroup, sName);
            stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] hSubSubGroup is %lx\n", hSubSubGroup); fflush(stdout);
            if (hSubSubGroup != H5P_DEFAULT)  {
                hsize_t dims=m_pvLBCs[0][i]->getNumUsed();
                //    in "SubPop_<i> create "AgentDataSet"
                //      write agent data
                //      close "AgentDataSet"
                hid_t hAgentType = this->m_pPop->getAgentQDFDataType(); 
                hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
                stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] hDataSpace is %lx\n", hDataSpace); fflush(stdout);
                    
                if (hDataSpace > 0) {
                    
                    // Create the dataset
                    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] creating dataset %s\n", AGENT_DATASET_NAME); fflush(stdout);
                    hid_t hDataSet = H5Dcreate2(hSubSubGroup, AGENT_DATASET_NAME.c_str(), hAgentType, hDataSpace, 
                                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                    
                    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] hDataSet is %lx\n", hDataSet); fflush(stdout);
                    if (hDataSet > 0) {
                        if (dims > 0) {
                            stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] writing %d agents to bin %d\n", m_pvLBCs[0][i]->getNumUsed(), i); fflush(stdout);
                            //@@@@this->m_pPop->writeAgentDataQDFSafe(hDataSpace, hDataSet, hAgentType);
                            this->m_pPop->writeAgentDataQDFSafe(m_pvLBCs[0][i], m_pvLBs[0][i], hDataSpace, hDataSet, hAgentType);
                        } else {
                            stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] no agents to write\n"); fflush(stdout);
                        }
                        stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] closing data set %lx\n", hDataSet); fflush(stdout);
                        qdf_closeDataSet(hDataSet);
                        stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] closed data set\n"); fflush(stdout);
                    } else {
                        // couldn't create dataset
                        stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] Error couldn't create dataset]\n"); fflush(stdout);
                    }
                    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] closing data space %lx\n", hDataSpace); fflush(stdout);
                    qdf_closeDataSpace(hDataSpace);
                    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] closed data space\n"); fflush(stdout);
                } else {
                    // couldn't create dataspace
                    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] Error couldn't create dataspace]\n"); fflush(stdout);
                    iResult = -1;
                }
                
                stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] closing group [%s] %lx\n", sName, hSubSubGroup); fflush(stdout);
                qdf_closeGroup(hSubSubGroup);
                stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] closed group [%s]\n", sName); fflush(stdout);
                
            } else {
                // couldn't create subsub group
                stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] Error couldn't create subsubgroup [%s]\n", sName); fflush(stdout);
                iResult = -1;
            }
            
        }

        stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] closing group [%s] %lx\n", SUB_POP_GROUP_NAME, hSubPopGroup); fflush(stdout);
        qdf_closeGroup(hSubPopGroup);
        
    } else {
        // couldn't open subpopgroup
        iResult = -1;
    }
    stdprintf("[AgentBinSplitter<T>::writeAdditionalDataQDF] end\n"); fflush(stdout);
    return iResult;

}   


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_ABS_BIN_MIN_NAME, 
//    ATTR_ABS_BIN_MAX_NAME
//    ATTR_ABS_NUM_BINS_NAME
//    ATTR_ABS_VAR_FIELD_NAME
//
template<typename T>
int AgentBinSplitter<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ABS_BIN_MIN_NAME,  1, (double *) &m_dBinMin);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ABS_BIN_MIN_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ABS_BIN_MAX_NAME,  1, (double *) &m_dBinMax);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ABS_BIN_MAX_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ABS_NUM_BINS_NAME, 1, (int *) &m_iNumBins);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ABS_NUM_BINS_NAME);
        }
    }
    
    if (iResult == 0) {
        std::string sTemp = "";
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_ABS_VAR_FIELD_NAME, sTemp);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ABS_VAR_FIELD_NAME);
        } else {
            m_sVarField = sTemp;
        }
    }
    
    stdprintf("[Genetics] ExtractParamsQDF:res %d\n", iResult);
    
    return iResult;
}

//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int AgentBinSplitter<T>::tryGetAttributes(const ModuleComplex *pMC) {

    int iResult = -1;
    std::string sTemp=""; 

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_ABS_BIN_MIN_NAME,           &m_dBinMin);
        iResult += getAttributeVal(mParams, ATTR_ABS_BIN_MAX_NAME,           &m_dBinMax);
        iResult += getAttributeVal(mParams, ATTR_ABS_NUM_BINS_NAME,          &m_iNumBins);

        int iResult2 = getAttributeStr(mParams, ATTR_ABS_VAR_FIELD_NAME,         sTemp);
    
        if (iResult2 == 0) {
            m_sVarField = sTemp;
        }
    }

    return iResult;
}

