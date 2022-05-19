#ifndef __PERSHYBLINBIRTHREL_H__
#define __PERSHYBLINBIRTHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_PERSHYBLINBIRTHREL_NAME            "PersHybLinearBirth"
#define ATTR_PERSHYBLINBIRTHREL_HYBMINPROB_NAME "PersHybLinearBirth_hybminprob"

class WELL512;

template<typename T>
class PersHybLinearBirthRel : public Action<T> {
    
 public:
    PersHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths);
    PersHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double dHybMinProb, double* adK, int iStride);
    ~PersHybLinearBirthRel();

    int preLoop();
    int postLoop();
    int initialize(float fT);
    int operator()(int iA, float fT);
    int finalize(float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
   int modifyParams(const std::string sAttrName, double dValue);


    bool isEqual(Action<T> *pAction, bool bStrict);
    
    void setAgentNumArray(ulong *pAgentNums) { m_pNumAgentsPerCell = pAgentNums;};

 protected:
    WELL512 **m_apWELL;
    double    m_dHybMinProb;
    double   *m_adK;
    int       m_iStride;
    int       m_iNumThreads; 
    int     **m_aiNumBirths;
    int     **m_aaiNumTemp;
    int       m_iWhich;
    ulong    *m_pNumAgentsPerCell;

    float     m_fHybScale;
    float     m_fHybShift;
    double   *m_adBBase;
    static const char *asNames[];
};

#endif
