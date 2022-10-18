#include <cstring>
#include <omp.h>
#include <cmath>

#include <algorithm>

#include "clsutils.cpp"
#include "stdstrutils.h"
#include "ParamProvider2.h"
#include "ArrayShare.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "IndexCollector.h"

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
IndexCollector<T>::IndexCollector(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, const char *pShareName)
    : Action<T>(pPop,pCG, "IndexCollector",sID),
      m_sShareName("") {
    
    if (pShareName != NULL) {
        m_sShareName = pShareName;
    }

    int iNumCells = this->m_pCG->m_iNumCells;

    m_avLocalIndexes = new std::vector<int>[iNumCells];

    m_aIndexLocks = new omp_lock_t[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        omp_init_lock(&m_aIndexLocks[i]);
    }
    this->m_vNames.clear();
}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
IndexCollector<T>::~IndexCollector() {
    
    if (m_avLocalIndexes != NULL) {
        delete[] m_avLocalIndexes;
    }

    if (m_aIndexLocks != NULL) {
        delete[] m_aIndexLocks;
    }
}


//-----------------------------------------------------------------------------
// setShareName
//
template<typename T>
void IndexCollector<T>::setShareName(const std::string sShareName) {
    if (!sShareName.empty()) {
        m_sShareName = sShareName;
    }
}


//-----------------------------------------------------------------------------
// preLoop
//  prepare mass-interfaces and share preyratio array
//
template<typename T>
int IndexCollector<T>::preLoop() {
    int iResult = 0;

    //share the array
    ArrayShare::getInstance()->shareArray(m_sShareName, this->m_pCG->m_iNumCells, m_avLocalIndexes);
    stdprintf("[IndexCollector<T>::preLoop][%s] xxxShare shared index array as [%s]: %p\n", this->m_pPop->getSpeciesName(), m_sShareName, m_avLocalIndexes);

    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int IndexCollector<T>::initialize(float fT) {
    stdprintf("[IndexCollector<T>::initialize] collecting indexes for [%s] at T %f\n", this->m_pPop->getSpeciesName(), fT);

    int iResult = 0;

#pragma omp parallel for 
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_avLocalIndexes[iC].clear();
    }

    // collect predators
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();
    
    // loop through agents and assign to appropriate vector
    if (iFirstAgent >= 0) {

#pragma omp parallel for 
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
            
            T* pA = &(this->m_pPop->m_aAgents[iA]);
    
            if (pA->m_iLifeState > LIFE_STATE_DEAD) {
                
                int iC = pA->m_iCellIndex;
                
                omp_set_lock(&m_aIndexLocks[iC]);

                m_avLocalIndexes[iC].push_back(iA);

                omp_unset_lock(&m_aIndexLocks[iC]);
                
            }
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool IndexCollector<T>::isEqual(Action<T> *pAction, bool bStrict) {
    return true;
}
