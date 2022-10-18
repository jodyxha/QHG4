#ifndef __NPERSLINDEATHREL_H__
#define __NPERSLINDEATHREL_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_NPERSLINDEATHREL_NAME "NPersLinearDeathRel"

class WELL512;

template<typename T>
class NPersLinearDeathRel : public Action<T> {
    
 public:
    NPersLinearDeathRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths);
    ~NPersLinearDeathRel();

    int preLoop();
    int initialize(float fT);
    int execute(int iA, float fT);
    int finalize(float fT);

    bool isEqual(Action<T> *pAction, bool bStrict);
    
    void setAgentNumArray(ulong *pAgentNums) { m_pNumAgentsPerCell = pAgentNums;};

 protected:
    WELL512 **m_apWELL;
    double   *m_adD;
    int     **m_aiNumBirths;
    int       m_iWhich;
    ulong    *m_pNumAgentsPerCell;

};

#endif
