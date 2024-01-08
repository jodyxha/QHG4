#ifndef __HUNTING_H__
#define __HUNTING_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_HUNTING_NAME         = "Hunting";
const static std::string ATTR_HUNTING_PREYSPECIES_NAME   = "Hunting_preyspecies";
const static std::string ATTR_HUNTING_EFFICIENCY_NAME    = "Hunting_efficiency";
const static std::string ATTR_HUNTING_USABILITY_NAME     = "Hunting_usability";

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
    int execute(int iA, float fT);
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

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
