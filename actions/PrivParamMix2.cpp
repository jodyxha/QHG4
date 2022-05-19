#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>

#include "types.h"
#include "clsutils.h"
#include "MessLoggerT.h"
#include "ParamProvider2.h"
#include "QDFUtils.h"
#include "PrivParamMix2.h"

static const std::map<std::string, int> asModeNames = {
    {"none",        MODE_NONE2},
    {"allmother",   MODE_ALLMOTHER2},
    {"allfather",   MODE_ALLFATHER2},
    {"avgparents",  MODE_AVGPARENTS2},
    {"weightedmix", MODE_WEIGHTEDMIX2},
    {"puremix",     MODE_PUREMIX2},
    {"randommix",   MODE_RANDOMMIX2},
};


const stringvec s_vNames = {
     ATTR_PRIVPARAMMIX2_MODE_NAME,
    //     ATTR_PRIVPARAMMIX2_HYBMINPROB_NAME,   
     ATTR_PRIVPARAMMIX2_B0_NAME + "_sapiens",           
     ATTR_PRIVPARAMMIX2_D0_NAME + "_sapiens",           
     ATTR_PRIVPARAMMIX2_THETA_NAME + "_sapiens",        
     ATTR_PRIVPARAMMIX2_OTHER_NAME + "_sapiens",        
     ATTR_PRIVPARAMMIX2_THIS_NAME + "_sapiens",         
     ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME + "_sapiens",     
     ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME + "_sapiens",     
     ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME + "_sapiens",  
     ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME + "_sapiens", 
    /*
     ATTR_PRIVPARAMMIX2_DECAY_NAME + "_sapiens",        
     ATTR_PRIVPARAMMIX2_DIST0_NAME + "_sapiens",        
     ATTR_PRIVPARAMMIX2_PROB0_NAME + "_sapiens",        
     ATTR_PRIVPARAMMIX2_MINDENS_NAME + "_sapiens",      
    */
     ATTR_PRIVPARAMMIX2_MAXAGE_NAME + "_sapiens",       
     ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME + "_sapiens",  

     ATTR_PRIVPARAMMIX2_B0_NAME + "_neander",           
     ATTR_PRIVPARAMMIX2_D0_NAME + "_neander",           
     ATTR_PRIVPARAMMIX2_THETA_NAME + "_neander",        
     ATTR_PRIVPARAMMIX2_OTHER_NAME + "_neander",        
     ATTR_PRIVPARAMMIX2_THIS_NAME + "_neander",         
     ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME + "_neander",     
     ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME + "_neander",     
     ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME + "_neander",  
     ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME + "_neander", 
    /*
     ATTR_PRIVPARAMMIX2_DECAY_NAME + "_neander",        
     ATTR_PRIVPARAMMIX2_DIST0_NAME + "_neander",        
     ATTR_PRIVPARAMMIX2_PROB0_NAME + "_neander",        
     ATTR_PRIVPARAMMIX2_MINDENS_NAME + "_neander",      
    */
     ATTR_PRIVPARAMMIX2_MAXAGE_NAME + "_neander",       
     ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME + "_neander",  
};


