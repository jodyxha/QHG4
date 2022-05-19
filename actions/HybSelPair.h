#ifndef __HYBSELPAIR_H__
#define __HYBSELPAIR_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_HYBSELPAIR_NAME "HybSelPair"

class WELL512;

typedef std::pair<int, float> hybidx;
typedef std::vector<hybidx>   hybidxvec;

template<typename T>
class HybSelPair : public Action<T> {
    
 public:
    HybSelPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    virtual ~HybSelPair();
    int initialize(float fT);
    int finalize(float fT);

    bool isEqual(Action<T> *pAction, bool bStrict);
 
 protected:
    int findMates();
    hybidxvec *m_vLocFemalesID;
    hybidxvec *m_vLocMalesID;

    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;

};

#endif
