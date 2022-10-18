#ifndef __RANDPAIR_H__
#define __RANDPAIR_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_RANDPAIR_NAME "RandomPair"

class WELL512;

template<typename T>
class RandomPair : public Action<T> {
    
 public:
    RandomPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~RandomPair();
    virtual int initialize(float fT);
    virtual int finalize(float fT);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
 
 protected:
    int findMates();
    WELL512 **m_apWELL;
    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;

    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;

    bool **m_abMales;
    bool **m_abFemales;
    int  *m_aiMalesSize;
    int  *m_aiFemalesSize;
};

#endif
