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
#include "SheepManager.h" 

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
SheepManager<T>::SheepManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID,
                                const std::string sNameGrassMassAvail, const std::string sNameGrassMassConsumed, 
                                double *pdPreferences)
    : Action<T>(pPop,pCG,ATTR_SHEEPMAN_NAME,sID),
      m_pAS(ArrayShare::getInstance()),
      m_iNumCells(pCG->m_iNumCells),
      m_adGrassMassAvail(NULL),
      m_adGrassMassConsumed(NULL),
      m_avSheepLocIDs(NULL),
      m_pdPreferences(pdPreferences),

    m_sNameGrassMassAvail(sNameGrassMassAvail),
    m_sNameGrassMassConsumed(sNameGrassMassConsumed)/*,
                                                      m_sNameSheepLocIDs(sNameSheepLocIDs)*/ {

    m_aRLocks = new omp_lock_t[m_iNumCells];
    for (int i = 0; i < m_iNumCells; i++) {
        omp_init_lock(&m_aRLocks[i]);
    }
    
    m_avSheepLocIDs = new cellnmvec[pCG->m_iNumCells];

}



//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
SheepManager<T>::~SheepManager() {
    
    // the other arrays will be deleted by the owners
    delete[] m_avSheepLocIDs;
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int SheepManager<T>::preLoop() {
    int iResult = 0;

    // get shared array: grass available mass (created by GrassPop)
    if (iResult == 0) {
       m_adGrassMassAvail = (double *) m_pAS->getArray(m_sNameGrassMassAvail);
        if (m_adGrassMassAvail == NULL) {
            iResult = -1;
            printf("[SheepManager<T>::preLoop] Error: Couldni't get array [%s]\n", m_sNameGrassMassAvail.c_str());
        } else {
            printf("[SheepManager<T>::preLoop] array [%s] is %p\n", m_sNameGrassMassAvail.c_str(), m_adGrassMassAvail);
        }
    }
    
    // get shared array: consumed grass mass (created by GrassPop)
    if (iResult == 0) {
        m_adGrassMassConsumed = (double *) m_pAS->getArray(m_sNameGrassMassConsumed);
        if (m_adGrassMassConsumed == NULL) {
            iResult = -1;
            printf("[SheepManager<T>::preLoop] Error: Couldn't get array [%s]\n", m_sNameGrassMassConsumed.c_str());
        } else {
            printf("[SheepManager<T>::preLoop] array [%s] is %p\n", m_sNameGrassMassConsumed.c_str(), m_adGrassMassConsumed);
        }
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// initialize
//  determine grass_mass_consumed and increase masses
//
template<typename T>
int SheepManager<T>::initialize(float fT) {
    int iResult = 0;


    iResult += feedSheep();

    iResult += makeCellVecsSheep();

    return iResult;
}

//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int SheepManager<T>::finalize(float fT) {
    int iResult = 0;

    // delete llocated array
    return iResult;
}


//-----------------------------------------------------------------------------
// feedSheep
//   feeds the rabbits and writes the consumed mass to the shared array m_adGrassMassConsumed
//
template<typename T>
int SheepManager<T>::feedSheep() {

    int iResult = 0;
    //printf("[feedSheep}using grassmass avail %p\n",   m_adGrassMassAvail);
         
#pragma omp parallel for 
    for (int iC = 0; iC < m_iNumCells; iC++) {
        m_adGrassMassConsumed[iC] = 0;
        if (m_avSheepLocIDs[iC].size() > 0) {
            // greedy: we eat all there is (dividing it equally between all present agents)
            double dLocGrassMass = m_adGrassMassAvail[iC]/m_avSheepLocIDs[iC].size();
            
            for (uint j = 0; j < m_avSheepLocIDs[iC].size(); j++) {
                T* pA = &(this->m_pPop->m_aAgents[m_avSheepLocIDs[iC][j].first]);
                pA->m_dMass += dLocGrassMass; 
                m_adGrassMassConsumed[iC] += dLocGrassMass;
            }
        }
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// makeCellVecsSheep
//   fills the feeds the rabbits and writes the consumed mass to the shared array m_adGrassMassConsumed
//
template<typename T>
int SheepManager<T>::makeCellVecsSheep() {

    int iResult = 0;
    // clear local rabbit ID/mass pairss
#pragma omp parallel for 
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_avSheepLocIDs[iC].clear();
    }

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

    // fill rabbit ID/mass pairs , v[i} = vector(IDs, mass) of  Sheep in cell i 
#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        T* pA = &(this->m_pPop->m_aAgents[iA]);

        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;
            omp_set_lock(&m_aRLocks[iC]);
                    
            m_avSheepLocIDs[iC].push_back({iA,pA->m_dMass});
                    
            omp_unset_lock(&m_aRLocks[iC]);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool SheepManager<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    //SheepManager<T>* pA = static_cast<SheepManager<T>*>(pAction);
    if ((true == true) &&
        (true == true)) {
        bEqual = true;
    } 
    return bEqual;
}
