#include <cstring>
#include <omp.h>

#include "utils.h" /// for dNegInf
#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "EventConsts.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PolyLine.h"
#include "Evaluator.h"
#include "MultiEvaluator.h"

#include "SingleEvaluator.h"  // debuggg

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
MultiEvaluator<T>::MultiEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, evaluatorinfos &mEvaluators, int iMode, bool bDeleteEvaluators)
    : Evaluator<T>(pPop,pCG,ATTR_MULTIEVAL_NAME, sID),  
      m_bDeleteEvaluators(bDeleteEvaluators),
      m_adOutputWeights(adOutputWeights),
      m_bFirst(true),
      m_adSingleEvalWeights(NULL),
      m_acAllowed(NULL) {

    init(mEvaluators, iMode);
}    


//-----------------------------------------------------------------------------
// init
//
template<typename T>
void MultiEvaluator<T>::init(evaluatorinfos &mEvalInfo, int iMode) {
    fprintf(stderr,"creating MultiEvaluator\n");
    m_iMode = iMode;

    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    m_adSingleEvalWeights = new double[this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1)];

    if ((m_iMaxNeighbors != 0) && (m_iMode == MODE_ADD_BLOCK)) {
        m_acAllowed = new uchar[this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1)];
    }

    // mEvalInfo is the map of preference info parameter names
    // and relative arrays of data (e.g. altitude) and weights
    
    m_iNumEvals = (int)mEvalInfo.size();
    m_aEvaluators = new Evaluator<T>*[m_iNumEvals];
    m_vCombinationWeightNames.clear();
    m_adCombinationWeights     = new double[m_iNumEvals];
    
    int iEval = 0;

    typename evaluatorinfos::iterator it;

    for (it = mEvalInfo.begin(); it != mEvalInfo.end(); ++it, ++iEval) {
	        
        m_vCombinationWeightNames.push_back(it->first); 
        
        //bool bCumulate = (m_iMode ==  MultiEvalModes::MODE_MUL_SIMPLE || m_iMode ==  MultiEvalModes::MODE_ADD_BLOCK) ? false : true;
        m_aEvaluators[iEval] = it->second;
        m_aEvaluators[iEval]->setOutputWeights(m_adSingleEvalWeights);
        addObserver(static_cast<Observer*>(m_aEvaluators[iEval]));
        stdprintf("MultiEvaluator added Evaluator for [%s]\n",   m_vCombinationWeightNames[iEval]);
        
        m_adCombinationWeights[iEval] = 0; // initialization to 0
    }

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
MultiEvaluator<T>::~MultiEvaluator() {
    

    if (m_adSingleEvalWeights != NULL) {
        delete[] m_adSingleEvalWeights;
    }
    
    if (m_aEvaluators != NULL) {
        if (m_bDeleteEvaluators) {
            for (int i = 0; i < m_iNumEvals; i++) {
                if (m_aEvaluators[i] != NULL) {
                    delete m_aEvaluators[i];
                }
            }
        }
        delete[] m_aEvaluators;
    }

       
    if (m_adCombinationWeights != NULL) {
        delete[] m_adCombinationWeights;
    }

    if (m_acAllowed != NULL) {
        delete[] m_acAllowed;
    }

}

