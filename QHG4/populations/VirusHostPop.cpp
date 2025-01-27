#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"

#include "GetOld.cpp"
#include "ATanDeath.cpp"
#include "RandomMove.cpp"
#include "Fertility.cpp"
#include "Verhulst.cpp"
#include "RandomPair.cpp"
#include "AgentBinSplitter.cpp"
#include "Virus.cpp"

#include "VirusHostPop.h"

//----------------------------------------------------------------------------
// constructor
//
VirusHostPop::VirusHostPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<VirusHostAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds) {


    m_pGO  = new GetOld<VirusHostAgent>(this, m_pCG, "");
    m_pAD  = new ATanDeath<VirusHostAgent>(this, m_pCG, "", m_apWELL);
    m_pRM  = new RandomMove<VirusHostAgent>(this, m_pCG, "", m_apWELL);
    m_pFert     = new Fertility<VirusHostAgent>(this, m_pCG, "");
    m_pVerhulst = new Verhulst<VirusHostAgent>(this, m_pCG, "", m_apWELL);

    m_pPair = new RandomPair<VirusHostAgent>(this, m_pCG, "", m_apWELL);
    m_pAgSplitV = new AgentBinSplitter<VirusHostAgent>(this, m_pCG, "VLoad", m_pAgentController);
    m_pAgSplitI = new AgentBinSplitter<VirusHostAgent>(this, m_pCG, "Immun", m_pAgentController);
    m_pVirus = new Virus<VirusHostAgent>(this, m_pCG, "", m_apWELL);
    
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pAD);
    m_prio.addAction(m_pRM);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pVerhulst);

    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pVirus);

    m_prio.addAction(m_pAgSplitV);
    m_prio.addAction(m_pAgSplitI);
    // AgentBinSplitter is not involved in th e simulation steps

    m_fMutationRate = 0.0;
    m_sInheritType = "mix";
}

//----------------------------------------------------------------------------
// destructor
//
VirusHostPop::~VirusHostPop() {

    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pAD != NULL) {
        delete m_pAD;
    }
    if (m_pRM != NULL) {
        delete m_pRM;
    }
    if (m_pFert != NULL) {
        delete m_pFert;
    }
    if (m_pVerhulst != NULL) {
        delete m_pVerhulst;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    
    if (m_pAgSplitV != NULL) {
        delete m_pAgSplitV;
    }
    
    if (m_pAgSplitI != NULL) {
        delete m_pAgSplitI;
    }
    
    if (m_pVirus != NULL) {
        delete m_pVirus;
    }
  
}


//----------------------------------------------------------------------------
// getPopParams
//   get the mutation rate
//
int  VirusHostPop::getPopParams(const stringmap &mVarDefs) {
    int iResult = -1;

    stringmap::const_iterator it = mVarDefs.find(VAR_VIRUSHOST_MUT_RATE_NAME);
    if (it != mVarDefs.end()) {
        float fMutTemp = 0;
        if (strToNum(it->second, &fMutTemp)) {
            m_fMutationRate = fMutTemp;
            printf("[VirusHostPop::getPopParams] have mutation rate %f\n", m_fMutationRate);
            iResult = 0;
        } else {
            // couldn't convert string to float
        }
    } else {
        // expected key VAR_VIRUSHOST_MUT_RATE_NAME
    }

    it = mVarDefs.find(VAR_VIRUSHOST_IMM_INHERIT_NAME);
    if (it != mVarDefs.end()) {
        std::string sInheritType = it->second;

        // handle the "+"
        if (!sInheritType.ends_with("+")) {
            m_fMutationRate = 0;
        } else {
            // drop the plus
            sInheritType = sInheritType.substr(0, sInheritType.size()-1);
        }
        // check if valid inherit type
        bool bSearching = true;
        for (int i  = 0; bSearching && (i < 4); i++) {
            if (sInheritType == INH_TYPES[i]) {
                bSearching = false;
            }
        }
        
        if (bSearching) {
            printf("[VirusHostPop::getPopParams] unknown inherit type [%s]\n", sInheritType.c_str());
        } else {
            m_sInheritType = sInheritType;
            printf("[VirusHostPop::getPopParams] have inherit type [%s]\n", m_sInheritType.c_str());
            iResult = 0;
        }

    } else {
        // expected key VAR_VIRUSHOST_IMM_INHERIT_NAME
    }
    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentData
// read additional data from pop file (the mate index is volatile, 
// so we don't try to read or write it)
//
int VirusHostPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
    }

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fViralLoad);
    }

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fImmunity);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
// extend the agent data type to include additional data in the output
//
void VirusHostPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    VirusHostAgent va;
    H5Tinsert(*hAgentDataType, "Age",       qoffsetof(va, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth", qoffsetof(va, m_fLastBirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "ViralLoad", qoffsetof(va, m_fViralLoad), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Immunity",  qoffsetof(va, m_fImmunity), H5T_NATIVE_FLOAT);

    // the mate index and incoming are volatile, so we don't add them to the type
}


//----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int VirusHostPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;
    
    m_aAgents[iAgent].m_fAge = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = -3;
    // SPopulation assigns random genders to a baby, ehich is ok here
    m_aAgents[iAgent].m_fViralLoad = 0;


    // register this date as the last birth of the mother
    m_aAgents[iAgent].m_fLastBirth = m_fCurTime;

    // the offspring's immunity is the average of the parents' immunities plus a little fuzz factor.
    float fImm = 0.0;
    if (m_sInheritType == INH_TYPES[INH_MIX]) {
        fImm =  (m_aAgents[iMother].m_fImmunity + m_aAgents[iFather].m_fImmunity)/2;
    } else if (m_sInheritType == INH_TYPES[INH_MAT]) {
        fImm =  m_aAgents[iMother].m_fImmunity;
    } else if (m_sInheritType == INH_TYPES[INH_PAT]) {
        fImm =  m_aAgents[iFather].m_fImmunity;
    } else if (m_sInheritType == INH_TYPES[INH_MIN]) {
        fImm = (m_aAgents[iMother].m_fImmunity < m_aAgents[iFather].m_fImmunity)?m_aAgents[iMother].m_fImmunity:m_aAgents[iFather].m_fImmunity;
    } else if (m_sInheritType == INH_TYPES[INH_MAX]) {
        fImm = (m_aAgents[iMother].m_fImmunity > m_aAgents[iFather].m_fImmunity)?m_aAgents[iMother].m_fImmunity:m_aAgents[iFather].m_fImmunity;
    } else {
        // shouldn't happen
        iResult = -1;
    }

    //mutate
    fImm += m_apWELL[omp_get_thread_num()]->wgauss(m_fMutationRate);
    if (fImm < 0) {
        fImm = 0;
    }else if (fImm > 1) {
        fImm = 1;
    }
    m_aAgents[iAgent].m_fImmunity = fImm;
    
    
    if (m_aAgents[iMother].m_fViralLoad > 0) {
        m_aAgents[iAgent].m_fViralLoad = 0.2;
        printf("Babyinfect: %d -> %d\nn", iMother, iAgent);
    }
    

    //printf("[VirusHostPop::makePopSpecificOffsprin] baby %d vl %f, imm %f\n",  iAgent, m_aAgents[iAgent].m_fViralLoad, m_aAgents[iAgent].m_fImmunity);
    return iResult;
}

