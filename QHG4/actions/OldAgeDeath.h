#ifndef __OLDAGEDEATH_H__
#define __OLDAGEDEATH_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_OLDAGEDEATH_NAME   = "OldAgeDeath";
const static std::string ATTR_OLDAGEDEATH_MAXAGE_NAME        = "OAD_max_age";
const static std::string ATTR_OLDAGEDEATH_UNCERTAINTY_NAME   = "OAD_uncertainty";

class WELL512;

template<typename T>
class OldAgeDeath : public Action<T> {
    
 public:
    OldAgeDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~OldAgeDeath();

    virtual int execute(int iA, float fT);
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 
protected:
    WELL512 **m_apWELL;
    double m_dMaxAge;
    double m_dUncertainty;

    static const char *asNames[];

};

#endif
