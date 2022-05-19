#ifndef __WEIGHTEDMOVE_H__
#define __WEIGHTEDMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"

#define ATTR_WEIGHTEDMOVE_NAME "WeightedMove"
#define ATTR_WEIGHTEDMOVE_PROB_NAME "WeightedMove_prob"

template<typename T>
class WeightedMove : public Action<T> {
    
 public:
    WeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights);
    ~WeightedMove();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
   int modifyParams(const std::string sAttrName, double dValue);
 
    virtual int postLoop();


    void displayInfo(const char *pPrefix, float fT, int iCellIndex, int iAgentIndex); 
    bool isEqual(Action<T> *pAction, bool bStrict);
    
 
 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;
    double m_dMoveProb;

};

#endif
