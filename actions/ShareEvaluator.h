#ifndef __SHAREEVALUATOR_H__
#define __SHAREEVALUATOR_H__

#include <string>

#include "Action.h"
#include "ParamProvider2.h"
#include "Evaluator.h"

#define ATTR_SHAREEVAL_NAME "ShareEvaluator"
#define ATTR_SHAREEVAL_ARRAYNAME "ShareEvaluator_%s_arrayname"
#define ATTR_SHAREEVAL_POLYNAME  "ShareEvaluator_%s_polyname"

#define SHARE_NAME_LEN 512

class PolyLine;


template<typename T>
class ShareEvaluator : public Evaluator<T> {
    //class ShareEvaluator : public Action<T>, public Evaluator {

public:
    ShareEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, bool bCumulate, intset &sTriggerIDs);
    ShareEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, bool bCumulate, int iTriggerID);
    ShareEvaluator() {};
    ~ShareEvaluator();
    int initialize(float fT);
    int finalize(float fT);
    int preLoop();
    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };

    void notify(Observable *pObs, int iEvent, const void *pData);

    bool isEqual(Action<T> *pAction, bool bStrict);
    //    bool isEqual(Evaluator *pEval, bool bStrict);
    void showAttributes();
protected:
    PolyLine *m_pPL;           // how to go from env values to weights
    double *m_adOutputWeights;    // scaled probabilities
    double *m_adInputData;                // array of values from which to compute weights
    int     m_iMaxNeighbors;
    bool    m_bCumulate;
    intset  m_sTriggerIDs;
    bool    m_bAlwaysUpdate;


    void calcValues();
    void exchangeAndCumulate();

    std::string m_sID;
    std::string m_sArrayName;
    std::string m_sPolyName;
};


#endif
