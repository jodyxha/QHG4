#ifndef __HYBAGENTLOADER_H__
#define __HYBAGENTLOADER_H__

#include <string>

#include "types.h"

typedef struct aginfo_t {
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fPhenHyb;
    //    float    m_fHybridization;
} aginfo;


class HybAgentLoader {
public:
    static HybAgentLoader *createInstance(const std::string sPop);
    ~HybAgentLoader();

    aginfo *getInfos() { return m_pInfos;};
    int getNumAgents() { return m_iNumAgents;};

protected:
    HybAgentLoader();
    int init(const char *pPop);

    int loadAgentsCell(const std::string sPop, const std::string *pPopName);
    
    int getPopulationNames(hid_t hPopGroup);

    int m_iNumCells;
    int m_iNumAgents;
 
    aginfo  *m_pInfos;
    std::vector<std::string> m_vNames;
};



#endif
