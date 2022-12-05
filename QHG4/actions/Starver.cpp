#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "Starver.h"

template<typename T>
const char *Starver<T>::asNames[] = {
    ATTR_STARVER_STARVE_MASS_NAME,
    ATTR_STARVER_MASS_DECAY_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Starver<T>::Starver(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID) 
    : Action<T>(pPop,pCG,ATTR_STARVER_NAME,sID),
    //m_apWELL(apWELL),
      m_dStarveMass(0),
      m_dMassDecay(0) {
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Starver<T>::~Starver() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int Starver<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);
    //stdprintf("Starver<T>::execute for agent %d m%f (starvemass %f)\n", iAgentIndex, pa->m_dMass, m_dStarveMass);
    if (pa->m_iLifeState > 0) { 
        pa->m_dMass *= (1-m_dMassDecay);
        //stdprintf("Starver<T>::execute agent %d starved to %f (starvemass %f)\n", iAgentIndex, pa->m_dMass, m_dStarveMass);

        if (pa->m_dMass < m_dStarveMass) {

            //printf("[Starver::execute] registering dead rabbit (%d,%d) (%f < %f)\n",pa->m_iCellIndex, iAgentIndex, pa->m_dMass, m_dStarveMass);
            this->m_pPop->registerDeath(pa->m_iCellIndex, iAgentIndex);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_STARVE_MASS_NAME
//    ATTR_MASS_DECAY_NAME
//
template<typename T>
int Starver<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_STARVER_STARVE_MASS_NAME, 1, &m_dStarveMass);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_STARVER_STARVE_MASS_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_STARVER_MASS_DECAY_NAME, 1, &m_dMassDecay);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_STARVER_MASS_DECAY_NAME);
        }
    }

    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_STARVE_MASS_NAME
//    ATTR_MASS_DECAY_NAME
//
template<typename T>
int Starver<T>:: writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_STARVER_STARVE_MASS_NAME, 1, &m_dStarveMass);
    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_STARVER_MASS_DECAY_NAME, 1, &m_dMassDecay);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int Starver<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams,  ATTR_STARVER_STARVE_MASS_NAME, &m_dStarveMass);           
        iResult += getAttributeVal(mParams,  ATTR_STARVER_MASS_DECAY_NAME, &m_dMassDecay); 
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool Starver<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Starver<T>* pA = static_cast<Starver<T>*>(pAction);
    if ((m_dStarveMass  == pA->m_dStarveMass) && 
        (m_dMassDecay   == pA->m_dMassDecay)) {
        bEqual = true;
    } 
    return bEqual;
}
