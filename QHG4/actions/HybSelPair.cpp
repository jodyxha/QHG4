#include <omp.h>
#include <cmath>
#include <algorithm>

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "HybSelPair.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
HybSelPair<T>::HybSelPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID) 
    : Action<T>(pPop,pCG,ATTR_HYBSELPAIR_NAME,sID) {
 
    int iNumCells = this->m_pCG->m_iNumCells;

    m_vLocFemalesID = new hybidxvec[iNumCells];
    m_vLocMalesID   = new hybidxvec[iNumCells];
 
    m_aFLocks = new omp_lock_t[iNumCells];
    m_aMLocks = new omp_lock_t[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
        omp_init_lock(&m_aMLocks[i]);
    }

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
HybSelPair<T>::~HybSelPair() {
    
    if (m_vLocFemalesID != NULL) {
        delete[] m_vLocFemalesID;
    }

    if (m_vLocMalesID != NULL) {
        delete[] m_vLocMalesID;
    }

    if (m_aFLocks != NULL) {
        delete[] m_aFLocks;
    }
    if (m_aMLocks != NULL) {
        delete[] m_aMLocks;
    }

}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int HybSelPair<T>::initialize(float fT) {
    
    int iResult = 0;

#pragma omp parallel for 
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_vLocFemalesID[iC].clear();
        m_vLocMalesID[iC].clear();
    }

   int iFirstAgent = this->m_pPop->getFirstAgentIndex();
   int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for 
   for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        this->m_pPop->m_aAgents[iA].m_iMateIndex = -3;
   }

    iResult = findMates();

    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
// reset 
//
template<typename T>
int HybSelPair<T>::finalize(float fT) {
    
    return 0;

}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T>
int HybSelPair<T>::findMates() {

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();


    // fill local male and female index vectors with fertile individuals only
#pragma omp parallel for
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {

        T* pA = &(this->m_pPop->m_aAgents[iA]);
        
        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;
            // we expect the agent to have a field 'm_fHybridization'
            float fHyb = pA->m_fHybridization;

            if (pA->m_iLifeState == LIFE_STATE_FERTILE) { 
                if (pA->m_iGender == 0)  { // FEMALE
                    
                    omp_set_lock(&m_aFLocks[iC]);
                    m_vLocFemalesID[iC].push_back(hybidx(iA, fHyb));
                    omp_unset_lock(&m_aFLocks[iC]);
                } else if (pA->m_iGender == 1) { // MALE

                    omp_set_lock(&m_aMLocks[iC]);
                    m_vLocMalesID[iC].push_back(hybidx(iA, fHyb));
                    omp_unset_lock(&m_aMLocks[iC]);
                }
            }
            
        }
    }


    // pairing
    // this is safe for cell parallelisation because all agents participating in the pairing arre in the same cell
#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
            
        hybidxvec *p1 = NULL;
        hybidxvec *p2 = NULL;
        if (m_vLocFemalesID[iC].size() < m_vLocMalesID[iC].size()) {
            p1 = &m_vLocFemalesID[iC];
            p2 = &m_vLocMalesID[iC];
        } else {
            p1 = &m_vLocMalesID[iC];
            p2 = &m_vLocFemalesID[iC];
        }

        // for each agent in p1 we look for the agent in p2 with smallest hybridization difference.
        // we swap matching agents in p2 to the positions of the corresponding agents in p1
        for (uint i = 0; i < p1->size(); i++) {
            // find the agent in p2 with thsmallest hyb distance
            float fMin = 2.0;
            int iMin = -1;
            float hCur1 = p1->at(i).second;
        
            for (uint j = i; j < p2->size(); j++) {
                float hCur2 = p2->at(j).second;

                float fDiff = fabs(hCur2-hCur1);
                if (fDiff < fMin) {
                    fMin = fDiff;
                    iMin = j;
                }
            }
            // put the selected agent from p2 to the correct position (swap it with the i-th elment)
            hybidx h = p2->at(iMin);
            p2->at(iMin) = p2->at(i);
            p2->at(i) = h;
        }

        // now the first p1->size() elements of p2 are the mates for the elements of p1
        uint iNumPaired = p1->size();
        for (uint k = 0; k < iNumPaired; k++) {
            int iIDF = m_vLocFemalesID[iC][k].first;
            int iIDM = m_vLocMalesID[iC][k].first;
            
            this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
            this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
            
        } 
        
    }        

    return 0;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool HybSelPair<T>::isEqual(Action<T> *pAction, bool bStrict) {
    return true;
}

