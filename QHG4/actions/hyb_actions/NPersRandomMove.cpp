#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "NPersRandomMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
NPersRandomMove<T>::NPersRandomMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_NPERSRANDOMMOVE_NAME,sID),
      m_apWELL(apWELL) {
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
NPersRandomMove<T>::~NPersRandomMove() {
    
    // nothing to do here  

}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int NPersRandomMove<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iCellIndex = pa->m_iCellIndex;

    int iThread = omp_get_thread_num();

    // random number to compare with move probability
    double dR = this->m_apWELL[iThread]->wrandd();

    // do we move ?
    if (dR <  pa->m_dMoveProb) {

        SCell &sc = this->m_pCG->m_aCells[iCellIndex];
        double dR2 =  this->m_apWELL[iThread]->wrandd();
        //        int iNewNeighborIndex = this->m_apWELL[iThread]->wrandi(0, sc.m_iNumNeighbors+1);
        int iNewNeighborIndex = dR2*(sc.m_iNumNeighbors+1);

        if (iNewNeighborIndex > 0)  {
            int iNewCellIndex = sc.m_aNeighbors[iNewNeighborIndex-1];
        
            
            if (iNewCellIndex >= 0)  {
                this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
            }               
        } 
                  
    }
        
    return iResult;
}



//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool NPersRandomMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    return bEqual;
}


    
