#ifndef __INDEXCOLLECTOR_H__
#define __INDEXCOLLECTOR_H__

#include <string>
#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_INDEXCOLLECTOR_NAME     "IndexCollector"


template<typename T>
class IndexCollector : public Action<T> {
public:
    IndexCollector(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, const char *pShareName);
    virtual ~IndexCollector();

    int preLoop();
    int initialize(float fTime);
    
    void setShareName(const std::string sShareName);

    bool isEqual(Action<T> *pAction, bool bStrict);
    

protected:
    std::string       m_sShareName;
    std::vector<int> *m_avLocalIndexes;

    omp_lock_t* m_aIndexLocks;

};

#endif

