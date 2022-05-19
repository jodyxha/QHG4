#ifndef __RANDOMMOVE_H__
#define __RANDOMMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_RANDOMMOVE_NAME "RandomMove"
#define ATTR_RANDOMMOVE_PROB_NAME "RandomMove_prob"

class WELL512;

template<typename T>
class RandomMove : public Action<T> {
    
 public:
    RandomMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    ~RandomMove();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
   int modifyParams(const std::string sAttrName, double dValue);


    bool isEqual(Action<T> *pAction, bool bStrict);
    
 
 protected:
    WELL512 **m_apWELL;
    double m_dMoveProb;
    int **m_apDirCounts;
};

#endif
