#ifndef __NPERSAHYBBIRTHDEATHREL_H__
#define __NPERSAHYBBIRTHDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "NPersAHybLinearBirthRel.h"
#include "NPersLinearDeathRel.h"

#define ATTR_NPERSAHYBBIRTHDEATHREL_NAME             "NPersAHybBirthDeathRel"
#define ATTR_NPERSAHYBBIRTHDEATHREL_HYBMINPROB_NAME  "HybBirthDeathRel_hybminprob"

template<typename T>
class NPersAHybBirthDeathRel : public Action<T> { 

public:
    NPersAHybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirth);
   ~NPersAHybBirthDeathRel();

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
    static int NUM_NPERSAHYBBIRTHDEATHREL_PARAMS;
protected:
    double    m_dHybMinProb;
    int       m_iNumSetParams;
    WELL512 **m_apWELL;
    int     **m_aiNumBirths;
    NPersAHybLinearBirthRel<T> *m_pLB;
    NPersLinearDeathRel<T> *m_pLD;
    ulong    *m_pNumAgentsPerCell;

    static const char *asNames[];    
};

#endif
