#ifndef __MASSMANAGER_H__
#define __MASSMANAGER_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_MASSMANAGER_NAME       "MassManager"
#define ATTR_MASSMANAGER_MIN_NAME   "MM_minmass"
#define ATTR_MASSMANAGER_MAX_NAME   "MM_maxmass"
#define ATTR_MASSMANAGER_DELTA_NAME "MM_deltamass"

class MassInterface;

template<typename T>
class MassManager : public Action<T> {
public:
    MassManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);

    int preLoop();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  

    double getMinMass() { return m_dMinMass;};

    bool isEqual(Action<T> *pAction, bool bStrict);
    
 
protected:
    double m_dMinMass;
    double m_dMaxMass;
    double m_dDelta;
    MassInterface *m_pMI;

    static const char *asNames[];
};


#endif
