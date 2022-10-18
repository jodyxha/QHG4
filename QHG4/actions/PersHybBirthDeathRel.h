#ifndef __PERSHYBBIRTHDEATHREL_H__
#define __PERSHYBBIRTHDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PersHybLinearBirthRel.h"
#include "PersLinearDeathRel.h"

#define ATTR_PERSHYBBIRTHDEATHREL_NAME             "PersHybBirthDeathRel"
#define ATTR_PERSHYBBIRTHDEATHREL_HYBMINPROB_NAME  "PersHybBirthDeathRel_hybminprob"
#define ATTR_PERSHYBBIRTHDEATHREL_GROUPASS_NAME    "PersHybBirthDeathRel_groupass"

template<typename T>
class PersHybBirthDeathRel : public Action<T> { 

public:
    PersHybBirthDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double *adK, int iStride);
   ~PersHybBirthDeathRel();

    virtual int preLoop(); 

    virtual int initialize(float fTime);
    virtual int execute(int iA, float fT);
    virtual int finalize(float fTime);
    
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetAttributes(const ModuleComplex *pMC);
    virtual int modifyAttributes(const std::string sAttrName, double dValue);
   
    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    double  getGroupAssProb() {return m_dGroupAssProb;};
 

protected:
    static int NUM_PERSHYBBIRTHDEATHREL_PARAMS;
protected:
    double    m_dHybMinProb;
    double    m_dGroupAssProb;

    double   *m_adK;
    int       m_iStride;
    int       m_iNumSetParams;
    WELL512 **m_apWELL;
    int     **m_aiNumBirths;
    PersHybLinearBirthRel<T> *m_pLB;
    //@@    HybLinearDeathRel<T> *m_pLD;
    PersLinearDeathRel<T> *m_pLD;
    ulong    *m_pNumAgentsPerCell;

    static const char *asNames[];    
};

#endif
