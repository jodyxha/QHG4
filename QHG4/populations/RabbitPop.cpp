#include <stdio.h>
#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "LayerBuf.cpp"
#include "Prioritizer.cpp"
#include "Action.cpp"

#include "GrassManager.cpp"
#include "RabbitManager.cpp"
#include "Starver.cpp"
#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "AnimalReproducer.cpp"
//#include "MoveStats.cpp"

#include "RabbitPop.h"


//----------------------------------------------------------------------------
// constructor
//
RabbitPop::RabbitPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<RabbitAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
    m_pWM(NULL),
    m_pME(NULL),
    m_pRM(NULL),
    m_pRS(NULL),
    m_pAR(NULL),
    m_pdGrassMassAvailable(NULL),
    m_pdGrassMassConsumed(NULL),
    m_avRabbitLocIDs(NULL),   
    m_adFoxCount(NULL),       
    m_adRabbitMassAvail(NULL),
    m_avRabbitDead(NULL),
    m_adPreferences(NULL) {

    
    ArrayShare *pAS = ArrayShare::getInstance();
    int iMaxNeighbors = this->m_pCG->m_iConnectivity;
    int iArrSize = this->m_pCG->m_iNumCells * (iMaxNeighbors + 1);

    m_adPreferences = new double[iArrSize];
    memset(m_adPreferences, 0, pCG->m_iNumCells*sizeof(double));


    int iResult = 0;

    // create and share rabbit locations/id array
    m_avRabbitLocIDs = new cellnmvec[pCG->m_iNumCells];
    iResult = pAS->shareArray(SHARE_RABBIT_LOC_IDS, pCG->m_iNumCells, m_avRabbitLocIDs);
    if (iResult != 0) {
        printf("Couldnt share array [%p] with name [%s]\n", m_avRabbitLocIDs, SHARE_RABBIT_LOC_IDS.c_str()); 
    }
   
    // create and share rabbit total mass percell array
    m_adRabbitMassAvail = new double[pCG->m_iNumCells];
    iResult = pAS->shareArray(SHARE_RABBIT_MASS_AVAILABLE, pCG->m_iNumCells, m_adRabbitMassAvail);
    if (iResult != 0) {
        printf("Couldnt share array [%p] with name [%s]\n", m_adRabbitMassAvail, SHARE_RABBIT_MASS_AVAILABLE.c_str()); 
    }

    // create and share the dead rabbis array
    m_avRabbitDead = new agcellvec[1];
    iResult = pAS->shareArray(SHARE_RABBIT_DEAD, pCG->m_iNumCells, m_avRabbitDead);
    if (iResult != 0) {
        printf("Couldnt share array [%p] with name [%s]\n", m_avRabbitDead, SHARE_RABBIT_DEAD.c_str()); 
    }

    // create and share fox count array (the rabbit's fox radar)
    m_adFoxCount = new double[pCG->m_iNumCells];  
    // currently, no foxes around
    memset(m_adFoxCount, 0, pCG->m_iNumCells*sizeof(double));
    iResult = pAS->shareArray(SHARE_FOX_COUNT, pCG->m_iNumCells, m_adFoxCount);
    if (iResult != 0) {
        printf("Couldnt share array [%p] with name [%s]\n", m_adFoxCount, SHARE_FOX_COUNT.c_str()); 
    }


    MultiEvaluator<RabbitAgent>::evaluatorinfos mEvalInfo;

    // add grass amount - output array is NULL because it will be set by MultiEvaluator
    m_pSEGrass = new SingleEvaluator<RabbitAgent>(this, m_pCG, "Grass", NULL, (double*)m_pdGrassMassAvailable, "", false, EVENT_ID_NONE, true);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<RabbitAgent>*>("Multi_weight_grass", m_pSEGrass));

    // add fox count - output array is NULL because it will be set by MultiEvaluator
    SingleEvaluator<RabbitAgent> *pSEFox = new SingleEvaluator<RabbitAgent>(this, m_pCG, "Fox", NULL, (double *)m_adFoxCount, "", false, EVENT_ID_NONE, true);
    mEvalInfo.push_back(std::pair<std::string, Evaluator<RabbitAgent>*>("Multi_weight_fox", pSEFox));

    m_pME = new MultiEvaluator<RabbitAgent>(this, m_pCG, "Grass+Fox", m_adPreferences, mEvalInfo, MODE_ADD_SIMPLE, true);  // true: delete evaluators

    m_pWM   = new WeightedMove<RabbitAgent>(this, m_pCG, "", m_apWELL, m_adPreferences);


    m_pRM = new RabbitManager<RabbitAgent>(this, m_pCG, "",
                                           SHARE_RABBIT_MASS_AVAILABLE, SHARE_GRASS_MASS_CONSUMED,
                                           SHARE_RABBIT_LOC_IDS,        SHARE_FOX_COUNT,
                                           SHARE_GRASS_MASS_AVAILABLE,  SHARE_RABBIT_DEAD, m_adPreferences);

    m_pRS   = new Starver<RabbitAgent>(this, m_pCG, "");
 
    m_pAR = new AnimalReproducer<RabbitAgent>(this, m_pCG, "", m_apWELL);

    //    m_pMoveStats = new MoveStats<RabbitAgent>(this, m_pCG, "");

    m_prio.addAction(m_pRM);
    m_prio.addAction(m_pRS);
    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pAR);
    m_prio.addAction(m_pME);
    //    m_prio.addAction(m_pMoveStats);

}

