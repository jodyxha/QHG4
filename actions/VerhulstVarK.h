#ifndef __VERHULSTVARK_H__
#define __VERHULSTVARK_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "LinearBirth.h"
#include "LinearDeath.h"

#define ATTR_VERHULSTVARK_NAME "VerhulstVarK"
#define ATTR_VERHULST_B0_NAME "Verhulst_b0"
#define ATTR_VERHULST_D0_NAME "Verhulst_d0"
#define ATTR_VERHULST_TURNOVER_NAME "Verhulst_theta"

template<typename T>
class VerhulstVarK : public Action<T> { 

public:
    VerhulstVarK(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adK, int iStride);
    ~VerhulstVarK();
    
    int initialize(float fTime);
    int operator()(int iA, float fT);
    int finalize(float fTime);
    
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetParams(const ModuleComplex *pMC);  
   int modifyParams(const std::string sAttrName, double dValue);
    
    bool isEqual(Action<T> *pAction, bool bStrict);
    
 
    
protected:
    static int NUM_VERHULSTVARK_PARAMS;
protected:
    double m_dB0;
    double m_dD0;
    double m_dTheta;
    double* m_adK;
    int m_iStride;
    int m_iNumSetParams;
    WELL512 **m_apWELL;
    LinearBirth<T> *m_pLB;
    LinearDeath<T> *m_pLD;
    
    static const char *asNames[];
};

#endif
