#ifndef __ATANDEATH_H__
#define __ATANDEATH_H__

#include "Action.h"
#include "ParamProvider2.h"

class WELL512;

#define ATTR_ATANDEATH_NAME "AtanDeath"
#define ATTR_ATANDEATH_MAXAGE_NAME "AtanDeath_max_age"
#define ATTR_ATANDEATH_RANGE_NAME  "AtanDeath_range"
#define ATTR_ATANDEATH_SLOPE_NAME  "AtanDeath_slope"

const double EPS = 0.001;


template<typename T>
class AtanDeath : public Action<T> {
    
 public:
    AtanDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~AtanDeath();

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
