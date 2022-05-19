#include <cstdio>
#include <vector>
#include <string>
#include "types.h" 
#include "PseudoPopArray.h"
#include "PseudoPopManager.h"

#include "PseudoPopCounts.h"

//----------------------------------------------------------------------------
// createInstance
//
PseudoPopManager *PseudoPopManager::createInstance() {
    PseudoPopManager *pPPM = new PseudoPopManager();
    int iResult = pPPM->init();
    if (iResult != 0) {
        delete pPPM;
        pPPM = NULL;
    }
    return pPPM;
}


//----------------------------------------------------------------------------
// findMatches
//
const stringvec &PseudoPopManager::findMatches(const std::string sPopQDF, const std::string sEnvQDF) {
    m_vMatches.clear();
    
    ppopvec::const_iterator it;
    for (it = m_vPseudos.begin(); it != m_vPseudos.end(); ++it) {
        const stringvec vTemp = (*it)->findMatches(sPopQDF, sEnvQDF);
        m_vMatches.insert(m_vMatches.end(), vTemp.begin(), vTemp.end()); 
    }
    return m_vMatches;
}


//----------------------------------------------------------------------------
// createArray
//
double *PseudoPopManager::createArray(const std::string sPath, const std::string sPopQDF, const std::string sEnvQDF) {
    double *pdData = NULL;
    m_sFullPath = "";
    
    ppopvec::const_iterator it;
    for (it = m_vPseudos.begin(); (pdData == NULL) && (it != m_vPseudos.end()); ++it) {
        pdData = (*it)->createArray(sPath, sPopQDF, sEnvQDF);
        if (pdData != NULL) {
            m_iArrSize  = (*it)->getArraySize();
            m_sFullPath = (*it)->getUsedPath();
        }
    }


    return pdData;
}


//----------------------------------------------------------------------------
// destructor
//
PseudoPopManager::~PseudoPopManager() {
    ppopvec::const_iterator it;
    for (it = m_vPseudos.begin(); it != m_vPseudos.end(); ++it) {
        delete *it;
    }

}


//----------------------------------------------------------------------------
// constructor
//
PseudoPopManager::PseudoPopManager()
    : m_iArrSize(0) {
}

//----------------------------------------------------------------------------
// init
//
int PseudoPopManager::init() {
    m_vPseudos.push_back(PseudoPopCounts::createInstance());
    return 0;
}
