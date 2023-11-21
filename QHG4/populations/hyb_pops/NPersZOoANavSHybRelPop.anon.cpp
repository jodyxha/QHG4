#include <hdf5.h>

#include "EventConsts.h"
#include "WELLUtils.h"
#include "GeneUtils.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "NPersWeightedMove.cpp"
#include "NPersXHybBirthDeathRel.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "PersOldAgeDeath.cpp"
#include "PersFertility.cpp"
#include "LocEnv.cpp"
#include "PrivParamMix.cpp"
#include "Navigate.cpp"
#include "NPersXOoANavSHybRelPop.h"

static bool g_bFirst = true;
static double PC_AVG  = 0.5;
static double PC_SDEV = 0.07;
//----------------------------------------------------------------------------
// constructor
//
NPersXOoANavSHybRelPop::NPersXOoANavSHybRelPop(SCellGrid *pCG, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<NPersXOoANavSHybRelAgent>(pCG, iLayerSize, apIDG, aulState),
      m_bCreateGenomes(true),
      m_bPendingEvents(false),
      m_dParentContribAvg(PC_AVG),
      m_dParentContribSDev(PC_SDEV),
      m_pGeography(pCG->m_pGeography) {

    m_aiNumBirths  = new int*[2];
    m_aiNumBirths[0]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[0], 0, m_pCG->m_iNumCells*sizeof(int));
    m_aiNumBirths[1]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[1], 0, m_pCG->m_iNumCells*sizeof(int));

    m_pLE = new LocEnv<NPersXOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_pAgentController); 
    m_pLE->init();
    // addObserver(m_pLE);
    m_pPPM = new PrivParamMix<NPersXOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL); 
   
    m_pHybBirthDeathRel = new NPersXHybBirthDeathRel<NPersXOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_aiNumBirths);


    m_pWM = new NPersWeightedMove<NPersXOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_pLE);

    m_pPair = new RandomPair<NPersXOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<NPersXOoANavSHybRelAgent>(this, m_pCG, "");

    m_pOAD = new PersOldAgeDeath<NPersXOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new PersFertility<NPersXOoANavSHybRelAgent>(this, m_pCG, "");

    m_pNavigate = new Navigate<NPersXOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    // adding all actions to prioritizer

    m_prio.addAction(m_pWM);
    m_prio.addAction(m_pHybBirthDeathRel);
    m_prio.addAction(m_pPair);
    m_prio.addAction(m_pGO);
    m_prio.addAction(m_pOAD);
    m_prio.addAction(m_pFert);
    m_prio.addAction(m_pLE);
    m_prio.addAction(m_pPPM);
    m_prio.addAction(m_pNavigate);

}



///----------------------------------------------------------------------------
// destructor
//
NPersXOoANavSHybRelPop::~NPersXOoANavSHybRelPop() {

    if (m_aiNumBirths != NULL) {
        delete[] m_aiNumBirths[0];
        delete[] m_aiNumBirths[1];
        delete[] m_aiNumBirths;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
     if (m_pHybBirthDeathRel != NULL) {
        delete m_pHybBirthDeathRel; 
    }
    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pOAD != NULL) {
        delete m_pOAD;
    }
    if (m_pFert != NULL) {
        delete m_pFert;
    }
    if (m_pLE != NULL) {
        delete m_pLE;
    }
    if (m_pPPM != NULL) {
        delete m_pPPM;
    }
    if (m_pNavigate != NULL) {
        delete m_pNavigate;
    }

}


