#ifndef __NAVIGATE2_H__
#define __NAVIGATE2_H__

#include <map>

#include "Observer.h"
#include "Action.h"
#include "ParamProvider2.h"
#include "Geography.h"
#include "Navigation2.h"


typedef std::pair<int, double> targprob;

// start grid node => (number of targets, array of targprobs)
typedef std::map<int, std::pair<int, targprob*> > jumpprobmap;

// for manual bridges
typedef std::pair<gridtype,gridtype> bridgedef;
typedef std::vector<bridgedef>        bridgelist;



const static std::string ATTR_NAVIGATE2_NAME    = "Navigate2";
//required
const static std::string ATTR_NAVIGATE2_DECAY_NAME          = "Navigate2_decay";
const static std::string ATTR_NAVIGATE2_DIST0_NAME          = "Navigate2_dist0";
const static std::string ATTR_NAVIGATE2_PROB0_NAME          = "Navigate2_prob0";
const static std::string ATTR_NAVIGATE2_MINDENS_NAME        = "Navigate2_min_dens";
// optional
const static std::string ATTR_NAVIGATE2_BRIDGE_PROB_NAME    = "Navigate2_bridge_prob";


template<typename T>
class Navigate2 : public Action<T>, Observer {
    
 public:
    Navigate2(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    ~Navigate2();

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
    Navigation2 *m_pNavigation2;

public:
    static const std::string asNames[];

};

#endif
