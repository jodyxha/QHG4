#include <omp.h>

#include "MessLoggerT.h"
#include "EventConsts.h"

#include <cstring>
#include "clsutils.cpp"
#include "stdstrutilsT.h"
#include "ParamProvider2.h"
#include "ArrayShare.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PolyLine.h"
#include "Evaluator.h"
#include "SingleEvaluator.h"


//-----------------------------------------------------------------------------
// constructor
//   ATTENTION: using bCumulate=true for SingleEvaluators in a MultiEvaluator
//              can cause strange effects (icosahdral artefacts,  swimming)
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, intset &sTriggerIDs, bool bAlwaysUpdate) 
    : Evaluator<T>(pPop,pCG,ATTR_SINGLEEVAL_NAME, sID), 
      m_sPLParName(sPLParName),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(adInputData),
      m_bCumulate(bCumulate), 
      m_sTriggerIDs(sTriggerIDs),
      m_bAlwaysUpdate(bAlwaysUpdate),
      m_sInputArrayName(""),
      m_bFirst(true),
      m_pGeography(pCG->m_pGeography) {
    
    // the PolyLine is created when the parameters are read

    this->m_bNeedUpdate = m_bAlwaysUpdate;
    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    m_sTriggerIDs.insert(sTriggerIDs.begin(), sTriggerIDs.end());
}




//-----------------------------------------------------------------------------
// constructor
//   ATTENTION: using bCumulate=true for SingleEvaluators in a MultiEvaluator
//              can cause strange effects (icosahdral artefacts,  swimming)
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, int iTriggerID, bool bAlwaysUpdate)
    : Evaluator<T>(pPop,pCG,ATTR_SINGLEEVAL_NAME, sID),
      m_sPLParName(sPLParName),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(adInputData),
      m_bCumulate(bCumulate),
      m_bAlwaysUpdate(bAlwaysUpdate),
      m_sInputArrayName(""),
      m_bFirst(true),
      m_pGeography(pCG->m_pGeography) {
    
    this->m_bNeedUpdate = m_bAlwaysUpdate;
    if (iTriggerID == EVENT_ID_NONE) {
        m_bAlwaysUpdate = true;
    } else {
        m_sTriggerIDs.insert(iTriggerID);
    }

    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
}




//-----------------------------------------------------------------------------
// constructor
//   ATTENTION: using bCumulate=true for SingleEvaluators in a MultiEvaluator
//              can cause strange effects (icosahdral artefacts,  swimming)
//
template<typename T>
SingleEvaluator<T>::SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, const char *pInputArrayName, const char *sPLParName, bool bCumulate, intset &sTriggerIDs, bool bAlwaysUpdate) 
    : Action<T>(pPop,pCG,ATTR_SINGLEEVAL_NAME, sID),
      m_sPLParName(sPLParName),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(NULL),
      m_bCumulate(bCumulate), 
      m_sTriggerIDs(sTriggerIDs),
      m_bAlwaysUpdate(bAlwaysUpdate),
      m_sInputArrayName(pInputArrayName),
      m_pGeography(pCG->m_pGeography) {
    
    this->m_bNeedUpdate = m_bAlwaysUpdate;
    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    m_sTriggerIDs.insert(sTriggerIDs.begin(), sTriggerIDs.end());
}



//-----------------------------------------------------------------------------
// destructorsimple
//
template<typename T>
SingleEvaluator<T>::~SingleEvaluator() {

    if (m_pPL != NULL) {
        delete m_pPL;
    }

}




