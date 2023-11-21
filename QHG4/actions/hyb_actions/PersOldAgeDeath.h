#ifndef __PERSOLDAGEDEATH_H__
#define __PERSOLDAGEDEATH_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_PERSOLDAGEDEATH_NAME "PersOldAgeDeath"

class WELL512;

template<typename T>
class PersOldAgeDeath : public Action<T> {
    
 public:
    PersOldAgeDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    ~PersOldAgeDeath();

    int execute(int iA, float fT);

    bool isEqual(Action<T> *pAction, bool bStrict);
    
protected:
    WELL512 **m_apWELL;

};

#endif
