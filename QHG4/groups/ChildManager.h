#ifndef __CHILDMANAGER_H__
#define __CHILDMANAGER_H__

#include <vector>
#include <map>

#include "types.h"
#include "WELL512.h"
#include "Action.h"
#include "GroupInterface.h"

#define ATTR_CHILDMAN_NAME "ChildManager"
#define ATTR_CHILDMAN_ADULT_AGE_NAME "ChildManager_adultage"

#define DS_CHILDMAN_DATA "ChildManagerData"

template<typename T>
class ChildManager : public Action<T> {
    
public:
    ChildManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    //    ChildManager(GroupInterface *pPop, float fAdultAge);
    ~ChildManager();

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

   

    float getAdultAge() {return m_fAdultAge;};

    std::vector<int> getChildren(int iParent);
    
    int checkAdult();
    int addChild(int iParent, int iChild);
    int removeChild(int iParent, int iChild);
  
    bool isEqual(Action<T> *pAction, bool bStrict);
  
protected:
    WELL512      **m_apWELL;
    GroupInterface                 *m_pGPop;
    float m_fAdultAge;
    std::map<int, std::vector<int>> m_mvChildren; // parentid->children
    std::map<int,int>               m_mParents;   // agent->parentid
};

#endif
