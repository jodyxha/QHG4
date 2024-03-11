#include <omp.h>
#include <cmath>
#include <algorithm>

#include "MessLoggerT.h"

#include "types.h"
#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GrassManager.h"

template<typename T>
const std::string GrassManager<T>::asNames[] = {
    ATTR_GRASSMAN_MIN_MASS_NAME,
    ATTR_GRASSMAN_MAX_MASS_NAME,
    ATTR_GRASSMAN_GROWTH_RATE_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
GrassManager<T>::GrassManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID,
                              const std::string sNameGrassMassAvail, const std::string sNameGrassMassConsumed) :
    Action<T>(pPop,pCG,ATTR_GRASSMAN_NAME,sID),
    m_pAS(ArrayShare::getInstance()),
    m_adGrassMassAvail(NULL),
    m_adGrassMassConsumed(NULL),
    m_iNumCells(pCG->m_iNumCells),
    m_sNameGrassMassAvail(sNameGrassMassAvail),
    m_sNameGrassMassConsumed(sNameGrassMassConsumed) {
 
    // we need locks in setAvailableMass
    m_aGLocks = new omp_lock_t[m_iNumCells];
 
    for (int i = 0; i < m_iNumCells; i++) {
        omp_init_lock(&m_aGLocks[i]);
    }

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}



//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
GrassManager<T>::~GrassManager() {
    
    if (m_aGLocks != NULL) {
        delete[] m_aGLocks;
    }
                                     
}

//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int GrassManager<T>::preLoop() {
    int iResult = 0;

    // get shared array for available grass mass (from GrassPop)
    if (iResult == 0) {
        m_adGrassMassAvail = (double *) m_pAS->getArray(m_sNameGrassMassAvail);
        if (m_adGrassMassAvail == NULL) {
            iResult = -1;
        }
    }

    // get shared array for consumed grass mass (from GrassPop)
    if (iResult == 0) {
        m_adGrassMassConsumed = (double *) m_pAS->getArray(m_sNameGrassMassConsumed);
        if (m_adGrassMassConsumed == NULL) {
            iResult = -1;
        }
    }

    // fill grass_mass_avail
    setAvailableMass();

    return iResult;
}


//-----------------------------------------------------------------------------
// execute
//   Here we let the agnt increase its mass logistically
//
template<typename T>
int GrassManager<T>::execute(int iAgentIndex, float fT) {
    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        double dM = m_dGrowthRate*pa->m_dMass*(1 - pa->m_dMass/m_dMaxMass);
        pa->m_dMass += dM;
        if (pa->m_dMass < m_dMinMass) {
            pa->m_dMass = m_dMinMass;
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// finalize
//  bookkeeping of consumed mass
//
template<typename T>
int GrassManager<T>::finalize(float fT) {
    int iResult = 0;
    
    subtractMassConsumed();

    setAvailableMass();

    return iResult;
}

//-----------------------------------------------------------------------------
// setAvailableMass
//   writes the available mass to the shared array m_adGrassMassAvail
//
template<typename T>
int GrassManager<T>::setAvailableMass() {
    printf("[GrassManager<T>::setAvailableMass()] start\n");
    memset(m_adGrassMassAvail, 0, m_iNumCells*sizeof(double));
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        T* pA = &(this->m_pPop->m_aAgents[iA]);

        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;
            omp_set_lock(&m_aGLocks[iC]);
            m_adGrassMassAvail[iC] += pA->m_dMass - m_dMinMass;
            omp_unset_lock(&m_aGLocks[iC]);
        }
    }

    
    //for (int i = 0; i < m_iNumCells; i++) {
    //    printf("[GrassManager<T>::setAvailableMass] C:%d, M:%f\n", i, m_adGrassMassAvail[i]);
    //}


    return 0;
}

//-----------------------------------------------------------------------------
// subtractMassConsumed
//   subtract the eaten grass mass from the agent's mass
//
template<typename T>
int GrassManager<T>::subtractMassConsumed() {

    //memset(m_adGrassMassConsumed, 0, m_iNumCells*sizeof(double));
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        T* pA = &(this->m_pPop->m_aAgents[iA]);

        if (pA->m_iLifeState > 0) {
            
            int iC = pA->m_iCellIndex;

            /*
            if (m_adGrassMassConsumed[iC] > 0) {
                printf("[GrassManager<T>::subtractMassConsumed] Cell:%d, M:%f (%f) subtract %f\n", iC, m_adGrassMassAvail[iC], pA->m_dMass, m_adGrassMassConsumed[iC]);
            }
            */
            pA->m_dMass -= m_adGrassMassConsumed[iC];
            if (pA->m_dMass < m_dMinMass) {
                pA->m_dMass = m_dMinMass;
            }
                    
        }
    }
    //setAvailableMass();
    /*
    for (int i = 0; i < m_iNumCells; i++) {
        printf("[GrassManager<T>::subtractMassConsumed] C:%d, M:%f\n", i, m_adGrassMassAvail[i]);
    }
    */
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_GRASSMAN_MIN_MASS_NAME
//    ATTR_GRASSMAN_MAX_MASS_NAME
//    ATTR_GRASSMAN_GROWTH_RATE_NAME
//
template<typename T>
int GrassManager<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GRASSMAN_MIN_MASS_NAME, 1, &m_dMinMass);
        if (iResult != 0) {
            LOG_ERROR("[GrassManager] couldn't read attribute [%s]", ATTR_GRASSMAN_MIN_MASS_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GRASSMAN_MAX_MASS_NAME, 1, &m_dMaxMass);
        if (iResult != 0) {
            LOG_ERROR("[GrassManager] couldn't read attribute [%s]", ATTR_GRASSMAN_MAX_MASS_NAME);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GRASSMAN_GROWTH_RATE_NAME, 1, &m_dGrowthRate);
        if (iResult != 0) {
            LOG_ERROR("[GrassManager] couldn't read attribute [%s]", ATTR_GRASSMAN_GROWTH_RATE_NAME);
        }
    }
    
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_MIN_MASS_NAME  
//    ATTR_MAX_MASS_NAME  
//    ATTR_GROTH_RATE_NAME
//
template<typename T>
int GrassManager<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_GRASSMAN_MIN_MASS_NAME,    1, &m_dMinMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_GRASSMAN_MAX_MASS_NAME,    1, &m_dMaxMass);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_GRASSMAN_GROWTH_RATE_NAME, 1, &m_dGrowthRate);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int GrassManager<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_GRASSMAN_MIN_MASS_NAME) {
        m_dMinMass = dValue;
    } else if (sAttrName == ATTR_GRASSMAN_MAX_MASS_NAME) {
        m_dMaxMass = dValue;

    } else if (sAttrName == ATTR_GRASSMAN_GROWTH_RATE_NAME) {
        m_dGrowthRate = dValue;
    } else {
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int GrassManager<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_GRASSMAN_MIN_MASS_NAME,    &m_dMinMass);         
        iResult += getAttributeVal(mParams, ATTR_GRASSMAN_MAX_MASS_NAME,    &m_dMaxMass);
        iResult += getAttributeVal(mParams, ATTR_GRASSMAN_GROWTH_RATE_NAME, &m_dGrowthRate);    
    }
    return iResult;
}




//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool GrassManager<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    GrassManager<T>* pA = static_cast<GrassManager<T>*>(pAction);
    if ((m_dMinMass    == pA->m_dMinMass) &&
        (m_dMaxMass    == pA->m_dMaxMass) &&
        (m_dGrowthRate == pA->m_dGrowthRate)) {
        bEqual = true;
    } 
    return bEqual;
}


