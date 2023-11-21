#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PersOldAgeDeath.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PersOldAgeDeath<T>::PersOldAgeDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_PERSOLDAGEDEATH_NAME,sID),
      m_apWELL(apWELL) {
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PersOldAgeDeath<T>::~PersOldAgeDeath() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
// we assume the agents have the fields
//    double  dMaxAge
//    double  dUncertainty
//
template<typename T>
int PersOldAgeDeath<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        pa->m_fAge = fT - pa->m_fBirthTime;
        // death possible starting at 0.8 times m_dMaxAge
        double dR =  this->m_apWELL[omp_get_thread_num()]->wrandr(1 - pa->m_dUncertainty, 1 + pa->m_dUncertainty);
//        printf("OldAgeDeath a%d: age %f, rand %f, max %f\n", iAgentIndex,  pa->m_fAge, dR, m_dMaxAge);
        if (pa->m_fAge*dR > pa->m_dMaxAge) {
            this->m_pPop->registerDeath(pa->m_iCellIndex, iAgentIndex);
        }
    }

    return iResult;
}




//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool PersOldAgeDeath<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    
    return bEqual;
}

