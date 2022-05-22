#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <omp.h>

#include <vector>

#include "stdstrutilsT.h"
#include "HybChecker.h"

//----------------------------------------------------------------------------
// constructor
//
HybChecker::HybChecker()
    : m_iNumAgents(-1),
      m_pHAL(NULL) {

}


//----------------------------------------------------------------------------
// destructor
//
HybChecker::~HybChecker() {
    
    delete m_pHAL;

}


//----------------------------------------------------------------------------
// createInstance
//
HybChecker *HybChecker::createInstance(const std::string sPop, BinFunc *pBF) {
    HybChecker *pAC = new HybChecker();
    int iResult = pAC->init(sPop, pBF);
    if (iResult != 0) {
        delete pAC;
        pAC = NULL;
    }
    return pAC;
}


//----------------------------------------------------------------------------
// init
//
int HybChecker::init(const std::string sPop, BinFunc *pBF) {
    int iResult = 0;

    m_pHAL = HybAgentLoader::createInstance(sPop);
    m_pBF  = pBF;
    
    m_iNumAgents = m_pHAL->getNumAgents();
    return iResult;
}

#define EPS 1e-4



//----------------------------------------------------------------------------
// analyse
//
int HybChecker::analyze() {
    int iResult  = 0;
    m_iNumBins = m_pBF->getNumBins();
    uint *piBins = m_pBF->getBins();
    memset(piBins, 0, m_iNumBins*sizeof(uint));

    m_fMinNonZero = 1;
    m_fMaxNonOne  = 0;
    m_iZeros = 0;
    m_iOnes = 0;
    int iC = 0;
    aginfo *pInfos = m_pHAL->getInfos();
    for (int i = 0; i < m_iNumAgents; i++)  {
        //float v = pInfos[i].m_fHybridization;
        float v = pInfos[i].m_fPhenHyb;
        //printf("v %f\n", v);
        
        int iBin = m_pBF->calcBin(v);
        //printf("val %f -> bin %d\n", v, iBin);
        piBins[iBin]++; 

        iC++;
        if (v > m_fMaxNonOne) {
            if ((v < 1.0-EPS)) {
                m_fMaxNonOne = v;
            }
        }
        if (v < m_fMinNonZero) {
            if ((v > 0.0+EPS)) {
                m_fMinNonZero = v;
            }
        }
        if (v <= 0+EPS) {
            m_iZeros++;
        }
        if (v >= 1-EPS) {
            m_iOnes++;
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// showSimple
//
void HybChecker::showSimple() {
    //    printf("loop: %d\n", iC); 
    stdprintf("number of agents: %d\n", m_iNumAgents);
    stdprintf("number of zeros:  %d\n", m_iZeros);
    stdprintf("number of ones:   %d\n", m_iOnes);
    stdprintf("min non zero: %e\n", m_fMinNonZero);
    stdprintf("max non one:  %e\n",  1-m_fMaxNonOne);
    stringvec vHeaders;
    stringvec vValues;
    m_pBF->getHeaders(vHeaders);
    m_pBF->getValues(vValues);
    for (int i = 0; i < vHeaders.size(); i++) {
        int j = i;
        stdprintf("[%02d] %s   %s\n", j-1, vHeaders[i], vValues[i]);
    }
}  




//----------------------------------------------------------------------------
// showCSVHeader
//
void HybChecker::showCSVHeader() {
    stdprintf("#Agents;#Zeros;#Ones;Min non Zero;1-(Max non One)");
    stringvec vHeaders;
    m_pBF->getHeaders(vHeaders);

    for (int i = 0; i < vHeaders.size(); i++) {
        stdprintf(";\"%s\"", vHeaders[i]);
    }
    stdprintf("\n");

}


//----------------------------------------------------------------------------
// showCSVLine
//
void HybChecker::showCSVLine() {
    stdprintf("%d;%d;%d;%e;%e", m_iNumAgents, m_iZeros, m_iOnes, m_fMinNonZero, 1-m_fMaxNonOne);
    stringvec vValues;
    m_pBF->getValues(vValues);
    for (int i = 0; i < vValues.size(); i++) {
        stdprintf(";%s", vValues[i]);
    }
    stdprintf("\n");
}

