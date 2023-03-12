#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"

#include "GetOld.cpp"
#include "ATanDeath.cpp"

#include "RandomMove.cpp"

#include "tut_MovePop.h"

//----------------------------------------------------------------------------
// constructor
//
tut_MovePop::tut_MovePop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds) {
    
    m_pGO = new GetOld<tut_MoveAgent>(this, m_pCG, "");
    m_pAD = new ATanDeath<tut_MoveAgent>(this, m_pCG, "", m_apWELL);

    m_pRM = new RandomMove<tut_MoveAgent>(this, m_pCG, "", m_apWELL);

    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pAD);

    m_prio.addAction(m_pRM);
 
}

///----------------------------------------------------------------------------
// destructor
//
tut_MovePop::~tut_MovePop() {
    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pAD != NULL) {
        delete m_pAD;
    }
    if (m_pRM != NULL) {
        delete m_pRM;
    }
}



//----------------------------------------------------------------------------
// addPopSpecificAgentData
// read additional data from pop file (the mate index is volatile, so we don't try to read it)
//
int tut_MovePop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
// extend the agent data type to include additional data in the output
//
void tut_MovePop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    tut_MoveAgent ma;
    H5Tinsert(*hAgentDataType, "Age",  qoffsetof(ma, m_fAge), H5T_NATIVE_FLOAT);

}
