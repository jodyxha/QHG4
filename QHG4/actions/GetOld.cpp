#include <omp.h>

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GetOld.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
GetOld<T>::GetOld(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID) 
    : Action<T>(pPop,pCG,ATTR_GETOLD_NAME,sID) {
    
    this->m_vNames.clear();
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
GetOld<T>::~GetOld() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int GetOld<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

	if (pa->m_iLifeState > 0) {
	    pa->m_fAge = fT - pa->m_fBirthTime;
	}

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool GetOld<T>::isEqual(Action<T> *pAction, bool bStrict) {
    GetOld<T> *pGO = dynamic_cast<GetOld<T>*>(pAction);
    return (pGO != NULL);
}
