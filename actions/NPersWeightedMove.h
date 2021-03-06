#ifndef __NPERSWEIGHTEDMOVE_H__
#define __NPERSWEIGHTEDMOVE_H__

#include "Action.h"
#include "LocEnv.h"
#include "ParamProvider2.h"

#define ATTR_NPERSWEIGHTEDMOVE_NAME "NPersWeightedMove"

template<typename T>
class NPersWeightedMove : public Action<T> {
    
 public:
    NPersWeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, LocEnv<T> *pLE);
    ~NPersWeightedMove();
    int operator()(int iA, float fT);

    virtual int postLoop();


       bool isEqual(Action<T> *pAction, bool bStrict);
    
 protected:
    WELL512 **m_apWELL;
    LocEnv<T>    *m_pLE;
};

#endif
