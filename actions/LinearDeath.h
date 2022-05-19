#ifndef __LINDEATH_H__
#define __LINDEATH_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_LINDEATH_NAME "LinearDeath"
#define ATTR_LINDEATH_D0_NAME "LinearDeath_d0"
#define ATTR_LINDEATH_TURNOVER_NAME "LinearDeath_theta"
#define ATTR_LINDEATH_CAPACITY_NAME "LinearDeath_K"

class WELL512;

template<typename T>
class LinearDeath : public Action<T> {
    
 public:
    LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double dK);
    LinearDeath(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double *adK, int iStride);
    ~LinearDeath();

    int initialize(float fT);
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
   int modifyParams(const std::string sAttrName, double dValue);

    bool isEqual(Action<T> *pAction, bool bStrict);



 protected:
    WELL512 **m_apWELL;
    double *m_adD;
    double m_dD0;
    double m_dTheta;
    double m_dK;
    double *m_adK;
    int m_iStride;

    static const char *asNames[];
};

#endif
;
