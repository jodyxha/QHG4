#ifndef __HUNTING_H__
#define __HUNTING_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_HUNTING_NAME       "Hunting"
#define ATTR_HUNTING_PREYSPECIES_NAME "Hunting_preyspecies"
#define ATTR_HUNTING_EFFICIENCY_NAME  "Hunting_efficiency"
#define ATTR_HUNTING_USABILITY_NAME   "Hunting_usability"

#define NAME_LEN 1024

class MassInterface;
class PopFinder;

// This action expects both prey and the predator population to implement the MassInterface

template<typename T>
class Hunting : public Action<T> {
public:
    Hunting(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, PopFinder *pPopFinder);
    virtual ~Hunting();

    int preLoop();
    int initialize(float fTime);
    int operator()(int iA, float fT);
    
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);
    
 

protected:
    WELL512 **m_apWELL;
    double m_dEfficiency;
    double m_dUsability;
    MassInterface *m_pMIPrey;
    MassInterface *m_pMIPred;
    PopFinder *m_pPopFinder;
    PopBase   *m_pPreyPop;
    std::string m_sPreyPopName;
  
    std::vector<int> *m_vLocPredIdx;
    std::vector<int> *m_vLocPreyIdx;

    omp_lock_t* m_aPredLocks;
    omp_lock_t* m_aPreyLocks;

    static const char *asNames[];
};


#endif
