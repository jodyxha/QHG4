#include <stdio.h>
#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"
#include "GrassManager.cpp"

#include "GrassPop.h"


//----------------------------------------------------------------------------
// constructor
//
GrassPop::GrassPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<GrassAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
    m_pGM(NULL),
    m_pdGrassAvailable(NULL),
    m_pdGrassConsumed(NULL) {

    
    ArrayShare *pAS = ArrayShare::getInstance();

    // create and share available grass mass array
    m_pdGrassAvailable = new double[pCG->m_iNumCells];
    int iResult = pAS->shareArray(SHARE_GRASS_MASS_AVAILABLE, pCG->m_iNumCells, m_pdGrassAvailable);
    if (iResult != 0) {
        printf("Couldnt share array [%p] with name [%s]\n", m_pdGrassAvailable, SHARE_GRASS_MASS_AVAILABLE.c_str()); 
    } else {
        printf("GrassPop shared [%p] as [%s]\n", m_pdGrassAvailable, SHARE_GRASS_MASS_AVAILABLE.c_str()); 
            
    }

    // create and share consumed grass array
    m_pdGrassConsumed  = new double[pCG->m_iNumCells];
    iResult = pAS->shareArray(SHARE_GRASS_MASS_CONSUMED, pCG->m_iNumCells, m_pdGrassConsumed);
    if (iResult != 0) {
        printf("Couldnt share array [%p] with name [%s]\n", m_pdGrassConsumed, SHARE_GRASS_MASS_CONSUMED.c_str()); 
    } else {
        printf("GrassPop shared [%p] as [%s]\n", m_pdGrassConsumed, SHARE_GRASS_MASS_CONSUMED.c_str()); 
    }

    // we only have one action
    m_pGM = new GrassManager<GrassAgent>(this, m_pCG, "", SHARE_GRASS_MASS_AVAILABLE, SHARE_GRASS_MASS_CONSUMED);

 
    m_prio.addAction(m_pGM);

}

//----------------------------------------------------------------------------
// destructor
//    delete the allocated arrays
//
GrassPop::~GrassPop() {
    if (m_pGM != NULL) {
        delete m_pGM;
    }

    if (m_pdGrassAvailable != NULL) {
        delete[] m_pdGrassAvailable;
    }

    if (m_pdGrassConsumed != NULL) {
        delete[] m_pdGrassConsumed;
    }

}


//----------------------------------------------------------------------------
// addPopSpecificAgentData
//   read additional data (m_dMass) from pop file
//
int GrassPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    int iResult = 0;

    iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dMass);

    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//    extend the agent data type to include additional data in the output
//
void GrassPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    GrassAgent ga;
    H5Tinsert(*hAgentDataType, "Mass",  qoffsetof(ga, m_dMass), H5T_NATIVE_DOUBLE);

}

//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//   write the available grass mass as an array in the population group
//
int GrassPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    stdprintf("GrassPop::writeAdditionalDataQDF() reached\n");
    int iResult = qdf_writeArray(hSpeciesGroup, "GrassMassAvail", m_pCG->m_iNumCells, m_pdGrassAvailable, H5T_NATIVE_DOUBLE);
    if (iResult != 0) {
        LOG_ERROR("[GrassPop::writeAdditionalDataQDF] couldn't write GrassMassAvail data");
    } 
    return iResult;
}
