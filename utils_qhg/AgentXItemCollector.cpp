#include <cstdio>
#include <cstring>
#include <omp.h>
// for stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "hdf5.h"

#include "strutils.h"
#include "stdstrutilsT.h"
#include "geomutils.h"
#include "SPopulation.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "AgentXItemCollector.h"

stringmap c_mTypeTranslation = {
    {"byte",   "int_8"},
    {"int",    "int_32"},
    {"long",   "int_64"},
    {"float",  "float_32"},
    {"double", "float_64"},
};
stringmap c_mInvTypeTranslation = {
    {"int_8",    "byte",  },
    {"int_32",   "int",   },
    {"int_64",   "long",  },
    {"float_32", "float", },
    {"float_64", "double",},
};

//----------------------------------------------------------------------------
// createInstance
//
AgentXItemCollector *AgentXItemCollector::createInstance(const std::string sPopQDF, const std::string sSpecies, field_data_vec &vFieldInfo, size_t iStructSize) {
    AgentXItemCollector *pAIC = new AgentXItemCollector();
    int iResult = pAIC->init(sPopQDF, sSpecies, vFieldInfo, iStructSize);
    if (iResult != 0) {
        delete pAIC;
        pAIC = NULL;
    }
    return pAIC;
}


//----------------------------------------------------------------------------
// constructor
//
AgentXItemCollector::AgentXItemCollector() 
    : m_pInfos(NULL) {
}
  

//----------------------------------------------------------------------------
// destructor
//
AgentXItemCollector::~AgentXItemCollector() {
    if (m_pInfos != NULL) {
        delete[] m_pInfos;
    }

}


//----------------------------------------------------------------------------
// init
//
int AgentXItemCollector::init(const std::string sPopQDF, const std::string sSpecies, field_data_vec &vFieldInfo, size_t iStructSize) {
    int iResult = 0;

    float f1 = omp_get_wtime();
          
    iResult = loadAgentsCell(sPopQDF, sSpecies, vFieldInfo, iStructSize);
    float f2 = omp_get_wtime();
    if (iResult == 0) {
        
        stdfprintf(stderr, "Successfully read agent items from [%s] (%fs)\n", sPopQDF, f2 - f1);
    } else {
        stdfprintf(stderr, "Couldn't get agent items\n");
    }
    return iResult;
}



