#ifndef __PERSLINDEATHREL_H__
#define __PERSLINDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_PERSLINDEATHREL_NAME "PersLinearDeathRel"

class WELL512;

template<typename T>
class PersLinearDeathRel : public Action<T> {
    
 public:
    PersLinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths);
    PersLinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double *adK, int iStride);
    ~PersLinearDeathRel();

    int preLoop();
    int initialize(float fT);
    int execute(int iA, float fT);
    int finalize(float fT);

    bool isEqual(Action<T> *pAction, bool bStrict);
    
    void setAgentNumArray(ulong *pAgentNums) { m_pNumAgentsPerCell = pAgentNums;};


 protected:
    WELL512 **m_apWELL;
    double   *m_adD1;
    double   *m_adD2;
    double   *m_adK;
    int       m_iStride;
    int     **m_aiNumBirths;
    int       m_iWhich;
    ulong    *m_pNumAgentsPerCell;

};

#endif
