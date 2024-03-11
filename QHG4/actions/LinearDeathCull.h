#ifndef __LINDEATHCULL_H__
#define __LINDEATHCULL_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_LINDEATHCULL_NAME   = "LinearDeathCull";
const static std::string ATTR_LINDEATHCULL_D0_NAME   = "LinearDeathCull_d0";
const static std::string ATTR_LINDEATHCULL_TURNOVER_NAME   = "LinearDeathCull_theta";
const static std::string ATTR_LINDEATHCULL_CAPACITY_NAME   = "LinearDeathCull_K";
const static std::string ATTR_LINDEATHCULL_EPS_NAME        = "LinearDeathCull_eps";

class WELL512;

template<typename T>
class LinearDeathCull : public Action<T> {
    
 public:
    LinearDeathCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    LinearDeathCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double dK, double dEps);
    LinearDeathCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dD0, double dTheta, double *adK, double dEps, int iStride);
    ~LinearDeathCull();

    int initialize(float fT);
    int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
   int modifyAttributes(const std::string sAttrName, double dValue);


    bool isEqual(Action<T> *pAction, bool bStrict);
    


 protected:
    WELL512 **m_apWELL;
    double *m_adD;
    double m_dD0;
    double m_dTheta;
    double m_dK;
    double *m_adK;
    double m_dEps;
    int m_iStride;
    double *m_adCullProb;

public:
    static const std::string asNames[];
};

#endif
