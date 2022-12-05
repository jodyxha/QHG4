#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "AnimalReproducer.h"

template<typename T>
const char *AnimalReproducer<T>::asNames[] = {
    ATTR_ANIMALREP_MASS_FERT_NAME,
    ATTR_ANIMALREP_MASS_BABY_NAME,
    ATTR_ANIMALREP_BIRTH_PROB_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
AnimalReproducer<T>::AnimalReproducer(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_ANIMALREP_NAME,sID),
      m_apWELL(apWELL),
      m_dMassFert(0),
      m_dMassBaby(0),
      m_dBirthProb(0) {
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
AnimalReproducer<T>::~AnimalReproducer() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int AnimalReproducer<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {

        int iCellIndex = pa->m_iCellIndex;
        int iThread = omp_get_thread_num();

        if (pa->m_dMass > m_dMassFert) {
            //printf("Agent %d: is fertile (m=%f)\n", iAgentIndex, pa->m_dMass);
            double dR =  m_apWELL[iThread]->wrandd();
            //printf("Agent %d:R is %f, bprob %f\n", iAgentIndex, dR, m_dBirthProb);
  
            if (dR < m_dBirthProb) {
                pa->m_dBabyMass = m_dMassBaby;
                this->m_pPop->registerBirth(iCellIndex, iAgentIndex, iAgentIndex);
            }
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_ANIMALREP_MASS_FERT_NAME
//    ATTR_ANIMALREP_MASS_DECAY_NAME
//    ATTR_ANIMALREP_BIRTH_PROB_NAME
//
template<typename T>
int AnimalReproducer<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ANIMALREP_MASS_FERT_NAME, 1, &m_dMassFert);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_ANIMALREP_MASS_FERT_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ANIMALREP_MASS_BABY_NAME, 1, &m_dMassBaby);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_ANIMALREP_MASS_BABY_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ANIMALREP_BIRTH_PROB_NAME, 1, &m_dBirthProb);
        if (iResult != 0) {
            LOG_ERROR("[OldAgeDeath] couldn't read attribute [%s]", ATTR_ANIMALREP_BIRTH_PROB_NAME);
        }
    }

    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_ANIMALREP_MASS_FERT_NAME
//    ATTR_ANIMALREP_MASS_BABY_NAME
//    ATTR_ANIMALREP_BIRTH_PROB_NAME
//
template<typename T>
int AnimalReproducer<T>:: writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ANIMALREP_MASS_FERT_NAME,  1, &m_dMassFert);
    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ANIMALREP_MASS_BABY_NAME,  1, &m_dMassBaby);
    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ANIMALREP_BIRTH_PROB_NAME, 1, &m_dBirthProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int AnimalReproducer<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams,  ATTR_ANIMALREP_MASS_FERT_NAME,  &m_dMassFert);           
        iResult += getAttributeVal(mParams,  ATTR_ANIMALREP_MASS_BABY_NAME,  &m_dMassBaby); 
        iResult += getAttributeVal(mParams,  ATTR_ANIMALREP_BIRTH_PROB_NAME, &m_dBirthProb); 
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool AnimalReproducer<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    AnimalReproducer<T>* pA = static_cast<AnimalReproducer<T>*>(pAction);
    if ((m_dMassFert  == pA->m_dMassFert) && 
        (m_dMassBaby  == pA->m_dMassBaby) && 
        (m_dBirthProb == pA->m_dBirthProb)) {
        bEqual = true;
    } 
    return bEqual;
}
