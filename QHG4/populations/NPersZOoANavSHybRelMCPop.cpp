#include <hdf5.h>
#include <omp.h>


#include "EventConsts.h"
#include "Geography.h"
#include "WELLUtils.h"
#include "GeneUtils.h"
#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
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
#include "NPersZOoANavSHybRelMCPop.h"

static bool g_bFirst = true;
static double MATC_AVG  = 0.5;
static double MATC_SDEV = 0.05;
static double PATC_AVG  = 0.5;
static double PATC_SDEV = 0.06;

//----------------------------------------------------------------------------
// constructor
//
NPersZOoANavSHybRelMCPop::NPersZOoANavSHybRelMCPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) 
    : SPopulation<NPersZOoANavSHybRelMCAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds),
      m_bCreateGenomes(true),
      m_bPendingEvents(false),
      m_dMaternalContribAvg(MATC_AVG),
      m_dMaternalContribSDev(MATC_SDEV),
      m_dPaternalContribAvg(PATC_AVG),
      m_dPaternalContribSDev(PATC_SDEV) {

    m_aiNumBirths  = new int*[2];
    m_aiNumBirths[0]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[0], 0, m_pCG->m_iNumCells*sizeof(int));
    m_aiNumBirths[1]  = new int[m_pCG->m_iNumCells];
    memset(m_aiNumBirths[1], 0, m_pCG->m_iNumCells*sizeof(int));

    m_pLE = new LocEnv<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "", m_apWELL, m_pAgentController); 
    m_pLE->init();
    // addObserver(m_pLE);
    m_pPPM = new PrivParamMix<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "", m_apWELL); 
   
    m_pHybBirthDeathRel = new NPersZHybBirthDeathRel<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "", m_apWELL, m_aiNumBirths);


    m_pWM = new NPersWeightedMove<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "", m_apWELL, m_pLE);

    m_pPair = new RandomPair<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "", m_apWELL);

    m_pGO = new GetOld<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "");

    m_pOAD = new PersOldAgeDeath<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "", m_apWELL);

    m_pFert = new PersFertility<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "");

    m_pNavigate = new Navigate<NPersZOoANavSHybRelMCAgent>(this, m_pCG, "", m_apWELL);

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
NPersZOoANavSHybRelMCPop::~NPersZOoANavSHybRelMCPop() {

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
int NPersZOoANavSHybRelMCPop::setParams(const std::string sParams) {
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
                                stdprintf("[NPersZOoANavSHybRelMCPop::setParams] unknown avg contribution [%s]\n", sAvgName);
                                iResult = -1;
                            }

                            if (sDevName == "MaternalContribSDev") {
                                m_dMaternalContribSDev = dSDev;
                            } else if (sDevName == "PaternalContribSDev") {
                                m_dPaternalContribSDev = dSDev;
                            } else {
                                stdprintf("[NPersZOoANavSHybRelMCPop::setParams] unknown sdev contribution [%s]\n", sDevName);
                                iResult = -1;
                            }
  
                        } else {
                            stdprintf("[NPersZOoANavSHybRelMCPop::setParams] bad value for pcsdev [%s]\n", vSubs1[1]);
                            iResult = -1;
                        }
                    } else {
                        stdprintf("[NPersZOoANavSHybRelMCPop::setParams] expected <ParentContribSDev>=<pcsdev> [%s]\n", sParams);
                        iResult = -1;
                    }
                } else {
                    stdprintf("[NPersZOoANavSHybRelMCPop::setParams] bad value for pcavg [%s]\n", vSubs0[1]);
                    iResult = -1;
                }
                        
            } else {
                stdprintf("[NPersZOoANavSHybRelMCPop::setParams] expected <ParentContribSDev>=<pcsdev> [%s]\n", sParams);
                iResult = -1;
            }
        } else {
            stdprintf("[NPersZOoANavSHybRelMCPop::setParams] expected <ParentContribAvg>=<pcavg>+<ParentContribSDev>=<pcsdev> [%s]\n", sParams);
            iResult = -1;

        }
    } else {
        stdprintf("[NPersZOoANavSHybRelMCPop::setParams] empty params: do nothing\n");
        iResult = 0;
    }

    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int NPersZOoANavSHybRelMCPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = SPopulation<NPersZOoANavSHybRelMCAgent>::preLoop();

    // get the position vectors for all nodes
    Geography *pGeo = m_pCG->m_pGeography;
    for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
        double dLon = pGeo->m_adLongitude[i];
        double dLat = pGeo->m_adLatitude[i];
        sphercoord lola(dLon, dLat);
        Vec3D vPos = polarD2Cart(lola);
        m_mVectors[i] = vPos;
    }
   
    return iResult;
}


