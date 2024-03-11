#ifndef __WEIGHTEDMOVE_H__
#define __WEIGHTEDMOVE_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"
#include "Geography.h"

const static std::string ATTR_WEIGHTEDMOVE_NAME        = "WeightedMove";
const static std::string ATTR_WEIGHTEDMOVE_PROB_NAME   = "WeightedMove_prob";

template<typename T>
class WeightedMove : public Action<T> {
    
 public:
    WeightedMove(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *adEnvWeights);
    virtual ~WeightedMove();

    virtual int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);
 
 
    void displayInfo(const char *pPrefix, float fT, int iCellIndex, int iAgentIndex); 
    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 
 protected:
    WELL512 **m_apWELL;
    double *m_adEnvWeights;
    double m_dMoveProb;

    Geography *m_pGeography;
public:
    static const std::string asNames[];
};

#endif
