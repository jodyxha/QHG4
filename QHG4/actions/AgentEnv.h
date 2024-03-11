#ifndef __AGENTENV_H__X
#define __AGENTENV_H__X

#include "types.h"
#include "Observable.h"
#include "Observer.h"
#include "Action.h"
#include "ParamProvider2.h"
#include "LayerArrBuf.h"
#include "WELL512.h"

class LBController;

const static std::string ATTR_AGENTENV_NAME = "AgentEnv";
const static std::string ATTR_AGENTENV_REQUIREMENTS_NAME = "AgentEnv_requirements";

template<typename T>
class AgentEnv : public Action<T> , public Observer {
public:
    AgentEnv(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512** apWELL, LBController *pAgentController, double *pdFoodAvailable, int *pAgentCounts);
    virtual ~AgentEnv();

    // get action parameters from QDF 
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);

    // write action parameters to QDF
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
    virtual int tryGetAttributes(const ModuleComplex *pMC);

    void notify(Observable *pObs, int iEvent, const void *pData);
    int recalculateGlobalCapacities();
    
    // from Action
    virtual int preLoop();
    virtual int initialize(float fTime);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);

    const double *getArr(int iAgentIndex);
    int init();
protected:
    int getAgentCounts();
    int calculateEnvArr(int iAgentIndex);
    int calculateGroupArr(int iAgentIndex);

    int decreaseHealth();

    LBController *m_pAgentController;
    bool          m_bNeedUpdate;

    LayerArrBuf<double> m_aEnvVals; // 7 entries per agent (self + 6 neighbors)

  


    int m_iNumThreads;
    int m_iLocArrSize;
    double m_dRequirements;

    double  *m_adCapacities;

    double  *m_pEnvArr;
    double  *m_pGroupArr;
    int    **m_ppCounts;
    double  *m_pFoodAvailable;
    int     *m_pAgentCounts;

public:
    static const stringvec s_vNames;
   

};

#endif