///----------------------------------------------------------------------------
//  initializeStep
//
int NPersZOoANavSHybRelMCPop::initializeStep(float fCurTime) {
    int iResult = SPopulation::initializeStep(fCurTime);
    g_bFirst = true;
    return iResult;
}

///----------------------------------------------------------------------------
//  finalizeStep
//
int NPersZOoANavSHybRelMCPop::finalizeStep() {

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
int NPersZOoANavSHybRelMCPop::updateEvent(int iEventID, char *pData, float fT) {
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
                    if ((this->m_pCG->m_pGeography->m_adAltitude[iCellIndex] < 0) ||
                        (this->m_pCG->m_pGeography->m_abIce[iCellIndex] > 0)) {
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
void NPersZOoANavSHybRelMCPop::flushEvents(float fT) {
    if (m_bPendingEvents) {
        notifyObservers(EVENT_ID_FLUSH, NULL);
        m_bPendingEvents = false;
    }
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int NPersZOoANavSHybRelMCPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    int iResult = 0;    
    NPersZOoANavSHybRelMCAgent &curMother = m_aAgents[iMother];
    NPersZOoANavSHybRelMCAgent &curFather = m_aAgents[iFather];
    
    NPersZOoANavSHybRelMCAgent &curAgent = m_aAgents[iAgent];
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

    // so we don't have to change LocEnv & co
    curAgent.m_fHybridization = curAgent.m_fPhenHyb;
   
    
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
int NPersZOoANavSHybRelMCPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {
    int iResult = 0;
    NPersZOoANavSHybRelMCAgent &newAgent  = m_aAgents[iAgentIndex];

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
void NPersZOoANavSHybRelMCPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    NPersZOoANavSHybRelMCAgent agent;
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



//----------------------------------------------------------------------------
// performMoves
//  update the migration counts, then
//  do all the moves registered by registerMove()
//
int NPersZOoANavSHybRelMCPop::performMoves() {
    int iResult = 0;

    std::map<cellpair,int> *pmCurCounts = new  std::map<cellpair,int>[omp_get_num_threads()];
    int iMoveQueueDataSize = 3; // single move: fromCell, AgentID, toCell


    //#pragma omp parallel
    for (int iT = 0; iT <  omp_get_num_threads(); iT++)
    {
        //int iT = omp_get_thread_num();
        for (uint i = 0; i <m_pCG->m_iNumCells; i++) {
            int iN = m_pCG->m_aCells[i].m_iNumNeighbors;
            for (int j = 0; j < iN; j++) {
                pmCurCounts[iT][cellpair(i, m_pCG->m_aCells[i].m_aNeighbors[j])] = 0;
            }
            if (iN < 6) {
                pmCurCounts[iT][cellpair(i, -1)] = 0;
            }
        }
    }

   //#pragma omp parallel
    for (int iT = 0; iT <  omp_get_num_threads(); iT++)
    {
        //int iT = omp_get_thread_num();
        //        printf("movelist[%d]: %zd values\n", iT, m_vMoveList[iT]->size());
        for (unsigned int iIndex = 0; iIndex < m_vMoveList[iT]->size(); iIndex += iMoveQueueDataSize) {
            //std::map<int>::const_iterator it = m_mStartCells.find((*m_vMoveList[iT])[iIndex]);
            if (true /* (it != m_mStartCells.end()*/) {
                printf("Migration T %f b %d e %d a %d\n", m_fCurTime, (*m_vMoveList[iT])[iIndex], (*m_vMoveList[iT])[iIndex+2], (*m_vMoveList[iT])[iIndex+1]);
                pmCurCounts[iT][cellpair((*m_vMoveList[iT])[iIndex],(*m_vMoveList[iT])[iIndex+2])]++;
            }
        }
        //        printf("curcounts[%d]: %zd entries\n", iT, pmCurCounts[iT].size());
    }    
    for (int iT = 0; iT < omp_get_num_threads(); ++iT) {
        std::map<cellpair,int>::const_iterator it;
        for (it = pmCurCounts[iT].begin(); it != pmCurCounts[iT].end(); ++it) {
            //            printf("adding vector for (%d %d)\n", it->first.first, it->first.second);
            //           m_mvMigCounts[it->first].push_back(it->second);
            m_mvMigCounts[it->first]+=it->second;
        }
    }
    delete[] pmCurCounts;
    
    // now the actual moves
    iResult = SPopulation<NPersZOoANavSHybRelMCAgent>::performMoves();
    return iResult;
}

//----------------------------------------------------------------------------
// condenseCounts
//  xondense the counts into sclaed direction vectors 
//  (data to be stored separately from agent data, e.g. ancestors IDs)
//
int  NPersZOoANavSHybRelMCPop::condenseCounts() {
    
    // clear the diff vectors
    for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
        m_mDiffs[i].set(0,0,0);
    }
    // write mig stuff
    std::map<cellpair, int> mSummed;
    migcounts::const_iterator it;
    printf("MigCounts.size %zd\n", m_mvMigCounts.size());
    for (it = m_mvMigCounts.begin(); it != m_mvMigCounts.end(); ++it) {
        //        printf("(%d,%d)=>[", it->first.first, it->first.second);
        
        // time integration: sum up counts along the count vectors
        /*
        int s = 0;
        for (uint h = 0; h < it->second.size(); h++) {
            //            printf(" %+4d", it->second[h]);
            s += it->second[h];
        }
        //        printf("]\n");
        */
        mSummed[it->first] = it->second; // s;

        // calc the vector -> normalize -> scale with sum
        if (it->first.second >= 0) {
            printf("subtracting (%d)[%f,%f,%f] ",  it->first.first,  m_mVectors[it->first.first].m_fX,  m_mVectors[it->first.first].m_fY,  m_mVectors[it->first.first].m_fZ);
            printf("       from (%d)[%f,%f,%f]\n", it->first.second, m_mVectors[it->first.second].m_fX, m_mVectors[it->first.second].m_fY, m_mVectors[it->first.second].m_fZ);
            fflush(stdout);
            // direction vector: normalize & scale
            //     std::map<int, Vec3D*> ::const_iterator it2 = m_mDiffs.find(it->first.first);
       
            Vec3D vDiff = m_mVectors[it->first.second] -m_mVectors[it->first.first];
            printf("       result   [%f,%f,%f]\n", vDiff.m_fX, vDiff.m_fY, vDiff.m_fZ);
            vDiff.normalize();
            vDiff *= mSummed[it->first]/* *1000*/;
            printf("       normscale[%f,%f,%f]\n", vDiff.m_fX, vDiff.m_fY, vDiff.m_fZ);

            m_mDiffs[it->first.first] += vDiff;
        }

    }
    // clear the counts for the next batch
    m_mvMigCounts.clear();

    return 0;
}


#define VECTOR_GROUP_NAME "VectorData"
#define VECTOR_SET_NAME "VectorDataSet"

//----------------------------------------------------------------------------
// opencreateGroup
//   if the group exists, open it
//   otherwise create it
//
hid_t opencreateGroup(hid_t hLoc, const char *pName) {
    hid_t hNewGroup = H5P_DEFAULT;
    
    hNewGroup = qdf_openGroup(hLoc, pName);
    if (hNewGroup != H5P_DEFAULT) {
        // already exists
        // do nothing
    
    } else {
        // does not exist: create
        hNewGroup = qdf_createGroup(hLoc, pName);
        if (hNewGroup != H5P_DEFAULT) {
            // success
            // do nothingiResult = 0;
        } else {
            hNewGroup = H5P_DEFAULT;
        }
    }
    return hNewGroup;
}
    

//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//  write additional data to the group 
//  (data to be stored separately from agent data, e.g. ancestors IDs)
//
int  NPersZOoANavSHybRelMCPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQD] start\n");
    int iResult = -1;
    condenseCounts();

    hid_t hVecGroup = opencreateGroup(hSpeciesGroup, VECTOR_GROUP_NAME);
    // we'll do 2 arrays: positions, and vectors

    if (hVecGroup != H5P_DEFAULT) {    
        hid_t hVecSubGroup = opencreateGroup(hVecGroup, "migcount");
        if (hVecSubGroup != H5P_DEFAULT) {    
            printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQD] have subgroup [%s]\n", "migcount");
            int iNumVecs = m_pCG->m_iNumCells;
            iResult = qdf_insertAttribute(hVecSubGroup, "NumVecs",  1, &iNumVecs);
            if (iResult == 0) {
                //hsize_t dims[1] = {m_pCG->m_iNumCells*6};
                //hid_t hDataSpace = H5Screate_simple(1, dims, NULL);
                hsize_t dims[2] = {m_pCG->m_iNumCells, 6};
                hid_t hDataSpace = H5Screate_simple(2, dims, NULL);
                if (hDataSpace != H5P_DEFAULT) {
                    printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQD] have data space\n");
                    hid_t hDataSet = H5Dcreate2(hVecSubGroup, VECTOR_SET_NAME, H5T_NATIVE_DOUBLE, hDataSpace, 
                                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                    if (hDataSet != H5P_DEFAULT) {
                        printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQD] have data set\n");
                        //hid_t hMemSpace = H5Screate_simple (1, dims, NULL); 
                        hid_t hMemSpace = H5Screate_simple (2, dims, NULL); 
                        double *pData = new double[m_pCG->m_iNumCells*6];
                        double *pCur = pData;
                        for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                    
                            std::map<int, Vec3D>::const_iterator it = m_mDiffs.find(i);
                            memcpy(pCur, &(m_mVectors[i].m_fX), 3*sizeof(double));
                            pCur += 3;
                            if (it != m_mDiffs.end()) {
                                memcpy(pCur, &(it->second.m_fX), 3*sizeof(double));
                            } else {
                                memset(pCur, 0, 3*sizeof(double));
                            }
                            pCur += 3;
                    
                        }
                        printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQD] now write it\n");
                        herr_t status = H5Dwrite(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                                                 hDataSpace, H5P_DEFAULT, pData);

                        qdf_closeDataSpace(hMemSpace); 
         
                        qdf_closeDataSet(hDataSet);

                        iResult = (status >= 0)?0:1;

                    } else {
                        printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQDF] couldn't create dataset\n");
                    }
                    qdf_closeDataSpace(hDataSpace);
                } else {
                    printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQDF] couldn't create dataspace\n");

                }
            } else {
                printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQDF] couldn't create attribute\n");
            }
            qdf_closeGroup(hVecSubGroup);
        } else {
            printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQDF] couldn't create vecSubGroup [%s]\n", "migcount");
        }

        qdf_closeGroup(hVecGroup);

    } else {
        printf("[NPersZOoANavSHybRelMCPop::writeAdditionalDataQDF] couldn't create VecGroup [%s]\n", VECTOR_GROUP_NAME);
        
    }

    return iResult;
    
}
