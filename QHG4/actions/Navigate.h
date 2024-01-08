#ifndef __NAVIGATE_H__
#define __NAVIGATE_H__

#include <map>

#include "Observer.h"
#include "Action.h"
#include "ParamProvider2.h"
#include "Geography.h"
#include "Navigation.h"


typedef std::pair<int, double> targprob;

// start grid node => (number of targets, array of targprobs)
typedef std::map<int, std::pair<int, targprob*> > jumpprobmap;

// for manual bridges
typedef std::pair<gridtype,gridtype> bridgedef;
typedef std::vector<bridgedef>        bridgelist;



const static std::string ATTR_NAVIGATE_NAME    = "Navigate";
//required
const static std::string ATTR_NAVIGATE_DECAY_NAME          = "Navigate_decay";
const static std::string ATTR_NAVIGATE_DIST0_NAME          = "Navigate_dist0";
const static std::string ATTR_NAVIGATE_PROB0_NAME          = "Navigate_prob0";
const static std::string ATTR_NAVIGATE_MINDENS_NAME        = "Navigate_min_dens";
// optional
const static std::string ATTR_NAVIGATE_BRIDGE_PROB_NAME    = "Navigate_bridge_prob";


template<typename T>
class Navigate : public Action<T>, Observer {
    
 public:
    Navigate(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    ~Navigate();

    void recalculate();
    void cleanup();

    virtual int execute(int iA, float fT);

    virtual int preLoop(); 
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);

    void notify(Observable *pObs, int iEvent, const void *pData);

    

 protected:
    WELL512 **m_apWELL;
    double    m_dDecay;
    double    m_dDist0;
    double    m_dProb0;
    double    m_dMinDens;
    double    m_dA;
    bool      m_bNeedUpdate;

    double    m_dBridgeProb;
    jumpprobmap m_mJumpProbs;
    bridgelist  m_vCurBridges;

    Geography  *m_pGeography;
    Navigation *m_pNavigation;
public:
    static const char *asNames[];

};

#endif
