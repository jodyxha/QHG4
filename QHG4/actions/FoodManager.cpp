
#include "MessLoggerT.h"
#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SCellGrid.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"


#include "FoodManager.h"

#define DS_FOOD_ACTUAL_AMOUNT "ActalFoodAmount"
//-----------------------------------------------------------------------------
// constructor
//  the array pdFoodAvailable is an output array
//    (initialization from m_pVegetation->m_adTotalNPP)
//  the array pAgentCounts is an inpout array
//   
template<typename T> 
FoodManager<T>::FoodManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *pdFoodAvailable, int *pAgentCounts) 
    : Action<T>(pPop, pCG, ATTR_FOODMAN_NAME, sID),
    m_apWELL(apWELL),
    m_iNumCells(pCG->m_iNumCells),
    m_pAgentCounts(pAgentCounts),
    m_pFoodArray(pdFoodAvailable),
    m_pActualFood(NULL),
    m_pFoodMaxAmount(NULL),
    m_dFoodGrowthRate(0),
    m_dFoodMinAmount(0), 
    m_pTotalRequests(NULL),
    m_bMaxAmountsLoaded(false) {

    m_pActualFood    = new double[m_iNumCells];
    m_pTotalRequests = new double[m_iNumCells];
}



//-----------------------------------------------------------------------------
// destructor
//
template<typename T> 
FoodManager<T>::~FoodManager() {

    if (m_pActualFood != NULL) {
        delete[] m_pActualFood;
    }
    if (m_pTotalRequests != NULL) {
        delete[] m_pTotalRequests;
    }
}

