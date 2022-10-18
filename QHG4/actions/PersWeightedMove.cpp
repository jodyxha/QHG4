#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PersWeightedMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PersWeightedMove<T>::PersWeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights) 
    : Action<T>(pPop,pCG,ATTR_PERSWEIGHTEDMOVE_NAME, sID),
      m_apWELL(apWELL),
      m_adEnvWeights(adEnvWeights) {

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PersWeightedMove<T>::~PersWeightedMove() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//  we assume th aggent has a field
//      double m_dMoveProb
//
template<typename T>
int PersWeightedMove<T>::execute(int iAgentIndex, float fT) {

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    int iNewIndex = -1; 
    
    if (pa->m_iLifeState > 0) {
        
        int iThread = omp_get_thread_num();
        
        double dR =  m_apWELL[iThread]->wrandd();

        if (dR < pa->m_dMoveProb) {

            int iCellIndex = pa->m_iCellIndex;
        
            int iMaxNeighbors = this->m_pCG->m_iConnectivity;
        
            int iOffset = iCellIndex*(iMaxNeighbors+1);
            
            // count how many neighbors are not eligible for movement
            // m_adEnvWeight contains accumulated probabilities; 
            // if 2 subsequent values are equal, one of them has probability 0
            int iInaccessible = 0;
            for (int i = iOffset+1; i < iOffset+iMaxNeighbors+1; i++) {
                if (m_adEnvWeights[i] == m_adEnvWeights[i - 1]) {
                    iInaccessible++;
                }
            }

            double dR1 = m_apWELL[iThread]->wrandd();

            if (dR1 > iInaccessible / (double)iMaxNeighbors) {

                if (m_adEnvWeights[iOffset] == m_adEnvWeights[iOffset+iMaxNeighbors-1]) {
                    iNewIndex = m_apWELL[iThread]->wrandi(0, iMaxNeighbors);
                } else {
                    // get a random number between current cell's and max
                    double dR2 =  m_adEnvWeights[iOffset] + 
                        m_apWELL[iThread]->wrandd() * (m_adEnvWeights[iOffset + iMaxNeighbors] - m_adEnvWeights[iOffset]);
                    
                    
                    int iI = 0;
                    while (iI < iMaxNeighbors + 1) {
                        if (dR2 < m_adEnvWeights[iOffset + iI]) {
                            iNewIndex = iI;
                            iI = iMaxNeighbors + 1;
                        } else {
                            iI++;
                        }
                    }
                }

                if (iNewIndex > 0) {
                    int iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewIndex-1];
                    if (iNewCellIndex >= 0) {
                        if ((this->m_pCG->m_pGeography == NULL) || (!this->m_pCG->m_pGeography->m_abIce[iCellIndex])) {
                            this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                        }
                    } else {
                        printf("Illegal neighbor %d (cellindex %d) for agent %d on cell %d at %f\n", iNewIndex, iNewCellIndex, iAgentIndex, iCellIndex, fT);
                        for (int i = 0; i < iMaxNeighbors; i++) {
                            int iNeighIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[i];
                            printf("  %d: %d %f (acc %f)\n", i, iNeighIndex, m_adEnvWeights[iNeighIndex*(iMaxNeighbors+1)], m_adEnvWeights[iOffset + i]);
                        }
                    }
                }
            }
        }
        
    }
    
    return (iNewIndex > -1) ? 0 : -1;
}

//-----------------------------------------------------------------------------
// postLoop
//
template<typename T>
int PersWeightedMove<T>::postLoop() {
    int iResult = 0;
    return iResult;
}


//-----------------------------------------------------------------------------
// displayInfo
//
template<typename T>
void PersWeightedMove<T>::displayInfo(const char *pPrefix, float fT, int iCellIndex, int iAgentIndex) {
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
bool PersWeightedMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    
    return bEqual;
}

