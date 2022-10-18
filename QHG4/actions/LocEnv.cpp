#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>


#include "MessLoggerT.h"
#include "EventConsts.h"
#include "WELLUtils.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "SPopulation.h"
#include "Action.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "NPPCalc.h"
#include "NPPCalcMiami.h"
#include "LocEnv.h"

#define MAX_PL_NAME 512
#define MAX_PL_DATA 512

const double REG_OCEANIA_LONMIN = 115.0;
const double REG_OCEANIA_LONMAX = 150.0;
const double REG_OCEANIA_LATMIN = -12.0;
const double REG_OCEANIA_LATMAX =   1.0;

template<typename T>
const stringvec LocEnv<T>::s_vNames = {
    ATTR_LOCENV_WATERFACTOR_NAME + "_sapiens",
    ATTR_LOCENV_COASTALFACTOR_NAME +  "_sapiens",
    ATTR_LOCENV_COASTAL_MIN_LAT_NAME + "_sapiens",
    ATTR_LOCENV_COASTAL_MAX_LAT_NAME + "_sapiens",
    ATTR_LOCENV_NPPMIN_NAME + "_sapiens",
    ATTR_LOCENV_NPPMAX_NAME + "_sapiens",
    ATTR_LOCENV_KMAX_NAME + "_sapiens",
    ATTR_LOCENV_KMIN_NAME + "_sapiens",
    ATTR_LOCENV_ALT_PREF_POLY_NAME + "_sapiens",

    ATTR_LOCENV_WATERFACTOR_NAME + "_neander",
    ATTR_LOCENV_COASTALFACTOR_NAME + "_neander",
    ATTR_LOCENV_COASTAL_MIN_LAT_NAME + "_neander",
    ATTR_LOCENV_COASTAL_MAX_LAT_NAME + "_neander",
    ATTR_LOCENV_NPPMIN_NAME + "_neander",
    ATTR_LOCENV_NPPMAX_NAME + "_neander",
    ATTR_LOCENV_KMAX_NAME + "_neander",
    ATTR_LOCENV_KMIN_NAME + "_neander",
    ATTR_LOCENV_ALT_PREF_POLY_NAME + "_neander",
};

template<typename T>
const std::string LocEnv<T>::asSpecies[] = {"sapiens", "neander"};
  


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
LocEnv<T>::LocEnv(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, WELL512** apWELL, LBController *pAgentController) 
    : Action<T>(pPop, pCG, ATTR_LOCENV_NAME,sID),
      m_pAgentController(pAgentController),
    m_bNeedUpdate(true),
      m_pNPPCalc(NULL) {
    
    m_iNumThreads = omp_get_max_threads();
    m_iLocArrSize = this->m_pCG->m_iConnectivity + 1;    

    this->m_pPop->addObserver(this);

    m_pNPPCalc = new NPPCalcMiami(apWELL);

    this->m_vNames.insert(this->m_vNames.end(), s_vNames.begin(), s_vNames.end());

    m_pAltPrefPoly[0] = NULL;
    m_pAltPrefPoly[1] = NULL;

    m_adCapacities[0] = NULL;
    m_adCapacities[1] = NULL;
}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
LocEnv<T>::~LocEnv() {
    delete[] m_adCapacities[0];
    delete[] m_adCapacities[1];

    delete m_pAltPrefPoly[0]; 
    delete m_pAltPrefPoly[1]; 

    if (m_pNPPCalc != NULL) {
        delete m_pNPPCalc;
    }

}


//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void LocEnv<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if ((iEvent == EVENT_ID_GEO) || 
        (iEvent == EVENT_ID_CLIMATE) || 
        (iEvent == EVENT_ID_VEG)) { 
        m_bNeedUpdate = true;
    
    } else if (iEvent == EVENT_ID_FLUSH) {
        // no more events: recalculate
        recalculateGlobalCapacities();
    }
}

//-----------------------------------------------------------------------------
// getArr
//
template<typename T>
const double *LocEnv<T>::getArr(int iAgentIndex) {
    return  &(m_aEnvVals[iAgentIndex]);
}