//-----------------------------------------------------------------------------
// finalize
//   reset m_bNeeedUpdate to false
//
template<typename T>
int SingleEvaluator<T>::finalize(float fT) {
    if ((!m_bAlwaysUpdate) && (this->m_bNeedUpdate)) {
        this->m_bNeedUpdate = false;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// initialize
// here the weights are calculated if needed
//
template<typename T>
int SingleEvaluator<T>::initialize(float fT) {

    int iResult = 0;

    // get array if it does not exist
    if (m_adInputData == NULL) {
        m_adInputData = (double *) ArrayShare::getInstance()->getArray(m_sInputArrayName);
    }


    if (m_adInputData != NULL) {
    

        if (this->m_bNeedUpdate || (m_bFirst)) {   // need for sure at first step

            m_bFirst = false;
            if (!m_sPLParName.empty()) {stdprintf("SingleEvaluator::initialize is updating weights for %s\n", m_sPLParName);} 
            
            calcValues(); // get cell values from PolyLine
            
            exchangeAndCumulate(); // compute actual weights

        }
    } else {
        stdprintf("No array with name [%s] found in ArrayExchange\n", m_sInputArrayName);
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// calcValues
//
template<typename T>
void SingleEvaluator<T>::calcValues() {
    memset(m_adOutputWeights, 0, this->m_pCG->m_iNumCells*(m_iMaxNeighbors+1)*sizeof(double));
    if (m_pPL != NULL) {  
        //        printf("[SingleEvaluator<T>::calcValues] Creating modified output\n");

#pragma omp parallel for
        for (uint iCellIndex = 0; iCellIndex < this->m_pCG->m_iNumCells; iCellIndex++) {
            
            // please do not hard-code here things that can be set as parameters :-)
            if ((m_pGeography == NULL) || ( ! m_pGeography->m_abIce[iCellIndex] )) {
                
                double dV = this->m_pPL->getVal((float)m_adInputData[iCellIndex]);
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = (dV > 0) ? dV : 0;

            } else {
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = 0;
            }
        }
    } else {
        //        printf("[SingleEvaluator<T>::calcValues] Creating direct output\n");
#pragma omp parallel for
        for (uint iCellIndex = 0; iCellIndex < this->m_pCG->m_iNumCells; iCellIndex++) {
            
            // please do not hard-code here things that can be set as parameters :-)
            if ((m_pGeography == NULL) || ( ! m_pGeography->m_abIce[iCellIndex] )) {
                double dV = m_adInputData[iCellIndex];
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = (dV > 0) ? dV : 0;
            } else {
                m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)] = 0;
            }
        }
    }

}


//----------------------------------------------------------------------------
// exchangeAndNormalize
// here the actual normalized weights are computed 
// for all cells and their neighbors
//
template<typename T>
void SingleEvaluator<T>::exchangeAndCumulate() {

    // fill neighbor buffer for all cells
    

#pragma omp parallel for
    for (uint iCellIndex = 0; iCellIndex < this->m_pCG->m_iNumCells; iCellIndex++) {
        
        SCell &sc = this->m_pCG->m_aCells[iCellIndex];
        double dW = m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1)];  
        
        for (int i = 0; i < m_iMaxNeighbors; i++) {

            double dCW = 0;
            int iCurIndex = sc.m_aNeighbors[i];

            if (iCurIndex >= 0) {
                dCW = m_adOutputWeights[iCurIndex*(m_iMaxNeighbors+1)];  // Current Weight
            }

            dCW = (dCW > 0) ? dCW : 0;  // put negative weights to 0
            dW = (m_bCumulate) ? dW + dCW : dCW;  // cumulate if necessary
            
            m_adOutputWeights[iCellIndex*(m_iMaxNeighbors+1) + i + 1] = dW;
        }
    }

}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attribute specified by m_sPLParName
//  and creates a PolyLine from it
//
template<typename T>
int SingleEvaluator<T>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = -1;
    if (!m_sPLParName.empty()) {
        stdprintf("SingleEvaluator::extractAttributesQDF will work on %s\n", m_sPLParName);
        if (this->m_pPL != NULL) {
            delete (this->m_pPL);
        }
        m_pPL = qdf_createPolyLine(hSpeciesGroup, m_sPLParName);
        if (m_pPL != NULL) {
            if (m_pPL->m_iNumSegments == 0) {
                delete m_pPL;
                m_pPL = NULL;
            }
            iResult = 0;
        } else {
            LOG_ERROR("[SingleEvaluator] couldn't read attribute [%s]", m_sPLParName);
            
            iResult = -1;
        }
    } else {
        //no polyline
        m_pPL = NULL;
        iResult = 0;
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attribute m_sPLParName
//
template<typename T>
int SingleEvaluator<T>::writeAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (!m_sPLParName.empty()) {
        iResult = qdf_writePolyLine(hSpeciesGroup, m_pPL, m_sPLParName);
    }

    return iResult;

}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   If there is a poly line name, use it.
//   No name is ok too (direct use of environment values)
//
template<typename T>
int SingleEvaluator<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = 0;
    if (!m_sPLParName.empty()) {
        const stringmap &mParams = pMC->getAttributes();
        std::string sName = "";
        iResult = getAttributeStr(mParams, m_sPLParName,  sName);
        if (iResult == 0) {
            m_pPL = PolyLine::readFromString(sName);
            
            if (m_pPL == NULL) {
                iResult = -1;
            }  
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void SingleEvaluator<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if (!m_bAlwaysUpdate) {
        if (iEvent == EVENT_ID_FLUSH) {   
            // no need to act - recalculation will happen at next step's initialize()
        } else {
            intset::const_iterator its;

            for (its = m_sTriggerIDs.begin(); !this->m_bNeedUpdate && (its != m_sTriggerIDs.end()); ++its) {
                if (iEvent == *its) {
                    this->m_bNeedUpdate = true;
                }
            }
        }
    }
}



//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool SingleEvaluator<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    SingleEvaluator<T>* pA = static_cast<SingleEvaluator<T>*>(pAction);
    if ((!m_sPLParName.empty()) && (!pA->m_sPLParName.empty())) {
        if (m_sPLParName == pA->m_sPLParName) {
            bEqual = true;
        }
    } else if (m_sPLParName.empty() && pA->m_sPLParName.empty()) {
        bEqual = true;
    }
    return bEqual;
}
/*
//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool SingleEvaluator<T>::isEqual(Evaluator *pEval, bool bStrict) {
    bool bEqual = false;
    SingleEvaluator<T>* pA = static_cast<SingleEvaluator<T>*>(pEval);
    if ((m_sPLParName != NULL) && (pA->m_sPLParName != NULL)) {
        if (strcmp(m_sPLParName, pA->m_sPLParName) == 0) {
            bEqual = true;
        }
    } else if((m_sPLParName == NULL) && (pA->m_sPLParName == NULL)) {
        bEqual = true;
    }
    return bEqual;
}
*/


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void SingleEvaluator<T>::showAttributes() {
    stdprintf("  %s\n", m_sPLParName);
    if (m_pPL != NULL) {
        m_pPL->display("  ", "PolyLine");
    } else {
        stdprintf("PolyLine null\n");
    }
}


