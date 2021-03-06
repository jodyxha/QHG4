#include <cstring>
#include <cmath>

#include "MessLoggerT.h"
#include "EventConsts.h"
#include "WELL512.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "Action.h"
#include "QDFUtils.h"
#include "Climate.h"
#include "NPPCalc.h"
#include "NPPCalcMiami.h"
#include "NPPCapacity.h"

//#define NPP_LOW_THRESH 32
//#define NPP_CC_CONST   1.04184031637e-5

const double REG_OCEANIA_LONMIN = 115.0;
const double REG_OCEANIA_LONMAX = 150.0;
const double REG_OCEANIA_LATMIN = -12.0;
const double REG_OCEANIA_LATMAX =   1.0;

template<typename T>
const char *NPPCapacity<T>::asNames[] = {//ATTR_NPPCAPACITY_VEGSELECTION_NAME, 
                         ATTR_NPPCAPACITY_WATERFACTOR_NAME,
                         ATTR_NPPCAPACITY_COASTALFACTOR_NAME,
                         ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME,
                         ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME,
                         ATTR_NPPCAPACITY_NPPMIN_NAME,
                         ATTR_NPPCAPACITY_NPPMAX_NAME,
                         ATTR_NPPCAPACITY_KMIN_NAME,
                         ATTR_NPPCAPACITY_KMAX_NAME,
                         ATTR_NPPCAPACITY_EFFICIENCY_NAME};

