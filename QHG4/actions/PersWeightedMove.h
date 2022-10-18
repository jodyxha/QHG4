#ifndef __PERSWEIGHTEDMOVE_H__
#define __PERSWEIGHTEDMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"

#define ATTR_PERSWEIGHTEDMOVE_NAME "PersWeightedMove"

template<typename T>
class PersWeightedMove : public Action<T> {
    
 public:
    PersWeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights);
    ~PersWeightedMove();
    int execute(int iA, float fT);

    virtual int postLoop();


    void displayInfo(const char *pPrefix, float fT, int iCellIndex, int iAgentIndex); 
    bool isEqual(Action<T> *pAction, bool bStrict);
    
 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;

};

#endif
