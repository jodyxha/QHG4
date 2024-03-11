#ifndef __CONDWEIGHTEDMOVE_H__
#define __CONDWEIGHTEDMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"
#include "MoveCondition.h"
#include "Geography.h"

const static std::string ATTR_CONDWEIGHTEDMOVE_NAME   = "CondWeightedMove";
const static std::string ATTR_CONDWEIGHTEDMOVE_PROB_NAME   = "CondWeightedMove_prob";



template<typename T>
class CondWeightedMove : public Action<T> {
    
 public:
    CondWeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights, MoveCondition *pMC);
    ~CondWeightedMove();
    int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
   int modifyAttributes(const std::string sAttrName, double dValue);

    bool isEqual(Action<T> *pAction, bool bStrict);
    

 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;
    double m_dMoveProb;
    MoveCondition *m_pMC;

    Geography *m_pGeography;

public:
    static const std::string asNames[];
};

#endif
