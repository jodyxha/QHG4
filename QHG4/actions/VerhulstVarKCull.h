#ifndef __VERHULSTVARKCULL_H__
#define __VERHULSTVARKCULL_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "LinearBirth.h"
#include "LinearDeathCull.h"

const static std::string ATTR_VERHULSTVARKCULL_NAME            = "VerhulstVarKCull";
const static std::string ATTR_VERHULSTVARKCULL_B0_NAME         = "VerhulstVarKCull_b0";
const static std::string ATTR_VERHULSTVARKCULL_D0_NAME         = "VerhulstVarKCull_d0";
const static std::string ATTR_VERHULSTVARKCULL_TURNOVER_NAME   = "VerhulstVarKCull_theta";
const static std::string ATTR_VERHULSTVARKCULL_EPS_NAME        = "VerhulstVarKCull_eps";

template<typename T>
class VerhulstVarKCull : public Action<T> { 

public:
    VerhulstVarKCull(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adK, int iStride);
   ~VerhulstVarKCull();

    int initialize(float fTime);
    int execute(int iA, float fT);
    int finalize(float fTime);
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);

 

protected:
    static int NUM_VERHULSTVARKCULL_PARAMS;
protected:
    double m_dB0;
    double m_dD0;
    double m_dTheta;
    double* m_adK;
    double m_dEps;
    int m_iStride;
    int m_iNumSetParams;
    WELL512 **m_apWELL;
    LinearBirth<T> *m_pLB;
    LinearDeathCull<T> *m_pLD;
  
    static const char*asNames[];
};

#endif
