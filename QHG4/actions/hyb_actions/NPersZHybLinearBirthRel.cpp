#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"  //@@debugging
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "NPersZHybLinearBirthRel.h"

template<typename T>
const char *NPersZHybLinearBirthRel<T>::asNames[] = {
    ATTR_NPERSZHYBLINBIRTHREL_HYBMINPROB_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
NPersZHybLinearBirthRel<T>::NPersZHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths) 
    : Action<T>(pPop,pCG,ATTR_NPERSZHYBLINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dHybMinProb(1.0),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0),
      m_pNumAgentsPerCell(NULL),
      m_fHybScale(2),
      m_fHybShift(0.5) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// constructor for use with VerhulstVarK action
//
template<typename T>
NPersZHybLinearBirthRel<T>::NPersZHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double dHybMinProb) 
    : Action<T>(pPop,pCG,ATTR_NPERSZHYBLINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dHybMinProb(dHybMinProb),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0) ,
      m_pNumAgentsPerCell(NULL),
      m_fHybScale(2),
      m_fHybShift(0.5) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
NPersZHybLinearBirthRel<T>::~NPersZHybLinearBirthRel() {
    if (m_aaiNumTemp != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            if (m_aaiNumTemp != NULL) {
                delete[] m_aaiNumTemp[i];
            }
        }
        delete[] m_aaiNumTemp;
    }

    // we don't delete m_aiNumBirths because it comes from outside
}



//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int NPersZHybLinearBirthRel<T>::preLoop() {
    int iResult = -1;

    //    xha_printf("!!!intertest!!! NPersZHybLinearBirthRel<T>::preLoop\n");

    m_fHybScale = 2*(1 - m_dHybMinProb);
    m_fHybShift = 1 - (1 - m_dHybMinProb)/2;
  
    if (m_pNumAgentsPerCell == NULL) {
        m_pNumAgentsPerCell = this->m_pPop->getNumAgentsArray();
    }

    if ((m_aiNumBirths != NULL)) {
#pragma omp parallel for
        for (int i = 0; i < m_iNumThreads; i++) {
            memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
        }       
        memset(m_aiNumBirths[0], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        memset(m_aiNumBirths[1], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int NPersZHybLinearBirthRel<T>::postLoop() {
    int iResult = 0;

    return iResult;
 }

//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int NPersZHybLinearBirthRel<T>::initialize(float fT) {

#pragma omp parallel for
    for (int i = 0; i < m_iNumThreads; i++) {
        memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
    }
    //    printf("Num agents in cell 0: %lu\n", this->m_pPop->getNumAgents(0));
    // set 
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
    //#pragma omp parallel for schedule(static,iChunk)

    
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int NPersZHybLinearBirthRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *paM = &(this->m_pPop->m_aAgents[iAgentIndex]);
    
    if (paM->m_iLifeState > 0) {

    	int iMate      = paM->m_iMateIndex;
        int iCellIndex = paM->m_iCellIndex;
        int iThread    = omp_get_thread_num();
        // we'll need a random  number in any case
        double dR      = this->m_apWELL[iThread]->wrandd();
        
        // if reproduction is in couples, use mate index, 
        // otherwise use agent index also as father
        
        // note: the offset is needed because if the agent structure 
        // does not have a m_iMateIndex member, the compiler 
        // would not know what to do
	
        
        if (paM->m_dCC > 0) {
            
            // we *must* calculate m_dBReal - it will be used in NPersLInearDeathRel
            paM->m_dBReal =  paM->m_dB0 + ((paM->m_dTheta - paM->m_dB0)*this->m_pPop->getNumAgents(iCellIndex))/paM->m_dCC;
            /*
#pragma omp critical
                {
                    printf("agent %d (g%d,m%d): b0 %f, theta %f, numags %lu, cc %f -> bReal %f\n", iAgentIndex, paM->m_iGender, iMate, paM->m_dB0, paM->m_dTheta, this->m_pPop->getNumAgents(iCellIndex), paM->m_dCC,  paM->m_dBReal);
                    }*/ 
                           
            
            if (paM->m_dBReal > 0) { // positive birth prob
                if ((paM->m_iGender == 0) && (iMate >= 0)) {
                    // get the father
                    T *paP =  &(this->m_pPop->m_aAgents[iMate]);
                    // parent fertility factors
                    float fFertM = m_dHybMinProb + 2*(1-m_dHybMinProb)*fabs(paM->m_fPhenHyb-0.5);
                    float fFertP = m_dHybMinProb + 2*(1-m_dHybMinProb)*fabs(paP->m_fPhenHyb-0.5);
                    
                    /*#pragma omp critical
                    {
                        printf("agent %d: fertM %f, fertP %f: fert %f R%f\n", iAgentIndex, fFertM, fFertP, fFullFert, dR);
                        } */

                    //if (dR < paM->m_dBReal*fFullFert) {
                    /*
                    if (paM->m_fPhenHyb != paP->m_fPhenHyb) {
                        xha_printf("!!!intertest!!! momH %f, dadH %f, fertM:%f fertP:%f, BReal:%f, R:%f\n", paM->m_fPhenHyb, paP->m_fPhenHyb , fFertM, fFertP, paM->m_dBReal, dR);
                    }
                    */
                    if (dR < paM->m_dBReal*fFertM*fFertP) {
                        
                        this->m_pPop->registerBirth(iCellIndex, iAgentIndex, iMate);
                        m_aaiNumTemp[iThread][iCellIndex]++;
                        iResult = 1;
                    } 
                }
            } else if (paM->m_dBReal < 0) {
                
                if (dR < -paM->m_dBReal) { // convert negative birth prob to death prob
                    /*#pragma omp critical
                    {
                        printf("agent %d, mate %d: registrering death\n", iAgentIndex, iMate);
                        }*/
                    this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
                    
                }
            } 
        } else {
            paM->m_dBReal  = 0;
            // negative K : no births
        }
    } // male or noi mate
    
    
    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int NPersZHybLinearBirthRel<T>::finalize(float fT) {

#pragma omp parallel for
    for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
        m_aiNumBirths[m_iWhich][i] = 0;
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            m_aiNumBirths[m_iWhich][i] += m_aaiNumTemp[iT][i];
        }
    }
    //    printf("Num births in cell 0: %d\n", m_aiNumBirths[m_iWhich][0]);

    m_iWhich = 1 - m_iWhich;
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_NPERSLINBIRTHREL_HYBMINPROB_NAME
//
template<typename T>
int NPersZHybLinearBirthRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPERSZHYBLINBIRTHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
        if (iResult != 0) {
            LOG_ERROR("[NPersZHybLinearBirthRel] couldn't read attribute [%s]", ATTR_NPERSZHYBLINBIRTHREL_HYBMINPROB_NAME);
        }
    }
    
    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_NPERSLINBIRTHREL_HYBMINPROB_NAME
//
template<typename T>
int NPersZHybLinearBirthRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPERSZHYBLINBIRTHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int NPersZHybLinearBirthRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_NPERSZHYBLINBIRTHREL_HYBMINPROB_NAME) {
        m_dHybMinProb = dValue;
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
int NPersZHybLinearBirthRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_NPERSZHYBLINBIRTHREL_HYBMINPROB_NAME, &m_dHybMinProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool NPersZHybLinearBirthRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    NPersZHybLinearBirthRel<T>* pA = static_cast<NPersZHybLinearBirthRel<T>*>(pAction);
    if  (m_dHybMinProb == pA->m_dHybMinProb) {
        bEqual = true;
    } 
    return bEqual;
}

