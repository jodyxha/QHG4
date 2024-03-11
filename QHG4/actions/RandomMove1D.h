#ifndef __RANDOMMOVE1D_H__
#define __RANDOMMOVE1D_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_RANDOMMOVE1D_NAME   = "RandMove1D";
const static std::string ATTR_RANDOMMOVE1D_PROB_NAME   = "RandMove1D_prob";

class WELL512;

template<typename T>
class RandomMove1D : public Action<T> {
    
 public:
    RandomMove1D(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    ~RandomMove1D();
    int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);
 
 protected:
    WELL512 **m_apWELL;
    double m_dMoveProb;
    bool m_bAbsorbing;

public:
    static const std::string asNames[];
};

#endif
