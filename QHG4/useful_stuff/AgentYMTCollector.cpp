#include <cstdio>
#include <cstring>
#include <omp.h>
// for stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "hdf5.h"

#include "strutils.h"
#include "xha_strutilsT.h"
#include "geomutils.h"
#include "SPopulation.h"
#include "QDFUtils.h"
#include "QDFArray.h"

#include "AgentYMTCollector.h"




//----------------------------------------------------------------------------
// createInstance
//
AgentYMTCollector *AgentYMTCollector::createInstance(const std::string sPopQDF, const std::string sSpecies) {
    AgentYMTCollector *pAIC = new AgentYMTCollector();
    int iResult = pAIC->init(sPopQDF, sSpecies);
    if (iResult != 0) {
        delete pAIC;
        pAIC = NULL;
           }
    return pAIC;
}


//----------------------------------------------------------------------------
// constructor
//
AgentYMTCollector::AgentYMTCollector() 
    : m_pInfos(NULL),
      m_sDataType(""),
      m_iCellIDArr(NULL),
      m_iGenderArr(NULL),
      m_iYchrArr(NULL),
      m_imtDNAArr(NULL) {
}
  

//----------------------------------------------------------------------------
// destructor
//
AgentYMTCollector::~AgentYMTCollector() {
    if (m_pInfos != NULL) {
        delete[] m_pInfos;
    }

    if (m_imtDNAArr != NULL) {
        delete[] m_imtDNAArr;
    }
    if (m_iYchrArr != NULL) {
        delete[] m_iYchrArr;
    }
    if (m_iGenderArr != NULL) {
        delete[] m_iGenderArr;
    }
    if (m_iCellIDArr != NULL) {
        delete[] m_iCellIDArr;
    }
}


