#ifndef __RANDOMMOVE_H__
#define __RANDOMMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_RANDOMMOVE_NAME   = "RandomMove";
const static std::string ATTR_RANDOMMOVE_PROB_NAME   = "RandomMove_prob";

class WELL512;

template<typename T>
class RandomMove : public Action<T> {
    
 public:
    RandomMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~RandomMove();
    virtual int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);


    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 
 protected:
    WELL512 **m_apWELL;
    double m_dMoveProb;
    int **m_apDirCounts;
    int m_iNumDirs;
};

#endif
