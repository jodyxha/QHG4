#ifndef __NPERSZHYBLINBIRTHREL_H__
#define __NPERSZHYBLINBIRTHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_NPERSZHYBLINBIRTHREL_NAME            "NPersZHybLinearBirth"
#define ATTR_NPERSZHYBLINBIRTHREL_HYBMINPROB_NAME "HybLinearBirth_hybminprob"

class WELL512;

template<typename T>
class NPersZHybLinearBirthRel : public Action<T> {
    
 public:
    NPersZHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths);
    NPersZHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double dHybMinProb);
    ~NPersZHybLinearBirthRel();

    int preLoop();
    int postLoop();
    int initialize(float fT);
    int execute(int iA, float fT);
    int finalize(float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
   int modifyAttributes(const std::string sAttrName, double dValue);


    bool isEqual(Action<T> *pAction, bool bStrict);
    
    void setAgentNumArray(ulong *pAgentNums) { m_pNumAgentsPerCell = pAgentNums;};

 protected:
    WELL512 **m_apWELL;
    double    m_dHybMinProb;
    int       m_iNumThreads; 
    int     **m_aiNumBirths;
    int     **m_aaiNumTemp;
    int       m_iWhich;
    ulong    *m_pNumAgentsPerCell;

    float     m_fHybScale;
    float     m_fHybShift;
    static const char *asNames[];
};

#endif
