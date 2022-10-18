#include <omp.h>
#include <cmath>
#include <algorithm>

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "RandomPair.h"

#define INIT_SIZE 100

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RandomPair<T>::RandomPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_RANDPAIR_NAME,sID),
    m_apWELL(apWELL),
    m_abMales(NULL),
    m_abFemales(NULL),
    m_aiMalesSize(NULL),
    m_aiFemalesSize(NULL) {
 
    int iNumCells = this->m_pCG->m_iNumCells;

    m_vLocFemalesID = new std::vector<int>[iNumCells];
    m_vLocMalesID = new std::vector<int>[iNumCells];
 
    m_aFLocks = new omp_lock_t[iNumCells];
    m_aMLocks = new omp_lock_t[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
        omp_init_lock(&m_aMLocks[i]);
    }

    int iNumThreads = omp_get_max_threads();
    m_abMales   = new bool*[iNumThreads];
    m_abFemales = new bool*[iNumThreads];
    
    m_aiMalesSize   = new int[iNumThreads];
    m_aiFemalesSize = new int[iNumThreads];

    for (int i = 0; i < iNumThreads; i++) {
        m_aiMalesSize[i]   = INIT_SIZE;
        m_aiFemalesSize[i] = INIT_SIZE;

        m_abMales[i]   = new bool[INIT_SIZE];
        m_abFemales[i] = new bool[INIT_SIZE];
    
    }
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
RandomPair<T>::~RandomPair() {
    
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

    if (m_abMales != NULL) {
        for (int i = 0; i < omp_get_max_threads(); i++) {
            delete[] m_abMales[i];
        }
        delete[] m_abMales;
    }
    if (m_abFemales != NULL) {
        for (int i = 0; i < omp_get_max_threads(); i++) {
            delete[] m_abFemales[i];
        }
        delete[] m_abFemales;
    }

    if (m_aiMalesSize != NULL) {
        delete[] m_aiMalesSize;
    }
    if (m_aiFemalesSize != NULL) {
        delete[] m_aiFemalesSize;
    }
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int RandomPair<T>::initialize(float fT) {
    
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
int RandomPair<T>::finalize(float fT) {

    return 0;

}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T>
int RandomPair<T>::findMates() {

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for

    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {

        T* pA = &(this->m_pPop->m_aAgents[iA]);
        
        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;

            if (pA->m_iLifeState == LIFE_STATE_FERTILE) { 
                if (pA->m_iGender == 0)  { // FEMALE
                    
                    omp_set_lock(&m_aFLocks[iC]);
                    
                    m_vLocFemalesID[iC].push_back(iA);
                    
                    omp_unset_lock(&m_aFLocks[iC]);
                    
                } else if (pA->m_iGender == 1) { // MALE
                    
                    omp_set_lock(&m_aMLocks[iC]);
                    
                    m_vLocMalesID[iC].push_back(iA);
                    
                    omp_unset_lock(&m_aMLocks[iC]);
                    
                }
            }
        }
    }

#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        int iT = omp_get_thread_num();
        int iFNum = (int)m_vLocFemalesID[iC].size();
        int iMNum = (int)m_vLocMalesID[iC].size();


        if ((iFNum > 0) && (iMNum > 0)) {
            
            // sort to ensure reproducibility of simulation with same random seeds
            // this should be faster than using STL sets 
            // because insertions at the end of an STL vector are fast
            std::sort(m_vLocFemalesID[iC].begin(),  m_vLocFemalesID[iC].end());
            std::sort(m_vLocMalesID[iC].begin(), m_vLocMalesID[iC].end()); 
                
            // let's pair them up
            
            if (iFNum <= iMNum) {
                
                // here we'll indicate if a male is "taken"
                if (iMNum > m_aiMalesSize[iT]) {
                    delete[] m_abMales[iT];
                    m_abMales[iT] = new bool[iMNum];
                    m_aiMalesSize[iT] = iMNum;
                }
                memset(m_abMales[iT], 0,  m_aiMalesSize[iT]*sizeof(bool));
                
                int iF = 0;
                
                while ((iF < iFNum) && (iMNum > 0)) {
                    int iChoose = (int)(this->m_apWELL[iT]->wrandr(0,iMNum)); // which unpaired male do we want?
                    int iMSearch = -1;  // count how many unpaired males we encouter in the vector
                    int iM = -1;  // go through local males vector
                    
                    while (iMSearch < iChoose) {
                        iM++;
                        if ( ! m_abMales[iT][iM] ) {
                            iMSearch++;
                        }
                    }
                    
                    m_abMales[iT][iM] = true; // mark male as already paired 
                    
                    int iIDF = m_vLocFemalesID[iC][iF];
                    int iIDM = m_vLocMalesID[iC][iM];
                    //printf("[RandomPair::findMates]T%f: C%d paired F %d to M %d\n", this->m_fCurTime, iC, iIDF, iIDM);
                    
                    this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                    this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
                    
                    iMNum--;
                    iF++;
                }
                
            } else {

                // here we'll indicate if a female is "taken"
                if (iFNum > m_aiFemalesSize[iT]) {
                    delete[] m_abFemales[iT];
                    m_abFemales[iT] = new bool[iFNum];
                    m_aiFemalesSize[iT] = iFNum;
                }
                memset(m_abFemales[iT], 0,  m_aiFemalesSize[iT]*sizeof(bool));
                
                int iM = 0;
                
                while ((iM < iMNum) && (iFNum > 0)) {
                    int iChoose = (int)(this->m_apWELL[omp_get_thread_num()]->wrandr(0,iFNum)); // which unpaired female do we want?
                    int iFSearch = -1;  // count how many unpaired females we encouter in the vector
                    int iF = -1;  // go through local females vector
                    
                    while (iFSearch < iChoose) {
                        iF++;
                        if ( ! m_abFemales[iT][iF] ) {
                            iFSearch++;
                        }
                    }
                    
                    m_abFemales[iT][iF] = true; // mark female as already paired 
                    
                    int iIDF = m_vLocFemalesID[iC][iF];
                    int iIDM = m_vLocMalesID[iC][iM];
                    //printf("[RandomPair::findMates]T%f: C%d paired M %d to F %d\n", this->m_fCurTime, iC, iIDM, iIDF);
                    
                    this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                    this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
                    
                    iFNum--;
                    iM++;
                }
                
            } 
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool RandomPair<T>::isEqual(Action<T> *pAction, bool bStrict) {
    return true;
}