///----------------------------------------------------------------------------
// setParams
//   expect pParams to be of the form
//    "ParentContribAvg="<pcavg>+"ParentContribSDev="<pcsdev>]
//
int NPersXOoANavSHybRelPop::setParams(const std::string sParams) {
    int iResult = 0;
    char sParams[1024];
    if (pParams != NULL) {
        strcpy(sParams, pParams);
        char *p = strtok(sParams, "+");
        while ((iResult == 0) && (p != NULL)) {
            char *p1 = strchr(p, '=');
            if (p1 != NULL) {
                *p1 = '\0';
                 p1++;
                 if (strcmp(p, "ParentContribAvg") == 0) {
                     if (strToNum(p1, &m_dParentContribAvg)) {

                     } else {
                         iResult = -1;
                         printf("[NPersXOoANavSHybRelPop::setParams] expected a float for 'ParentContribAvg', not [%s]\n", p1);
                     }
                 } else if (strcmp(p, "ParentContribSDev") == 0) {
                     if (strToNum(p1, &m_dParentContribSDev)) {

                     } else {
                         iResult = -1;
                         printf("[NPersXOoANavSHybRelPop::setParams] expected a float for 'ParentContribSDev', not [%s]\n", p1);
                     }
                 } else {
                     iResult = -1;
                     printf("[NPersXOoANavSHybRelPop::setParams] unknown parameter  [%s]\n", p1);
                 }
            } else {
                iResult = -1;
                printf("[NPersXOoANavSHybRelPop::setParams] expected a '='\n");
            }
            p = strtok(NULL, "+");
        }
    }
    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int NPersXOoANavSHybRelPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<NPersXOoANavSHybRelAgent>::preLoop();
   
    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int NPersXOoANavSHybRelPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    g_bFirst = true;
    return iResult;
}

///----------------------------------------------------------------------------
//  finalizeStep
//
int NPersXOoANavSHybRelPop::finalizeStep() {

    int iResult = SPopulation::finalizeStep();
    return iResult;
}



///----------------------------------------------------------------------------
// updateEvent
//  react to events
//    EVENT_ID_GEO      : kill agents in water or ice
//    EVENT_ID_CLIMATE  : (NPP is given, so no NPP calculation
//    EVENT_ID_VEG      : update NPP
//    EVENT_ID_NAV      : update seaways
//
int NPersXOoANavSHybRelPop::updateEvent(int iEventID, char *pData, float fT) {
    if (iEventID == EVENT_ID_GEO) {
        // drown  or ice
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != L2List::NIL) {
            int iLastAgent = getLastAgentIndex();
            
            int iCount = 0;
            
#pragma omp parallel for reduction(+:iCount)
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    int iCellIndex = m_aAgents[iAgent].m_iCellIndex;
                    if ((m_pGeography->m_adAltitude[iCellIndex] < 0) ||
                        (m_pGeography->m_abIce[iCellIndex] > 0)) {
                        registerDeath(iCellIndex, iAgent);
                        iCount++;
                    }
                }
            }
            
            // make sure they are removed before step starts
            if (iCount > 0) {
                if (m_bRecycleDeadSpace) {
                    recycleDeadSpaceNew();
                } else {
                    performDeaths();
                }
                // update counts
                updateTotal();
                updateNumAgentsPerCell();
                // clear lists to avoid double deletion
                initListIdx();
            }
        }
        
    }

    notifyObservers(iEventID, pData);
    m_bPendingEvents = true;
    return 0;
};


