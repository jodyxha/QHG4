#ifndef __AGENTCOUNTER_H__
#define __AGENTCOUNTER_H__

#include "hdf5.h"

#include "types.h"

typedef struct aginfo_t {
    idtype   m_ulID;
    gridtype m_ulCellID;
} aginfo;

typedef std::map<std::string, std::pair<size_t, aginfo*>> popaginfos;
typedef std::map<std::string, int *> popcounts;
typedef std::map<std::string, double *> popcc;
class AgentCounter {
public:
    static AgentCounter *createInstance(const std::string sPop, const std::string sGeo, const std::string sVeg);

    ~AgentCounter();

    int countAgentsInCells();

    int           getNumCells()  { return m_iNumCells;};
    const double *getLongitude() { return m_pLongitude;};
    const double *getLatitude()  { return m_pLatitude;};
    const double *getAltitude()  { return m_pAltitude;};
    const double *getIceCover()  { return m_pIceCover;};
    const popcc  &getCC()        { return m_mpCC;};
    double *getCC(const std::string sSpecies);
    const popcounts &getPopCounts() { return m_mpCounts;};
    int    *getPopCounts(const std::string sSpecies);

    int getTotal() {return m_iTotal;};
protected:
    AgentCounter();
    int init(const std::string sPop, const std::string sGeo, const std::string sVeg);

    int loadAltIce(const std::string sGeo);
    int loadLonLat(const std::string sGeo);
    int loadCC(const std::string sPop);
    int loadAgentsCell(const std::string sPop, const std::string sPopName);
    
    int getPopulationNames(hid_t hPopGroup);


    int m_iNumCells;
    int m_iNumAgents;
    int m_iTotal;
     
    double     *m_pLongitude;
    double     *m_pLatitude;
    double     *m_pAltitude;
    double     *m_pIceCover;
    popcc       m_mpCC;
    popaginfos  m_mpInfos;
    popcounts   m_mpCounts;
    stringvec   m_vNames;

    aginfo     *m_pInfos;
};



#endif
