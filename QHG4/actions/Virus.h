#ifndef __VIRUS_H__
#define __VIRUS_H__
    
#include "types.h"
#include "Action.h"
#include "ParamProvider2.h"
    
const static std::string ATTR_VIRUS_NAME                   = "Virus";
const static std::string ATTR_VIRUS_INFECTION_PROB_NAME    = "InfectionProb";
const static std::string ATTR_VIRUS_INITIAL_LOAD_NAME      = "InitialLoad";
const static std::string ATTR_VIRUS_GROWTH_RATE_NAME       = "GrowthRate";
const static std::string ATTR_VIRUS_CONTAGION_LEVEL_NAME   = "ContagionLevel";
const static std::string ATTR_VIRUS_LETHALITY_LEVEL_NAME   = "LethalityLevel";

typedef std::map<int, intvec> cellagentmap;
typedef std::map<int, float>  agentloadmap;

template<typename T>
class Virus : public Action<T> {
        
public:
    Virus(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~Virus();
    
    virtual int initialize(float fT);
    virtual int execute(int iA, float fT);
    virtual int finalize(float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
    
    virtual int tryGetAttributes(const ModuleComplex *pMC);
    
    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
protected:
    WELL512 **m_apWELL;
    float m_fInfectionProb;
    float m_fInitialLoad;
    float m_fGrowthRate;
    float m_fContagionLevel;
    float m_fLethalityLevel;
    
    int m_iNumThreads;
    cellagentmap  *m_amCellAgents;
    agentloadmap  *m_amAgentInfects; 

    static const std::string asNames[];
    
};
    
    
#endif
