#ifndef __SINGLEEVALUATOR_H__
#define __SINGLEEVALUATOR_H__

#include <string>

#include "types.h"
#include "Action.h"
#include "ParamProvider2.h"
#include "Evaluator.h"
#include "Geography.h"

const static std::string ATTR_SINGLEEVAL_NAME   = "SingleEvaluator";

class PolyLine;

template<typename T>
class SingleEvaluator : public Evaluator<T> {
    
 public:
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, intset &sTriggerIDs, bool bAlwaysUpdate=false);
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, double *adInputData, const char *sPLParName, bool bCumulate, int iTriggerID, bool bAlwaysUpdate=false);
    SingleEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, const char *pInputArrayName, const char *sPLParName, bool bCumulate, intset &sTriggerIDs, bool bAlwaysUpdate=false);
    ~SingleEvaluator();
    virtual int initialize(float fT);
    virtual int finalize(float fT);
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };
    void setInputData(double *adInput) { m_adInputData = adInput; };

    void notify(Observable *pObs, int iEvent, const void *pData);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
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

    Geography *m_pGeography;
};

#endif
