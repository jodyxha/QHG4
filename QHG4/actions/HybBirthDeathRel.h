#ifndef __HYBBIRTHDEATHREL_H__
#define __HYBBIRTHDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "HybLinearBirthRel.h"
#include "LinearDeathRel.h"

#define ATTR_HYBBIRTHDEATHREL_NAME             "HybBirthDeathRel"
#define ATTR_HYBBIRTHDEATHREL_B0_NAME          "HybBirthDeathRel_b0"
#define ATTR_HYBBIRTHDEATHREL_D0_NAME          "HybBirthDeathRel_d0"
#define ATTR_HYBBIRTHDEATHREL_THETA_NAME       "HybBirthDeathRel_theta"
#define ATTR_HYBBIRTHDEATHREL_HYBMINPROB_NAME  "HybBirthDeathRel_hybminprob"

template<typename T>
class HybBirthDeathRel : public Action<T> { 

public:
    HybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double *adK, int iStride);
   ~HybBirthDeathRel();

    virtual int preLoop(); 

    virtual int initialize(float fTime);
    virtual int execute(int iA, float fT);
    virtual int finalize(float fTime);
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetAttributes(const ModuleComplex *pMC);
    virtual int modifyAttributes(const std::string sAttrName, double dValue);
   
    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 

protected:
    static int NUM_HYBBIRTHDEATHREL_PARAMS;
protected:
    double    m_dB0;
    double    m_dD0;
    double    m_dTheta;
    double    m_dHybMinProb;
    double   *m_adK;
    int       m_iStride;
    int       m_iNumSetParams;
    WELL512 **m_apWELL;
    int     **m_aiNumBirths;
    double   *m_adBirthRates;
    HybLinearBirthRel<T> *m_pLB;
    //@@    HybLinearDeathRel<T> *m_pLD;
    LinearDeathRel<T> *m_pLD;
    ulong    *m_pNumAgentsPerCell;

    static const char *asNames[];    
};

#endif
