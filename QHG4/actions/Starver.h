#ifndef __STARVER_H__
#define __STARVER_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_STARVER_NAME                = "Starver";
const static std::string ATTR_STARVER_STARVE_MASS_NAME    = "StarveMass";
const static std::string ATTR_STARVER_MASS_DECAY_NAME     = "MassDecay";

template<typename T>
class Starver : public Action<T> {
    
 public:
    Starver(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    virtual ~Starver();

    virtual int execute(int iA, float fT);
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 
protected:
    double m_dStarveMass;
    double m_dMassDecay;

    static const char *asNames[];

};

#endif
