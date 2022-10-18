#ifndef __FOODMANAGER_H__
#define __FOODMANAGER_H__

#include <vector>
#include <map>

#include "Action.h"
#include "WELL512.h"

#define ATTR_FOODMAN_NAME             "FoodManager"
#define ATTR_FOODMAN_GROWTH_RATE_NAME "FoodManager_growthrate"
#define ATTR_FOODMAN_MIN_AMOUNT_NAME  "FoodManager_minamount"



typedef std::vector<std::pair<int, double>> single_requests;  //agent id, amount
typedef std::map<int, single_requests>     total_requests;    //cell id => request 

template<typename T> 
class FoodManager : public Action<T> {

public:
    FoodManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double *pdFoodAvailable, int *pAgentCounts);
    virtual ~FoodManager();

    virtual int preLoop();
    virtual int initialize(float fT);
   
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);
    /*
    virtual int AdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int AdditionalDataQDF(hid_t hSpeciesGroup);
    */
    int registerForFood(int iAgentID, int iCellID, double dAmount);
    int growFood();
    int handOutFood(); // if no registration:divide equally amongs all agents per cell

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
    void createRandomMaxAmounts();

protected:
    WELL512 **m_apWELL;
    //double  *m_adEnvWeights;
    int      m_iNumCells;
    int     *m_pAgentCounts;
    
    double  *m_pFoodArray;      // available food; one per cell
    double  *m_pActualFood;     // actual food; one per cell
    double  *m_pFoodMaxAmount;  // "capacity"; one per cell
    double   m_dFoodGrowthRate;
    double   m_dFoodMinAmount;

    total_requests m_mAllRequests;
    double  *m_pTotalRequests;  // sum of requests; one per cell

    bool m_bMaxAmountsLoaded;
};

#endif
