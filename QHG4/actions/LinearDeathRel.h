#ifndef __LINDEATHREL_H__
#define __LINDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_LINDEATHREL_NAME   = "LinearDeathRel";
const static std::string ATTR_LINDEATHREL_D0_NAME   = "LinearDeathRel_d0";
const static std::string ATTR_LINDEATHREL_TURNOVER_NAME   = "LinearDeathRel_theta";
const static std::string ATTR_LINDEATHREL_CAPACITY_NAME   = "LinearDeathRel_K";

class WELL512;

template<typename T>
class LinearDeathRel : public Action<T> {
    
 public:
    LinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths);
    LinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adBirthRates, int **aiNumBirths, double dD0, double dTheta, double *adK, int iStride);
    ~LinearDeathRel();

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
    double   *m_adD;
    double    m_dD0;
    double    m_dTheta;
    double   *m_adK;
    int       m_iStride;
    double   *m_adB;
    int     **m_aiNumBirths;
    int       m_iWhich;
    ulong    *m_pNumAgentsPerCell;

    static const std::string asNames[];
};

#endif
