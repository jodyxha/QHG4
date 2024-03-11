#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "RandomMove1D.h"

template<typename T>
const std::string RandomMove1D<T>::asNames[] = {
    ATTR_RANDOMMOVE1D_PROB_NAME,};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
RandomMove1D<T>::RandomMove1D(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_RANDOMMOVE1D_NAME,sID),
      m_apWELL(apWELL),
      m_dMoveProb(0),
      m_bAbsorbing(false) {
    
    this->m_vNames.push_back(ATTR_RANDOMMOVE1D_PROB_NAME);
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
RandomMove1D<T>::~RandomMove1D() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int RandomMove1D<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iCellIndex = pa->m_iCellIndex;

    int iThread = omp_get_thread_num();

    // random number to compare with move probability
    double dR = this->m_apWELL[iThread]->wrandd();

    // do we move ?
    if (dR < m_dMoveProb) {

        SCell &sc = this->m_pCG->m_aCells[iCellIndex];

        double dR1 = this->m_apWELL[iThread]->wrandd();

        if (dR1 < 0.5) {
            int iNewIndex = sc.m_aNeighbors[0];
            if (iNewIndex >= 0) {
                this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewIndex);
            }
        } else {
            int iNewIndex = sc.m_aNeighbors[2];
            if (iNewIndex >= 0) {
                this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewIndex);
            }
        }
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_RANDOMMOVE1D_PROB_NAME
// and creates a PolyLine
//
template<typename T>
int RandomMove1D<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {    
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_RANDOMMOVE1D_PROB_NAME, 1, &m_dMoveProb);
        if (iResult != 0) {
            LOG_ERROR("[RandomMove1D] couldn't read attribute [%s]", ATTR_RANDOMMOVE1D_PROB_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_RANDOMMOVE1D_PROB_NAME
//
template<typename T>
int RandomMove1D<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_RANDOMMOVE1D_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int RandomMove1D<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = getAttributeVal(mParams,  ATTR_RANDOMMOVE1D_PROB_NAME, &m_dMoveProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool RandomMove1D<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    RandomMove1D<T>* pA = static_cast<RandomMove1D<T>*>(pAction);
    if (m_dMoveProb == pA->m_dMoveProb) {
        bEqual = true;
    } 
    return bEqual;
}