template<typename T>
const std::string PrivParamMix2<T>::s_asSpecies[] = {"sapiens", "neander"};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PrivParamMix2<T>::PrivParamMix2(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, WELL512** apWELL) 
    : Action<T>(pPop, pCG,  ATTR_PRIVPARAMMIX2_NAME,sID) {
    
    this->m_vNames.insert(this->m_vNames.end(), s_vNames.begin(), s_vNames.end());
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PrivParamMix2<T>::~PrivParamMix2() {
}

//-----------------------------------------------------------------------------
// extractParamsQDF
//
//   tries to load the attributes
//       ATTR_PRIVPARAMMIX2_MODE_NAME
//       ATTR_PRIVPARAMMIX2_NAME              
//       ATTR_PRIVPARAMMIX2_MODE_NAME         
//       ATTR_PRIVPARAMMIX2_BO_NAME           
//       ATTR_PRIVPARAMMIX2_D0_NAME           
//       ATTR_PRIVPARAMMIX2_THETA_NAME        
//     ( ATTR_PRIVPARAMMIX2_OTHER_NAME)        
//     ( ATTR_PRIVPARAMMIX2_THIS_NAME)         
//       ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME     
//       ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME     
//       ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME  
//       ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME 
//     ( ATTR_PRIVPARAMMIX2_DECAY_NAME)        
//     ( ATTR_PRIVPARAMMIX2_DIST0_NAME)        
//     ( ATTR_PRIVPARAMMIX2_PROB0_NAME)        
//     ( ATTR_PRIVPARAMMIX2_MINDENS_NAME)      
//       ATTR_PRIVPARAMMIX2_MAXAGE_NAME       
//       ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME  
//
template<typename T>
int PrivParamMix2<T>::extractParamsQDF(hid_t hSpeciesGroup) {
   
    int iResult = 0;
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_PRIVPARAMMIX2_MODE_NAME, 1, &m_iMode);
        if (iResult != 0) {
            LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]",  ATTR_PRIVPARAMMIX2_MODE_NAME);
        }
    }

    std::string sNameID;
    const char *sMask = "%s_%s";

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup,  ATTR_PRIVPARAMMIX2_MODE_NAME, 1, &m_iMode);
        if (iResult != 0) {
            LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]",  ATTR_PRIVPARAMMIX2_MODE_NAME);
        }
    }

    for (int iGroup = 0; (iResult == 0) && (iGroup < 2); iGroup++) {

        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_B0_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adB0[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_D0_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adD0[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_THETA_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adTheta[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adFertMinAge[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adFertMaxAge[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adInterbirth[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_MAXAGE_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adMaxAge[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adUncertainty[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }
        if (iResult == 0) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME, s_asSpecies[iGroup]);
            iResult = qdf_extractAttribute(hSpeciesGroup, sNameID, 1, &(m_adMoveProb[iGroup]));
            if (iResult != 0) {
                LOG_ERROR("[PrivParamMix2] couldn't read attribute [%s]", sNameID);
            }
        }

    }
    return iResult;
} 


//-----------------------------------------------------------------------------
// writeParamsQDF
//
//   tries to write the attributes
//       ATTR_PRIVPARAMMIX2_MODE_NAME         
//       ATTR_PRIVPARAMMIX2_B0_NAME           
//       ATTR_PRIVPARAMMIX2_D0_NAME           
//       ATTR_PRIVPARAMMIX2_THETA_NAME        
//     ( ATTR_PRIVPARAMMIX2_OTHER_NAME)        
//     ( ATTR_PRIVPARAMMIX2_THIS_NAME)         
//       ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME     
//       ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME     
//       ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME  
//       ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME 
//     ( ATTR_PRIVPARAMMIX2_DECAY_NAME)        
//     ( ATTR_PRIVPARAMMIX2_DIST0_NAME)        
//     ( ATTR_PRIVPARAMMIX2_PROB0_NAME)        
//     ( ATTR_PRIVPARAMMIX2_MINDENS_NAME)      
//       ATTR_PRIVPARAMMIX2_MAXAGE_NAME       
//       ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME  
//
template<typename T>
int PrivParamMix2<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    std::string sNameID;
    const char *sMask = "%s_%s";

    iResult += qdf_insertAttribute(hSpeciesGroup,  ATTR_PRIVPARAMMIX2_MODE_NAME, 1, &m_iMode);
    
    for (int iGroup = 0; (iResult == 0) && (iGroup < 2); iGroup++) {
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_B0_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adB0[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_D0_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adD0[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_THETA_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adTheta[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adFertMaxAge[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adFertMinAge[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adInterbirth[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adMoveProb[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_MAXAGE_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adMaxAge[iGroup]));
        sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME, s_asSpecies[iGroup]);
        iResult += qdf_insertAttribute(hSpeciesGroup, sNameID, 1, &(m_adUncertainty[iGroup]));
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetParams
//   
//
template<typename T>
int PrivParamMix2<T>::tryGetParams(const ModuleComplex *pMC) {
    int iResult = -1;
    std::string sNameID;
    const char *sMask = "%s_%s";

    const stringmap &mParams = pMC->getParams();
    if (this->checkParams(mParams) == 0) {
        iResult = 0;
         iResult += getParamVal(mParams,  ATTR_PRIVPARAMMIX2_MODE_NAME, &m_iMode);            

        for (int iGroup = 0; (iResult == 0) && (iGroup < 2); iGroup++) {
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_B0_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adB0[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_D0_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adD0[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_THETA_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adTheta[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adFertMaxAge[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adFertMinAge[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adInterbirth[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adMoveProb[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_MAXAGE_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adMaxAge[iGroup]));            
            sNameID = stdsprintf(sMask,  ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME, s_asSpecies[iGroup]);
            iResult += getParamVal(mParams, sNameID, &(m_adUncertainty[iGroup]));            
        }
        
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int PrivParamMix2<T>::preLoop() {
    int iResult = 0;
    switch (m_iMode) {
    case MODE_ALLFATHER2:
        curFunc = &PrivParamMix2::allFather;
        break;
    case MODE_ALLMOTHER2:
        curFunc = &PrivParamMix2::allMother;
        break;
    case MODE_AVGPARENTS2:
        curFunc = &PrivParamMix2::averageParents;
        break;
    case MODE_WEIGHTEDMIX2:
        curFunc = &PrivParamMix2::weightedMix;
        break;
    case MODE_PUREMIX2:
        curFunc = &PrivParamMix2::weightedPureMix;
        break;
    case MODE_RANDOMMIX2:
        curFunc = &PrivParamMix2::randomMix;
        break;
    default:
        stdprintf("Unknown mode %d\n", m_iMode);
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// calcParams
//
template<typename T>
void PrivParamMix2<T>::calcParams(const T &tMother, const T &tFather, T &tChild, double dParam) {
    (this->*curFunc)(tMother, tFather, tChild, dParam);
}


//-----------------------------------------------------------------------------
// allMother
//   inherit parameters from mother
//
template<typename T>
void PrivParamMix2<T>::allMother(const T &tMother, const T &tFather, T &tChild, double dParam) {
        // for WeightedMove
    tChild.m_dMoveProb         = tMother.m_dMoveProb;
    //  for OldAgeDeath
    tChild.m_dMaxAge           = tMother.m_dMaxAge;
    tChild.m_dUncertainty      = tMother.m_dUncertainty;
    // for Fertility
    tChild.m_fFertilityMinAge  = tMother.m_fFertilityMinAge;
    tChild.m_fFertilityMaxAge  = tMother.m_fFertilityMaxAge;
    tChild.m_fInterbirth       = tMother.m_fInterbirth;
    // for HybBiirthDeathRel
    tChild.m_dB0               = tMother.m_dB0;
    tChild.m_dD0               = tMother.m_dD0;
    tChild.m_dTheta            = tMother.m_dTheta;

    tChild.m_fParentalHybridization = tMother.m_fParentalHybridization;
}


//-----------------------------------------------------------------------------
// allFather
//  inherit parameters from father
//
template<typename T>
void PrivParamMix2<T>::allFather(const T &tMother, const T &tFather, T &tChild, double dParam) {
        // for WeightedMove
    tChild.m_dMoveProb         = tFather.m_dMoveProb;
    //  for OldAgeDeath
    tChild.m_dMaxAge           = tFather.m_dMaxAge;
    tChild.m_dUncertainty      = tFather.m_dUncertainty;
    // for Fertility
    tChild.m_fFertilityMinAge  = tFather.m_fFertilityMinAge;
    tChild.m_fFertilityMaxAge  = tFather.m_fFertilityMaxAge;
    tChild.m_fInterbirth       = tFather.m_fInterbirth;
    // for HybBirthDeathRel
    tChild.m_dB0               = tFather.m_dB0;
    tChild.m_dD0               = tFather.m_dD0;
    tChild.m_dTheta            = tFather.m_dTheta;

    tChild.m_fParentalHybridization = tFather.m_fParentalHybridization;
}


//-----------------------------------------------------------------------------
// averageParents
//  the child's parameters are the exact average of the parents' parameters
//
template<typename T>
void PrivParamMix2<T>::averageParents(const T &tMother, const T &tFather, T &tChild, double dParam) {
    tChild.m_dMoveProb         =  (tMother.m_dMoveProb        + tFather.m_dMoveProb)/2.0;;
    //  for OldAgeDeath
    tChild.m_dMaxAge           =  (tMother.m_dMaxAge          + tFather.m_dMaxAge)/2.0;
    tChild.m_dUncertainty      =  (tMother.m_dUncertainty     + tFather.m_dUncertainty)/2.0;
    // for Fertility
    tChild.m_fFertilityMinAge  =  (tMother.m_fFertilityMinAge + tFather.m_fFertilityMinAge)/2.0;
    tChild.m_fFertilityMaxAge  =  (tMother.m_fFertilityMaxAge + tFather.m_fFertilityMaxAge)/2.0;
    tChild.m_fInterbirth       =  (tMother.m_fInterbirth      + tFather.m_fInterbirth)/2.0;
    // for HybBirthDeathRel
    tChild.m_dB0               =  (tMother.m_dB0              + tFather.m_dB0)/2.0;
    tChild.m_dD0               =  (tMother.m_dD0              + tFather.m_dD0)/2.0;
    tChild.m_dTheta            =  (tMother.m_dTheta           + tFather.m_dTheta)/2.0;

    tChild.m_fParentalHybridization = (tMother.m_fParentalHybridization + tFather.m_fParentalHybridization)/2.0;

}


//-----------------------------------------------------------------------------
// weightedMix
//   the child's parameters are a weighted mix of the parents' parameters 
//
template<typename T>
void PrivParamMix2<T>::weightedMix(const T &tMother, const T &tFather, T &tChild, double dParam) {
    tChild.m_dMoveProb         =  (1-dParam)*tMother.m_dMoveProb        + dParam*tFather.m_dMoveProb;
    //  for OldAgeDeath
    tChild.m_dMaxAge           =  (1-dParam)*tMother.m_dMaxAge          + dParam*tFather.m_dMaxAge;
    tChild.m_dUncertainty      =  (1-dParam)*tMother.m_dUncertainty     + dParam*tFather.m_dUncertainty;
    // for Fertility
    tChild.m_fFertilityMinAge  =  (1-dParam)*tMother.m_fFertilityMinAge + dParam*tFather.m_fFertilityMinAge;
    tChild.m_fFertilityMaxAge  =  (1-dParam)*tMother.m_fFertilityMaxAge + dParam*tFather.m_fFertilityMaxAge;
    tChild.m_fInterbirth       =  (1-dParam)*tMother.m_fInterbirth      + dParam*tFather.m_fInterbirth;
    // for HybBirthDeathRel
    tChild.m_dB0               =  (1-dParam)*tMother.m_dB0              + dParam*tFather.m_dB0;
    tChild.m_dD0               =  (1-dParam)*tMother.m_dD0              + dParam*tFather.m_dD0;
    tChild.m_dTheta            =  (1-dParam)*tMother.m_dTheta           + dParam*tFather.m_dTheta;

    tChild.m_fParentalHybridization = (1-dParam)*tMother.m_fParentalHybridization + dParam*tFather.m_fParentalHybridization;
}


//-----------------------------------------------------------------------------
// weightedPureMix
//   the child's parameters are a weighted mix of the pure sapiens/neander parameters 
//
template<typename T>
void PrivParamMix2<T>::weightedPureMix(const T &tMother, const T &tFather, T &tChild, double dParam) {
    tChild.m_dMoveProb         =  (1-dParam)*m_adMoveProb[0]        + dParam*m_adMoveProb[1];
    //  for OldAgeDeath
    tChild.m_dMaxAge           =  (1-dParam)*m_adMaxAge[0]          + dParam*m_adMaxAge[1];
    tChild.m_dUncertainty      =  (1-dParam)*m_adUncertainty[0]     + dParam*m_adUncertainty[1];
    // for Fertility
    tChild.m_fFertilityMinAge  =  (1-dParam)*m_adFertMinAge[0]      + dParam*m_adFertMinAge[1];
    tChild.m_fFertilityMaxAge  =  (1-dParam)*m_adFertMaxAge[0]      + dParam*m_adFertMaxAge[1];
    tChild.m_fInterbirth       =  (1-dParam)*m_adInterbirth[0]      + dParam*m_adInterbirth[1];
    // for HybBirthDeathRel
    tChild.m_dB0               =  (1-dParam)*m_adB0[0]              + dParam*m_adB0[1];
    tChild.m_dD0               =  (1-dParam)*m_adD0[0]              + dParam*m_adD0[1];
    tChild.m_dTheta            =  (1-dParam)*m_adTheta[0]           + dParam*m_adTheta[1];

    tChild.m_fParentalHybridization = dParam;
}


//-----------------------------------------------------------------------------
// randomMix
//
template<typename T>
void PrivParamMix2<T>::randomMix(const T &tMother, const T &tFather, T &tChild, double dParam) {
    if ((2.0*rand())/(1.0+RAND_MAX) > 1) {
        allFather(tMother, tFather, tChild, dParam);
    } else {
        allMother(tMother, tFather, tChild, dParam);
    }
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool PrivParamMix2<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = true;
    PrivParamMix2<T>* pA = static_cast<PrivParamMix2<T>*>(pAction);
    if (m_iMode != pA->m_iMode) {
        bEqual &= false;
    }
    for (int iGroup = 0; iGroup < 2; iGroup++) {
        if ((m_adMoveProb[iGroup] != pA->m_adMoveProb[iGroup]) ||
            (m_adMaxAge[iGroup] != pA->m_adMaxAge[iGroup]) ||
            (m_adUncertainty[iGroup] != pA->m_adUncertainty[iGroup]) ||
            (m_adFertMaxAge[iGroup] != pA->m_adFertMaxAge[iGroup]) ||
            (m_adFertMinAge[iGroup] != pA->m_adFertMinAge[iGroup]) ||
            (m_adInterbirth[iGroup] != pA->m_adInterbirth[iGroup]) ||
            (m_adB0[iGroup] != pA->m_adB0[iGroup]) ||
            (m_adD0[iGroup] != pA->m_adD0[iGroup]) ||
            (m_adTheta[iGroup] != pA->m_adTheta[iGroup])) {
            bEqual &= false;
        }

    }
    return bEqual;
}
