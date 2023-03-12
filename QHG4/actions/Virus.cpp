#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "Virus.h"


template<typename T>
const char *Virus<T>::asNames[] = {
    ATTR_VIRUS_INFECTION_PROB_NAME,
    ATTR_VIRUS_INITIAL_LOAD_NAME,
    ATTR_VIRUS_GROWTH_RATE_NAME,
    ATTR_VIRUS_CONTAGION_LEVEL_NAME,
    ATTR_VIRUS_LETHALITY_LEVEL_NAME,
};


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Virus<T>::Virus(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_VIRUS_NAME,sID),
    m_apWELL(apWELL),
    m_fInfectionProb(0),
    m_fInitialLoad(0),
    m_fGrowthRate(0),
    m_fContagionLevel(0), 
    m_fLethalityLevel(0),
    m_iNumThreads(omp_get_max_threads()) {
        
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

    // create array with one int map for each thread
    m_amCellAgents = new cellagentmap[m_iNumThreads];

    // create array with one map for each thread
    m_amAgentInfects = new  agentloadmap[m_iNumThreads];

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Virus<T>::~Virus() {
    // selete the arrays
    delete[] m_amCellAgents;
    delete[] m_amAgentInfects;
}


//-----------------------------------------------------------------------------
// initialize
//   here the agents-per-cell vectors ar set couples are created
//
template<typename T>
int Virus<T>::initialize(float fT) {
    int iResult = 0;

    // clear the agent arrays and the infection registerr
    for (int iT = 0; iT < m_iNumThreads; iT++) {
        m_amCellAgents[iT].clear();
        m_amAgentInfects[iT].clear();
    }

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        int iT = omp_get_thread_num();
        T* pA = &(this->m_pPop->m_aAgents[iA]);
        m_amCellAgents[iT][pA->m_iCellIndex].push_back(iA);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// execute
//  - let the the agent's virus load grow logisically
//  - if the agent's virus load exceeds the lethality level its register it for death
//  - if the agent's viorus load exceeds the contagion level, it infects all agents in the cell (infection: increase the ``m_fIncoming`` value of the agent)
//
template<typename T>
int Virus<T>::execute(int iAgentIndex, float fT) {
    int iResult = 0;

    int iT = omp_get_thread_num();
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);
    // we only handle agents not already marked as dead
    if (pa->m_iLifeState > 0) {
        //printf("[ Virus<T>::execute] T:%f, agent %d: vload %f, immu %f\n", fT, iAgentIndex, pa->m_fViralLoad, pa->m_fImmunity);
        // perform the logistic growth of the agent's viral load
        float fM = pa->m_fViralLoad;
        pa->m_fViralLoad += m_fGrowthRate*fM*(1-fM)* (1-pa->m_fImmunity); 
        if (fM > 0) {
            //printf("[] T%f: before M=%f, after %f, G %f\n", fT, fM, pa->m_fViralLoad, m_fGrowthRate);
        }
        int iCellIndex = pa->m_iCellIndex;

        // check if the agent must die
        if (pa->m_fViralLoad > m_fLethalityLevel) {
            printf("VirVir[Virus::execute] T%f: have a deady: %d (%f)\n", fT, iAgentIndex, pa->m_fViralLoad);
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);

            // is the agent contagious?
        } else if  (pa->m_fViralLoad > m_fContagionLevel) {
            //printf("VirVir[Virus::execute] T%f: agent %d is contagipus (%f). Checking %zd neighbors\n", fT, iAgentIndex, pa->m_fViralLoad, m_amCellAgents[iT][iCellIndex].size());
            // loop through agents in cell
            for (uint i = 0; i < m_amCellAgents[iT][iCellIndex].size(); i++) {
                int iOtherAgent = m_amCellAgents[iT][iCellIndex][i];
                // we only infect other agents
                if (iOtherAgent != iAgentIndex) {
                    T *pao = &(this->m_pPop->m_aAgents[iOtherAgent]);
                    // infect!
                    double dR =  this->m_apWELL[omp_get_thread_num()]->wrandd();
                    // we reduce the effective infection probability by multiplyint it with (1-immunity)
                    if (dR < m_fInfectionProb * (1-pao->m_fImmunity)) {
                        // register infection load for agent
                        if (pao->m_fViralLoad < m_fInitialLoad) {
                            printf("VirVir[Virus::execute] T%f: have an infection: %d (%ld) -> %d (%ld) (%f)\n", fT, iAgentIndex, pa->m_ulID, iOtherAgent, pao->m_ulID,pao->m_fViralLoad);
                            m_amAgentInfects[iT][iOtherAgent] /*+*/= m_fInitialLoad;
                        }
                    }
                }
            }
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//   here the infects are added to thagernts virus loads
//
template<typename T>
int Virus<T>::finalize(float fT) {
    int iResult = 0;

    for (int iT = 0; iT < m_iNumThreads; iT++) {
        for (agentloadmap::const_iterator it = m_amAgentInfects[iT].begin(); it !=  m_amAgentInfects[iT].end(); ++it) {
            // first:  agent index
            // second: infecttload
            
            T *pa = &(this->m_pPop->m_aAgents[it->first]);
            pa->m_fViralLoad += it->second;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_VIRUS_INFECTION_PROB_NAME
//    ATTR_VIRUS_INITIAL_LOAD_NAME
//    ATTR_VIRUS_GROWTH_RATE_NAME
//    ATTR_VIRUS_CONTAGION_LEVEL_NAME
//    ATTR_VIRUS_LETHALITY_LEVEL_NAME
//
template<typename T>
int Virus<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VIRUS_INFECTION_PROB_NAME, 1, &m_fInfectionProb);
        if (iResult != 0) {
            LOG_ERROR("[Virus] couldn't read attribute [%s]", ATTR_VIRUS_INFECTION_PROB_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VIRUS_INITIAL_LOAD_NAME, 1, &m_fInitialLoad);
        if (iResult != 0) {
            LOG_ERROR("[Virus] couldn't read attribute [%s]", ATTR_VIRUS_INITIAL_LOAD_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VIRUS_GROWTH_RATE_NAME, 1, &m_fGrowthRate);
        if (iResult != 0) {
            LOG_ERROR("[Virus] couldn't read attribute [%s]", ATTR_VIRUS_GROWTH_RATE_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VIRUS_CONTAGION_LEVEL_NAME, 1, &m_fContagionLevel);
        if (iResult != 0) {
            LOG_ERROR("[Virus] couldn't read attribute [%s]", ATTR_VIRUS_CONTAGION_LEVEL_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_VIRUS_LETHALITY_LEVEL_NAME, 1, &m_fLethalityLevel);
        if (iResult != 0) {
            LOG_ERROR("[Virus] couldn't read attribute [%s]", ATTR_VIRUS_LETHALITY_LEVEL_NAME);
        }
    }
    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_VIRUS_INFECTION_PROB_NAME
//    ATTR_VIRUS_INITIAL_LOAD_NAME
//    ATTR_VIRUS_GROWTH_RATE_NAME
//    ATTR_VIRUS_CONTAGION_LEVEL_NAME
//    ATTR_VIRUS_LETHALITY_LEVEL_NAME
//
template<typename T>
int Virus<T>:: writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VIRUS_INFECTION_PROB_NAME, 1, &m_fInfectionProb);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VIRUS_INITIAL_LOAD_NAME,  1, &m_fInitialLoad);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VIRUS_GROWTH_RATE_NAME,  1, &m_fGrowthRate);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VIRUS_CONTAGION_LEVEL_NAME,  1, &m_fContagionLevel);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_VIRUS_LETHALITY_LEVEL_NAME,  1, &m_fLethalityLevel);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int Virus<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams,  ATTR_VIRUS_INFECTION_PROB_NAME, &m_fInfectionProb);           
        iResult += getAttributeVal(mParams,  ATTR_VIRUS_INITIAL_LOAD_NAME,  &m_fInitialLoad); 
        iResult += getAttributeVal(mParams,  ATTR_VIRUS_GROWTH_RATE_NAME,  &m_fGrowthRate); 
        iResult += getAttributeVal(mParams,  ATTR_VIRUS_CONTAGION_LEVEL_NAME,  &m_fContagionLevel); 
        iResult += getAttributeVal(mParams,  ATTR_VIRUS_LETHALITY_LEVEL_NAME,  &m_fLethalityLevel); 
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool Virus<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Virus<T>* pA = static_cast<Virus<T>*>(pAction);
    if ((m_fInfectionProb  == pA->m_fInfectionProb) &&
        (m_fInitialLoad    == pA->m_fInitialLoad) &&
        (m_fGrowthRate     == pA->m_fGrowthRate) &&
        (m_fContagionLevel == pA->m_fContagionLevel) &&
        (m_fLethalityLevel == pA->m_fLethalityLevel)) {
        bEqual = true;
    } 
    return bEqual;
}