//#define NPP_CC_CONST   1e-4
//#define NPP_MAX 2600
//#define NPP_MIN  150
//#define K_MIN 2
//----------------------------------------------------------------------------
// constructor 
//
template<typename T>
NPPCapacity<T>::NPPCapacity(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adCapacities, int iStride) 
    : Action<T>(pPop, pCG,ATTR_NPPCAPACITY_NAME,sID),
      m_apWELL(apWELL),
      m_adCapacities(adCapacities),
      m_iStride(iStride),
      m_bNeedUpdate(true),
      m_dWaterFactor(0),
      m_dCoastalFactor(0),
      m_dCoastalMinLatitude(0),
      m_dCoastalMaxLatitude(0),
      m_dNPPMin(0),
      m_dNPPMax(0),
      m_dKMin(0),
      m_dKMax(0),
      m_dEfficiency(1.0), 
      m_pNPPCalc(NULL) {

    memset(m_adSelection, 0, 3*sizeof(double));

    this->m_pPop->addObserver(this);

    m_pNPPCalc = new NPPCalcMiami(m_apWELL);
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//----------------------------------------------------------------------------
// destructor 
//
template<typename T>
NPPCapacity<T>::~NPPCapacity() {
    if (m_pNPPCalc != NULL) {
        delete m_pNPPCalc;
    }
}



//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int NPPCapacity<T>::preLoop() {
    int iResult = 0;
    // we need 
    if ((this->m_pCG->m_pGeography != NULL) && 
        (this->m_pCG->m_pClimate != NULL) && 
        (this->m_pCG->m_pVegetation != NULL)) {
        recalculate();
    } else {
        iResult = -1;
        if (this->m_pCG->m_pGeography != NULL) {
            printf("[NPPCapacity] m_pGeography is NULL!\n");
            printf("  Make sure your gridfile has a geography group!\n");
        }
        if (this->m_pCG->m_pClimate != NULL) {
            printf("[NPPCapacity] m_pClimate is NULL!\n");
            printf("  Make sure your gridfile has a climate group!\n");
        }
        if (this->m_pCG->m_pVegetation != NULL) {
            printf("[NPPCapacity] m_pVegetation is NULL!\n");
            printf("  Make sure your gridfile has a vegetation group!\n");
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void NPPCapacity<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if ((iEvent == EVENT_ID_GEO) || 
        (iEvent == EVENT_ID_CLIMATE) || 
        (iEvent == EVENT_ID_VEG)) { 
        m_bNeedUpdate = true;
    
    } else if (iEvent == EVENT_ID_FLUSH) {
        // no more events: recalculate
        recalculate();
    }
}


//-----------------------------------------------------------------------------
// recalculate
//
template<typename T>
void NPPCapacity<T>::recalculate() {
    if (m_bNeedUpdate) {
        printf("NPPCapacity::recalculate\n");
        int iNumCells = this->m_pCG->m_iNumCells; 
        double *dT = this->m_pCG->m_pClimate->m_adAnnualMeanTemp;
        double *dP = this->m_pCG->m_pClimate->m_adAnnualRainfall;
        double *dW = this->m_pCG->m_pGeography->m_adWater;
        double *dNPP = this->m_pCG->m_pVegetation->m_adBaseANPP;
        double *dNPPTot = this->m_pCG->m_pVegetation->m_adTotalANPP;
        double *dAlt = this->m_pCG->m_pGeography->m_adAltitude;
        // fill m_adCapacities with npp miami-values for current climate
        memset(m_adCapacities, 0, iNumCells*m_iStride*sizeof(double));
        m_pNPPCalc->calcNPP(dT, dP, iNumCells, m_iStride, m_adCapacities, NULL);

#pragma omp parallel for
        for (int i = 0; i < iNumCells; i++) {
            double dTempCap = -1;
            // we only consider land cells
            if (dAlt[i] > 0) {

                float A0 = 1500;
                float A1 = 2500; 
                double dAltFactor =   (dAlt[i] < A0)?1:((A1-dAlt[i])/(A1-A0));
                if (dAltFactor < 0) dAltFactor = 0;    

                double dTempNPP =  dNPP[i];
                // the timmermann npp data does not cover all islands in oceania -
                // most of them have verey small or even zero NPP values.
                // Therefore we use use the miami npp values in a lon-lat-rectangle 
                // encompassing these islands
                if  ((this->m_pCG->m_pGeography->m_adLongitude[i] > REG_OCEANIA_LONMIN) &&
                     (this->m_pCG->m_pGeography->m_adLatitude[i]  > REG_OCEANIA_LATMIN) &&
                     (this->m_pCG->m_pGeography->m_adLongitude[i] < REG_OCEANIA_LONMAX) &&
                     (this->m_pCG->m_pGeography->m_adLatitude[i]  < REG_OCEANIA_LATMAX)) {
                    if (dNPP[i] < m_dNPPMin) {
                        dTempNPP = m_adCapacities[i*m_iStride];
                    } 
                }

                
                dTempNPP *= dAltFactor;

                // apply ramp function to get carrying capacity
                // (similar to  Eriksson2012)
                if (dTempNPP < m_dNPPMin) {
                    dTempCap = m_dKMin;
                } else if (dTempNPP > m_dNPPMax) {
                    dTempCap = m_dKMax;
                } else {
                    dTempCap = m_dKMin+ dTempNPP*m_dKMax/(m_dNPPMax - m_dNPPMin);
                }

                // add coastal bonus
                if  ((this->m_pCG->m_pGeography->m_abCoastal[i]) &&
                     (this->m_pCG->m_pGeography->m_adLatitude[i] > m_dCoastalMinLatitude) &&
                     (this->m_pCG->m_pGeography->m_adLatitude[i] < m_dCoastalMaxLatitude)) {
                    dTempCap += m_dCoastalFactor*m_dKMax;
                }

                // add water bonus
                dTempCap += dW[i]*m_dWaterFactor*m_dKMax;

                // cutoff at m_dmaxCapacity
                if (dTempCap > m_dKMax) {
                    dTempCap = m_dKMax;
                }
            } else {
                dTempCap = 0;
            }
                
            // store value in m_adCapacities (for use in Evaluators or VerhulstVarK etc)
            m_adCapacities[i*m_iStride] = dTempCap*m_dEfficiency;
                
            // store value in Vegetation array (to visualize with VisIt)
            dNPPTot[i] = dTempCap;
        }
                    
        m_bNeedUpdate = false;
    }
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
//   tries to load the attributes
//      ATTR_NPPCAPACITY_WATERFACTOR_NAME
//      ATTR_NPPCAPACITY_COASTALFACTOR_NAME
//      ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME
//      ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME
//      ATTR_NPPCAPACITY_NPPMIN_NAME
//      ATTR_NPPCAPACITY_NPPMAX_NAME
//      ATTR_NPPCAPACITY_KMAX_NAME
//      ATTR_NPPCAPACITY_KMIN_NAME
//      ATTR_NPPCAPACITY_EFFICIENCY_NAME
//
template<typename T>
int NPPCapacity<T>::extractParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    
    /*
    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_VEGSELECTION_NAME, 3, m_adSelection);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_VEGSELECTION_NAME);
        }
    }
    */
    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_WATERFACTOR_NAME, 1, &m_dWaterFactor);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_WATERFACTOR_NAME);
        }
    }

    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_COASTALFACTOR_NAME, 1, &m_dCoastalFactor);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_COASTALFACTOR_NAME);
        }
    }

    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME, 1, &m_dCoastalMinLatitude);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME);
        }
    }

    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME, 1, &m_dCoastalMaxLatitude);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME);
        }
    }

    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_NPPMIN_NAME, 1, &m_dNPPMin);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_NPPMIN_NAME);
        }
    }

    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_NPPMAX_NAME, 1, &m_dNPPMax);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_NPPMAX_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_KMAX_NAME, 1, &m_dKMax);
        if (iResult != 0) {
            iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_OLD_KMAX_NAME, 1, &m_dKMax);
            if (iResult == 0) {
                LOG_WARNING("[NPPCapacity] used old KMax name [%s]", ATTR_NPPCAPACITY_OLD_KMAX_NAME);
            } else {
                LOG_ERROR("[NPPCapacity] couldn't read attributes [%s] or [%s]", ATTR_NPPCAPACITY_KMAX_NAME, ATTR_NPPCAPACITY_OLD_KMAX_NAME);
            }
        }
    }

    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_KMIN_NAME, 1, &m_dKMin);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_KMIN_NAME);
        }
    }

    if (iResult == 0) {
    iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_EFFICIENCY_NAME, 1, &m_dEfficiency);
        if (iResult != 0) {
            LOG_ERROR("[NPPCapacity] couldn't read attribute [%s]", ATTR_NPPCAPACITY_EFFICIENCY_NAME);
        }
    }

    // now that we have the parameters, we can actuall create life and death objects
    
    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
