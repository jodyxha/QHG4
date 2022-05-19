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
    virtual int operator()(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };
    virtual int postLoop() { return 0; };
    virtual int preWrite(float fTime) {return 0; };

    // get action parameters from QDF 
    virtual int extractParamsQDF(hid_t hSpeciesGroup) { return 0; };

    // write action parameters to QDF
    virtual int writeParamsQDF(hid_t hSpeciesGroup) {return 0; };


    virtual int modifyParams(const std::string sAttrName, double dValue) {return -1; };

    virtual int dumpStateQDF(hid_t hSpeciesGroup) {return 0; };
    virtual int restoreStateQDF(hid_t hSpeciesGroup) {return 0; };

    // collect action parameters from stringmap
    virtual int tryGetParams(const ModuleComplex * pMC) {return 0;};

    // virtual void showAttributes() = 0;
    void showAttributes();

    bool hasParam(std::string sAtt);

    virtual bool isEqual(Action *pAction, bool bStrict) = 0;
    int checkParams(const stringmap &mParams);

    int getNumParams() { return m_vNames.size();};
    const std::string &getActionName() { return m_sActionName;};
    const stringvec &getParamNames() { return m_vNames;};
protected:
    SPopulation<T> *m_pPop;
    SCellGrid      *m_pCG;    
    stringvec       m_vNames;
    std::string     m_sActionName;
        
    template<typename T2> int readPopKeyVal(std::string sLine, const std::string sKey, T2* pParam);
    template<typename T2> int readPopKeyArr(std::string sLine, const std::string sKey, int iNum, T2* pParam);
    int readPopKeyString(std::string sLine, const std::string sKey, int iSize, std::string &sValue);
};

#endif
