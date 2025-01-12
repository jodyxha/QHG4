#ifndef __MULTIEVALUATOR_H__
#define __MULTIEVALUATOR_H__

#include <string>
#include <map>

#include "xha_strutilsT.h"

#include "Observable.h"
#include "Observer.h"

#include "Action.h"
#include "Evaluator.h"
#include "ParamProvider2.h"


const static std::string ATTR_MULTIEVAL_NAME   = "MultiEvaluator";

enum MultiEvalModes {
    MODE_ADD_SIMPLE = 0,
    MODE_ADD_BLOCK  = 1,
    MODE_MUL_SIMPLE = 2,
    MODE_MAX_SIMPLE = 3,
    MODE_MAX_BLOCK  = 4,
    MODE_MIN_SIMPLE = 5,
};


template<typename T>
class MultiEvaluator : public Evaluator<T>, public Observable {
    //class MultiEvaluator : public Action<T>, public Evaluator, public Observable{

 public:
    typedef std::vector<std::pair<std::string, Evaluator<T>*> > evaluatorinfos;

    MultiEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, double *adOutputWeights, evaluatorinfos &mEvaluators, int iMode, bool bDeleteEvaluators);
    virtual ~MultiEvaluator();
    virtual int initialize(float fT);
    virtual int finalize(float fT);
    virtual int preLoop();
    virtual int postLoop();
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);
    void setOutputWeights(double *adOutput) { m_adOutputWeights = adOutput; };

    void notify(Observable *pObs, int iEvent, const void *pData);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    //    bool isEqual(Evaluator *pEval, bool bStrict);
    
    void showAttributes();
 protected:
    bool  m_bDeleteEvaluators;
    double *m_adOutputWeights;    // scaled probabilities
    int m_iMaxNeighbors;
    int m_iNumEvals;
    int m_iMode;
    bool m_bFirst;

    // here the array of Evaluators
    Evaluator<T> **m_aEvaluators;

    // here the array with the parameter names for the weights
    stringvec m_vCombinationWeightNames;

    // here the array with the weights to combine the evaluators
    double *m_adCombinationWeights;

    // this array will be used by each SingleEvaluator in turn to save space
    double *m_adSingleEvalWeights;

    // called by the different constructors
    void init(evaluatorinfos &mEvalInfo, int iMode);

    int addSingleWeights(float fT);
  
    int addSingleWeightsBlock(float fT);
  
    int multiplySingleWeights(float fT);

    int maxSingleWeights(float fT);

    int maxSingleWeightsBlock(float fT);

    int minSingleWeights(float fT);

    int findBlockings(float fT);
    uchar *m_acAllowed;

};

#endif
