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
#include "RabbitManager.h"

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RabbitManager<T>::RabbitManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID,
                                const std::string sNameRabbitMassAvail, const std::string sNameGrassMassConsumed, 
                                const std::string sNameRabbitLocIDs,    const std::string sNameFoxCount,  
                                const std::string sNameGrassMassAvail,  const std::string sNameRabbitDead, 
                                double *pdPreferences)
    : Action<T>(pPop,pCG,ATTR_RABBITMAN_NAME,sID),
      m_pAS(ArrayShare::getInstance()),
      m_iNumCells(pCG->m_iNumCells),
      m_adRabbitMassAvail(NULL),
      m_adGrassMassConsumed(NULL),
      m_aiFoxCount(NULL),
      m_adGrassMassAvail(NULL),
      m_avRabbitLocIDs(NULL),
      m_avRabbitDead(NULL),
      m_pdPreferences(pdPreferences),

      m_sNameRabbitMassAvail(sNameRabbitMassAvail),
      m_sNameGrassMassConsumed(sNameGrassMassConsumed),
      m_sNameRabbitLocIDs(sNameRabbitLocIDs),
      m_sNameFoxCount(sNameFoxCount),
      m_sNameGrassMassAvail(sNameGrassMassAvail),
      m_sNameRabbitDead(sNameRabbitDead) {

    m_aRLocks = new omp_lock_t[m_iNumCells];
    for (int i = 0; i < m_iNumCells; i++) {
        omp_init_lock(&m_aRLocks[i]);
    }
}



//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
RabbitManager<T>::~RabbitManager() {
    
    // the arrays will be deleted by the owners
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int RabbitManager<T>::preLoop() {
    int iResult = 0;

    // get shared array: grass available mass (created by GrassPop)
    if (iResult == 0) {
       m_adGrassMassAvail = (double *) m_pAS->getArray(m_sNameGrassMassAvail);
        if (m_adGrassMassAvail == NULL) {
            iResult = -1;
        } else {
            printf("array [%s] is %p\n", m_sNameGrassMassAvail.c_str(), m_adGrassMassAvail);
        }
    }
    
    // get shared array: consumed grass mass (created by GrassPop)
    if (iResult == 0) {
        m_adGrassMassConsumed = (double *) m_pAS->getArray(m_sNameGrassMassConsumed);
        if (m_adGrassMassConsumed == NULL) {
            iResult = -1;
        } else {
            printf("array [%s] is %p\n", m_sNameGrassMassConsumed.c_str(), m_adGrassMassConsumed);
        }
    }


    // get shared array: total available rabbit mass per cell (created by RabbitPop)
    if (iResult == 0) {
        m_adRabbitMassAvail = (double *) m_pAS->getArray(m_sNameRabbitMassAvail);
        if (m_adRabbitMassAvail == NULL) {
            iResult = -1;
        } else {
            printf("array [%s] is %p\n", m_sNameRabbitMassAvail.c_str(), m_adRabbitMassAvail);
        }
    }


    // get shared array: vectors of IDs of agents per cell (created by RabbitPop)
    if (iResult == 0) {
         m_avRabbitLocIDs = (cellnmvec *) m_pAS->getArray(m_sNameRabbitLocIDs);
        if (m_avRabbitLocIDs == NULL) {
            iResult = -1;
        } else {
            printf("array [%s] is %p\n", m_sNameRabbitLocIDs.c_str(), m_avRabbitLocIDs);
            
        }
    }

    // get shared array: pairs of cell IDs and IDs of dead (killed) agents (created by RabbitPop)
    if (iResult == 0) {
         m_avRabbitDead = (agcellvec *) m_pAS->getArray(m_sNameRabbitDead);
        if (m_avRabbitDead == NULL) {
            iResult = -1;
        } else {
            printf("array [%s] is %p\n", m_sNameRabbitDead.c_str(), m_avRabbitDead);
        }
    }   
 

    // get shared array: number of foxes per cell (created by RabbitPop)
    if (iResult == 0) {
        m_aiFoxCount = (int *) m_pAS->getArray(m_sNameFoxCount);
        if (m_aiFoxCount == NULL) {
            iResult = -1;
        } else {
            printf("array [%s] is %p\n", m_sNameFoxCount.c_str(), m_aiFoxCount);
        }
    }
    
        
    // fill rabbit_mass_avail
    iResult = setAvailableRabbitMass();

    return iResult;
}

//-----------------------------------------------------------------------------
// initialize
//  determine grass_mass_consumed and increase masses
//
template<typename T>
int RabbitManager<T>::initialize(float fT) {
    int iResult = 0;


    iResult += feedRabbits();

    iResult += makeCellVecsRabbit();

    return iResult;
}

//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int RabbitManager<T>::finalize(float fT) {
    int iResult = 0;

    //  update available rabbitMass
    
    if (iResult == 0) {
        iResult = setAvailableRabbitMass();
    }
    
    // 'officially' register dead rabbits
    for (uint i = 0; i < m_avRabbitDead[0].size(); i++) {
        printf("[RabbitManager::finalize] registering dead rabbit (%d,%d)\n",m_avRabbitDead[0][i].first, m_avRabbitDead[0][i].second);
        this->m_pPop->registerDeath(m_avRabbitDead[0][i].first, m_avRabbitDead[0][i].second);
    } 

    return iResult;
}

