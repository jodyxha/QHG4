#ifndef __VERHULST_H__
#define __VERHULST_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "LinearBirth.h"
#include "LinearDeath.h"

#define ATTR_VERHULST_NAME "Verhulst"
#define ATTR_VERHULST_B0_NAME "Verhulst_b0"
#define ATTR_VERHULST_D0_NAME "Verhulst_d0"
#define ATTR_VERHULST_TURNOVER_NAME "Verhulst_theta"
#define ATTR_VERHULST_CAPACITY_NAME "Verhulst_K"

template<typename T>
class Verhulst : public Action<T> { 

public:
    Verhulst(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    ~Verhulst();
    
    virtual int initialize(float fTime);
    virtual int execute(int iA, float fT);
    virtual int finalize(float fTime);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);
   
    virtual bool isEqual(Action<T> *pAction, bool bStrict);
 

protected:
    static int NUM_VERHULST_PARAMS;
protected:
    double m_dB0;
    double m_dD0;
    double m_dTheta;
    double m_dK;
    int m_iNumSetParams;
    WELL512 **m_apWELL;
    LinearBirth<T> *m_pLB;
    LinearDeath<T> *m_pLD;
   
    static const char *asNames[];
};

#endif