///----------------------------------------------------------------------------
// flushEvents
//
void NPersXOoANavSHybRelPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int NPersXOoANavSHybRelPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    
    NPersXOoANavSHybRelAgent &curMother = m_aAgents[iMother];
    NPersXOoANavSHybRelAgent &curFather = m_aAgents[iFather];
    
    NPersXOoANavSHybRelAgent &curAgent = m_aAgents[iAgent];
    curAgent.m_fAge = 0.0;
    curAgent.m_fLastBirth = 0.0;
    curAgent.m_iMateIndex = -3;

    /*
    if (iAgent == 969912) {
        printf("[mpso] T %f Agent %d  born in %d, M %d, F %d\n", m_fCurTime, iAgent, curAgent.m_iCellIndex, iMother, iFather);
    }
    if ((iAgent == 237) && (m_fCurTime > 6)) {
        printf("breakli\n");
    }
    */
    //@@TODO: use christophs [B(h1,R1)+B(h2,R2)]/2
   

    //printf("makePopSpecificOffspring A%d before gauss1 hmom%f (sig%f, prev %f)\n", iAgent, curMother.m_fGenHybM, m_dParentContribSDev, m_apWELL[0]->getPrevNormal());
    //WELLUtils::showStates(m_apWELL, omp_get_max_threads(), false);
    double p1 = m_dParentContribAvg + this->m_apWELL[0]->wgauss(m_dParentContribSDev);
    curAgent.m_fGenHybM = p1*curMother.m_fGenHybM + (1-p1)*curMother.m_fGenHybP;
    //printf("makePopSpecificOffspring A%d before gauss2 hdad%f (sig%f, prev %f)\n", iAgent, curFather.m_fGenHybP, m_dParentContribSDev, m_apWELL[0]->getPrevNormal());
    //WELLUtils::showStates(m_apWELL, omp_get_max_threads(), false);
    double p2 = m_dParentContribAvg + this->m_apWELL[0]->wgauss(m_dParentContribSDev);
    curAgent.m_fGenHybP = p2*curFather.m_fGenHybM + (1-p2)*curFather.m_fGenHybP;
    //printf("makePopSpecificOffspring A%d after gauss2\n", iAgent);
    //WELLUtils::showStates(m_apWELL, omp_get_max_threads(), false);
   
    curAgent.m_fPhenHyb    = (curAgent.m_fGenHybM + curAgent.m_fGenHybP)/2;
    // so we don't have to change LocEnv & co
    curAgent.m_fHybridization = curAgent.m_fPhenHyb;


    // if mix mode is 4 (MODE_WEIGHTEDMIX) or 5 (MODE_PUREMIX), the mix parameter should be the hybridisation;
    // for other modes the parameter is ignored 
    m_pPPM->calcParams(curMother, curFather, curAgent, curAgent.m_fHybridization); 


    // child & death stistics
    curAgent.m_iNumBabies = 0;
    curMother.m_iNumBabies++;
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int NPersXOoANavSHybRelPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    NPersXOoANavSHybRelAgent &newAgent  = m_aAgents[iAgentIndex];

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fAge);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fLastBirth);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fAge from [%s]\n", *ppData);
    }

    // last birth are set during the simulation last birth are volatile

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fGenHybM);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fLastBirth from [%s]\n", *ppData);
    }

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fGenHybP);
        if (iResult == 0) {
            newAgent.m_fPhenHyb = (newAgent.m_fGenHybM + newAgent.m_fGenHybP)/2;
            newAgent.m_fHybridization = newAgent.m_fPhenHyb;
        }
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fGenHybMh from [%s]\n", *ppData);
    }

    //@@ TODO: same for all the other fileds
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dMoveProb);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fGenHybP from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dMaxAge);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dMoveProb from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dUncertainty);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dMaxAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fFertilityMinAge);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dUncertainty from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fFertilityMaxAge);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fFertilityMinAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fInterbirth);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fFertilityMaxAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dB0);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_fInterbirth from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dD0);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dB0 from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dTheta);
    } else {
        printf("[addPopSpecificAgentData] Couldn't read m_dD0 from [%s]\n", *ppData);
    }
    
    /* bReal is not persistent
    if (iResult != 0) {
        printf("[addPopSpecificAgentData] Couldn't read m_dBReal from [%s]\n", *ppData);
    }
    */
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void NPersXOoANavSHybRelPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    NPersXOoANavSHybRelAgent agent;
    H5Tinsert(*hAgentDataType, "Age",             qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth",       qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "GeneticHybM",     qoffsetof(agent, m_fGenHybM), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "GeneticHybP",     qoffsetof(agent, m_fGenHybP), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "PheneticHyb",     qoffsetof(agent, m_fPhenHyb), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "MoveProb",        qoffsetof(agent, m_dMoveProb), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "MaxAge",          qoffsetof(agent, m_dMaxAge), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "Uncertainty",     qoffsetof(agent, m_dUncertainty), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "FertilityMinAge", qoffsetof(agent, m_fFertilityMinAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "FertilityMaxAge", qoffsetof(agent, m_fFertilityMaxAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Interbirth",      qoffsetof(agent, m_fInterbirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "B0",              qoffsetof(agent, m_dB0), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "D0",              qoffsetof(agent, m_dD0), H5T_NATIVE_DOUBLE);
    H5Tinsert(*hAgentDataType, "Theta",           qoffsetof(agent, m_dTheta), H5T_NATIVE_DOUBLE);
    /* bReal is not persistent
    H5Tinsert(*hAgentDataType, "BReal",         qoffsetof(agent, m_dBReal), H5T_NATIVE_DOUBLE);
    */
}




