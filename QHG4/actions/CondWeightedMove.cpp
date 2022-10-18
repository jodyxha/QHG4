#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "CondWeightedMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
CondWeightedMove<T>::CondWeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights, MoveCondition *pMC) 
    : Action<T>(pPop,pCG,ATTR_CONDWEIGHTEDMOVE_NAME,sID),
      m_apWELL(apWELL),
      m_adEnvWeights(adEnvWeights),
      m_dMoveProb(0),
      m_pMC(pMC) {

    this->m_vNames.push_back(ATTR_CONDWEIGHTEDMOVE_PROB_NAME);
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
CondWeightedMove<T>::~CondWeightedMove() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int CondWeightedMove<T>::execute(int iAgentIndex, float fT) {

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iNewIndex = -1; 
    
    if (pa->m_iLifeState > 0) {
        
        int iThread = omp_get_thread_num();
        
        double dR =  m_apWELL[iThread]->wrandd();

        if (dR < m_dMoveProb) {
    
            int iCellIndex = pa->m_iCellIndex;
        
            int iMaxNeighbors = this->m_pCG->m_iConnectivity;
        
            int iOffset = iCellIndex*(iMaxNeighbors+1);

            // get a random number between 0 and max 
            double dR2 =  m_apWELL[iThread]->wrandd() * m_adEnvWeights[iOffset + iMaxNeighbors];
           
            int iI = 0;
            while (iI < iMaxNeighbors + 1) {
                if (dR2 < m_adEnvWeights[iOffset + iI]) {
                    iNewIndex = iI;
                    iI = iMaxNeighbors + 1;
                } else {
                    iI++;
                }
            }
            if (iNewIndex > 0) {

                int iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewIndex-1];
                if (!this->m_pCG->m_pGeography->m_abIce[iCellIndex]) {
                    // only move if MoveCondition allows it
                    if (m_pMC->allow(iCellIndex, iNewCellIndex)) {
                        this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                    } else {
                        iNewCellIndex = -1;
                    }
                }

            }
            
        }
        
    }
    
    return (iNewIndex > -1) ? 0 : -1;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
// tries to read the attribute
//    ATTR_CONDWEIGHTEDMOVE_PROB_NAME
//
template<typename T>
int CondWeightedMove<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_CONDWEIGHTEDMOVE_PROB_NAME, 1, &m_dMoveProb);
        if (iResult != 0) {
            LOG_ERROR("[CondWeightedMove] couldn't read attribute [%s]", ATTR_CONDWEIGHTEDMOVE_PROB_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
// tries to write the attribute
//    ATTR_CONDWEIGHTEDMOVE_PROB_NAME
//
template<typename T>
int CondWeightedMove<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_CONDWEIGHTEDMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int CondWeightedMove<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_CONDWEIGHTEDMOVE_PROB_NAME) {
        m_dMoveProb = dValue;
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
int CondWeightedMove<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = getAttributeVal(mParams,  ATTR_CONDWEIGHTEDMOVE_PROB_NAME, &m_dMoveProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool CondWeightedMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    CondWeightedMove<T>* pA = static_cast<CondWeightedMove<T>*>(pAction);
    if ((m_dMoveProb == pA->m_dMoveProb) &&
        m_pMC->isEqual(pA->m_pMC, bStrict)) {
        bEqual = true;
    } 
    return bEqual;
}

