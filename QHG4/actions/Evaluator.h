#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__


#include "Observer.h"
#include "Action.h"

//class Evaluator :  public Observer {
template<typename T>
class Evaluator :  public Action<T>, public Observer {
public:
    Evaluator(SPopulation<T> *pPop, SCellGrid *pCG, const std::string sActionName, const std::string sID)
        : Action<T>(pPop,pCG,sActionName, sID), 
        m_bNeedUpdate(false) {};
    Evaluator() {};
    virtual ~Evaluator() {};
    virtual void setOutputWeights(double *adOutput)=0;
    /*
    virtual int initialize(float fTime)=0;
    virtual int preLoop(){ return 0;};
    virtual int postLoop(){ return 0;};
    

    virtual int finalize(float fTime) = 0;

    // get action parameters from QDF 
    virtual int extractAttributesQDF(hid_t hSpeciesGroup) { return 0; };

    // write action parameters to QDF
    virtual int writeAttributesQDF(hid_t hSpeciesGroup) {return 0; };

    virtual bool isEqual(Evaluator *pEval, bool bStrict) = 0;
    virtual int tryGetAttributes(const stringmap &mParams) { return 0; }; // we need to return 0 for Prioritizer
    */
    bool needUpdate() { return m_bNeedUpdate;};
protected:
    bool m_bNeedUpdate;
};


#endif
