#ifndef __PERSFERTILITY_H__
#define __PERSFERTILITY_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_PERSFERTILITY_NAME            "PersFertility"

template<typename T>
class PersFertility : public Action<T> {
    
public:
    PersFertility(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    ~PersFertility();
    int execute(int iA, float fT);

   

    bool isEqual(Action<T> *pAction, bool bStrict);
    
  
protected:

    static const char *asNames[];
};

#endif