//----------------------------------------------------------------------------
// getRootAttributes
//  extract the attribtes of the root group
//
int AgentXItemCollector::getRootAttributes(hid_t hFile) {
    int iResult = 0;

    std::string sValue = "";
    iResult = qdf_extractSAttribute(hFile, ROOT_STEP_NAME, sValue);
    if (iResult == 0) {
        if (strToNum(sValue, &m_iCurStep)) {
            iResult = qdf_extractSAttribute(hFile, ROOT_TIME_NAME, sValue);
            if (iResult == 0) {
                if (strToNum(sValue, &m_fStartTime)) {

                    stdfprintf(stderr, "step %d, start %f\n", m_iCurStep, m_fStartTime);
                    iResult = 0;
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "Couldn't convert attribute [%s] (%s) to number\n", ROOT_TIME_NAME, sValue);
                }
            } else {
                iResult = -1;
                stdfprintf(stderr, "Couldn't get attribute [%s]\n", ROOT_TIME_NAME);
            }
        } else {
            iResult = -1;
            stdfprintf(stderr, "Couldn't convert attribute [%s] (%s) to number\n", ROOT_STEP_NAME, sValue);
        }
    } else {
        iResult = -1;
        stdfprintf(stderr, "Couldn't get attribute [%s]\n", ROOT_STEP_NAME);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getItemNames
//  extract the name of the items in the AgentDataSet
//
int AgentXItemCollector::getItemNames(const std::string sPopQDF, const std::string sSpecies) {
    int iResult = -1; 

    hid_t hFile  = qdf_openFile(sPopQDF, "r");
    if (hFile != H5P_DEFAULT) {
        iResult = getRootAttributes(hFile);
        if (iResult == 0) {
            std::string sGroupName = stdsprintf("%s/%s", POPGROUP_NAME, sSpecies);
        hid_t hGroup = qdf_openGroup(hFile, sGroupName);
        if (hGroup != H5P_DEFAULT) {
            hid_t hDSet  = qdf_openDataSet(hGroup, AGENT_DATASET_NAME);
            if (hDSet != H5P_DEFAULT) {
                hid_t hType  = H5Dget_type(hDSet);
                if (hType > H5P_DEFAULT) {
                    int n = H5Tget_nmembers(hType);
                    for (int i = 0; i < n; i++) {
                        hid_t st = H5Tget_member_type(hType, i);
                        H5T_class_t c = H5Tget_member_class(hType, i);
                        size_t ss = 8*H5Tget_size(st);
                        char sc[1024];

                        sprintf(sc, "unsupported class");
                        if ((c == H5T_INTEGER) || (c==H5T_FLOAT)) {
                            if (c == H5T_INTEGER) {
                                sprintf(sc,"int_%zd", ss);
                            } else { 
                                sprintf(sc,"float_%zd", ss);
                            }
                        }
                        m_mItemNames[H5Tget_member_name(hType,i)] = sc;
                    }
                    iResult = 0;
                    
                    H5Tclose(hType);
                } else {
                    stdfprintf(stderr, "Couldn't open get data type for data set [%s]\n", AGENT_DATASET_NAME);
                }
                qdf_closeDataSet(hDSet);
            } else {
                stdfprintf(stderr, "Couldn't open data set [%s]\n", AGENT_DATASET_NAME);
            }
            qdf_closeGroup(hGroup);
        } else {
            stdfprintf(stderr, "Couldn't open group [%s]\n", sGroupName);
        }

        } else {
            stdfprintf(stderr, "Couldn't get root attributesof [%s]\n", sPopQDF);
        }
        qdf_closeFile(hFile);

    } else {
        stdfprintf(stderr, "Couldn't open [%s] for reading\n", sPopQDF);
    }
    return iResult;

}




//----------------------------------------------------------------------------
// createCompoundDataType
//   create a compound datattype containing
//      agent id
//      cell  id
//      item 
//
hid_t AgentXItemCollector::createCompoundDataType(field_data_vec &vFieldInfo, size_t iStructSize) {
    hid_t hAgentDataType = H5P_DEFAULT;

    hAgentDataType = H5Tcreate (H5T_COMPOUND, iStructSize);

    field_data_vec::const_iterator it;
    for (it = vFieldInfo.begin(); it != vFieldInfo.end(); ++it) {
        H5Tinsert(hAgentDataType, it->m_sNameIn.c_str(), it->m_iOffset,  it->m_hType);
    }        
        
    return hAgentDataType;
}


typedef struct aginfo_ymt {
    aginfo_ymt(): m_ulID(0), m_ulCellID(0), m_iGender(0), m_fHybridization(0), m_iYchr(0), m_imtDNA(0) {};
    aginfo_ymt(idtype ulID, gridtype ulCellID, uchar iGender, float fHyb, uchar iYchr, uchar imtDNA): m_ulID(ulID), m_ulCellID(ulCellID), m_iGender(iGender), m_fHybridization(fHyb), m_iYchr(iYchr), m_imtDNA(imtDNA) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    uchar    m_iGender;
    float    m_fHybridization;
    uchar    m_iYchr;
    uchar    m_imtDNA;
} aginfo_ymt;

//----------------------------------------------------------------------------
// loadAgentsCell
//  read the compound data from the AgentDataSet
//
int AgentXItemCollector::loadAgentsCell(const std::string sPop, const std::string sPopName, field_data_vec &vFieldInfo, size_t iStructSize) {

    hid_t hFilePop     = qdf_openFile(sPop);
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    stdfprintf(stderr, "[loadAgentsCell] pop %s,popname %s\n", sPop, sPopName);
    // at this point m_iNumCells should be known (loadNPP() loadAltIce() already called)

    int iResult = getRootAttributes(hFilePop);
    
    hid_t hSpecies     = qdf_openGroup(hPopulation, sPopName);
    hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace   = H5Dget_space(hDataSet);

    hid_t hAgentDataType    = createCompoundDataType(vFieldInfo, iStructSize);
    if (hAgentDataType != H5P_DEFAULT) {

        hsize_t dims;
        H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        m_iNumAgents = dims;
        m_pInfos =  new uchar[m_iNumAgents*iStructSize];
 
        hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
        herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pInfos);
        if (status >= 0) {
            stdfprintf(stderr, "pop %s: %llu\n", sPopName, dims);
        } else {
            stdfprintf(stderr, "bad status for pop %s\n", sPopName);
            
            delete[] m_pInfos;
            m_pInfos = NULL;
            iResult = -1;
        }
        qdf_closeDataSpace(hMemSpace);
    }

    qdf_closeDataType(hAgentDataType);
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
    qdf_closeGroup(hSpecies);
   
    qdf_closeGroup(hPopulation);
    qdf_closeFile(hFilePop);
   

    /*
    stdfprintf(stderr, "AgentXItemCollector: some values of %u after reading\n", m_iNumAgents); 
    for (uint i = 0; i < 5; i++) {
        aginfo_ymt *pai = (aginfo_ymt *)m_pInfos;
        stdfprintf(stderr, "%d: agent id %u, cell id %d, hyb %f\n", i, pai[i].m_ulID, pai[i].m_ulCellID, pai[i].m_fHybridization);
        int j = m_iNumAgents-i-1;
        stdfprintf(stderr, "%d: agent id %u, cell id %d, hyb %f\n", j, pai[j].m_ulID, pai[j].m_ulCellID, pai[j].m_fHybridization);
    }
    */
    return iResult;
}


