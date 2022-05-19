#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "QDFUtils.h"
#include "RandomMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RandomMove<T>::RandomMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_RANDOMMOVE_NAME,sID),
      m_apWELL(apWELL),
      m_dMoveProb(0) {
    
    this->m_vNames.push_back(ATTR_RANDOMMOVE_PROB_NAME);
    m_apDirCounts = new int *[omp_get_max_threads()];
    for (int i= 0; i < omp_get_max_threads(); i++) {
        m_apDirCounts[i] = new int[1+this->m_pCG->m_iConnectivity];
        memset(m_apDirCounts[i], 0, (1+this->m_pCG->m_iConnectivity)*sizeof(int));
    }
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
RandomMove<T>::~RandomMove() {
    
    // nothing to do here  
    for (int k= 0; k < 1+this->m_pCG->m_iConnectivity; k++) {
        for (int i= 1; i < omp_get_max_threads(); i++) {
            m_apDirCounts[0][k] += m_apDirCounts[i][k];
        }
    }
    printf("RMC:");
    for (int k= 0; k < 1+this->m_pCG->m_iConnectivity; k++) {
        printf(" %5d", m_apDirCounts[0][k]);
    }
    
    for (int i= 0; i < omp_get_max_threads(); i++) {
        delete[] m_apDirCounts[i];
    }
    delete[]m_apDirCounts;

}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int RandomMove<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iCellIndex = pa->m_iCellIndex;

    int iThread = omp_get_thread_num();

    // random number to compare with move probability
    double dR = this->m_apWELL[iThread]->wrandd();

    // do we move ?
    if (dR < m_dMoveProb) {

        SCell &sc = this->m_pCG->m_aCells[iCellIndex];
        double dR2 =  this->m_apWELL[iThread]->wrandd();
        //        int iNewNeighborIndex = this->m_apWELL[iThread]->wrandi(0, sc.m_iNumNeighbors+1);
        int iNewNeighborIndex = dR2*(sc.m_iNumNeighbors+1);

        m_apDirCounts[omp_get_thread_num()][iNewNeighborIndex]++;

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
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_RANDOMMOVE_PROB_NAME
//  and creates a PolyLine
// 
template<typename T>
int RandomMove<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_RANDOMMOVE_PROB_NAME, 1, &m_dMoveProb);
        if (iResult != 0) {
            LOG_ERROR("[RandomMove] couldn't read attribute [%s]", ATTR_RANDOMMOVE_PROB_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int RandomMove<T>::writeParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_RANDOMMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// modifyParams
//
//
template<typename T>
int RandomMove<T>::modifyParams(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_RANDOMMOVE_PROB_NAME) {
        m_dMoveProb = dValue;
    } else {
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetParams
//   
//
template<typename T>
int RandomMove<T>::tryGetParams(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getParams();
    if (this->checkParams(mParams) == 0) {  
        iResult = getParamVal(mParams,  ATTR_RANDOMMOVE_PROB_NAME, &m_dMoveProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool RandomMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    RandomMove<T>* pA = static_cast<RandomMove<T>*>(pAction);
    if (m_dMoveProb  == pA->m_dMoveProb) {
        bEqual = true;
    } 
    return bEqual;
}


    
