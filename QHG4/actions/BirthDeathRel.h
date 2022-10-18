#ifndef __BIRTHDEATHREL_H__
#define __BIRTHDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "LinearBirthRel.h"
#include "LinearDeathRel.h"

#define ATTR_BIRTHDEATHREL_NAME        "BirthDeathRel"
#define ATTR_BIRTHDEATHREL_B0_NAME     "BirthDeathRel_b0"
#define ATTR_BIRTHDEATHREL_D0_NAME     "BirthDeathRel_d0"
#define ATTR_BIRTHDEATHREL_THETA_NAME  "BirthDeathRel_theta"

template<typename T>
class BirthDeathRel : public Action<T> { 

public:
    BirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double *adK, int iStride);
   ~BirthDeathRel();

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
    static int NUM_BIRTHDEATHREL_PARAMS;
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
    ulong    *m_pNumAgentsPerCell;

    static const char *asNames[];    
};

#endif
