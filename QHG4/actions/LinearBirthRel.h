#ifndef __LINBIRTHREL_H__
#define __LINBIRTHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_LINBIRTHREL_NAME            = "LinearBirth";
const static std::string ATTR_LINBIRTHREL_B0_NAME         = "LinearBirth_b0";
const static std::string ATTR_LINBIRTHREL_TURNOVER_NAME   = "LinearBirth_theta";

class WELL512;

template<typename T>
class LinearBirthRel : public Action<T> {
    
 public:
    LinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths);
    LinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double dB0, double dTheta, double* adK, int iStride);
    ~LinearBirthRel();

    int preLoop();
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
    double   *m_adK;
    int       m_iStride;
    int       m_iNumThreads; 
    int     **m_aiNumBirths;
    int     **m_aaiNumTemp;
    int       m_iWhich;
    ulong    *m_pNumAgentsPerCell;

    static const std::string asNames[];
};

#endif
