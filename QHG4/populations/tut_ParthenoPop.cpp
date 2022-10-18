#include <omp.h>
#include <hdf5.h>

#include "EventConsts.h"
#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"

#include "GetOld.cpp"
#include "OldAgeDeath.cpp"
#include "RandomMove.cpp"

#include "Fertility.cpp"
#include "Verhulst.cpp"

#include "tut_ParthenoPop.h"

//----------------------------------------------------------------------------
// constructor
//
tut_ParthenoPop::tut_ParthenoPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<tut_ParthenoAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds) {


    m_pGO  = new GetOld<tut_ParthenoAgent>(this, m_pCG, "");
    m_pOAD = new OldAgeDeath<tut_ParthenoAgent>(this, m_pCG, "", m_apWELL);
    m_pRM  = new RandomMove<tut_ParthenoAgent>(this, m_pCG, "", m_apWELL);

    m_pFert     = new Fertility<tut_ParthenoAgent>(this, m_pCG, "");
    m_pVerhulst = new Verhulst<tut_ParthenoAgent>(this, m_pCG, "", m_apWELL);


    // adding all actions to prioritizer

    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pRM);

    m_prio.addAction(m_pVerhulst);
    m_prio.addAction(m_pFert);

}



///----------------------------------------------------------------------------
// destructor
//
tut_ParthenoPop::~tut_ParthenoPop() {

    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pOAD != NULL) {
        delete m_pOAD;
    }
    if (m_pRM != NULL) {
        delete m_pRM;
    }
     if (m_pVerhulst != NULL) {
        delete m_pVerhulst;
    }
    if (m_pFert != NULL) {
        delete m_pFert;
    }
   
}


//----------------------------------------------------------------------------
// addPopSpecificAgentData
// read additional data from pop file (the mat index is volatile, so we don't try to read it)
//
int tut_ParthenoPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fLastBirth);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
// extend the agent data type to include additional data in the output
//
void tut_ParthenoPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    tut_ParthenoAgent sa;
    H5Tinsert(*hAgentDataType, "Age",  qoffsetof(sa, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth",  qoffsetof(sa, m_fLastBirth), H5T_NATIVE_FLOAT);

    // the mate index is volatile, so we don't add it to the type
}

//----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int tut_ParthenoPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;

    m_aAgents[iAgent].m_fAge       = 0.0;
    m_aAgents[iAgent].m_fLastBirth = 0.0;
    m_aAgents[iAgent].m_iMateIndex = iAgent;
    // SPopulation assigns random genders to a baby
    // but we want females only
    m_aAgents[iAgent].m_iGender    = 0;

    return iResult;
}
