#ifndef __WEIGHTEDMOVERAND_H__
#define __WEIGHTEDMOVERAND_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"

#define ATTR_WEIGHTEDMOVERAND_NAME      "WeightedMoveRand"
#define ATTR_WEIGHTEDMOVERAND_PROB_NAME "WeightedMoveRand_prob"

template<typename T>
class WeightedMoveRand : public Action<T> {
    
 public:
    WeightedMoveRand(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights);
    ~WeightedMoveRand();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);
 
 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;
    double m_dMoveProb;

};

#endif
