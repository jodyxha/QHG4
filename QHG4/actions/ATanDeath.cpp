#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "ATanDeath.h"

template<typename T>
const std::string ATanDeath<T>::asNames[] = {
    ATTR_ATANDEATH_MAXAGE_NAME,
    ATTR_ATANDEATH_RANGE_NAME,
    ATTR_ATANDEATH_SLOPE_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
ATanDeath<T>::ATanDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_ATANDEATH_NAME,sID),
      m_apWELL(apWELL),
      m_dMaxAge(0),
    m_dRange(0),
    m_dSlope(0) {
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
ATanDeath<T>::~ATanDeath() {
    
    // nothing to do here
}

//-----------------------------------------------------------------------------
// action: preLoop
//
template<typename T>
int ATanDeath<T>::preLoop() {
    
    m_dScale = (Q_PI/2-EPS)/atan(m_dSlope*m_dRange);

    stdprintf("[ATanDeath<T>::preLoop()] MaxAge: %f\n", m_dMaxAge);
    stdprintf("[ATanDeath<T>::preLoop()] Range:  %f\n", m_dRange);
    stdprintf("[ATanDeath<T>::preLoop()] Slope:  %f\n", m_dSlope);
    stdprintf("[ATanDeath<T>::preLoop()] Scale:  %f\n", m_dScale);
    
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int ATanDeath<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        pa->m_fAge = fT - pa->m_fBirthTime;

        double dDeathProb = 0.5+m_dScale*atan(m_dSlope*(pa->m_fAge - m_dMaxAge))/Q_PI;


        // death possible starting at 0.8 times m_dMaxAge
        double dR =  this->m_apWELL[omp_get_thread_num()]->wrandd();

        if (dR < dDeathProb) {
            // printf("[ATanDeath::execute] have a deady (%d)\n", iAgentIndex);
            this->m_pPop->registerDeath(pa->m_iCellIndex, iAgentIndex);
        }
        // variation:
        // kill_age = maxAge + wrandr(-m_dUncertainty, m_dUncertainty),
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_ATANDEATH_MAXAGE_NAME
//    ATTR_ATANDEATH_RANGE_NAME
//    ATTR_ATANDEATH_SLOPE_NAME
//
template<typename T>
int ATanDeath<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ATANDEATH_MAXAGE_NAME, 1, &m_dMaxAge);
        if (iResult != 0) {
            LOG_ERROR("[ATanDeath] couldn't read attribute [%s]", ATTR_ATANDEATH_MAXAGE_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ATANDEATH_RANGE_NAME, 1, &m_dRange);
        if (iResult != 0) {
            LOG_ERROR("[ATanDeath] couldn't read attribute [%s]", ATTR_ATANDEATH_RANGE_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ATANDEATH_SLOPE_NAME, 1, &m_dSlope);
        if (iResult != 0) {
            LOG_ERROR("[ATanDeath] couldn't read attribute [%s]", ATTR_ATANDEATH_SLOPE_NAME);
        }
    }
    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_ATANDEATH_MAXAGE_NAME
//    ATTR_ATANDEATH_RANGE_NAME
//    ATTR_ATANDEATH_SLOPE_NAME
//
template<typename T>
int ATanDeath<T>:: writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ATANDEATH_MAXAGE_NAME, 1, &m_dMaxAge);
    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ATANDEATH_RANGE_NAME,  1, &m_dRange);
    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ATANDEATH_SLOPE_NAME,  1, &m_dSlope);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int ATanDeath<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams,  ATTR_ATANDEATH_MAXAGE_NAME, &m_dMaxAge);           
        iResult += getAttributeVal(mParams,  ATTR_ATANDEATH_RANGE_NAME,  &m_dRange); 
        iResult += getAttributeVal(mParams,  ATTR_ATANDEATH_SLOPE_NAME,  &m_dSlope); 
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool ATanDeath<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    ATanDeath<T>* pA = static_cast<ATanDeath<T>*>(pAction);
    if ((m_dMaxAge  == pA->m_dMaxAge) && 
        (m_dRange   == pA->m_dRange) && 
        (m_dSlope   == pA->m_dSlope)) {
        bEqual = true;
    } 
    return bEqual;
}

