#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"

#include "GetOld.cpp"
//#include "OldAgeDeath.cpp"
#include "AtanDeath.cpp"

#include "tut_OldAgeDiePop.h"

//----------------------------------------------------------------------------
// constructor
//
tut_OldAgeDiePop::tut_OldAgeDiePop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<tut_OldAgeDieAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds) {
    
    m_pGO = new GetOld<tut_OldAgeDieAgent>(this, m_pCG, "");

    m_pAD = new AtanDeath<tut_OldAgeDieAgent>(this, m_pCG, "", m_apWELL);
 
    m_prio.addAction(m_pAD);
    m_prio.addAction(m_pGO);
 

}

///----------------------------------------------------------------------------
// destructor
//
tut_OldAgeDiePop::~tut_OldAgeDiePop() {
    delete m_pGO;
    delete m_pAD;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentData
// read additional data from pop file
//
int tut_OldAgeDiePop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    int iResult = 0;

    iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
// extend the agent data type to include additional data in the output
//
void tut_OldAgeDiePop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    tut_OldAgeDieAgent oada;
    H5Tinsert(*hAgentDataType, "Age",  qoffsetof(oada, m_fAge), H5T_NATIVE_FLOAT);

}