//-----------------------------------------------------------------------------
// preLoop
//
template<typename T> 
int FoodManager<T>::preLoop() {
    if (!m_bMaxAmountsLoaded) {
        createRandomMaxAmounts();
#pragma omp parallel for
        for (int i = 0; i < m_iNumCells; i++) {
            m_pActualFood[i] = m_pFoodMaxAmount[i];
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// initialize
//
template<typename T> 
int FoodManager<T>::initialize(float fT) {

    m_mAllRequests.clear();
   
    
    // calculate available food
    memset(m_pFoodArray, 0, m_iNumCells*sizeof(float));
#pragma omp parallel for
    for (int i = 0; i < m_iNumCells; i++) {
        double dF = m_pActualFood[i] - m_dFoodMinAmount;
        m_pFoodArray[i] = (dF < 0) ? 0 : dF;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_FOODMAN_GROWTH_RATE_NAME
//    ATTR_FOODMAN_MIN_AMOUNT_NAME
//
template<typename T> 
int FoodManager<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FOODMAN_GROWTH_RATE_NAME, 1, &m_dFoodGrowthRate);
        if (iResult != 0) {
            LOG_ERROR("[FoodManager<T>::extractAttributesQDF] couldn't read attribute [%s]", ATTR_FOODMAN_GROWTH_RATE_NAME);
        }
    }
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_FOODMAN_MIN_AMOUNT_NAME, 1, &m_dFoodMinAmount);
        if (iResult != 0) {
            LOG_ERROR("[FoodManager<T>::extractAttributesQDF] couldn't read attribute [%s]", ATTR_FOODMAN_MIN_AMOUNT_NAME);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_FOODMAN_GROWTH_RATE_NAME
//    ATTR_FOODMAN_MIN_AMOUNT_NAME
//
template<typename T> 
int FoodManager<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FOODMAN_GROWTH_RATE_NAME, 1, &m_dFoodGrowthRate);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_FOODMAN_MIN_AMOUNT_NAME,  1, &m_dFoodMinAmount);
 
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//
template<typename T> 
int FoodManager<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {  
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_FOODMAN_GROWTH_RATE_NAME, &m_dFoodGrowthRate);         
        iResult += getAttributeVal(mParams, ATTR_FOODMAN_MIN_AMOUNT_NAME,  &m_dFoodMinAmount);
 
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
template<typename T>
int FoodManager<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    iResult = qdf_writeArray(hSpeciesGroup, DS_FOOD_ACTUAL_AMOUNT, m_iNumCells, m_pActualFood, H5T_NATIVE_DOUBLE);
    if (iResult != 0) {
        LOG_ERROR("[ FoodManager<T>::writeAdditionalDataQDF] couldn't write ChildManager data");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//  MaxAmount should be handled by vegetation (mis-use NPP)
//  Actual amount should be saved
//
template<typename T>
int FoodManager<T>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    std::vector<hsize_t> vSizes;
    iResult = qdf_getDataExtents(hSpeciesGroup, DS_FOOD_ACTUAL_AMOUNT, vSizes);
    if (iResult == 0) {
        if ((int)vSizes[0] == m_iNumCells) {
    
            iResult = qdf_readArray(hSpeciesGroup, DS_FOOD_ACTUAL_AMOUNT, m_iNumCells, m_pActualFood);
            if (iResult == 0) {
                // success
                m_bMaxAmountsLoaded = true;
            } else  {
                LOG_ERROR("[ChildManager<T>::readAdditionalDataQDF] couldn't read food max amount array");
            }
        } else  {
            LOG_ERROR("[ChildManager<T>::readAdditionalDataQDF] array in qdf file has wrong dimension (should be %d, but is %d)\n", m_iNumCells, vSizes[0]);
        }
    } else {
        LOG_ERROR("[ChildManager<T>::readAdditionalDataQDF] couldn't determine data extents for [%s]", DS_FOOD_ACTUAL_AMOUNT);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// registerForFood
//
template<typename T> 
int FoodManager<T>::registerForFood(int iAgentID, int iCellID, double dAmount) {
    int iResult = 0;
    total_requests::iterator it = m_mAllRequests.find(iCellID);
    if (it != m_mAllRequests.end()) {
        it->second.push_back({iAgentID,dAmount});
    } else {
        m_mAllRequests = {{iAgentID, dAmount}};
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// growFood
//  delta_F = r*F(1-F/K)
//
template<typename T> 
int FoodManager<T>::growFood() {
    int iResult = 0;
#pragma omp parallel for
    for (int i = 0; i < m_iNumCells; i++){
        m_pActualFood[i] += m_dFoodGrowthRate*m_pActualFood[i]*(1 - m_pActualFood[i]/m_pFoodMaxAmount[i]);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// handOutFood
//  
template<typename T> 
int FoodManager<T>::handOutFood() {
    int iResult = 0;

    memset(m_pTotalRequests, 0, m_iNumCells*sizeof(double));
    total_requests::const_iterator it;
    for (it = m_mAllRequests.begin(); it != m_mAllRequests.end(); ++it) {
        for (uint k = 0; k < it->second.size(); ++k) {
            m_pTotalRequests[it->first] += it->second[k].second;
        }
    }

#pragma omp parallel for
    for (uint iC = 0; iC < m_iNumCells; ++iC) {
        const single_requests &v = m_mAllRequests[iC];
        if (m_pTotalRequests[iC] < m_pFoodArray[iC]) { 
            for (uint k = 0; k < v.size(); ++k) {
                // hand out requested amounts (single_request=(agentid, amount)
                // this should be thread safe since agent ids are unique
                T *pa = &(this->m_pPop->m_aAgents[v[k].first]);
                //@@TODO maybe need upper limit for food intake, or
                //@@TODO agents never request for too much
                //@@TODO maybe feeding to be done by different object ("health")   
                pa.health += v[k].second;
            }       
            // remove total requested amount from m_pActualFood[iC]
            m_pActualFood[iC] -= m_pTotalRequests[iC];
        } else {
            double fAmount = m_pFoodArray[iC];
            for (uint k = 0; k < v.size(); ++k) {
                // hand out fAmount/v.size()
                // this should be thread safe since agent ids are unique
                T *pa = &(this->m_pPop->m_aAgents[v[k].first]);
                //@@TODO maybe need upper limit for food intake, or
                //@@TODO agents never request for too much
                //@@TODO maybe feeding to be done by different object ("health")   
                pa.health += v[k].second;
            }       
            // remove fAMount from m_pActualFood[iC]
            m_pActualFood[iC] -= fAmount;
        } 
        
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// createRandomMaxAmounts
//
template<typename T>
void FoodManager<T>::createRandomMaxAmounts() {
#pragma omp parallel for
    for (int i = 0; i < m_iNumCells; i++) {
        float f = this->m_apWELL[omp_get_thread_num()]->wrandr(m_dFoodMinAmount, 1.0);
        m_pFoodMaxAmount[i] = f;
        //@@TODO maybe later: patchy instead of noisy (or mixed)
    }

}

//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool FoodManager<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    FoodManager<T>* pA = static_cast<FoodManager<T>*>(pAction);
    if ((m_dFoodGrowthRate != pA->m_dFoodGrowthRate) ||
        (m_dFoodMinAmount != pA->m_dFoodMinAmount)) {
        bEqual = false;
    }
    return bEqual;
}