//----------------------------------------------------------------------------
// init
//
int AgentYMTCollector::init(const std::string sPopQDF, const std::string sSpecies) {
    int iResult = 0;

    float f1 = omp_get_wtime();
    iResult = loadAgentsCell(sPopQDF, sSpecies);
    float f2 = omp_get_wtime();
    if (iResult == 0) {
        separateValues();
        
        xha_fprintf(stderr, "Successfully read agent items from [%s] (%fs)\n", sPopQDF, f2 - f1);
    } else {
        xha_fprintf(stderr, "Couldn't get agent items\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// getRootAttributes
//  extract the attribtes of the root group
//
int AgentYMTCollector::getRootAttributes(hid_t hFile) {
    int iResult = 0;

    std::string sValue = "";
    iResult = qdf_extractSAttribute(hFile, ROOT_STEP_NAME, sValue);
    if (iResult == 0) {
        if (strToNum(sValue, &m_iCurStep)) {
            iResult = qdf_extractSAttribute(hFile, ROOT_TIME_NAME, sValue);
            if (iResult == 0) {
                if (strToNum(sValue, &m_fStartTime)) {
                    iResult = 0;
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "Couldn't convert attribute [%s] (%s) to number\n", ROOT_TIME_NAME, sValue);
                }
            } else {
                iResult = -1;
                xha_fprintf(stderr, "Couldn't get attribute [%s]\n", ROOT_TIME_NAME);
            }
        } else {
            iResult = -1;
            xha_fprintf(stderr, "Couldn't convert attribute [%s] (%s) to number\n", ROOT_STEP_NAME, sValue);
        }
    } else {
        iResult = -1;
        xha_fprintf(stderr, "Couldn't get attribute [%s]\n", ROOT_STEP_NAME);
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
hid_t AgentYMTCollector::createCompoundDataType() {
    hid_t hAgentDataType = H5P_DEFAULT;
           
    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(aginfo_ymt));

    // reading compound data: the second argument to H5Tinsert must be the name of an element in the structure 
    // used in the HDF file
    // if the name dopes not exist, noerror is produced and the  results are undefined
    // Here we must use the names of the elements of OoANavSHybYchMTDPop written to QDF 

    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID.c_str(),   HOFFSET(aginfo_ymt, m_ulCellID),  H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID.c_str(),  HOFFSET(aginfo_ymt, m_ulID),      H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER.c_str(),    HOFFSET(aginfo_ymt, m_iGender),   H5T_NATIVE_UCHAR);
    H5Tinsert(hAgentDataType, "PheneticHyb",             HOFFSET(aginfo_ymt, m_fHybridization),     H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "Ychr",                    HOFFSET(aginfo_ymt, m_iYchr),     H5T_NATIVE_UCHAR);
    H5Tinsert(hAgentDataType, "mtDNA",                   HOFFSET(aginfo_ymt, m_imtDNA),    H5T_NATIVE_UCHAR);
    
    return hAgentDataType;
}


//----------------------------------------------------------------------------
// loadAgentsCell
//  read the compound data from the AgentDataSet
//
int AgentYMTCollector::loadAgentsCell(const std::string sPop, const std::string sPopName) {
    hid_t hFilePop     = qdf_openFile(sPop);

    int iResult = getRootAttributes(hFilePop);
    
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    xha_fprintf(stderr, "[loadAgentsCell] pop %s,popname %s\n", sPop, sPopName);
    // at this point m_iNumCells should be known (loadNPP() loadAltIce() already called)


    hid_t hSpecies     = qdf_openGroup(hPopulation, sPopName);
    hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace   = H5Dget_space(hDataSet);

    hid_t hAgentDataType    = createCompoundDataType();
    if (hAgentDataType != H5P_DEFAULT) {

        hsize_t dims;
        H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        m_iNumAgents = dims;
        m_pInfos = new uchar[m_iNumAgents*sizeof(aginfo_ymt)];

        
        hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
        herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pInfos);
        if (status >= 0) {
            xha_fprintf(stderr, "pop %s: %llu\n", sPopName, dims);
            /*
            xha_fprintf(stderr, "AgnetYMTCollector: some values after readin\n");
            for (uint i = 0; i < 5; i++) {
                aginfo_ymt *pai = (aginfo_ymt  *)m_pInfos;
                xha_fprintf(stderr, "%d: gender %d, hyb %f, y %d, mt %d\n", i, pai[i].m_iGender, pai[i].m_fHybridization, pai[i].m_iYchr, pai[i].m_imtDNA);
                int j = m_iNumAgents-i-1;
                xha_fprintf(stderr, "%d: gender %d, hyb %f, y %d, mt %d\n", j, pai[j].m_iGender, pai[j].m_fHybridization, pai[j].m_iYchr, pai[j].m_imtDNA);
            }
            */
        } else {
            xha_fprintf(stderr, "bad status for pop %s\n", sPopName);
            
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
   
    return iResult;
}


//----------------------------------------------------------------------------
// separateValues
//   create separate arrays from the array of compound data
//   for easier handling in parallelized loop
//
void AgentYMTCollector::separateValues() {
    m_iCellIDArr = new int[m_iNumAgents];
    memset(m_iCellIDArr, 0, m_iNumAgents*sizeof(int));
    m_iGenderArr = new uchar[m_iNumAgents];
    memset(m_iGenderArr, 0, m_iNumAgents*sizeof(uchar));
    m_iYchrArr = new uchar[m_iNumAgents];
    memset(m_iYchrArr, 0, m_iNumAgents*sizeof(uchar));
    m_imtDNAArr = new uchar[m_iNumAgents];
    memset(m_imtDNAArr, 0, m_iNumAgents*sizeof(uchar));

#pragma omp parallel for
    for (uint i = 0; i < m_iNumAgents; i++) {
        m_iCellIDArr[i] = (int)((aginfo_ymt*)m_pInfos)[i].m_ulCellID;
        m_iGenderArr[i] = (uchar)((aginfo_ymt*)m_pInfos)[i].m_iGender;
        m_iYchrArr[i]   = (uchar)((aginfo_ymt*)m_pInfos)[i].m_iYchr;
        m_imtDNAArr[i]  = (uchar)((aginfo_ymt*)m_pInfos)[i].m_imtDNA;
    }
}

