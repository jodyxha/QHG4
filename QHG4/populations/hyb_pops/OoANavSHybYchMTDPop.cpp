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
#include "NPersZHybBirthDeathRel.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "PersOldAgeDeath.cpp"
#include "PersFertility.cpp"
#include "LocEnv.cpp"
#include "PrivParamMix.cpp"
#include "Navigate.cpp"
#include "MoveStats.cpp"
#include "OoANavSHybYchMTDPop.h"

static bool g_bFirst = true;
static double MATC_AVG  = 0.5;
static double MATC_SDEV = 0.05;
static double PATC_AVG  = 0.5;
static double PATC_SDEV = 0.06;

//----------------------------------------------------------------------------
// constructor
//
OoANavSHybYchMTDPop::OoANavSHybYchMTDPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<OoANavSHybYchMTDAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true),
      m_bPendingEvents(false),
      m_dMaternalContribAvg(MATC_AVG),
      m_dMaternalContribSDev(MATC_SDEV),
      m_dPaternalContribAvg(PATC_AVG),
      m_dPaternalContribSDev(PATC_SDEV),
      m_pGeography(pCG->m_pGeography) {

    m_aiNumBirths  = new int*[2];
    m_aiNumBirths[0]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[0], 0, m_pCG->m_iNumCells*sizeof(int));
    m_aiNumBirths[1]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[1], 0, m_pCG->m_iNumCells*sizeof(int));

    m_pLE = new LocEnv<OoANavSHybYchMTDAgent>(this, m_pCG, "", m_apWELL, m_pAgentController); 
    m_pLE->init();
    // addObserver(m_pLE);
    m_pPPM = new PrivParamMix<OoANavSHybYchMTDAgent>(this, m_pCG, "", m_apWELL); 
   
    m_pHybBirthDeathRel = new NPersZHybBirthDeathRel<OoANavSHybYchMTDAgent>(this, m_pCG, "", m_apWELL, m_aiNumBirths);


    m_pWM = new NPersWeightedMove<OoANavSHybYchMTDAgent>(this, m_pCG, "", m_apWELL, m_pLE);

    m_pPair = new RandomPair<OoANavSHybYchMTDAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<OoANavSHybYchMTDAgent>(this, m_pCG, "");

    m_pOAD = new PersOldAgeDeath<OoANavSHybYchMTDAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new PersFertility<OoANavSHybYchMTDAgent>(this, m_pCG, "");

    m_pNavigate = new Navigate<OoANavSHybYchMTDAgent>(this, m_pCG, "", m_apWELL);

    m_pMoveStats = new MoveStats<OoANavSHybYchMTDAgent>(this, m_pCG, "");

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
    m_prio.addAction(m_pMoveStats);

}



///----------------------------------------------------------------------------
// destructor
//
OoANavSHybYchMTDPop::~OoANavSHybYchMTDPop() {

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
    if (m_pMoveStats != NULL) {
        delete m_pMoveStats;
    }
}


