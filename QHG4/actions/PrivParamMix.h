#ifndef __PRIVPARAMMIX_H__
#define __PRIVPARAMMIX_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "WELL512.h"

const static std::string ATTR_PRIVPARAMMIX_NAME              = "PrivParamMix";
const static std::string ATTR_PRIVPARAMMIX_MODE_NAME         = "PrivParamMix_mode";
const static std::string ATTR_PRIVPARAMMIX_B0_NAME           = "NPersHybBirthDeathRel_b0";
const static std::string ATTR_PRIVPARAMMIX_D0_NAME           = "NPersHybBirthDeathRel_d0";
const static std::string ATTR_PRIVPARAMMIX_THETA_NAME        = "NPersHybBirthDeathRel_theta";
const static std::string ATTR_PRIVPARAMMIX_OTHER_NAME        = "NPersHybBirthDeathRel_other";
const static std::string ATTR_PRIVPARAMMIX_THIS_NAME         = "NPersHybBirthDeathRel_this";
//const static std::string ATTR_PRIVPARAMMIX_HYBMINPROB_NAME   = "HybBirthDeathRel_hybminprob";
const static std::string ATTR_PRIVPARAMMIX_FMIN_AGE_NAME     = "Fertility_min_age";
const static std::string ATTR_PRIVPARAMMIX_FMAX_AGE_NAME     = "Fertility_max_age";
const static std::string ATTR_PRIVPARAMMIX_FINTERBIRTH_NAME  = "Fertility_interbirth";
const static std::string ATTR_PRIVPARAMMIX_WEIGHTEDMOVE_NAME = "NPersWeightedMove";
/*
const static std::string ATTR_PRIVPARAMMIX_DECAY_NAME        = "Navigate_decay";
const static std::string ATTR_PRIVPARAMMIX_DIST0_NAME        = "Navigate_dist0";
const static std::string ATTR_PRIVPARAMMIX_PROB0_NAME        = "Navigate_prob0";
const static std::string ATTR_PRIVPARAMMIX_MINDENS_NAME      = "Navigate_min_dens";
*/
const static std::string ATTR_PRIVPARAMMIX_MAXAGE_NAME       = "OAD_max_age";
const static std::string ATTR_PRIVPARAMMIX_UNCERTAINTY_NAME  = "OAD_uncertainty";                 


// it is not very nice to derive LocEns from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

const int MODE_NONE        = 0;
const int MODE_ALLMOTHER   = 1;
const int MODE_ALLFATHER   = 2;
const int MODE_AVGPARENTS  = 3;
const int MODE_WEIGHTEDMIX = 4;
const int MODE_PUREMIX     = 5;
const int MODE_RANDOMMIX   = 6;



template<typename T>
class PrivParamMix : public Action<T> {
public:
    typedef void(PrivParamMix::*mixfunctype)(const T &t1, const T &t2, T &t3, double dParam);
    
    PrivParamMix(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512** apWELL);
    virtual ~PrivParamMix();

    // get action parameters from QDF 
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    // write action parameters to QDF
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
    virtual int tryGetAttributes(const ModuleComplex *pMC);

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
