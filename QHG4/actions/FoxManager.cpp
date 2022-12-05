#include <omp.h>
#include <cmath>
#include <algorithm>

#include "types.h"
#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"

#include "FoxManager.h"



//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
FoxManager<T>::FoxManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID,
                          const std::string sNameFoxCount,    const std::string sNameRabbitMassAvail,  
                          const std::string sNameRabbitCells, const std::string sNameDeadRabbits)
    : Action<T>(pPop,pCG,ATTR_FOXMAN_NAME,sID),
     m_pAS(ArrayShare::getInstance()),
     m_aiFoxCount(NULL),
     m_adRabbitMassAvail(NULL),
    m_iNumCells(pCG->m_iNumCells),
    m_sNameFoxCount(sNameFoxCount),
    m_sNameRabbitMassAvail(sNameRabbitMassAvail),
    m_sNameRabbitCells(sNameRabbitCells),
    m_sNameRabbitCells(sNameDeadRabbits) {

    m_avDeadRabbits = new std::vector<std::pair<int,int>>[1];
    m_avLocFoxIDs = new std::vector<int>[m_iNumCells];
    m_aFLocks = new omp_lock_t[m_iNumCells];
 
    for (int i = 0; i < m_iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
    }
}



//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
FoxManager<T>::~FoxManager() {
     
    if (m_avLocFoxIDs != NULL) {
        delete[] m_avLocFoxIDs;
    }                           

    if (m_aFLocks != NULL) {
        delete[] m_aFLocks;
    }
    if (m_avDeadRabbits != NULL) {
        delete[] m_avDeadRabbits;
    }
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int FoxManager<T>::preLoop() {
    int iResult = 0;

    // get shared arrays
   

    if (iResult == 0) {
        m_aiFoxCount = (double *) m_pAS->getArray(m_sNameFoxCount);
        if (m_aiFoxCount != NULL) {
            iResult = 0;
        }
    }

    if (iResult == 0) {
        m_adRabbitMassAvail = (double *) m_pAS->getArray(m_sNameRabbitMassAvail);
        if (m_adRabbitMassAvail != NULL) {
            iResult = 0;
        }
    }

    if (iResult == 0) {
        m_vLocRabbitIDs = (std::vector<int> *) m_pAS->getArray(m_sNameRabbitCells);
        if (m_vLocRabbitIDs != NULL) {
            iResult = 0;
        }
    }
    if (iResult == 0) {
        m_avDeadRabbits = (std::vector<int> *) m_pAS->getArray(m_sNameDeadRabbits);
        if (m_avDeadRabbits != NULL) {
            iResult = 0;
        }
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// initialize
//  determine grass_mass_consumed and increase masses
//
template<typename T>
int FoxManager<T>::initialize(float fT) {
    int iResult = 0;

#pragma omp parallel for 
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_avLocFoxIDs[iC].clear();
    }
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();
    std::vector<idvec> v;
    // fill a vector of vectors, v[i} = IDs of  Rabbits in cell i 
#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        T* pA = &(this->m_pPop->m_aAgents[iA]);

        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;
            omp_set_lock(&m_aFLocks[iC]);
                    
            m_avLocFoxIDs[iC].push_back(iA);
                    
            omp_unset_lock(&m_aFLocks[iC]);
        }
    }
    // if prio(RabbitManager) < prio(FoxManager then the vRLocRabbitIDs is set
#pragma omp parallel for 
    for (int iC = 0; iC < m_iNumCells; iC++) {
        if (m_avLocFoxIDs[iC].size() < m_vLocRabbitIDs[iC].size()) {
            // drop the excess rabbits
            m_vLocRabbitIDs[iC].resize(m_avLocFoxIDs[iC].size());
        } else {
            // drop the excess rabbits
            m_avLocFoxIDs[iC].resize(m_vLocRabbitIDs[iC].size());
        }
        
        for (uint j = 0; j <  m_avLocFoxIDs[iC].size(); j++) {
            T* pA = &(this->m_pPop->m_aAgents[j]);
            pA->m_dMass += m_vLocRabbitIDs[iC][j].second;
        
        }
    }

    // all rabbits still here will die
    m_avDeadRabbits[0].clear();
    for (int iC = 0; iC < m_iNumCells; iC++) {
        
        for (uint j = 0; j <  m_vLocRabbitIDs[iC].size(); j++) {
            m_avDeadRabbits[0].push_back(std::pair<int,int>({iC,m_vLocRabbitIDs[iC][j].first}));
        }
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int FoxManager<T>::finalize(float fT) {
    int iResult = 0;

    // registerDeath was already called, so we only have to update fox count
    if (iResult == 0) {
        memset(m_aiFoxCount, 0, m_iNumCells*sizeof(int));
        for (uint iC = 0; iC < m_iNumCells; iC++) {
            m_aiFoxCount[iC] = m_avLocFoxIDs[iC].size();
        } 
    }
    
    return iResult;
}
