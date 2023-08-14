#ifndef __ACTION_H__
#define __ACTION_H__

// This is the base class for all actions

// DO NOT MODIFY THIS CLASS 
#include <vector> 
#include <string> 

#include <hdf5.h>
#include "types.h"
#include "ParamProvider2.h"

template<typename T> class SPopulation;
template<typename T> class LayerBuf;

class SCellGrid;

template<typename T>
class Action { 

 public:
    Action(SPopulation<T> *pPop, SCellGrid *pCG, const std::string sActionName, const std::string sID);
    Action() { };
    virtual ~Action() { };

    virtual int preLoop() { return 0; };
    virtual int initialize(float fTime) { return 0; };
    virtual int execute(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };
    virtual int postLoop() { return 0; };
    virtual int preWrite(float fTime) {return 0; };

    // get action parameters from QDF 
    virtual int extractAttributesQDF(hid_t hSpeciesGroup) { return 0; };

    // write action parameters to QDF
    virtual int writeAttributesQDF(hid_t hSpeciesGroup) {return 0; };

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup) {return 0; };
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup) {return 0; };


    virtual int modifyAttributes(const std::string sAttrName, double dValue) {return -1; };

    virtual int dumpStateQDF(hid_t hSpeciesGroup) {return 0; };
    virtual int restoreStateQDF(hid_t hSpeciesGroup) {return 0; };

    // collect action parameters from stringmap
    virtual int tryGetAttributes(const ModuleComplex * pMC) {return 0;};

    // virtual void showAttributes() = 0;
    virtual void showAttributes();

    virtual bool hasAttribute(std::string sAtt);

    //virtual bool isEqual(Action *pAction, bool bStrict) = 0;
    virtual bool isEqual(Action *pAction, bool bStrict) {return false;};;

    int checkAttributes(const stringmap &mParams);

    int getNumAttributes() { return m_vNames.size();};
    const std::string &getActionName() { return m_sActionName;};
    const stringvec &getAttributeNames() { return m_vNames;};
protected:
    SPopulation<T> *m_pPop;
    SCellGrid      *m_pCG;    
    stringvec       m_vNames;
    std::string     m_sID;
    std::string     m_sActionName;
  
};

#endif
