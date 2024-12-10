#include <omp.h>
#include <cmath>

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "DirMove.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
DirMove<T>::DirMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID) 
    : Action<T>(pPop,pCG,ATTR_DIRMOVE_NAME,sID),
    m_pGeography(pCG->m_pGeography) {

    this->m_vNames.clear();
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
DirMove<T>::~DirMove() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: execute
//  NOTE: agent class T musthave members 
//      double m_dDirection
//      double m_dError
//
template<typename T>
int DirMove<T>::execute(int iAgentIndex, float fT) {
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);
    
    int iNewIndex = -1; 
    if ((pa->m_iLifeState > 0) && (pa->m_fAge > 5)) {

        float fDirection = pa->m_fDirection;
        float fError     = pa->m_fError;
        // correct direction for error
        float fAngle = fDirection - fError;

        
        
        double d0 = 2*Q_PI;
        double fAng0 = 0;
        int iCellIndex = pa->m_iCellIndex;
        int iMaxNeighbors = this->m_pCG->m_aCells[iCellIndex].m_iNumNeighbors;//bad: m_iConnectivity
        
        // find direction closest to the corrected direction
        // we assume angles are in [-pi, pi]
        for (int i = 0; i < iMaxNeighbors; i++) {
            double fAng = m_pGeography->m_adAngles[iMaxNeighbors*iCellIndex+i];
            double d = 0;
            /*
            if (((fAngle >= 0) && (fAng >= 0)) ||
                ((fAngle < 0)  && (fAng < 0))) {
                d = fAngle - fAng;
            } else if (((fAngle >= 0) && (fAngle < Q_PI/2)) &&
                       ((fAng   < 0)  && (fAng >= -Q_PI/2))) {
                d = fAngle - fAng;
                
            } else {
                d = fAngle - (fAng + 2*Q_PI);
            }
            */
            d = fAngle - fAng;
            if (d > Q_PI/2) {
                d -= 2*Q_PI;
            } else if (d < -Q_PI/2) {
                d += 2*Q_PI;
            }
                
            /*
            if (std::isnan(d)) {
                printf("Have nan direction at cellindex %d for agent id %d @ %d\n", iCellIndex, pa->m_ulID, iAgentIndex);
            }
            */
            if (fabs(d) < fabs(d0)) {
                d0 = d;
                iNewIndex = i;
                fAng0 = fAng;
            }
        }
        pa->m_fOldError = fError;

        fError += fAng0 - fDirection;
        
        // somehow get the error back to the agent
        pa->m_fError = fError;
        
        if (iNewIndex >= 0) {
            int iNewCellIndex = this->m_pCG->m_aCells[iCellIndex].m_aNeighbors[iNewIndex];
            /*                       
            printf("C%6d T%3.0f: %7d -> % 2.4f (% 2.4f e % 2.4f  -> % 2.4f): %6d ( ", pa->m_ulID, fT, iCellIndex, fAng0, fDirection, pa->m_fOldError,  pa->m_fError, iNewCellIndex);
            for (int i = 0; i < iMaxNeighbors; i++) {
                float fAng = m_pGeography->m_adAngles[iMaxNeighbors*iCellIndex+i];
                printf("% 2.3f ", fAng);
            }
            printf(")\n");
            */
            this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
        } else {
            /*
            printf("X%d: %d -> %f (%f e %f -> %f):  ( ", pa->m_ulID, iCellIndex, fAng0, fDirection, pa->m_fOldError, fError);
            for (int i = 0; i < iMaxNeighbors; i++) {
                float fAng = m_pGeography->m_adAngles[iMaxNeighbors*iCellIndex+i];
                printf("%f ", fAng);
            }
            printf(")\n");
            */
        }
        
    } else {
        iNewIndex = 0;
    }
    return (iNewIndex > -1) ? 0 : -1;
    
}



//-----------------------------------------------------------------------------
// extractAttributesQDF
//

template<typename T>
int DirMove<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    // nothing to do here

    return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
template<typename T>
int DirMove<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    // nothing to do here

    return iResult; 
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool DirMove<T>::isEqual(Action<T> *pAction, bool bStrict) {
    return true;
}


