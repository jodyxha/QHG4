#include <stdio.h>
#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"

#include "SheepManager.cpp"
#include "Starver.cpp"
#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "AnimalReproducer.cpp"

#include "SheepPop.h"


//----------------------------------------------------------------------------
// constructor
//
SheepPop::SheepPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<SheepAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
    m_pSEGrass(NULL),
    m_pWM(NULL),
    m_pSM(NULL),
    m_pRS(NULL),
    m_pAR(NULL),
    m_pdGrassMassAvailable(NULL),
    m_pdGrassMassConsumed(NULL),
    m_adPreferences(NULL) {

    
    int iMaxNeighbors = this->m_pCG->m_iConnectivity;
    int iArrSize = this->m_pCG->m_iNumCells * (iMaxNeighbors + 1);

    m_adPreferences = new double[iArrSize];
    memset(m_adPreferences, 0, pCG->m_iNumCells*sizeof(double));


    m_pSEGrass = new SingleEvaluator<SheepAgent>(this, m_pCG, "Grass", m_adPreferences, (double*)m_pdGrassMassAvailable, "", true, EVENT_ID_NONE, true);
 
    m_pWM   = new WeightedMove<SheepAgent>(this, m_pCG, "", m_apWELL, m_adPreferences);
    

    m_pSM = new SheepManager<SheepAgent>(this, m_pCG, "",
                                         SHARE_GRASS_MASS_AVAILABLE, SHARE_GRASS_MASS_CONSUMED);
    
    m_pRS   = new Starver<SheepAgent>(this, m_pCG, "");
 
    m_pAR = new AnimalReproducer<SheepAgent>(this, m_pCG, "", m_apWELL);

  
    m_prio.addAction(m_pSEGrass);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pSM);
    m_prio.addAction(m_pRS);
    m_prio.addAction(m_pAR);
   

}

//----------------------------------------------------------------------------
// destructor
//
SheepPop::~SheepPop() {
    if (m_adPreferences != NULL) {
        delete[] m_adPreferences;
    }
    if (m_pSEGrass != NULL) {
        delete m_pSEGrass;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pSM != NULL) {
        delete m_pSM;
    }
    if (m_pRS != NULL) {
        delete m_pRS;
    }
    if (m_pAR != NULL) {
        delete m_pAR;
    }


}

//----------------------------------------------------------------------------
// preLoop
//   read additional data from pop file
//
int SheepPop::preLoop() {
    int iResult = 0;
    printf("[SheepPop::preLoop] ArrayShare is [%p]\n", ArrayShare::getInstance());
    m_pdGrassMassAvailable = (double *) ArrayShare::getInstance()->getArray(SHARE_GRASS_MASS_AVAILABLE);
    if (m_pdGrassMassAvailable == NULL) {
        printf("[SheepPop::preLoop] couldn't get shared array [%s]\n", SHARE_GRASS_MASS_AVAILABLE);
        iResult = -1; 
    } else {
        m_pSEGrass->setInputData(m_pdGrassMassAvailable);
        iResult = SPopulation<SheepAgent>::preLoop();
    }
    return iResult;

}

//----------------------------------------------------------------------------
// addPopSpecificAgentData
// read additional data from pop file
//
int SheepPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    int iResult = 0;

    iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dMass);
    m_aAgents[iAgentIndex].m_dBabyMass = 0;
    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
// extend the agent data type to include additional data in the output
//
void SheepPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    SheepAgent ra;
    H5Tinsert(*hAgentDataType, "Mass",  qoffsetof(ra, m_dMass), H5T_NATIVE_DOUBLE);

}

//----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int SheepPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    //int iThread = omp_get_thread_num();

    //double fM = 31 + this->m_apWELL[iThread]->wrandr(0.0,1.0);
    float fM = m_aAgents[iMother].m_dBabyMass;
    m_aAgents[iAgent].m_dMass = fM;
    m_aAgents[iMother].m_dMass -= fM;
    
    //    m_aAgents[iAgent].m_iMateIndex = -3;
    //printf("birth: M %d (%f), B %d (%f)\n", iMother, m_aAgents[iMother].m_dMass, iAgent, m_aAgents[iAgent].m_dMass);
    return 0;
}
