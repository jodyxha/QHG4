#ifndef __GETOLD_H__
#define __GETOLD_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_GETOLD_NAME "GetOld"


template<typename T>
class GetOld : public Action<T> {
    
 public:
    GetOld(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    ~GetOld();
    int execute(int iA, float fT);
    
    bool isEqual(Action<T> *pAction, bool bStrict);
    
  
};

#endif
