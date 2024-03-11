#ifndef __VERHULSTVARK_H__
#define __VERHULSTVARK_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "LinearBirth.h"
#include "LinearDeath.h"

const static std::string ATTR_VERHULSTVARK_NAME            = "VerhulstVarK";
const static std::string ATTR_VERHULSTVARK_B0_NAME         = "VerhulstVarK_b0";
const static std::string ATTR_VERHULSTVARK_D0_NAME         = "VerhulsVarKt_d0";
const static std::string ATTR_VERHULSTVARK_TURNOVER_NAME   = "VerhulstVarK_theta";

template<typename T>
class VerhulstVarK : public Action<T> { 

public:
    VerhulstVarK(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adK, int iStride);
    virtual ~VerhulstVarK();
    
    virtual int initialize(float fTime);
    virtual int execute(int iA, float fT);
    virtual int finalize(float fTime);
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);
    
    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 
    
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
public:
    static const std::string asNames[];
};

#endif
