#ifndef __CONDWEIGHTEDMOVE_H__
#define __CONDWEIGHTEDMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"
#include "MoveCondition.h"

#define ATTR_CONDWEIGHTEDMOVE_NAME "CondWeightedMove"
#define ATTR_CONDWEIGHTEDMOVE_PROB_NAME "CondWeightedMove_prob"



template<typename T>
class CondWeightedMove : public Action<T> {
    
 public:
    CondWeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights, MoveCondition *pMC);
    ~CondWeightedMove();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
   int modifyParams(const std::string sAttrName, double dValue);

    bool isEqual(Action<T> *pAction, bool bStrict);
    

 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;
    double m_dMoveProb;
    MoveCondition *m_pMC;
};

#endif
