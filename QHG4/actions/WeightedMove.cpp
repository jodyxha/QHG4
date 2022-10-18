#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "QDFUtilsT.h"
#include "QDFUtils.h"
#include "WeightedMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
WeightedMove<T>::WeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights) 
    : Action<T>(pPop,pCG,ATTR_WEIGHTEDMOVE_NAME, sID),
      m_apWELL(apWELL),
      m_adEnvWeights(adEnvWeights),
      m_dMoveProb(0) {

    this->m_vNames.push_back(ATTR_WEIGHTEDMOVE_PROB_NAME);

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
WeightedMove<T>::~WeightedMove() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int WeightedMove<T>::execute(int iAgentIndex, float fT) {
    int iNewCellIndex = -1; 

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    
    if (pa->m_iLifeState > 0) {
        
        int &iCellIndex = pa->m_iCellIndex;
        int iThread = omp_get_thread_num();
        
        double dR =  m_apWELL[iThread]->wrandd();

        if (dR < m_dMoveProb) {

            // some aliases
            int &iMaxNeighbors = this->m_pCG->m_iConnectivity;
            int iRealNeighbors = this->m_pCG->m_aCells[iCellIndex].m_iNumNeighbors;
            int iOffset = iCellIndex*(iMaxNeighbors+1);

            int iNewNeighborIndex = -1; 
            if (m_adEnvWeights[iOffset] == m_adEnvWeights[iOffset+iRealNeighbors]) {
                iNewNeighborIndex = m_apWELL[iThread]->wrandi(0, iRealNeighbors);
            } else {
                // get a random number between 0 and maximum env value
                double dR2 = m_apWELL[iThread]->wrandd() * (m_adEnvWeights[iOffset + iRealNeighbors]);
                
                int iI = 0;
                while (iI < iRealNeighbors + 1) {
                    if (dR2 < m_adEnvWeights[iOffset + iI]) {
                        iNewNeighborIndex = iI;
			// index found - leave loop
                        iI = iRealNeighbors + 1;
                    } else {
                        iI++;
                    }
                }
            }

            if (iNewNeighborIndex > 0) {
                iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewNeighborIndex-1];
                if (iNewCellIndex >= 0) {
                    if ((this->m_pCG->m_pGeography == NULL) || (!this->m_pCG->m_pGeography->m_abIce[iNewCellIndex])) {
                        this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                    }
                } else {
                    printf("Illegal neighbor %d (cellindex %d) for agent %d on cell %d at %f\n", iNewNeighborIndex, iNewCellIndex, iAgentIndex, iCellIndex, fT);
                    for (int i = 0; i < iMaxNeighbors; i++) {
                        int iNeighIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[i];
                        printf("  %d: %d %f (acc %f)\n", i, iNeighIndex, m_adEnvWeights[iNeighIndex*(iMaxNeighbors+1)], m_adEnvWeights[iOffset + i]);
                    }
                }
            }
        }
    }
    
    return (iNewCellIndex > -1) ? 0 : -1;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_WEIGHTEDMOVE_PROB_NAME
//
template<typename T>
int WeightedMove<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_WEIGHTEDMOVE_PROB_NAME, 1, &m_dMoveProb);
        if (iResult != 0) {
            LOG_ERROR("[WeightedMove] couldn't read attribute [%s]", ATTR_WEIGHTEDMOVE_PROB_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_WEIGHTEDMOVE_PROB_NAME
//
template<typename T>
int WeightedMove<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_WEIGHTEDMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int WeightedMove<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_WEIGHTEDMOVE_PROB_NAME) {
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
int WeightedMove<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;
    

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = getAttributeVal(mParams, ATTR_WEIGHTEDMOVE_PROB_NAME, &m_dMoveProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// displayInfo
//
template<typename T>
void WeightedMove<T>::displayInfo(const char *pPrefix, float fT, int iCellIndex, int iAgentIndex) {
    printf("%s T%f C%d A%d\n", pPrefix, fT, iCellIndex, iAgentIndex);
    SCell &sC  =  this->m_pCG->m_aCells[iCellIndex];
    int iNumNeigh = sC.m_iNumNeighbors;
    int iMaxNeighbors =  this->m_pCG->m_iConnectivity;
    int iOffset = iCellIndex*(iMaxNeighbors+1);
    printf("%s  0 C%d E%f\n", sC.m_iGlobalID, m_adEnvWeights[iOffset]);
    for (int i = 0; i < iNumNeigh; i++) {
        printf("%s  %d C%d E%f\n", pPrefix, i+1, sC.m_aNeighbors[i], m_adEnvWeights[iOffset+i+1] - m_adEnvWeights[iOffset+i]);
    }
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool WeightedMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    WeightedMove<T>* pA = static_cast<WeightedMove<T>*>(pAction);
    if (m_dMoveProb == pA->m_dMoveProb) {
        bEqual = true;
    } 
    return bEqual;
}

