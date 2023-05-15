
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
#include "stdstrutilsT.h"

#include "NPersWeightedMove.cpp"
#include "NPersAHybBirthDeathRel.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "PersOldAgeDeath.cpp"
#include "PersFertility.cpp"
#include "LocEnv.cpp"
#include "PrivParamMix.cpp"
#include "Navigate.cpp"
#include "NPersAOoANavSHybRelPop.h"

static bool g_bFirst = true;
static double SDEV = 0.07;

//----------------------------------------------------------------------------
// constructor
//
NPersAOoANavSHybRelPop::NPersAOoANavSHybRelPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<NPersAOoANavSHybRelAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true),
      m_bPendingEvents(false),
      m_dSDev(SDEV),
      m_pGeography(pCG->m_pGeography) {

    m_aiNumBirths  = new int*[2];
    m_aiNumBirths[0]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[0], 0, m_pCG->m_iNumCells*sizeof(int));
    m_aiNumBirths[1]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[1], 0, m_pCG->m_iNumCells*sizeof(int));

    m_pLE = new LocEnv<NPersAOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_pAgentController); 
    m_pLE->init();
    // addObserver(m_pLE);
    m_pPPM = new PrivParamMix<NPersAOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL); 
   
    m_pHybBirthDeathRel = new NPersAHybBirthDeathRel<NPersAOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_aiNumBirths);


    m_pWM = new NPersWeightedMove<NPersAOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL, m_pLE);

    m_pPair = new RandomPair<NPersAOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<NPersAOoANavSHybRelAgent>(this, m_pCG, "");

    m_pOAD = new PersOldAgeDeath<NPersAOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new PersFertility<NPersAOoANavSHybRelAgent>(this, m_pCG, "");

    m_pNavigate = new Navigate<NPersAOoANavSHybRelAgent>(this, m_pCG, "", m_apWELL);

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
NPersAOoANavSHybRelPop::~NPersAOoANavSHybRelPop() {

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
//   expect pParams to contain the standard deviation
//
int NPersAOoANavSHybRelPop::setParams(const std::string sParams) {
    int iResult = 0;
    
    if (!sParams.empty()) {
        if (strToNum(sParams, &m_dSDev)) {
            iResult = 0;
            stdprintf("[NPersAOoANavSHybRelPop::setParams] m_dSDev set to %f\n", m_dSDev);
        } else {
            iResult = -1;
            stdprintf("[NPersAOoANavSHybRelPop::setParams] expected a float, not [%s]\n", sParams);
        }
    } 
    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int NPersAOoANavSHybRelPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<NPersAOoANavSHybRelAgent>::preLoop();
   
    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int NPersAOoANavSHybRelPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    g_bFirst = true;
    return iResult;
}

///----------------------------------------------------------------------------
//  finalizeStep
//
int NPersAOoANavSHybRelPop::finalizeStep() {
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
int NPersAOoANavSHybRelPop::updateEvent(int iEventID, char *pData, float fT) {
    if (iEventID == EVENT_ID_GEO) {
        // drown  or ice
        int iFirstAgent = getFirstAgentIndex();
        if (iFirstAgent != LBController::NIL) {
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
void NPersAOoANavSHybRelPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int NPersAOoANavSHybRelPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    
    NPersAOoANavSHybRelAgent &curMother = m_aAgents[iMother];
    NPersAOoANavSHybRelAgent &curFather = m_aAgents[iFather];
    
    NPersAOoANavSHybRelAgent &curAgent = m_aAgents[iAgent];
    curAgent.m_fAge = 0.0;
    curAgent.m_fLastBirth = 0.0;
    curAgent.m_iMateIndex = -3;

    //@@TODO: use christophs [B(h1,R1)+B(h2,R2)]/2
    int iT = omp_get_thread_num();
    // get a temporary nea frac from gaussian around actual maternal nea frac
    double nM = curMother.m_fNeaFrac + this->m_apWELL[iT]->wgauss(curMother.m_fNeaSDev);
    nM = (nM < 0)?0:((nM > 1)?1:nM);
    // get a temporary nea frac from gaussian around actual paternal nea frac
    double nP = curFather.m_fNeaFrac + this->m_apWELL[iT]->wgauss(curFather.m_fNeaSDev);
    nP = (nP < 0)?0:((nP > 1)?1:nP);
    
    // actual nea frac is average of parental temporary nea fracs 
    curAgent.m_fNeaFrac = (nM + nP)/2;
    // get new nea sdev from 'V' function of nea frac 
    curAgent.m_fNeaSDev = m_dSDev * (1 - 2.0*fabs(0.5 -  curAgent.m_fNeaFrac));

    // so we don't have to change LocEnv & co
    curAgent.m_fHybridization = curAgent.m_fNeaFrac;
    

    // if mix mode is 4 (MODE_WEIGHTEDMIX) or 5 (MODE_PUREMIX), the mix parameter should be the hybridisation;
    // for other modes the parameter is ignored 
    m_pPPM->calcParams(curMother, curFather, curAgent, curAgent.m_fHybridization); 

    // birth statistics
    curAgent.m_iNumBabies = 0;
    curMother.m_iNumBabies++;

    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int NPersAOoANavSHybRelPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    NPersAOoANavSHybRelAgent &newAgent  = m_aAgents[iAgentIndex];
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fAge);
    }
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fLastBirth);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_fAge from [%s]\n", *ppData);
    }

    // last birth are set during the simulation last birth are volatile

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fNeaFrac);
        if (iResult == 0) {
            newAgent.m_fHybridization = newAgent.m_fNeaFrac;
         }
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_fLastBirth from [%s]\n", *ppData);
    }

    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fNeaSDev);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_fNeaFrac from [%s]\n", *ppData);
    }

    //@@ TODO: same for all the other fileds
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dMoveProb);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_fNeaSDev from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dMaxAge);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_dMoveProb from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dUncertainty);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_dMaxAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fFertilityMinAge);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_dUncertainty from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fFertilityMaxAge);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_fFertilityMinAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_fInterbirth);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_fFertilityMaxAge from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dB0);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_fInterbirth from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dD0);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_dB0 from [%s]\n", *ppData);
    }
    
    if (iResult == 0) {
        iResult = addAgentDataSingle(ppData, &newAgent.m_dTheta);
    } else {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_dD0 from [%s]\n", *ppData);
    }
    
    /* bReal is not persistent
    if (iResult != 0) {
        stdprintf("[addPopSpecificAgentData] Couldn't read m_dBReal from [%s]\n", *ppData);
    }
    */
    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void NPersAOoANavSHybRelPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    NPersAOoANavSHybRelAgent agent;
    H5Tinsert(*hAgentDataType, "Age",             qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth",       qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "NeanderFraction", qoffsetof(agent, m_fNeaFrac), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "NeanderSDev",     qoffsetof(agent, m_fNeaSDev), H5T_NATIVE_FLOAT);
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




