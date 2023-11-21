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
#include "PersLinearDeathRel.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PersLinearDeathRel<T>::PersLinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths)
    : Action<T>(pPop,pCG,ATTR_PERSLINDEATHREL_NAME,sID),
      m_apWELL(apWELL),
      m_adD1(NULL),
      m_adD2(NULL),
      m_adK(NULL),
      m_iStride(1),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(1),
      m_pNumAgentsPerCell(NULL) {
    
    m_adD1 = new double[this->m_pCG->m_iNumCells];
    m_adD2 = new double[this->m_pCG->m_iNumCells];
}


//-----------------------------------------------------------------------------
// constructor for use with varying K, e.g. VerhulstVarK action
//
template<typename T>
PersLinearDeathRel<T>::PersLinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double* adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_PERSLINDEATHREL_NAME,sID),
      m_apWELL(apWELL),
      m_adD1(NULL),
      m_adD2(NULL),
      m_adK(adK),
      m_iStride(iStride),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(1),
      m_pNumAgentsPerCell(NULL) {
    
    m_adD1 = new double[this->m_pCG->m_iNumCells];
    m_adD2 = new double[this->m_pCG->m_iNumCells];
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PersLinearDeathRel<T>::~PersLinearDeathRel() {
    
    if (m_adD1 != NULL) {
        delete[] m_adD1;
    }
    if (m_adD2 != NULL) {
        delete[] m_adD2;
    }
    
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int PersLinearDeathRel<T>::preLoop() {
    int iResult = -1;
    if (m_pNumAgentsPerCell == NULL) {
        m_pNumAgentsPerCell = this->m_pPop->getNumAgentsArray();
    }

    if ((m_adK  != NULL) && 
        (m_aiNumBirths != NULL)) {
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
int PersLinearDeathRel<T>::initialize(float fT) {

    memset(m_adD1,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    memset(m_adD2,0,sizeof(double)*(this->m_pCG->m_iNumCells));
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());


    // loop for varying K
    //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        const double &dNumTot = m_pNumAgentsPerCell[iC];
        const double &dNumLoc = this->m_pPop->getNumAgents(iC);
        if (m_adK[iC * m_iStride] <= 0) {
            m_adD1[iC] = -1;
            m_adD2[iC] = -1;
        } else {
            // space-varying K
            // we modify the birthrate by multiplying it with (num actual births)/(num avg births)
            // i.e. if more are born, more should die

            // we precalculate the agent-independent factor
            //            double d = m_dD0 + (m_dTheta - m_dD0) * (dNumTot / m_adK[iC * m_iStride]);
            //            m_adD[iC] = (m_aiNumBirths[m_iWhich][iC]*d)/(dNumLoc*m_adB[iC]);
            m_adD1[iC] = dNumTot /m_adK[iC * m_iStride];
            m_adD2[iC] = m_aiNumBirths[m_iWhich][iC]/dNumLoc; // m_adB[iC] is replaced by the agent's m_dBReal
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int PersLinearDeathRel<T>::finalize(float fT) {
    memset(m_aiNumBirths[m_iWhich], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
    m_iWhich = 1 - m_iWhich;
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int PersLinearDeathRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    if (pa->m_iLifeState > 0) {
        
    	int iCellIndex = pa->m_iCellIndex;
    	
    	int iThread = omp_get_thread_num();
    	
    	double dR = this->m_apWELL[iThread]->wrandd();
        double dDReal = 1;
        if (m_adD1[iCellIndex] >= 0) {
            double d      = pa->m_dD0 + (pa->m_dTheta - pa->m_dD0) * m_adD1[iCellIndex];
            dDReal = m_adD2[iCellIndex] * d/pa->m_dBReal;
        //       double dDReal = (m_aiNumBirths[m_iWhich][iCellIndex]*d)/(this->m_pPop->getNumAgents(iCellIndex)*pa->m_dBReal);
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
bool PersLinearDeathRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual =  true;
    
    return bEqual;
}

