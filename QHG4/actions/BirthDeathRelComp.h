#ifndef __BIRTHDEATHRELCOMP_H__
#define __BIRTHDEATHRELCOMP_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "LinearBirthRel.h"
#include "LinearDeathRel.h"

#define ATTR_BIRTHDEATHRELCOMP_NAME        "BirthDeathRelComp"
#define ATTR_BIRTHDEATHRELCOMP_B0_NAME     "BirthDeathRelComp_b0"
#define ATTR_BIRTHDEATHRELCOMP_D0_NAME     "BirthDeathRelComp_d0"
#define ATTR_BIRTHDEATHRELCOMP_THETA_NAME  "BirthDeathRelComp_theta"
#define ATTR_BIRTHDEATHRELCOMP_THIS_NAME   "BirthDeathRelComp_this"
#define ATTR_BIRTHDEATHRELCOMP_OTHER_NAME  "BirthDeathRelComp_other"

template<typename T>
class BirthDeathRelComp : public Action<T> { 

public:
    BirthDeathRelComp(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double *adK, int iStride);
   ~BirthDeathRelComp();
    
    int preLoop();
    
    int initialize(float fTime);
    int execute(int iA, float fT);
    int finalize(float fTime);
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetAttributes(const ModuleComplex *pMC);  
   int modifyAttributes(const std::string sAttrName, double dValue);

    bool isEqual(Action<T> *pAction, bool bStrict);
    
 

protected:
    static int NUM_BIRTHDEATHRELCOMP_PARAMS;
protected:
    double    m_dB0;
    double    m_dD0;
    double    m_dTheta;
    double   *m_adK;
    int       m_iStride;
    int       m_iNumSetParams;
    WELL512 **m_apWELL;
    int     **m_aiNumBirths;
    double   *m_adBirthRates;
    LinearBirthRel<T> *m_pLB;
    LinearDeathRel<T> *m_pLD;
    
    std::string m_sThis;
    std::string m_sOther;
    ulong *m_pTotalCounts;
    ulong *m_pOtherCounts;
    bool m_bReadDone;

    static const char *asNames[];
};

#endif
