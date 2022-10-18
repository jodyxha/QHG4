#ifndef __NPERSRANDOMMOVE_H__
#define __NPERSRANDOMMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_NPERSRANDOMMOVE_NAME "NPersRandomMove"

class WELL512;

template<typename T>
class NPersRandomMove : public Action<T> {
    
 public:
    NPersRandomMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    ~NPersRandomMove();
    int execute(int iA, float fT);

    bool isEqual(Action<T> *pAction, bool bStrict);
    
 
 protected:
    WELL512 **m_apWELL;
};

#endif
