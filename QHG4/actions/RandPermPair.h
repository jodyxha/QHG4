#ifndef __RANDPERMPAIR_H__
#define __RANDPERMPAIR_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_RANDPERMPAIR_NAME   = "RandPermPair";

class WELL512;

template<typename T>
class RandPermPair : public Action<T> {
    
 public:
    RandPermPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~RandPermPair();
    int initialize(float fT);
    int finalize(float fT);

    bool isEqual(Action<T> *pAction, bool bStrict);
 
 protected:
    int findMates();
    void permute(std::vector<int> &vData, uint iNumSel, int iT);

    WELL512 **m_apWELL;
    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;

    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;

};

#endif
