#ifndef __SIGDEATH_H__
#define __SIGDEATH_H__

#include "Action.h"
#include "ParamProvider2.h"

class WELL512;

const static std::string ATTR_SIGDEATH_NAME   = "SigDeath";
const static std::string ATTR_SIGDEATH_MAXAGE_NAME   = "SigDeath_max_age";
const static std::string ATTR_SIGDEATH_RANGE_NAME    = "SigDeath_range";
const static std::string ATTR_SIGDEATH_SLOPE_NAME    = "SigDeath_slope";

const double EPS = 0.001;


template<typename T>
class SigDeath : public Action<T> {
    
 public:
    SigDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~SigDeath();

    virtual int preLoop();
    virtual int execute(int iA, float fT);
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 
protected:
    WELL512 **m_apWELL;
    double m_dMaxAge;
    double m_dRange;
    double m_dSlope;
    double m_dScale;

    static const char *asNames[];

};

#endif
