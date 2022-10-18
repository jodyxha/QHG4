#include <cstdio>
#include <cstring>
#include <hdf5.h>

#include "QDFUtils.h"

#define POPGROUP_NAME "Populations"


struct TestAgent  {
    uint     m_iLifeState;
    int      m_iCellIndex;
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fBirthTime;
    uchar    m_iGender;

    float m_fAge;
    float m_fLastBirth;
    
    int m_iMateIndex;
    int m_iNumBabies;
    float m_fGenHybM;
    float m_fGenHybP;
    float m_fPhenHyb;
    // dummy
    float m_fHybridization;
    // for WeightedMove
    double  m_dMoveProb;
    //  for OldAgeDeath
    double  m_dMaxAge;
    double  m_dUncertainty;
    // for Fertility
    float   m_fFertilityMinAge;
    float   m_fFertilityMaxAge;
    float   m_fInterbirth;
    
    // for HybBiirthDeathRel
    double  m_dB0;
    double  m_dD0;
    double  m_dTheta;
    
    double  m_dBReal;
    // for NPP
    double  m_dCC;
    /**/
};

#define ABUFSIZE 12288
//----------------------------------------------------------------------------
// createAgentDataTypeQDF
//  Create the HDF5 datatype for Agent data
//
hid_t  createAgentDataTypeQDF() {

    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(TestAgent));
    printf("array needs %ld bytes\n", ABUFSIZE*sizeof(TestAgent));
    TestAgent agent;

    H5Tinsert(hAgentDataType, "LifeState",       qoffsetof(agent, m_iLifeState),       H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "CellIdx",         qoffsetof(agent, m_iCellIndex),       H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "CellID",          qoffsetof(agent, m_ulCellID),         H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "AgentID",         qoffsetof(agent, m_ulID),             H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, "BirthTime",       qoffsetof(agent, m_fBirthTime),       H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "Gender",          qoffsetof(agent, m_iGender),          H5T_NATIVE_UCHAR);
    H5Tinsert(hAgentDataType, "Age",             qoffsetof(agent, m_fAge),             H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "LastBirth",       qoffsetof(agent, m_fLastBirth),       H5T_NATIVE_FLOAT);
    
    H5Tinsert(hAgentDataType, "GeneticHybM",     qoffsetof(agent, m_fGenHybM), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "GeneticHybP",     qoffsetof(agent, m_fGenHybP), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "PheneticHyb",     qoffsetof(agent, m_fPhenHyb), H5T_NATIVE_FLOAT);
    
    H5Tinsert(hAgentDataType, "MoveProb",        qoffsetof(agent, m_dMoveProb), H5T_NATIVE_DOUBLE);
    H5Tinsert(hAgentDataType, "MaxAge",          qoffsetof(agent, m_dMaxAge), H5T_NATIVE_DOUBLE);
    H5Tinsert(hAgentDataType, "Uncertainty",     qoffsetof(agent, m_dUncertainty), H5T_NATIVE_DOUBLE);
    
    H5Tinsert(hAgentDataType, "FertilityMinAge", qoffsetof(agent, m_fFertilityMinAge), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "FertilityMaxAge", qoffsetof(agent, m_fFertilityMaxAge), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, "Interbirth",      qoffsetof(agent, m_fInterbirth), H5T_NATIVE_FLOAT);
    
    H5Tinsert(hAgentDataType, "B0",              qoffsetof(agent, m_dB0), H5T_NATIVE_DOUBLE);
    H5Tinsert(hAgentDataType, "D0",              qoffsetof(agent, m_dD0), H5T_NATIVE_DOUBLE);
    H5Tinsert(hAgentDataType, "Theta",           qoffsetof(agent, m_dTheta), H5T_NATIVE_DOUBLE);
    
    return hAgentDataType;
}

//----------------------------------------------------------------------------
// readAgentDataQDF
//  read attributes to species data
//
int  readAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {

    int iResult = 0;
    TestAgent aBuf[ABUFSIZE];
    hsize_t dims = 0;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    hsize_t iOffset = 0;
    hsize_t iCount  = 0;
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;
    // compacting seems ok, since we're changing the occupancy anyway
    //compactData();

    //updateTotal();
    long m_iMaxID  = 0;
    while ((iResult == 0) && (dims > 0)) {
        if (dims > ABUFSIZE) {
            iCount = ABUFSIZE;
        } else {
            iCount = dims;
        }

        // read a buffer full
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, &iStride, &iCount, &iBlock);
        status = H5Dread(hDataSet, hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, aBuf);
        if (status >= 0) {

            for (uint j =0; j < iCount; j++) {
                if (aBuf[j].m_ulID > m_iMaxID) {
                    m_iMaxID = aBuf[j].m_ulID;
                }
            }
 
            dims -= iCount;
            iOffset += iCount;

        } else {
            iResult = -1;
        }
    }
 
    //updateTotal();

    //updateNumAgentsPerCell();

    printf("maxiD: %ld\n", m_iMaxID);
    return iResult;
}

//----------------------------------------------------------------------------
// read
//
int read(const char *pFileName, const char *pSpeciesName) {
    int iResult = -1;    

    hid_t m_hFile = qdf_openFile(pFileName);
    if (m_hFile != H5P_DEFAULT) {
       
        hid_t m_hPopGroup = qdf_openGroup(m_hFile, POPGROUP_NAME);
        if (m_hPopGroup > 0) {
            //    printf("reading data for [%s]\n", pSpeciesName);
            hid_t m_hSpeciesGroup = qdf_openGroup(m_hPopGroup, pSpeciesName);
            // read attributes
            if (m_hSpeciesGroup > 0) {
                hid_t hAgentType = createAgentDataTypeQDF();
                hid_t hDataSet = H5Dopen2(m_hSpeciesGroup, AGENT_DATASET_NAME, H5P_DEFAULT);
                hid_t hDataSpace = H5Dget_space(hDataSet);

                //  printf("species group open: reading species data\n");
                iResult = readAgentDataQDF(hDataSpace, hDataSet, hAgentType);
                qdf_closeGroup(m_hSpeciesGroup);
            }
            qdf_closeGroup(m_hPopGroup);
        }
            qdf_closeFile(m_hFile);
    } else {
        iResult = -2;
    }
    return iResult;
}


int main (int iArgC, char *apArgV[]) {
    int iResult = 0;
    char pFileName[256];
    char pSpecies[256];
    strcpy(pFileName, "/home/jody/simmov/NPersZOoANavSHybRelPop.qdf");
    strcpy(pSpecies, "sapiens");
    iResult = read(pFileName, pSpecies);
    return iResult;
}