//   tries to write the attributes
//      ATTR_NPPCAPACITY_VEGSELECTION_NAME
//      ATTR_NPPCAPACITY_WATERFACTOR_NAME
//      ATTR_NPPCAPACITY_COASTALFACTOR_NAME
//      ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME
//      ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME
//      ATTR_NPPCAPACITY_NPPMIN_NAME
//      ATTR_NPPCAPACITY_NPPMAX_NAME
//      ATTR_NPPCAPACITY_KMAX_NAME
//      ATTR_NPPCAPACITY_KMIN_NAME
//      ATTR_NPPCAPACITY_EFFICIENCY_NAME
//
template<typename T>
int NPPCapacity<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
   
    //    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_VEGSELECTION_NAME, 3, m_adSelection);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_WATERFACTOR_NAME, 1, &m_dWaterFactor);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_COASTALFACTOR_NAME, 1, &m_dCoastalFactor);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME, 1, &m_dCoastalMinLatitude);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME, 1, &m_dCoastalMaxLatitude);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_NPPMIN_NAME, 1, &m_dNPPMin);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_NPPMAX_NAME, 1, &m_dNPPMax);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_KMAX_NAME, 1, &m_dKMax);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_KMIN_NAME, 1, &m_dKMin);
    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPPCAPACITY_EFFICIENCY_NAME, 1, &m_dEfficiency);

    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetParams
//   
//
template<typename T>
int NPPCapacity<T>::tryGetParams(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getParams();
    if (this->checkParams(mParams) == 0) {
        iResult = 0;
        //        iResult += getParamArr(mParams, ATTR_NPPCAPACITY_VEGSELECTION_NAME, 3,  m_adSelection);             
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_WATERFACTOR_NAME,      &m_dWaterFactor);            
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_COASTALFACTOR_NAME,    &m_dCoastalFactor);        
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME,  &m_dCoastalMinLatitude); 
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME,  &m_dCoastalMaxLatitude); 
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_NPPMIN_NAME,           &m_dNPPMin);                      
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_NPPMAX_NAME,           &m_dNPPMax);                      
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_KMAX_NAME,             &m_dKMax);                          
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_KMIN_NAME,             &m_dKMin);                          
        iResult += getParamVal(mParams, ATTR_NPPCAPACITY_EFFICIENCY_NAME,       &m_dEfficiency);              
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// modifyParams
//
//
template<typename T>
int NPPCapacity<T>::modifyParams(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_NPPCAPACITY_WATERFACTOR_NAME) {
        m_dWaterFactor = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_COASTALFACTOR_NAME) {
        m_dCoastalFactor = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME) {
        m_dCoastalMinLatitude = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME) {
        m_dCoastalMaxLatitude = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_NPPMIN_NAME) {
        m_dNPPMin = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_NPPMAX_NAME) {
        m_dNPPMax = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_KMAX_NAME) {
        m_dKMax = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_KMIN_NAME) {
        m_dKMin = dValue;
    } else if (sAttrName == ATTR_NPPCAPACITY_EFFICIENCY_NAME) {
        m_dEfficiency = dValue;
    } else {
        iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool NPPCapacity<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    NPPCapacity<T>* pA = static_cast<NPPCapacity<T>*>(pAction);
    if ((m_dWaterFactor        == pA->m_dWaterFactor) &&
        (m_dCoastalFactor      == pA->m_dCoastalFactor) &&
        (m_dCoastalMinLatitude == pA->m_dCoastalMinLatitude) &&
        (m_dCoastalMaxLatitude == pA->m_dCoastalMaxLatitude) &&
        (m_dNPPMin             == pA->m_dNPPMin) &&
        (m_dNPPMax             == pA->m_dNPPMax) &&
        (m_dKMin               == pA->m_dKMin) &&
        (m_dKMax               == pA->m_dKMax) &&
        (m_dEfficiency         == pA->m_dEfficiency)) {
        bEqual = true;
    } 
    return bEqual;
}




