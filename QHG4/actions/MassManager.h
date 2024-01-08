#ifndef __MASSMANAGER_H__
#define __MASSMANAGER_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_MASSMANAGER_NAME         = "MassManager";
const static std::string ATTR_MASSMANAGER_MIN_NAME     = "MM_minmass";
const static std::string ATTR_MASSMANAGER_MAX_NAME     = "MM_maxmass";
const static std::string ATTR_MASSMANAGER_DELTA_NAME   = "MM_deltamass";

class MassInterface;

template<typename T>
class MassManager : public Action<T> {
public:
    MassManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);

    int preLoop();
    int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

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
