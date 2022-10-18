#ifndef __BIRTHER_H__
#define __BIRTHER_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_BIRTHER_NAME       "Birther"
#define ATTR_BIRTHER_ADULTMASS_NAME   "Birther_adultmass"
#define ATTR_BIRTHER_BIRTHMASS_NAME   "Birther_birthmass"
#define ATTR_BIRTHER_UNCERTAINTY_NAME "Birther_uncertainty"

class WELL512;
class MassInterface;


template<typename T>
class Birther : public Action<T> {
public:
    Birther(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);

    int preLoop();
    int execute(int iA, float fT);
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);
    
 

protected:
    WELL512 **m_apWELL;
    double m_dAdultMass;
    double m_dBirthMass;
    double m_dUncertainty;
    MassInterface *m_pMI;

    static const char *asNames[];
};

#endif

