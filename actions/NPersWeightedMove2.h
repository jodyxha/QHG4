#ifndef __NPERSWEIGHTEDMOVE2_H__
#define __NPERSWEIGHTEDMOVE2_H__

#include "Action.h"
#include "LocEnv2.h"
#include "ParamProvider2.h"

#define ATTR_NPERSWEIGHTEDMOVE2_NAME "NPersWeightedMove2"

template<typename T>
class NPersWeightedMove2 : public Action<T> {
    
 public:
    NPersWeightedMove2(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, LocEnv2<T> *pLE);
    ~NPersWeightedMove2();
    int operator()(int iA, float fT);

    virtual int postLoop();


       bool isEqual(Action<T> *pAction, bool bStrict);
    
 protected:
    WELL512 **m_apWELL;
    LocEnv2<T>    *m_pLE;
};

#endif
