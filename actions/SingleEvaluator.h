#ifndef __SINGLEEVALUATOR_H__
#define __SINGLEEVALUATOR_H__

#include <string>

#include "types.h"
#include "Action.h"
#include "ParamProvider2.h"
#include "Evaluator.h"

#define ATTR_SINGLEEVAL_NAME "SingleEvaluator"

class PolyLine;

template<typename T>
class SingleEvaluator : public Evaluator<T> {
    
 public:
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights,  double *adInputData, const char *sPLParName, bool bCumulate, intset &sTriggerIDs);
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, int iTriggerID);
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, const char *pInputArrayName, const char *sPLParName, bool bCumulate, intset &sTriggerIDs);
    ~SingleEvaluator();
    virtual int initialize(float fT);
    virtual int finalize(float fT);
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };

    void notify(Observable *pObs, int iEvent, const void *pData);

    bool isEqual(Action<T> *pAction, bool bStrict);
    //    bool isEqual(Evaluator *pEval, bool bStrict);
    
    void showAttributes();
 protected:
    std::string  m_sPLParName; // name of PolyLine parameter
    PolyLine *m_pPL;           // how to go from env values to weights
    double *m_adOutputWeights; // scaled probabilities
    double *m_adInputData;     // array of values from which to compute weights
    int m_iMaxNeighbors;
    bool m_bCumulate;
    intset m_sTriggerIDs;
    bool m_bAlwaysUpdate;
    void calcValues();
    void exchangeAndCumulate();

    std::string m_sInputArrayName;
    bool m_bFirst;
};

#endif
