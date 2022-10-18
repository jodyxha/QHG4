#ifndef __NPERSAHYBLINBIRTHREL_H__
#define __NPERSAHYBLINBIRTHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_NPERSAHYBLINBIRTHREL_NAME            "NPersAHybLinearBirth"
#define ATTR_NPERSAHYBLINBIRTHREL_HYBMINPROB_NAME "HybLinearBirth_hybminprob"

class WELL512;

template<typename T>
class NPersAHybLinearBirthRel : public Action<T> {
    
 public:
    NPersAHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths);
    NPersAHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double dHybMinProb);
    ~NPersAHybLinearBirthRel();

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
