#ifndef __TUT_VIRUSHOSTPOP_H__
#define __TUT_VIRUSHOSTPOP_H__


#include "GetOld.h"
#include "ATanDeath.h"
#include "RandomMove.h"
#include "Fertility.h"
#include "Verhulst.h"
#include "RandomPair.h"
#include "AgentBinSplitter.h"
#include "Virus.h"
#include "SPopulation.h"

const std::string VAR_VIRUSHOST_MUT_RATE_NAME    = "MutationRate";
const std::string VAR_VIRUSHOST_IMM_INHERIT_NAME = "ImmunityInheritance";
const std::string INH_TYPES[] = {
    "mix",
    "mat", 
    "pat", 
    "min",
    "max",
    }; 

const int INH_MIX     = 0;
const int INH_MAT     = 1;
const int INH_PAT     = 2;
const int INH_MIN     = 3; 
const int INH_MAX     = 4; 

struct VirusHostAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
        
    float m_fViralLoad;
    float m_fImmunity;
};

class VirusHostPop : public  SPopulation<VirusHostAgent> {

public:
    VirusHostPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    virtual ~VirusHostPop();

    int  getPopParams(const stringmap &mVarDefs);

    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
protected:
    GetOld<VirusHostAgent>            *m_pGO;
    ATanDeath<VirusHostAgent>         *m_pAD;
    RandomMove<VirusHostAgent>        *m_pRM;
    Fertility<VirusHostAgent>         *m_pFert;
    Verhulst<VirusHostAgent>          *m_pVerhulst;
    RandomPair<VirusHostAgent>        *m_pPair;
    AgentBinSplitter<VirusHostAgent>  *m_pAgSplitV;
    AgentBinSplitter<VirusHostAgent>  *m_pAgSplitI;
    Virus<VirusHostAgent>             *m_pVirus;


    float       m_fMutationRate;         
    std::string m_sInheritType;
};

#endif 