//-----------------------------------------------------------------------------
// preLoop
//   basically call all evaluatzors' preLoop
//
template<typename T>
int MultiEvaluator<T>::preLoop() {
    int iResult = 0;
    for (int iEval = 0; (iResult == 0) && (iEval < m_iNumEvals); iEval++) {
        iResult = m_aEvaluators[iEval]->preLoop();
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// postLoop
//   basically call all evaluatzors' preLoop
//
template<typename T>
int MultiEvaluator<T>::postLoop() {
    int iResult = 0;
    for (int iEval = 0; (iResult == 0) && (iEval < m_iNumEvals); iEval++) {
        iResult = m_aEvaluators[iEval]->postLoop();
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
// here the weights are calculated if needed
//
template<typename T>
int MultiEvaluator<T>::initialize(float fT) {

    int iResult = 0;

    // previously    if (m_bNeedUpdate || (fT==0)) {
    if (this->m_bNeedUpdate || m_bFirst) {
        m_bFirst = false;
        // printf("[MultiEvaluator::initialize] updating...\n");

        switch (m_iMode) {
        case MultiEvalModes::MODE_ADD_SIMPLE:
            iResult = addSingleWeights(fT);
            break;
        case MultiEvalModes::MODE_ADD_BLOCK:
            iResult = addSingleWeightsBlock(fT);
            break;
        case MultiEvalModes::MODE_MUL_SIMPLE:
            iResult = multiplySingleWeights(fT);
            break;
        case MultiEvalModes::MODE_MAX_SIMPLE:
            iResult = maxSingleWeights(fT);
            break;
        case MultiEvalModes::MODE_MAX_BLOCK:
            iResult = maxSingleWeightsBlock(fT);
            break;
        case MultiEvalModes::MODE_MIN_SIMPLE:
            iResult = minSingleWeights(fT);
            break;
        default:
            stdprintf("Unknown mode [%d]\n", m_iMode);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//   reset m_bNeeedUpdate to false
//
template<typename T>
int MultiEvaluator<T>::finalize(float fT) {
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        m_aEvaluators[iEval]->finalize(fT);
    }
    this->m_bNeedUpdate = false;
    return 0;
}


//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void MultiEvaluator<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    
    notifyObservers(iEvent, pData);

    if (iEvent == EVENT_ID_FLUSH) {   
        this->m_bNeedUpdate = false;
        for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
            this->m_bNeedUpdate |= m_aEvaluators[iEval]->needUpdate();
        }
    }
}


//----------------------------------------------------------------------------
// addEnvWeights
//  add weighted contributions, no blocking
//
template<typename T>
int MultiEvaluator<T>::addSingleWeights(float fT) {
   
    int iResult = 0;
    stdprintf("addEnvWeights\n");

   int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);


    memset(m_adOutputWeights, 0, iArrSize*sizeof(double));
     
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
            
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#pragma omp parallel for
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            m_adOutputWeights[iArrIndex] += m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
        }
        
    }

    return iResult;
}


//----------------------------------------------------------------------------
// addEnvWeightsBlock
//  add weighted contributions, blocking
//
//  ATTENTION: SingleEvaluators must be non-cumulating
//
template<typename T>
int MultiEvaluator<T>::addSingleWeightsBlock(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);
    
    stdprintf("addEnvWeightsBlock\n");
    findBlockings(fT);

    memset(m_adOutputWeights, 0, iArrSize*sizeof(double));
     
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
            
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#pragma omp parallel for
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            if (m_acAllowed[iArrIndex] > 0) {
                m_adOutputWeights[iArrIndex] += m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
            }
        }
    }
    // now let's do the cumulant, since we have been getting non-cumulated values from this Evaluator
#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    


    return iResult;
}


//----------------------------------------------------------------------------
// multiplyEnvWeights
//  multiply contributions, no blocking
//
//  ATTENTION: SingleEvaluator must be non-cumulating
//
template<typename T>
int MultiEvaluator<T>::multiplySingleWeights(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);
    //    printf("multiplyEnvWeights\n");

#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = 1.0;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {

        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#pragma omp parallel for
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            m_adOutputWeights[iArrIndex] *= m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
        }

    }

    // now let's do the cumulant, since we have been getting non-cumulated values from SingleEvaluator
#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// maxEnvWeights
//  max contributions
//
template<typename T>
int MultiEvaluator<T>::maxSingleWeights(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);

    stdprintf("maxEnvWeights\n");

    // set all elements to negative infinity
#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = dNegInf;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#pragma omp parallel for
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            double dValNew = m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
            if (m_adOutputWeights[iArrIndex] < dValNew) {
                m_adOutputWeights[iArrIndex] = dValNew; 
            }
        }
        
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// maxEnvWeightsBlock
//  max contributions
//
//  ATTENTION: SingleEvaluator must be non-cumulating
//
template<typename T>
int MultiEvaluator<T>::maxSingleWeightsBlock(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);

    stdprintf("maxEnvWeightsBlock\n");
    findBlockings(fT);

    // set all elements to negative infinity
#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = dNegInf;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#pragma omp parallel for
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            if (m_acAllowed[iArrIndex] > 0) {
                double dValNew = m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
                if (m_adOutputWeights[iArrIndex] < dValNew) {
                    m_adOutputWeights[iArrIndex] = dValNew; 
                }
            }
        }
        
    }

    // now let's do the cumulant, since we have been getting non-cumulated values from SingleEvaluator
#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// minEnvWeights
//  min contributions
//
template<typename T>
int MultiEvaluator<T>::minSingleWeights(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);
    stdprintf("minEnvWeights\n");

    // set all elements to negative infinity
#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = dPosInf;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#pragma omp parallel for
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            double dValNew = m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
            if (m_adOutputWeights[iArrIndex] > dValNew) {
                m_adOutputWeights[iArrIndex] = dValNew; 
            }
        }
        
    }
    
    // now let's do the cumulant, since we have been getting non-cumulated values from SingleEvaluator
