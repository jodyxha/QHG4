#ifndef __NPERSHYBBIRTHDEATHREL_H__
#define __NPERSHYBBIRTHDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "NPersHybLinearBirthRel.h"
#include "NPersLinearDeathRel.h"

#define ATTR_NPERSHYBBIRTHDEATHREL_NAME             "NPersHybBirthDeathRel"
#define ATTR_NPERSHYBBIRTHDEATHREL_HYBMINPROB_NAME  "HybBirthDeathRel_hybminprob"

template<typename T>
class NPersHybBirthDeathRel : public Action<T> { 

public:
    NPersHybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirth);
   ~NPersHybBirthDeathRel();

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
    static int NUM_NPERSHYBBIRTHDEATHREL_PARAMS;
protected:
    double    m_dHybMinProb;
    bool      m_bUseLog ;
    int       m_iNumSetParams;
    WELL512 **m_apWELL;
    int     **m_aiNumBirths;
    NPersHybLinearBirthRel<T> *m_pLB;
    NPersLinearDeathRel<T> *m_pLD;
    ulong    *m_pNumAgentsPerCell;

    static const char *asNames[];    
};

#endif
