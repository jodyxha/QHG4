#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "WELL512.h"
#include "WELLUtils.h"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "NPersLinearDeathRel.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
NPersLinearDeathRel<T>::NPersLinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths)
    : Action<T>(pPop,pCG,ATTR_NPERSLINDEATHREL_NAME,sID),
      m_apWELL(apWELL),
      m_adD(NULL),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(1),
      m_pNumAgentsPerCell(NULL) {
    
    m_adD = new double[this->m_pCG->m_iNumCells];
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
NPersLinearDeathRel<T>::~NPersLinearDeathRel() {
    
    if (m_adD != NULL) {
        delete[] m_adD;
    }
    
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int NPersLinearDeathRel<T>::preLoop() {
    int iResult = -1;
    if (m_pNumAgentsPerCell == NULL) {
        m_pNumAgentsPerCell = this->m_pPop->getNumAgentsArray();
    }

    if ((m_aiNumBirths != NULL)) {
            iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
//   We want N_death = N_birth*(b/d)
//   Therefore d_mod = N_death/N
//
template<typename T>
int NPersLinearDeathRel<T>::initialize(float fT) {

    memset(m_adD,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());

    
    // loop for varying K
    //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        if (this->m_pPop->getNumAgents(iC) > 0) {
            m_adD[iC] = (1.0*m_aiNumBirths[m_iWhich][iC])/this->m_pPop->getNumAgents(iC);
        } else {
            m_adD[iC] = 1;
        }
        /*
        const double &dNumTot = m_pNumAgentsPerCell[iC];
        //const double &dNumLoc = this->m_pPop->getNumAgents(iC);
        if (m_adK[iC * m_iStride] <= 0) {
            m_adD[iC] = -1;
        } else {
            // space-varying K
            // we precalculate the agent-independent factor
            //            double d = m_dD0 + (m_dTheta - m_dD0) * (dNumTot / m_adK[iC * m_iStride]);
            //            m_adD[iC] = (m_aiNumBirths[m_iWhich][iC]*d)/(dNumLoc*m_adB[iC]);
            m_adD[iC] = dNumTot;
        }
        */
    }
    
    
    return 0;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int NPersLinearDeathRel<T>::finalize(float fT) {
    memset(m_aiNumBirths[m_iWhich], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
    m_iWhich = 1 - m_iWhich;
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int NPersLinearDeathRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
    	int iCellIndex = pa->m_iCellIndex;
    	
    	int iThread = omp_get_thread_num();
	
    	double dR = this->m_apWELL[iThread]->wrandd();
        double dDReal = 1; 
        const double &dNumTot = m_pNumAgentsPerCell[iCellIndex];
        if (m_adD[iCellIndex] >= 0) {
            if (pa->m_dCC > 0) {
                double d = pa->m_dD0 + (pa->m_dTheta - pa->m_dD0) * dNumTot/pa->m_dCC;
                dDReal = (m_adD[iCellIndex]*d)/pa->m_dBReal;
                /*
            if (iCellIndex == 0) {
#pragma omp critical
                {
                    printf("agent %d: d0 %f, theta %f, numtot %f, cc %f,tempd %f (nb %d/na %lu)-> dReal %f\n", 
                           iAgentIndex, pa->m_dD0, pa->m_dTheta, dNumTot, pa->m_dCC,  
                           m_adD[iCellIndex], m_aiNumBirths[m_iWhich][iCellIndex], this->m_pPop->getNumAgents(iCellIndex), dDReal);
                } 
            }
                */             
            }
        } 
        
        if (dR < dDReal) {
            this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
            iResult = 1;
        }
    }
	
    return iResult;
}




//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool NPersLinearDeathRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual =  true;
    
    return bEqual;
}