#pragma omp parallel for
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
template<typename T>
int MultiEvaluator<T>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    for (int i = 0; i < m_iNumEvals && iResult == 0; i++) {
        iResult = qdf_extractAttribute(hSpeciesGroup, m_vCombinationWeightNames[i], 1, &m_adCombinationWeights[i]);
        if (iResult == 0) {
            if (true /*|| (this->m_pPop->getQDFVersionIn() >= 4)*/) {
                hid_t hActionGroup = qdf_openGroup(hSpeciesGroup, m_aEvaluators[i]->getActionName());
                if (hActionGroup != H5P_DEFAULT) {
                    iResult = m_aEvaluators[i]->extractAttributesQDF(hActionGroup);
                    qdf_closeGroup(hActionGroup);
                } else {
                    stdprintf("[MultiEvaluator<T>::extractAttributesQDF] WARNING:  Couldn't open subgroup [%s]\n", m_aEvaluators[i]->getActionName());
                } 
            } else {
                //@@ to be removed
                iResult = m_aEvaluators[i]->extractAttributesQDF(hSpeciesGroup);
            }
        } 
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
template<typename T>
int MultiEvaluator<T>::writeAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    for (int i = 0; i < m_iNumEvals && iResult == 0; i++) {

        iResult = qdf_insertAttribute(hSpeciesGroup, m_vCombinationWeightNames[i], 1, &m_adCombinationWeights[i]);
        if (iResult == 0) {

            if (true /*|| (this->m_pPop->getQDFVersionOut() >= 4)*/) {
                hid_t hActionGroup = qdf_opencreateGroup(hSpeciesGroup, m_aEvaluators[i]->getActionName());
                if (hActionGroup != H5P_DEFAULT) {
                    iResult = m_aEvaluators[i]->writeAttributesQDF(hActionGroup);
                    qdf_closeGroup(hActionGroup);
                } else {
                    stdprintf("[MultiEvaluator<T>::writeAttributesQDF]Couldn't create subgroup [%s]\n", m_aEvaluators[i]->getActionName());
                    iResult = -1;
                }
            } else {
                // to be removed
                iResult = m_aEvaluators[i]->writeAttributesQDF(hSpeciesGroup);
            }
        }
    }
    //    printf("finished writing atts\n");

    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int MultiEvaluator<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = 0;
    
    const stringmap &mParams = pMC->getAttributes(); 
    const modulemap &mSubs   = pMC->getSubModules();
    //@@TODO checking of weightnames
    for (int i = 0; (i < m_iNumEvals) && (iResult == 0); i++) {
        iResult = getAttributeVal(mParams, m_vCombinationWeightNames[i], &m_adCombinationWeights[i]);
        if (iResult == 0) {
            modulemap::const_iterator it = mSubs.find(m_aEvaluators[i]->getActionName());
            if (it != mSubs.end()) {
                iResult = m_aEvaluators[i]->tryGetAttributes(it->second);
            } else { 
                stdprintf("Didn't find '%s' in modulemap\n", m_aEvaluators[i]->getActionName());
                iResult = 0; //maybe the action has no parameters to load
            }
        }
    }

    return iResult;
}   


//-----------------------------------------------------------------------------
// findBlockings
//  m_acAllowed[i] is 0 if any of the single evaluator arrays is zero at position i.
//  otherwise, it is 1
//
template<typename T>
int MultiEvaluator<T>::findBlockings(float fT) {
    int iResult = 0;
    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);

    
    memset(m_acAllowed, 0x01, iArrSize*sizeof(uchar));
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {

        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#pragma omp parallel for
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            if (m_adSingleEvalWeights[iArrIndex] <= 0) {
                m_acAllowed[iArrIndex] = 0;
            }
        }
        
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool MultiEvaluator<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    MultiEvaluator<T>* pA = static_cast<MultiEvaluator<T>*>(pAction);
    if (m_iNumEvals == pA->m_iNumEvals) {
        bEqual = true;
        for (int i = 0; (i < m_iNumEvals) && bEqual; i++) {
            if ((m_vCombinationWeightNames[i] == pA->m_vCombinationWeightNames[i]) && 
                (m_adCombinationWeights[i] == pA->m_adCombinationWeights[i]) && 
                m_aEvaluators[i]->isEqual(pA->m_aEvaluators[i], bStrict)) {
                bEqual = true;
            } else {
                bEqual = false;
            }
        }
    }
    return bEqual;
}


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void MultiEvaluator<T>::showAttributes() {
    
    for (int i = 0; i < m_iNumEvals; i++) {
        stdprintf("  %s\n", m_vCombinationWeightNames[i]);
    }

}