//----------------------------------------------------------------------------
// destructor
//
RabbitPop::~RabbitPop() {
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pRM != NULL) {
        delete m_pRM;
    }
    if (m_pRS != NULL) {
        delete m_pRS;
    }

    /*
    if (m_pMoveStats != NULL) {
        delete m_pMoveStats;
    }
    */
    /* these do not belong to us
    if (m_adGrassAvailable != NULL) {
        delete[] m_adGrassAvailable;
    }

    if (m_adGrassConsumed != NULL) {
        delete[] m_adGrassConsumed;
    }
    */
    if (m_avRabbitLocIDs != NULL) {
        delete[] m_avRabbitLocIDs;
    }

    if (m_adFoxCount != NULL) {
        delete[] m_adFoxCount;
    }

    if (m_adRabbitMassAvail != NULL) {
        delete[] m_adRabbitMassAvail;
    }

    if (m_avRabbitDead != NULL) {
        delete[] m_avRabbitDead;
    }

}

//----------------------------------------------------------------------------
// preLoop
//   read additional data from pop file
//
int RabbitPop::preLoop() {
    int iResult = 0;
    m_pdGrassMassAvailable = (double *) ArrayShare::getInstance()->getArray(SHARE_GRASS_MASS_AVAILABLE);
    if (m_pdGrassMassAvailable == NULL) {
        iResult = -1; 
    } else {
        m_pSEGrass->setInputData(m_pdGrassMassAvailable);
        iResult = SPopulation<RabbitAgent>::preLoop();
    }
    return iResult;

}

//----------------------------------------------------------------------------
// addPopSpecificAgentData
// read additional data from pop file
//
int RabbitPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    int iResult = 0;

    iResult = addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_dMass);
    m_aAgents[iAgentIndex].m_dBabyMass = 0;
    return iResult;
}

//----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
// extend the agent data type to include additional data in the output
//
void RabbitPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    RabbitAgent ra;
    H5Tinsert(*hAgentDataType, "Mass",  qoffsetof(ra, m_dMass), H5T_NATIVE_DOUBLE);

}

//----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int RabbitPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    //int iThread = omp_get_thread_num();

    //double fM = 31 + this->m_apWELL[iThread]->wrandr(0.0,1.0);
    float fM = m_aAgents[iMother].m_dBabyMass;
    m_aAgents[iAgent].m_dMass = fM;
    m_aAgents[iMother].m_dMass -= fM;
    
    //    m_aAgents[iAgent].m_iMateIndex = -3;
    //printf("birth: M %d (%f), B %d (%f)\n", iMother, m_aAgents[iMother].m_dMass, iAgent, m_aAgents[iAgent].m_dMass);
    return 0;
}
