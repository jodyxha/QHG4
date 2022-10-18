#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PersFertility.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PersFertility<T>::PersFertility(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID) 
    : Action<T>(pPop,pCG,ATTR_PERSFERTILITY_NAME,sID) {

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PersFertility<T>::~PersFertility() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//  must be called after GetOld
//  we assume the agents have the fields
//     float  m_fFertilityMinAge
//     float  m_fFertilityMaxAge
//     float  m_fInterbirth
//
template<typename T>
int PersFertility<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        if (pa->m_iGender == 0) {
            if ((pa->m_fAge > pa->m_fFertilityMinAge) && 
                (pa->m_fAge < pa->m_fFertilityMaxAge) && 
                ((fT - pa->m_fLastBirth) > pa->m_fInterbirth)) {
                pa->m_iLifeState = LIFE_STATE_FERTILE;
            } else {
                pa->m_iLifeState = LIFE_STATE_ALIVE;
            }
        } else {            
            if (pa->m_fAge > pa->m_fFertilityMinAge) {
                pa->m_iLifeState = LIFE_STATE_FERTILE;
            } else {
                pa->m_iLifeState = LIFE_STATE_ALIVE;
            }
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool PersFertility<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual =  true;
    
    return bEqual;
}


