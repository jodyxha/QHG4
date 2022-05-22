#ifndef __AGENTHYBCOLLECTOR_H__
#define __AGENTHYBCOLLECTOR_H__

#include <map>
#include "types.h"
#include "hdf5.h"

typedef struct locitem_t {
    float  m_fLon;
    float  m_fLat;
    float  m_fVal;
} locitem;

typedef struct range_t {
    double  m_dLon;
    double  m_dLat;
    double  m_dRad;
    range_t(double dLon, double dLat, double dRad):m_dLon(dLon),m_dLat(dLat),m_dRad(dRad){};
} range;

typedef struct bin_context_t {
    std::string  m_sName;
    uint         m_iNumBins;
    uint        *m_piBins;
    float        m_fMinNonZero;
    float        m_fMaxNonOne;
    uint         m_iNumZeros;
    uint         m_iNumOnes;
    uint         m_iNumAgents;
    uint         m_iNumChecked;
} bin_context;



typedef std::map<std::string, range_t*> named_ranges;

// map: cell id => coord pair
typedef std::map<int, std::pair<double, double> >    arrpos_coords;

typedef struct aginfo_t {
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fHybridization;
} aginfo;

class AgentHybCollector {
public:
    static AgentHybCollector *createInstance(const char *pGeoQDF, const char *pPopQDF, const char *pSpecies, const char *pItemName);
    ~AgentHybCollector();

    int init(const char *pGeoQDF, const char *pPopQDFF, const char *pSpecies, const char *pItemName);
    
    int getDistr(double dMin, double dMax, int iNumBins, const char *pItemName);

    
    //int analyzeRange(range *pRange, uint iNumBins);
    int analyzeRanges(named_ranges &mNamedRanges, uint iNumBins);
    /*
    uint getNumBins() {return m_iNumBins;};
    uint *getBins() {return m_piBins;};
    float getMinNonZero() {return m_fMinNonZero;};    
    float getMaxNonOne() {return m_fMinNonZero;};    
    uint  getNumZeros() {return m_iZeros;};    
    uint  getNumOnes() {return m_iOnes;};    
    uint  getNumChecked() {return m_iNumChecked;};    
    uint  getNumAgents() {return m_iNumAgents;};   
    */
    uint  getNumContexts() {return m_iNumContexts;};   
    bin_context *getContext(uint i) { return (i < m_iNumContexts)?m_ppBinContexts[i]:NULL;};
protected:
    AgentHybCollector();

    int fillCoordMap(const char *pGeoQDF);
    int loadAgentsCell(const char *pPopQDFF, const char *pSpecies, const char *pItemName);


    arrpos_coords m_mCoords;
    aginfo       *m_pInfos;
    
    bin_context **m_ppBinContexts;
    uint m_iNumContexts;

    
    uint  m_iNumAgents;
  
};





#endif
