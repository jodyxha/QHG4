#include <omp.h>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "LocEnv.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GroupMemberMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
GroupMemberMove<T>::GroupMemberMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, AgentEnv<T> *pAE) 
    : Action<T>(pPop,pCG,ATTR_GROUPMEMBERMOVE_NAME, sID),
    m_apWELL(apWELL),
    m_pAE(pAE) {

   }


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
GroupMemberMove<T>::~GroupMemberMove() {
    
    // nothing to do here

}


//-----------------------------------------------------------------------------
// action: execute
//  we assume th aggent has a field
//      double m_dMoveProb
//
template<typename T>
int GroupMemberMove<T>::execute(int iAgentIndex, float fT) {
    int iNewCellIndex = -1;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
        int iCellIndex = pa->m_iCellIndex;
        int iThread = omp_get_thread_num();
        
        double dR =  m_apWELL[iThread]->wrandd();
        
        if (dR < m_dMoveProb) {
            
            int iMaxNeighbors  = this->m_pCG->m_iConnectivity;
            int iRealNeighbors = this->m_pCG->m_aCells[iCellIndex].m_iNumNeighbors;

            const double *pdCurEnv =  m_pAE->getArr(iAgentIndex);

            int iNewNeighborIndex = -1; 
            if (pdCurEnv[0] == pdCurEnv[iMaxNeighbors-1]) {
                iNewNeighborIndex = m_apWELL[iThread]->wrandi(0, iRealNeighbors);
            } else {

                // get a random number between current cell's and max
                double dR2 =  m_apWELL[iThread]->wrandd() * (pdCurEnv[iRealNeighbors]);

                int iI = 0;
                while (iI < iRealNeighbors + 1) {
                    if (dR2 < pdCurEnv[iI]) {
                        iNewNeighborIndex = iI;
                        iI = iRealNeighbors + 1;
                    } else {
                        iI++;
                    }
                }
                
            }
            
            if (iNewNeighborIndex > 0) {
                iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewNeighborIndex-1];

                if (iNewCellIndex >= 0) {
                    if ((this->m_pCG->m_pGeography == NULL) || (!this->m_pCG->m_pGeography->m_abIce[iCellIndex])) {
                        this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                    }
                } else {
                    printf("Illegal neighbor %d (cellindex %d) for agent %d on cell %d at %f (conn %d)\n", iNewNeighborIndex, iNewCellIndex, iAgentIndex, iCellIndex, fT, iMaxNeighbors);
                    for (int i = 0; i < iMaxNeighbors; i++) {
                        int iNeighIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[i];
                        printf("  %d: %d %f (acc %f)\n", i, iNeighIndex, pdCurEnv[iNeighIndex*(iMaxNeighbors+1)], pdCurEnv[i]);
                    }
                }
            }
        }
    }
    
    return (iNewCellIndex > -1) ? 0 : -1;
}

//-----------------------------------------------------------------------------
// postLoop
//
template<typename T>
int GroupMemberMove<T>::postLoop() {
    int iResult = 0;
    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_WEIGHTEDMOVE_PROB_NAME
//
template<typename T>
int GroupMemberMove<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GROUPMEMBERMOVE_PROB_NAME, 1, &m_dMoveProb);
        if (iResult != 0) {
            LOG_ERROR("[WeightedMove] couldn't read attribute [%s]", ATTR_GROUPMEMBERMOVE_PROB_NAME);
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
int GroupMemberMove<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GROUPMEMBERMOVE_PROB_NAME, 1, &m_dMoveProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int GroupMemberMove<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;
    

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = getAttributeVal(mParams, ATTR_GROUPMEMBERMOVE_PROB_NAME, &m_dMoveProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool GroupMemberMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    
    bool bEqual = false;
    GroupMemberMove<T>* pA = static_cast<GroupMemberMove<T>*>(pAction);
    if (m_dMoveProb == pA->m_dMoveProb) {
        bEqual = true;
    } 
    
    return bEqual;
}
