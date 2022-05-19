#ifndef __PRIVPARAMMIX2_H__
#define __PRIVPARAMMIX2_H__

// like PrivParamMix, but propagates m_fParentalHybridization in the mix functions

#include "Action.h"
#include "ParamProvider2.h"
#include "WELL512.h"

const static std::string  ATTR_PRIVPARAMMIX2_NAME              = "PrivParamMix";
const static std::string  ATTR_PRIVPARAMMIX2_MODE_NAME         = "PrivParamMix_mode";
const static std::string  ATTR_PRIVPARAMMIX2_B0_NAME           = "NPersHybBirthDeathRel_b0";
const static std::string  ATTR_PRIVPARAMMIX2_D0_NAME           = "NPersHybBirthDeathRel_d0";
const static std::string  ATTR_PRIVPARAMMIX2_THETA_NAME        = "NPersHybBirthDeathRel_theta";
const static std::string  ATTR_PRIVPARAMMIX2_OTHER_NAME        = "NPersHybBirthDeathRel_other";
const static std::string  ATTR_PRIVPARAMMIX2_THIS_NAME         = "NPersHybBirthDeathRel_this";
//const static std::string  ATTR_PRIVPARAMMIX2_HYBMINPROB_NAME   = "HybBirthDeathRel_hybminprob";
const static std::string  ATTR_PRIVPARAMMIX2_FMIN_AGE_NAME     = "Fertility_min_age";
const static std::string  ATTR_PRIVPARAMMIX2_FMAX_AGE_NAME     = "Fertility_max_age";
const static std::string  ATTR_PRIVPARAMMIX2_FINTERBIRTH_NAME  = "Fertility_interbirth";
const static std::string  ATTR_PRIVPARAMMIX2_WEIGHTEDMOVE_NAME = "NPersWeightedMove";
/*
const static std::string  ATTR_PRIVPARAMMIX2_DECAY_NAME        = "Navigate_decay";
const static std::string  ATTR_PRIVPARAMMIX2_DIST0_NAME        = "Navigate_dist0";
const static std::string  ATTR_PRIVPARAMMIX2_PROB0_NAME        = "Navigate_prob0";
const static std::string  ATTR_PRIVPARAMMIX2_MINDENS_NAME      = "Navigate_min_dens";
*/
const static std::string  ATTR_PRIVPARAMMIX2_MAXAGE_NAME       = "OAD_max_age";
const static std::string  ATTR_PRIVPARAMMIX2_UNCERTAINTY_NAME  = "OAD_uncertainty";                 


// it is not very nice to derive LocEns from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

const int MODE_NONE2        = 0;
const int MODE_ALLMOTHER2   = 1;
const int MODE_ALLFATHER2   = 2;
const int MODE_AVGPARENTS2  = 3;
const int MODE_WEIGHTEDMIX2 = 4;
const int MODE_PUREMIX2     = 5;
const int MODE_RANDOMMIX2   = 6;



template<typename T>
class PrivParamMix2 : public Action<T> {
public:
    typedef void(PrivParamMix2::*mixfunctype)(const T &t1, const T &t2, T &t3, double dParam);
    
    PrivParamMix2(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512** apWELL);
    virtual ~PrivParamMix2();

    // get action parameters from QDF 
    virtual int extractParamsQDF(hid_t hSpeciesGroup);
    // write action parameters to QDF
    virtual int writeParamsQDF(hid_t hSpeciesGroup);
    virtual int tryGetParams(const ModuleComplex *pMC);

    // from Action
    virtual int preLoop();
    virtual bool isEqual(Action<T> *pAction, bool bStrict);

    void calcParams(const T &tMother, const T &tFather, T &tChild, double dParam);
    static const std::string s_asSpecies[];

    int getMode() { return m_iMode;};
protected:
    int m_iMode;

    double m_adMoveProb[2];
    double m_adMaxAge[2];
    double m_adUncertainty[2];
    double m_adFertMaxAge[2];
    double m_adFertMinAge[2];
    double m_adInterbirth[2];
    double m_adB0[2];
    double m_adD0[2];
    double m_adTheta[2];

    //   double m_dHybMinProb;
    mixfunctype curFunc;
    void allFather(const T &tMother, const T &tFather, T &tChild, double dParam);
    void allMother(const T &tMother, const T &tFather, T &tChild, double dParam);
    void averageParents(const T &tMother, const T &tFather, T &tChild, double dParam);
    void weightedMix(const T &tMother, const T &tFather, T &tChild, double dParam);
    void weightedPureMix(const T &tMother, const T &tFather, T &tChild, double dParam);
    void randomMix(const T &tMother, const T &tFather, T &tChild, double dParam);


};


#endif
