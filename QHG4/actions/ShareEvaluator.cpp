#include <omp.h>

#include "MessLoggerT.h"
#include "EventConsts.h"

#include <cstring>
#include "clsutils.cpp"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "ParamProvider2.h"
#include "ArrayShare.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PolyLine.h"
#include "ShareEvaluator.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
ShareEvaluator<T>::ShareEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, bool bCumulate, intset &sTriggerIDs) 
    : Evaluator<T>(pPop,pCG,ATTR_SHAREEVAL_NAME, sID), 
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(NULL),
      m_bCumulate(bCumulate), 
      m_sTriggerIDs(sTriggerIDs),
      m_bAlwaysUpdate(false),
      m_sID(NULL),
      m_sArrayName(""),
      m_sPolyName(""),
      m_pGeography(pCG->m_pGeography) {


    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    // the PolyLine is created when the parameters are read
    
    m_sID = sID;

}

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
ShareEvaluator<T>::ShareEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, bool bCumulate, int iTriggerID) 
    : Evaluator<T>(pPop,pCG,ATTR_SHAREEVAL_NAME, sID),
      m_pPL(NULL),
      m_adOutputWeights(adOutputWeights),
      m_adInputData(NULL),
      m_bCumulate(bCumulate), 
      m_bAlwaysUpdate(false),
      m_sID(""),
      m_sArrayName(""),
      m_sPolyName(""),
      m_pGeography(pCG->m_pGeography) {


    if (iTriggerID == EVENT_ID_NONE) {
        m_bAlwaysUpdate = true;
    } else {
        m_sTriggerIDs.insert(iTriggerID);
    }

    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    // the PolyLine is created when the parameters are read
    
    m_sID = sID;

    std::string sKeyArrName  = xha_sprintf(ATTR_SHAREEVAL_ARRAYNAME, m_sID);
    std::string sKeyPolyName = xha_sprintf(ATTR_SHAREEVAL_POLYNAME, m_sID);
    this->m_vNames.push_back(sKeyArrName);    
    this->m_vNames.push_back(sKeyPolyName);

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
ShareEvaluator<T>::~ShareEvaluator() {

    if (m_pPL != NULL) {
        delete m_pPL;
    }

}

//-----------------------------------------------------------------------------
// preLoop
//   load the shared input data
//
template<typename T>
int ShareEvaluator<T>::preLoop() {

    int iResult = 0;

    // get array if it does not exist
    m_adInputData = (double *) ArrayShare::getInstance()->getArray(m_sArrayName);
    if (m_adInputData != NULL) {
        xha_printf("[ShareEvaluator<T>::preLoop][%s] xxxShare Loaded shared array [%s]: %p\n", this->m_pPop->getSpeciesName(), m_sArrayName, m_adInputData);
    } else {
        xha_printf("No array with name [%s] found in ArrayExchange\n", m_sArrayName);
        iResult = -1;
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// initialize
// here the weights are calculated if needed
//
template<typename T>
int ShareEvaluator<T>::initialize(float fT) {
    int iResult = 0;

    if (this->m_bNeedUpdate || this->m_bAlwaysUpdate || (fT == 0)) {   // need for sure at first step
        
        xha_printf("ShareEvaluator::initialize is updating weights for %s\n", m_sPolyName); 
        
        calcValues(); // get cell values from PolyLine
        
        exchangeAndCumulate(); // compute actual weights
        
    }
         
    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//   reset m_bNeeedUpdate to false
//
template<typename T>
int ShareEvaluator<T>::finalize(float fT) {
    if (!m_bAlwaysUpdate) {
        this->m_bNeedUpdate = false;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void ShareEvaluator<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if (!m_bAlwaysUpdate) {
        if (iEvent == EVENT_ID_FLUSH) {   
            // no need to act - recalculation will happen at next step's initialize()
        } else {
            intset::const_iterator its;
            this->m_bNeedUpdate = false;
            for (its = m_sTriggerIDs.begin(); !this->m_bNeedUpdate && (its != m_sTriggerIDs.end()); ++its) {
                if (iEvent == *its) {
                    this->m_bNeedUpdate = true;
                }
            }
        }
    }
}


//-----------------------------------------------------------------------------
// calcValues
//
template<typename T>
void ShareEvaluator<T>::calcValues() {

    if (m_pPL != NULL) {

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
void ShareEvaluator<T>::exchangeAndCumulate() {

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
//  tries to read the attributes
//    ATTR_SHAREEVAL_ARRAYNAME (decorated with an ID)
//    ATTR_SHAREEVAL_POLYNAME (decorated with an ID)
//  and creates a PolyLine
//
template<typename T>
int ShareEvaluator<T>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (this->m_pPL != NULL) {
        delete (this->m_pPL);
    }

    std::string sKeyArrName  = xha_sprintf(ATTR_SHAREEVAL_ARRAYNAME, m_sID);
    std::string sKeyPolyName = xha_sprintf(ATTR_SHAREEVAL_POLYNAME, m_sID);

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, sKeyArrName, m_sArrayName);
        if (iResult != 0) {
            LOG_ERROR("[ShareEvaluator] couldn't read attribute [%s]", sKeyArrName);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, sKeyPolyName, m_sPolyName);
        if (iResult != 0) {
            LOG_ERROR("[ShareEvaluator] couldn't read attribute [%s]", sKeyPolyName);
        }
    }

    if (!m_sPolyName.empty()) {
        xha_printf("ShareEvaluator::extractAttributesQDF will work on %s\n", m_sPolyName);
        m_pPL = qdf_createPolyLine(hSpeciesGroup, m_sPolyName);
        if (m_pPL != NULL) {
            if (m_pPL->m_iNumSegments == 0) {
                delete m_pPL;
                m_pPL = NULL;
            }
            iResult = 0;
        } else {
            LOG_ERROR("[ShareEvaluator] couldn't read attribute [%s]", m_sPolyName);
            iResult = -1;
        }
    } else {
        xha_printf("ShareEvaluator::extractAttributesQDF: empty poly name\n");
        iResult = -1;
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
//
template<typename T>
int ShareEvaluator<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    std::string sKeyArrName  = xha_sprintf(ATTR_SHAREEVAL_ARRAYNAME, m_sID);
    std::string sKeyPolyName = xha_sprintf(ATTR_SHAREEVAL_POLYNAME, m_sID);

    //    xha_printf("writing att [%s]:%s\n", sKeyArrName, m_sArrayName);
    iResult += qdf_insertSAttribute(hSpeciesGroup, sKeyArrName, m_sArrayName);
    //    xha_printf("writing att [%s]:%s\n", sKeyPolyName, m_sPolyName);
    iResult += qdf_insertSAttribute(hSpeciesGroup, sKeyPolyName, m_sPolyName);
    if (!m_sPolyName.empty()) {
        //        xha_printf("writing poly [%s]:%s\n", sKeyPolyName, m_sPolyName);fflush(stdout);
        iResult += qdf_writePolyLine(hSpeciesGroup, m_pPL, m_sPolyName);
    }
    return iResult;

}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int ShareEvaluator<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;
    

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        std::string sKeyArrName  = xha_sprintf(ATTR_SHAREEVAL_ARRAYNAME, m_sID);
        std::string sKeyPolyName = xha_sprintf(ATTR_SHAREEVAL_POLYNAME, m_sID);

        iResult += getAttributeStr(mParams,  sKeyArrName, m_sArrayName);  
        iResult += getAttributeStr(mParams,  sKeyPolyName, m_sPolyName);  
        if ((iResult == 0) && (!m_sPolyName.empty())) {
            m_pPL = PolyLine::readFromString(m_sPolyName);
            if (m_pPL == NULL) {
                iResult = -1;
            }   
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool ShareEvaluator<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    ShareEvaluator<T>* pSE = static_cast<ShareEvaluator<T>*>(pAction);

    if (m_sID != pSE->m_sID) {
        bEqual = false;
    } 
    return bEqual;
}

/*
//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool ShareEvaluator<T>::isEqual(Evaluator *pEval, bool bStrict) {
    bool bEqual = false;
    ShareEvaluator<T>* pA = static_cast<ShareEvaluator<T>*>(pEval);
    if (strcmp(m_pID, pA->m_pID) == 0) {
        bEqual = true;
    } 
    return bEqual;
}
*/
//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void ShareEvaluator<T>::showAttributes() {

    std::string sKeyArrName = xha_sprintf(ATTR_SHAREEVAL_ARRAYNAME, m_sID);
    xha_printf("  %s\n", sKeyArrName);
    std::string sKeyPolyName = xha_sprintf(ATTR_SHAREEVAL_POLYNAME, m_sID);
    xha_printf("  %s\n", sKeyPolyName);
}