///----------------------------------------------------------------------------
// setParams
//   expect pParams to be of the form
//    "ParentContribAvg="<pcavg>+"ParentContribSDev="<pcsdev>]
//
int OoANavSHybYchMTDPop::setParams(const std::string sParams) {
    int iResult = 0;
    if (!sParams.empty()) {
        stringvec vParts;
        uint iNum = splitString(sParams, vParts, "+");
        if (iNum == 2) {
            stringvec vSubs0;
            uint iNumSub0 = splitString(vParts[0], vSubs0, "=");
            if (iNumSub0 == 2) {
                double dAvg = 0;
                if (strToNum(vSubs0[1], &dAvg)) {
                    stringvec vSubs1;
                    uint iNumSub1 = splitString(vParts[1], vSubs1, "=");
                    if (iNumSub1 == 2) {
                        double dSDev = 0;
                        if (strToNum(vSubs1[1], &dSDev)) {
                            const std::string &sAvgName = vSubs0[0];
                            const std::string &sDevName = vSubs1[0];

                            if (sAvgName == "MaternalContribAvg") {
                                m_dMaternalContribAvg = dAvg; 
                            } else if (sAvgName == "PaternalContribAvg") {
                                m_dPaternalContribAvg = dAvg;
                            } else {
                                xha_printf("[OoANavSHybYchMTDPop::setParams] unknown avg contribution [%s]\n", sAvgName);
                                iResult = -1;
                            }

                            if (sDevName == "MaternalContribSDev") {
                                m_dMaternalContribSDev = dSDev;
                            } else if (sDevName == "PaternalContribSDev") {
                                m_dPaternalContribSDev = dSDev;
                            } else {
                                xha_printf("[OoANavSHybYchMTDPop::setParams] unknown sdev contribution [%s]\n", sDevName);
                                iResult = -1;
                            }
  
                        } else {
                            xha_printf("[OoANavSHybYchMTDPop::setParams] bad value for pcsdev [%s]\n", vSubs1[1]);
                            iResult = -1;
                        }
                    } else {
                        xha_printf("[OoANavSHybYchMTDPop::setParams] expected <ParentContribSDev>=<pcsdev> [%s]\n", sParams);
                        iResult = -1;
                    }
                } else {
                    xha_printf("[OoANavSHybYchMTDPop::setParams] bad value for pcavg [%s]\n", vSubs0[1]);
                    iResult = -1;
                }
                        
            } else {
                xha_printf("[OoANavSHybYchMTDPop::setParams] expected <ParentContribSDev>=<pcsdev> [%s]\n", sParams);
                iResult = -1;
            }
        } else {
            xha_printf("[OoANavSHybYchMTDPop::setParams] expected <ParentContribAvg>=<pcavg>+<ParentContribSDev>=<pcsdev> [%s]\n", sParams);
            iResult = -1;

        }
    } else {
        xha_printf("[OoANavSHybYchMTDPop::setParams] empty params: do nothing\n");
        iResult = 0;
    }

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int OoANavSHybYchMTDPop::preLoop() {
    //    xha_printf("!!!intertest!!! OoANavSHybYchMTDPop::preLoop\n");
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<OoANavSHybYchMTDAgent>::preLoop();
   
    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int OoANavSHybYchMTDPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    g_bFirst = true;
    return iResult;
}

///----------------------------------------------------------------------------
//  finalizeStep
//
int OoANavSHybYchMTDPop::finalizeStep() {

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
int OoANavSHybYchMTDPop::updateEvent(int iEventID, char *pData, float fT) {
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
void OoANavSHybYchMTDPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int OoANavSHybYchMTDPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    
    OoANavSHybYchMTDAgent &curMother = m_aAgents[iMother];
    OoANavSHybYchMTDAgent &curFather = m_aAgents[iFather];
    
    OoANavSHybYchMTDAgent &curAgent = m_aAgents[iAgent];
    curAgent.m_fAge = 0.0;
    curAgent.m_fLastBirth = 0.0;
    curAgent.m_iMateIndex = -3;

    //@@TODO: use christophs [B(h1,R1)+B(h2,R2)]/2
    int iT = omp_get_thread_num();

    // random weight for mixing maternal and paternal genetic hybridization
    double p1 = m_dMaternalContribAvg + this->m_apWELL[iT]->wgauss(m_dMaternalContribSDev);
    p1 = (p1 < 0)?0:((p1 > 1)?1:p1);
    curAgent.m_fGenHybM = p1*curMother.m_fGenHybM + (1-p1)*curMother.m_fGenHybP;

    // random weight for mixing maternal and paternal genetic hybridization
    double p2 = m_dPaternalContribAvg + this->m_apWELL[iT]->wgauss(m_dPaternalContribSDev);
    p2 = (p2 < 0)?0:((p2 > 1)?1:p2);
    curAgent.m_fGenHybP = p2*curFather.m_fGenHybM + (1-p2)*curFather.m_fGenHybP;

    // phenetic hybridization is average of genetic hybridizations 
    curAgent.m_fPhenHyb    = (curAgent.m_fGenHybM + curAgent.m_fGenHybP)/2;
    /*
    if (curAgent.m_fGenHybM != curAgent.m_fGenHybP) {
        xha_printf("!!!intertest!!! agent #%d: phen=%f\n", iAgent, curAgent.m_fPhenHyb);
    }
    */
    // so we don't have to change LocEnv & co
    curAgent.m_fHybridization = curAgent.m_fPhenHyb;
   
    
    // if mix mode is 4 (MODE_WEIGHTEDMIX) or 5 (MODE_PUREMIX), the mix parameter should be the hybridisation;
    // for other modes the parameter is ignored 
    m_pPPM->calcParams(curMother, curFather, curAgent, curAgent.m_fHybridization); 

    // handle Ychromosome and mt DNA
    if (curAgent.m_iGender == 0) {
        curAgent.m_imtDNA = curMother.m_imtDNA;
        curAgent.m_iYchr  = curFather.m_iYchr;   // will never be used
    } else {
        curAgent.m_imtDNA = curMother.m_imtDNA;  // will never be used 
        curAgent.m_iYchr  = curFather.m_iYchr;
    }
    // birth statistics
    curAgent.m_iNumBabies = 0;
    curMother.m_iNumBabies++;

    return iResult;
}



///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int OoANavSHybYchMTDPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    OoANavSHybYchMTDAgent &newAgent = m_aAgents[iAgentIndex];
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

            // Ychr and mtDNA have sapiens (0) or neander (1), i.e. the same as the initial Genetic hybridisation
            newAgent.m_iYchr  = newAgent.m_fGenHybP;
            newAgent.m_imtDNA = newAgent.m_fGenHybP;
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
void OoANavSHybYchMTDPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    OoANavSHybYchMTDAgent agent;
    H5Tinsert(*hAgentDataType, "Age",             qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "LastBirth",       qoffsetof(agent, m_fLastBirth), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "GeneticHybM",     qoffsetof(agent, m_fGenHybM), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "GeneticHybP",     qoffsetof(agent, m_fGenHybP), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "PheneticHyb",     qoffsetof(agent, m_fPhenHyb), H5T_NATIVE_FLOAT);
    H5Tinsert(*hAgentDataType, "Ychr",            qoffsetof(agent, m_iYchr), H5T_NATIVE_UCHAR);
    H5Tinsert(*hAgentDataType, "mtDNA",           qoffsetof(agent, m_imtDNA), H5T_NATIVE_UCHAR);
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