//-----------------------------------------------------------------------------
// init
//
template<typename T>
int LocEnv<T>::init() {
    int iResult = -1;
    stdprintf("init called\n");
    
    if ((this->m_pCG != NULL) &&
        (this->m_pCG->m_pGeography  != NULL) &&
        (this->m_pCG->m_pClimate    != NULL) &&
        (this->m_pCG->m_pVegetation != NULL)) {

        int iNumCells = this->m_pCG->m_iNumCells; 
        m_adCapacities[0] = new double[iNumCells];
        m_adCapacities[1] = new double[iNumCells];
    

        // initialize the buffer ...
        stdprintf("initializing m_aEnvVals with (%d, %d)\n", m_pAgentController->getLayerSize(), m_iLocArrSize);
        m_aEnvVals.init(m_pAgentController->getLayerSize(), m_iLocArrSize);
        
        // ... and add it to the AgentController
        iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aEnvVals));
        if (iResult == 0) {
            // everything OK
        } else {
            stdprintf("Couldn't add buffer EnvVals to controller\n");
        }
    } else {
        if (this->m_pCG == NULL) {
            stdprintf("[LocEnv] No cell grid!\n");
        } else {
            if (this->m_pCG->m_pGeography  == NULL) {
                stdprintf("[LocEnv] Cellgrid has no geography\n");
            }
            if (this->m_pCG->m_pClimate    == NULL) {
                stdprintf("[LocEnv] Cellgrid has no climate\n");
            }
            if (this->m_pCG->m_pVegetation == NULL) {
                stdprintf("[LocEnv] Cellgrid has no vegetation\n");
            }
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// recalculate
//
template<typename T>
int LocEnv<T>::recalculateGlobalCapacities() {
    if (m_bNeedUpdate) {
        stdprintf("LocEnv::recalculateGlobalCapacities\n");
        int iNumCells = this->m_pCG->m_iNumCells; 
        double *dT = this->m_pCG->m_pClimate->m_adAnnualMeanTemp;
        double *dP = this->m_pCG->m_pClimate->m_adAnnualRainfall;
        double *dW = this->m_pCG->m_pGeography->m_adWater;
        double *dNPP = this->m_pCG->m_pVegetation->m_adBaseANPP;
        double *dNPPTot = this->m_pCG->m_pVegetation->m_adTotalANPP;
        double *dAlt = this->m_pCG->m_pGeography->m_adAltitude;
        // fill m_adCapacities with npp miami-values for current climate

        for (int iGroup = 0; iGroup < 2; iGroup++) {
            memset(m_adCapacities[iGroup], 0, iNumCells*sizeof(double));
            m_pNPPCalc->calcNPP(dT, dP, iNumCells, 1, m_adCapacities[iGroup], NULL);

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
                        if (dNPP[i] < m_dNPPMin[iGroup]) {
                            dTempNPP = m_adCapacities[iGroup][i];
                        } 
                    }

                
                    dTempNPP *= dAltFactor;

                    // apply ramp function to get carrying capacity
                    // (similar to  Eriksson2012)
                    if (dTempNPP < m_dNPPMin[iGroup]) {
                        dTempCap = m_dKMin[iGroup];
                    } else if (dTempNPP > m_dNPPMax[iGroup]) {
                        dTempCap = m_dKMax[iGroup];
                    } else {
                        dTempCap = m_dKMin[iGroup]+ dTempNPP*m_dKMax[iGroup]/(m_dNPPMax[iGroup] - m_dNPPMin[iGroup]);
                    }
                    
                    // add coastal bonus
                    if  ((this->m_pCG->m_pGeography->m_abCoastal[i]) &&
                         (this->m_pCG->m_pGeography->m_adLatitude[i] > m_dCoastalMinLatitude[iGroup]) &&
                         (this->m_pCG->m_pGeography->m_adLatitude[i] < m_dCoastalMaxLatitude[iGroup])) {
                        dTempCap += m_dCoastalFactor[iGroup]*m_dKMax[iGroup];
                    }
                    
                    // add water bonus
                    dTempCap += dW[i]*m_dWaterFactor[iGroup]*m_dKMax[iGroup];
                    
                    // cutoff at m_dmaxCapacity
                    if (dTempCap > m_dKMax[iGroup]) {
                        dTempCap = m_dKMax[iGroup];
                    }
                } else {
                    dTempCap = 0;
                }
                
                // store value in m_adCapacities (for use in Evaluators or VerhulstVarK etc)
                m_adCapacities[iGroup][i] = dTempCap;
                dNPPTot[i] = dTempCap;
                            
            }
        }
        m_bNeedUpdate = false;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int LocEnv<T>::preLoop() {
    int iResult = 0;
    recalculateGlobalCapacities();
    return iResult;
}


//----------------------------------------------------------------------------
// initialize
//   calculates the hyb-weighted parameter averages for all agents
//
template<typename T>
int LocEnv<T>::initialize(float fTime) { 
    int iResult = 0;
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        T pA = this->m_pPop->m_aAgents[iA];

        double *pLocArr =  &(m_aEnvVals[iA]);

        float fH     = pA.m_fHybridization;
        int iCurCell = pA.m_iCellIndex;
        for (int iN = 0; iN <= this->m_pCG->m_iConnectivity; iN++) {
            int iC = iCurCell;
            if (iN <= this->m_pCG->m_aCells[iCurCell].m_iNumNeighbors) {
                if (iN > 0) {
                    iC = this->m_pCG->m_aCells[iC].m_aNeighbors[iN-1];
                }
                pLocArr[iN] = (1-fH)*m_adCapacities[0][iC] + fH*m_adCapacities[1][iC]; 
                
                if (pLocArr[iN] < 0) {
                    pLocArr[iN]  = 0;
                }
            } else {
                pLocArr[iN]  = 0;
            }
        }
        this->m_pPop->m_aAgents[iA].m_dCC = pLocArr[0];
       
        // combine with alt
        for (int iN = 0; iN <= this->m_pCG->m_aCells[iCurCell].m_iNumNeighbors; iN++) {
            int iC = iCurCell;
            if (iN > 0) {
                iC = this->m_pCG->m_aCells[iC].m_aNeighbors[iN-1];
            }
            
            double fAltFactor = (1-fH)*m_pAltPrefPoly[0]->getVal(this->m_pCG->m_pGeography->m_adAltitude[iC]) +
                                    fH*m_pAltPrefPoly[1]->getVal(this->m_pCG->m_pGeography->m_adAltitude[iC]);
            // here without interpolation
            fAltFactor = m_pAltPrefPoly[0]->getVal((float)this->m_pCG->m_pGeography->m_adAltitude[iC]);
            if ((fAltFactor < 0) || (this->m_pCG->m_pGeography->m_abIce[iC]>0)) {  
                fAltFactor = 0;
            }  
            pLocArr[iN] *= fAltFactor;
            
        }

        // accumulate
        for (int iN = 1; iN < m_iLocArrSize; iN++) {
            pLocArr[iN] += pLocArr[iN -1];
        }
    }

    return iResult; 
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//   tries to load the attributes
//      ATTR_LOCENV_WATERFACTOR_NAME
//      ATTR_LOCENV_COASTALFACTOR_NAME
//      ATTR_LOCENV_COASTAL_MIN_LAT_NAME
//      ATTR_LOCENV_COASTAL_MAX_LAT_NAME
//      ATTR_LOCENV_NPPMIN_NAME
//      ATTR_LOCENV_NPPMAX_NAME
//      ATTR_LOCENV_KMAX_NAME
//      ATTR_LOCENV_KMIN_NAME
//      ATTR_LOCENV_EFFICIENCY_NAME
//   each in 2 
template<typename T>
int LocEnv<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    std::string sNameID;
    const char *sMask = "%s_%s";

    for (int iGroup = 0; (iResult == 0) && (iGroup < 2); iGroup++) {
        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_WATERFACTOR_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dWaterFactor[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }

        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTALFACTOR_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dCoastalFactor[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }
    

        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTAL_MIN_LAT_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dCoastalMinLatitude[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }

        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTAL_MAX_LAT_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dCoastalMaxLatitude[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }

        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_NPPMIN_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dNPPMin[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }

        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_NPPMAX_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dNPPMax[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }

        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_KMAX_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dKMax[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }


        if (iResult == 0) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_KMIN_NAME, asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_dKMin[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[LocEnv] couldn't read attribute [%s]", sNameID);
            }
        }


        if (iResult == 0) {
            
             m_sPLParName[iGroup] = stdsprintf(sMask, ATTR_LOCENV_ALT_PREF_POLY_NAME, asSpecies[iGroup]);
             stdprintf("SingleEvaluator::extractAttributesQDF will work on %s\n", m_sPLParName[iGroup]);
             m_pAltPrefPoly[iGroup] = qdf_createPolyLine(hSpeciesGroup, m_sPLParName[iGroup]);
             if (m_pAltPrefPoly[iGroup] == NULL) {
                 iResult = -1;
             }
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//   tries to write the attributes
//      ATTR_LOCENV_VEGSELECTION_NAME
//      ATTR_LOCENV_WATERFACTOR_NAME
//      ATTR_LOCENV_COASTALFACTOR_NAME
//      ATTR_LOCENV_COASTAL_MIN_LAT_NAME
//      ATTR_LOCENV_COASTAL_MAX_LAT_NAME
//      ATTR_LOCENV_NPPMIN_NAME
//      ATTR_LOCENV_NPPMAX_NAME
//      ATTR_LOCENV_KMAX_NAME
//      ATTR_LOCENV_KMIN_NAME
//      ATTR_LOCENV_EFFICIENCY_NAME
//
template<typename T>
int LocEnv<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    std::string sNameID;
    const char *sMask = "%s_%s";

 
    //    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_LOCENV_VEGSELECTION_NAME, 3, m_adSelection);
    for (int iGroup = 0; (iResult == 0) && (iGroup < 2); iGroup++) {
        sNameID = stdsprintf(sMask, ATTR_LOCENV_WATERFACTOR_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_dWaterFactor[iGroup]));

        sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTALFACTOR_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_dCoastalFactor[iGroup]));
        sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTAL_MIN_LAT_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_dCoastalMinLatitude[iGroup]));
        sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTAL_MAX_LAT_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup,sNameID , 1, &(m_dCoastalMaxLatitude[iGroup]));
        sNameID = stdsprintf(sMask, ATTR_LOCENV_NPPMIN_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_dNPPMin[iGroup]));
        sNameID = stdsprintf(sMask, ATTR_LOCENV_NPPMAX_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_dNPPMax[iGroup]));
        sNameID = stdsprintf(sMask, ATTR_LOCENV_KMAX_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_dKMax[iGroup]));
        sNameID = stdsprintf(sMask, ATTR_LOCENV_KMIN_NAME, asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_dKMin[iGroup]));

        iResult = qdf_writePolyLine(hSpeciesGroup, m_pAltPrefPoly[iGroup], m_sPLParName[iGroup]);
    }

    
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int LocEnv<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;
    std::string sNameID;
    const char *sMask = "%s_%s";

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
 

        for (int iGroup = 0; (iResult == 0) && (iGroup < 2); iGroup++) {
            sNameID = stdsprintf(sMask, ATTR_LOCENV_WATERFACTOR_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dWaterFactor[iGroup]));            
            sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTALFACTOR_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dCoastalFactor[iGroup]));        
            sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTAL_MIN_LAT_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dCoastalMinLatitude[iGroup])); 
            sNameID = stdsprintf(sMask, ATTR_LOCENV_COASTAL_MAX_LAT_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dCoastalMaxLatitude[iGroup])); 
            sNameID = stdsprintf(sMask, ATTR_LOCENV_NPPMIN_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dNPPMin[iGroup]));                      
            sNameID = stdsprintf(sMask, ATTR_LOCENV_NPPMAX_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dNPPMax[iGroup]));                      
            sNameID = stdsprintf(sMask, ATTR_LOCENV_KMAX_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dKMax[iGroup]));                          
            sNameID = stdsprintf(sMask, ATTR_LOCENV_KMIN_NAME, asSpecies[iGroup]);
            iResult += getAttributeVal(mParams, sNameID, &(m_dKMin[iGroup]));        

            std::string sPolyDesc = "";
            m_sPLParName[iGroup] = stdsprintf(sMask, ATTR_LOCENV_ALT_PREF_POLY_NAME, asSpecies[iGroup]);
            iResult = getAttributeStr(mParams, m_sPLParName[iGroup],  sPolyDesc);
            if (iResult == 0) {
                m_pAltPrefPoly[iGroup] = PolyLine::readFromString(sPolyDesc);
                if (m_pAltPrefPoly[iGroup] == NULL) {
                    iResult = -1;
                }  
            }
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool LocEnv<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    LocEnv<T>* pA = static_cast<LocEnv<T>*>(pAction);
    for (int iGroup = 0; iGroup < 2; iGroup++) {
        if ((m_dWaterFactor[iGroup]        != pA->m_dWaterFactor[iGroup]) ||
            (m_dCoastalFactor[iGroup]      != pA->m_dCoastalFactor[iGroup]) ||
            (m_dCoastalMinLatitude[iGroup] != pA->m_dCoastalMinLatitude[iGroup]) ||
            (m_dCoastalMaxLatitude[iGroup] != pA->m_dCoastalMaxLatitude[iGroup]) ||
            (m_dNPPMin[iGroup]             != pA->m_dNPPMin[iGroup]) ||
            (m_dNPPMax[iGroup]             != pA->m_dNPPMax[iGroup]) ||
            (m_dKMin[iGroup]               != pA->m_dKMin[iGroup]) ||
            (m_dKMax[iGroup]               != pA->m_dKMax[iGroup])) {
            bEqual &= false;
        } 
    }
    return bEqual;
}

