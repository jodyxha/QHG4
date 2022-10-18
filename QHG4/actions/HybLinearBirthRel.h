#ifndef __HYBLINBIRTHREL_H__
#define __HYBLINBIRTHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_HYBLINBIRTHREL_NAME            "HybLinearBirth"
#define ATTR_HYBLINBIRTHREL_B0_NAME         "HybLinearBirth_b0"
#define ATTR_HYBLINBIRTHREL_TURNOVER_NAME   "HybLinearBirth_theta"
#define ATTR_HYBLINBIRTHREL_HYBMINPROB_NAME "HybLinearBirth_hybminprob"

class WELL512;

typedef struct {
    int iCIndex;
    int iMIndex; 
    int iFIndex;
} birthevent;

template<typename T>
class HybLinearBirthRel : public Action<T> {
    
 public:
    HybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths);
    HybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double dB0, double dTheta, double dHybMinProb, double* adK, int iStride);
    ~HybLinearBirthRel();

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
    double   *m_adB;
    double    m_dB0;
    double    m_dTheta;
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
    static const char *asNames[];
};

#endif
