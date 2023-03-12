#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "WeightedMoveRand.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
WeightedMoveRand<T>::WeightedMoveRand(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights) 
    : Action<T>(pPop,pCG,ATTR_WEIGHTEDMOVERAND_NAME, sID),
      m_apWELL(apWELL),
      m_adEnvWeights(adEnvWeights),
      m_dMoveProb(0),
      m_pGeography(pCG->m_pGeography) {

    this->m_vNames.push_back(ATTR_WEIGHTEDMOVERAND_PROB_NAME);
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
WeightedMoveRand<T>::~WeightedMoveRand() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int WeightedMoveRand<T>::execute(int iAgentIndex, float fT) {

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iNewIndex = -1; 
    
    if (pa->m_iLifeState > 0) {
        
        int iThread = omp_get_thread_num();
        
        double dR =  m_apWELL[iThread]->wrandd();

        if (dR < m_dMoveProb) {
    
            int iCellIndex = pa->m_iCellIndex;
        
            int iMaxNeighbors = this->m_pCG->m_iConnectivity;
        
            int iOffset = iCellIndex*(iMaxNeighbors+1);
       
            if ( m_adEnvWeights[iOffset + iMaxNeighbors] > 0) {
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
       
            } else {
                int iNumActualNeigh = this->m_pCG->m_aCells[iCellIndex].m_iNumNeighbors;
                // do a random move if everything's zero
                iNewIndex = (int) m_apWELL[iThread]->wrandr(0, iNumActualNeigh+1);
            }

            if (iNewIndex > 0) {
                int iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewIndex-1];
                // on flat grid, some neighbo cells have "ID" -1
                if (iNewCellIndex >= 0) {
                        if ((m_pGeography == NULL) || (!m_pGeography->m_abIce[iNewCellIndex])) {
                            this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                        /*
                            printf("Moving %p from (%f,%f) to (%f,%f)\n", pa,  
                                   m_pGeography->m_adLongitude[iCellIndex],
                                   m_pGeography->m_adLatitude[iCellIndex],
                                   m_pGeography->m_adLongitude[iNewCellIndex],
                                   m_pGeography->m_adLatitude[iNewCellIndex]);
                        */
                    }

                    /*
                    if (iNewCellIndex < 0) {
                        printf("vvvvv[WeightedMoveRand<T>::execute]Negative target from cell %d\n", iCellIndex);
                        printf("vvvvv[WeightedMoveRand<T>::execute]m_adEnvweights (offset %d)\n", iOffset);
                        for (int z = 0; z < iMaxNeighbors + 1; z++) {
                            printf("vvvvv  %.4f\n",  m_adEnvWeights[iOffset + z]);
                        }
                    }
                    */
                }
            }
        }
        
    }
    
    return (iNewIndex > -1) ? 0 : -1;
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//  tries to read the atttribute
//    ATTR_WEIGHTEDMOVERAND_PROB_NAME
//
template<typename T>
int WeightedMoveRand<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_WEIGHTEDMOVERAND_PROB_NAME, 1, &m_dMoveProb);
        if (iResult != 0) {
            LOG_ERROR("[WeightedMoveRand] couldn't read attribute [%s]", ATTR_WEIGHTEDMOVERAND_PROB_NAME);
        }
    }

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//  tries to write the atttribute
//    ATTR_WEIGHTEDMOVERAND_PROB_NAME
//
template<typename T>
int WeightedMoveRand<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_WEIGHTEDMOVERAND_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int WeightedMoveRand<T>::tryGetAttributes(const ModuleComplex *pMC) { 
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = getAttributeVal(mParams,  ATTR_WEIGHTEDMOVERAND_PROB_NAME, &m_dMoveProb);
    } 
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool WeightedMoveRand<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    WeightedMoveRand<T>* pA = static_cast<WeightedMoveRand<T>*>(pAction);
    if (m_dMoveProb == pA->m_dMoveProb) {
        bEqual = true;
    } 
    return bEqual;
}

