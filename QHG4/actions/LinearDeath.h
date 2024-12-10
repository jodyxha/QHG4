#ifndef __LINDEATH_H__
#define __LINDEATH_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_LINDEATH_NAME   = "LinearDeath";
const static std::string ATTR_LINDEATH_D0_NAME   = "LinearDeath_d0";
const static std::string ATTR_LINDEATH_TURNOVER_NAME   = "LinearDeath_theta";
const static std::string ATTR_LINDEATH_CAPACITY_NAME   = "LinearDeath_K";

class WELL512;

template<typename T>
class LinearDeath : public Action<T> {
    
 public:
    LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double dK);
    LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double *adK, int iStride);

    virtual ~LinearDeath();

    virtual int initialize(float fT);
    virtual int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);



 protected:
    WELL512 **m_apWELL;
    double *m_adD;
    double m_dD0;
    double m_dTheta;
    double m_dK;
    double *m_adK;
    int m_iStride;

    static const std::string asNames[];
};

#endif
;