//-----------------------------------------------------------------------------
// setAvailableRabbitMass
//   writes the available mass to the shared array m_adRabbitMassAvail
//
template<typename T>
int RabbitManager<T>::setAvailableRabbitMass() {

    int iResult = 0;    
    memset(m_adRabbitMassAvail, 0, m_iNumCells*sizeof(double));
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        T* pA = &(this->m_pPop->m_aAgents[iA]);

        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;
            omp_set_lock(&m_aRLocks[iC]);
                    
            m_adRabbitMassAvail[iC] += pA->m_dMass;
                    
            omp_unset_lock(&m_aRLocks[iC]);
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// feedRabbits
//   feeds the rabbits and writes the consumed mass to the shared array m_adGrassMassConsumed
//
template<typename T>
int RabbitManager<T>::feedRabbits() {

    int iResult = 0;
    //printf("[feedRabbits}using grassmass avail %p\n",   m_adGrassMassAvail);
         
#pragma omp parallel for 
    for (int iC = 0; iC < m_iNumCells; iC++) {
        m_adGrassMassConsumed[iC] = 0;
        if (m_avRabbitLocIDs[iC].size() > 0) {
            // greedy: we eat all there is (dividing it equally between all present agents)
            double dLocGrassMass = m_adGrassMassAvail[iC]/m_avRabbitLocIDs[iC].size();
            //printf("[RabbitManager<T>::feedRabbits] Grass in cell %d: %f; %zd rabbits present - remove %f for each\n", iC,   m_adGrassMassAvail[iC], m_avRabbitLocIDs[iC].size(), dLocGrassMass);
            for (uint j = 0; j < m_avRabbitLocIDs[iC].size(); j++) {
                T* pA = &(this->m_pPop->m_aAgents[m_avRabbitLocIDs[iC][j].first]);
                pA->m_dMass += dLocGrassMass; 
                m_adGrassMassConsumed[iC] += dLocGrassMass;
                //printf("[feedRabbits] agent %d got %f, has %f\n",  m_avRabbitLocIDs[iC][j].first, dLocGrassMass, pA->m_dMass);
                //printf("[feedRabbits] so now mass consumed in %d is %f\n",  iC,  m_adGrassMassConsumed[iC]);
            }
        }
    }
    //for (int iC = 0; iC < m_iNumCells; iC++) {
    //    printf("[RabbitManager<T>::feedRabbits] m_adGrassMassConsumed[%02d] = %f\n", iC,  m_adGrassMassConsumed[iC]);
    //}
    return iResult;
}

//-----------------------------------------------------------------------------
// makeCellVecsRabbit
//   fills the feeds the rabbits and writes the consumed mass to the shared array m_adGrassMassConsumed
//
template<typename T>
int RabbitManager<T>::makeCellVecsRabbit() {

    int iResult = 0;
    // clear local rabbit ID/mass pairss
#pragma omp parallel for 
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_avRabbitLocIDs[iC].clear();
    }

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

    // fill rabbit ID/mass pairs , v[i} = vector(IDs, mass) of  Rabbits in cell i 
#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        T* pA = &(this->m_pPop->m_aAgents[iA]);

        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;
            omp_set_lock(&m_aRLocks[iC]);
                    
            m_avRabbitLocIDs[iC].push_back({iA,pA->m_dMass});
                    
            omp_unset_lock(&m_aRLocks[iC]);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool RabbitManager<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    //RabbitManager<T>* pA = static_cast<RabbitManager<T>*>(pAction);
    if ((true == true) &&
        (true == true)) {
        bEqual = true;
    } 
    return bEqual;
}
