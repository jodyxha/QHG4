#include <cstdio>
#include <string>
#include <vector>
#include <set>
#include <regex>

#include <omp.h>

#include "types.h"
#include "stdstrutilsT.h"
#include "QDFUtils.h"
#include "AgentCounter.h"
#include "HDataSetCollector.h"
#include "PseudoPopArray.h"
#include "PseudoPopCounts.h"


//----------------------------------------------------------------------------
// createInstance
//
PseudoPopArray *PseudoPopCounts::createInstance() {
    PseudoPopCounts *pPCA = new PseudoPopCounts();
    int iResult = pPCA->init();
    if (iResult != 0) {
        delete pPCA;
        pPCA = NULL;
    }
    return pPCA;
}


//----------------------------------------------------------------------------
// findMatches
//
const stringvec &PseudoPopCounts::findMatches(const std::string sPopQDF, const std::string sEnvQDF) {
    HDataSetCollector *pHDSC = HDataSetCollector::createInstance(sPopQDF);
    if (pHDSC != NULL) {
        
        const stringvec vAllDataSets = pHDSC->getPaths();
        delete pHDSC;
        std::string sPat = m_sPathPat;
        sPat += "/(.*)";
 
        std::regex pseudoPat(sPat);

        std::set<std::string> sSpeciesSet;
        stringvec::const_iterator it;
        std::cmatch m;
        for (it = vAllDataSets.begin(); it != vAllDataSets.end(); ++it) {
            if(regex_match(it->c_str(), m, pseudoPat)) {
                std::string sSpecies = m.format("$2");
                sSpeciesSet.insert(sSpecies);
            }
        }
        std::set<std::string>::const_iterator it2;
        for (it2 = sSpeciesSet.begin(); it2 != sSpeciesSet.end(); ++it2) {
            
            if (checkRequired(*it2, sPopQDF, sEnvQDF) == 0) {
                m_vAvailablePop.push_back("Populations/"+(*it2)+"/"+m_sArrayName);
            }
        }
       
    }
    return m_vAvailablePop;
}

//----------------------------------------------------------------------------
// checkRequired
//
int PseudoPopCounts::checkRequired(std::string sSpecies, const std::string sPopQDF, const std::string sEnvQDF) {
    int iResult = 0;
    std::string sTemp  = "";
    stringvec::const_iterator it;
    for (it = m_vRequired.begin(); (iResult == 0) && (it != m_vRequired.end()); it++) {
        std::string s2 = *it;
        std::size_t iPos = s2.find("###");
        if (iPos!=std::string::npos) {
            s2.replace(iPos, 3, sSpecies);
        }
        
        sTemp = s2; 
        iResult = qdf_checkPathExists(sPopQDF, sTemp);
        if (iResult != 0) {
            iResult = qdf_checkPathExists(sEnvQDF, sTemp);
            if (iResult != 0) {
                stdprintf("The path [%s] can not be found; neither in [%s] nor in [%s]\n", s2, sPopQDF, sEnvQDF);
            }
        }
    }
 
    return iResult;
}


//----------------------------------------------------------------------------
// createArray
//
double *PseudoPopCounts::createArray(const std::string sPath, const std::string sPopQDF, const std::string sEnvQDF) {
    int iResult = 0;
    if (m_pdData != NULL) {
        delete[] m_pdData;
        m_pdData = NULL;
    }
    std::cmatch m;

    std::string sPat = m_sPathPat + "/" + m_sArrayName;

    std::regex pseudoPat(sPat);
    if(regex_match(sPath.c_str(), m, pseudoPat)) {
        m_sUsedPath = m.format("$1/$2");
        std::string sSpecies = m.format("$2"); 
        
        std::string sPathTemp = m_sUsedPath;
        iResult = qdf_checkPathExists(sPopQDF, sPathTemp);
        if (iResult == 0) {
            iResult = checkRequired(sSpecies, sPopQDF, sEnvQDF);
            
            if (iResult == 0) {
                AgentCounter *pAC = AgentCounter::createInstance(sPopQDF, sEnvQDF, NULL);
                /*int iTot = */pAC->countAgentsInCells();
                int *pCounts = pAC->getPopCounts(sSpecies);
                m_iArrSize = pAC->getNumCells();
                m_pdData = new double[m_iArrSize];
                
#pragma omp parallel for 
                for (int i = 0; i < m_iArrSize; i++) {
                    m_pdData[i] = pCounts[i];
                }
                
                delete[] pCounts;
            }
        } else {
            stdprintf("The path [%s] does not exist in the pop file [%s]\n", sPath, sPopQDF);
            
        }
        
    } else {
        stdprintf("Path [%s] does not match this PseudoPopArray [%s]\n", sPath, m_sPathPat);
    }
    return m_pdData;
}
    


//----------------------------------------------------------------------------
// constructor
//
PseudoPopCounts::PseudoPopCounts()
    : PseudoPopArray(PSEUDO_COUNT_NAME, PSEUDO_COUNT_PAT) {
}


//----------------------------------------------------------------------------
// init
//
int PseudoPopCounts::init() {
    char sTemp[1024];
    strcpy(sTemp, "Populations/###/AgentDataSet");
    m_vRequired.push_back(sTemp);
    return 0;
}

