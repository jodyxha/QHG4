#ifndef __SPOPULATION_CPP__
#define __SPOPULATION_CPP__

#include <cstdio>
#include <cstring>
#include <omp.h>
#include <cmath>

#include <map>
#include <vector>
#include <algorithm>

#include "types.h"
#include "strutils.h"
#include "stdstrutilsT.h"
#include "geomutils.h"
#include "crypto.h"
#include "WELLUtils.h"
#include "WELLDumpRestore.h"
#include "LineReader.h"
#include "SCell.h"
#include "SCellGrid.h"
#include "PopFinder.h"
#include "IDGen.h"
#include "stacktrace.h"

#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "ParamProvider2.h"
#include "LayerBuf.h"
#include "LBController.h"

#include "SPopulation.h"

static const int BQD_SIZE = 3; // birth queue data size (cellindex, motherindex, fatherindex)

static double fPerfBirthTime = 0;


#define ABUFSIZE 16384

//----------------------------------------------------------------------------
// constructor
//
template<typename T>
SPopulation<T>::SPopulation(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, 
                            IDGen **apIDGen, uint32_t *aulState, uint *piSeeds) 
      : m_iNumCells(pCG->m_iNumCells), 
        m_iTotal(0),
        m_iNumBirths(0),
        m_iNumDeaths(0),
        m_iNumMoves(0),
        m_aiNumAgentsPerCell(NULL),
        m_aiTempAgentsPerCell(NULL),
        m_fCurTime(-1),
        m_pCG(pCG),
        m_pPopFinder(pPopFinder),
        m_pAgentController(NULL),
        m_pWriteCopyController(NULL),
        m_iMaxID(0),
        m_apIDGen(apIDGen),
        m_iMaxReuse(1024),
        m_iMaxNormalB(1024),
        m_iMaxNormalD(1024),
        m_iNumPrevDeaths(0),
        m_bRecycleDeadSpace(true),
        m_hAgentDataType(H5P_DEFAULT),
        m_iQDFVersionIn(4),
        m_iQDFVersionOut(4),
        m_pReadBuf(NULL) {

    m_iNumThreads = omp_get_max_threads();
    stdprintf("Have %d threads\n", m_iNumThreads);

    m_sSpeciesName = "";
    m_sClassName   = "";

//    m_dRecycleDeadTime = 0;
    
    memcpy(m_aulInitialState, aulState, STATE_SIZE*sizeof(uint32_t));
    memcpy(m_aiSeeds, piSeeds, NUM_SEEDS*sizeof(int));

    prepareLists(iLayerSize, iLayerSize, aulState);    



    // the species specific arrays must be created here, 
    // as they might be needed in derived constructors


}


//----------------------------------------------------------------------------
// destructor
//
template<typename T>
SPopulation<T>::~SPopulation() {

//    stdprintf("TIME TAKEN TO RECYCLE DEAD SPACE: %f\n",m_dRecycleDeadTime);

    delete[] m_aiNumAgentsPerCell;
    delete m_pAgentController;
    delete m_pWriteCopyController;

    for (int iT = 0; iT < m_iNumThreads; iT++) {
        delete m_apWELL[iT];
        delete m_vMoveList[iT];
        delete m_vBirthList[iT];
        delete m_vDeathList[iT];
        delete[] m_aiTempAgentsPerCell[iT];
    }
    delete[] m_apWELL;

    delete[] m_vBirthList;
    delete[] m_vDeathList;
    delete[] m_vMoveList;

    delete[] m_aiTempAgentsPerCell;

    delete[] m_pReuseB;
    delete[] m_pNormalB;
    delete[] m_pPrevD;

    delete[] m_pReadBuf;

    //    H5Tclose(m_hAgentDataType);

    stdprintf("*** %s: time spent in performBirths(ulong iNumBirths, int *piBirthData): %f\n", m_sSpeciesName, fPerfBirthTime);
    stdprintf("*** total number of births  %lu\n", m_iNumBirths);
    stdprintf("*** total number of deaths  %lu\n", m_iNumDeaths);
    stdprintf("*** total number of moves   %lu\n", m_iNumMoves);

}


//----------------------------------------------------------------------------
// prepareLists
//
template<typename T>
void SPopulation<T>::prepareLists(int iAgentLayerSize,int iListLayerSize, uint32_t *aulState) {

    m_aiNumAgentsPerCell = new ulong[m_iNumCells];

    m_pAgentController = new LBController;

    m_aAgents.init(iAgentLayerSize);
    m_pAgentController->init(iAgentLayerSize);
    m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aAgents));

    m_pWriteCopyController = new LBController;

    m_aWriteCopy.init(iAgentLayerSize);
    m_pWriteCopyController->init(iAgentLayerSize);
    m_pWriteCopyController->addBuffer(static_cast<LBBase *>(&m_aWriteCopy));
    m_pWriteCopyController->addLayer();

    stdprintf("Making array lists (%d entries)\n", m_iNumThreads);
    m_vMoveList  = new std::vector<int>*[m_iNumThreads];
    m_vBirthList = new std::vector<int>*[m_iNumThreads];
    m_vDeathList = new std::vector<int>*[m_iNumThreads];
    m_apWELL = new WELL512*[m_iNumThreads];

    m_aiTempAgentsPerCell = new ulong*[m_iNumThreads];

#pragma omp parallel 
    {
        int iT = omp_get_thread_num();
        unsigned int temp[STATE_SIZE]; 
        for (unsigned int j = 0; j < STATE_SIZE; j++) {
            temp[j] = aulState[(iT+13*j)%16];
        }
        // create new objects to make sure array[i] is on processor i
        m_apWELL[iT]     = new WELL512(temp);
        m_vMoveList[iT]  = new std::vector<int>;
        m_vBirthList[iT] = new std::vector<int>;
        m_vDeathList[iT] = new std::vector<int>;
        m_aiTempAgentsPerCell[iT] = new ulong[m_iNumCells];;
    }

    m_pReuseB  = new int[BQD_SIZE*m_iMaxReuse];
    m_pNormalB = new int[BQD_SIZE*m_iMaxNormalB];
    m_pPrevD   = new int[m_iMaxNormalD];

    m_pReadBuf = new T[ABUFSIZE];

}

/*
//----------------------------------------------------------------------------
// showStates
//
template<typename T>
void SPopulation<T>::showStates(bool bFull) {
    char *sStates = new char [m_iNumThreads * 256];
    *sStates = '\0';
    for (int i = 0; i < m_iNumThreads; i++) {
        char sTemp[256];
        sprintf(sTemp, "[%08x] ", m_apWELL[i]->getIndex());
        strcat(sStates, sTemp);
        m_apWELL[i]->state2String(sTemp);
        strcat(sStates, sTemp);
        strcat(sStates, "\n");
    }
    uchar umd5[crypto::MD5_SIZE];
    char  smd5[2*crypto::MD5_SIZE+1];
    *smd5 = '\0';

    crypto::md5sumS(sStates, umd5);
    for (int i = 0; i < crypto::MD5_SIZE; i++) {
        char s[4];
        sprintf(s, "%02x", umd5[i]);
        strcat(smd5, s);
    }

    stdprintf("WELL hash %s\n", smd5);
    if (bFull) {
        stdprintf("%s", sStates);
    }
    delete[] sStates;
}
*/

//----------------------------------------------------------------------------
// randomize
//  do some random numbers 
//  (this leads to a shifted random number sequence for the simulation)
//
template<typename T>
void SPopulation<T>::randomize(int i) {
    stdprintf("WARNING!!!  RANDOMIZE WAS CALLED!!!\n");
    int iT = omp_get_thread_num();
    double dDummy = 0;
    for (unsigned int j = 0; j < STATE_SIZE; j++) {
        for (int k = 0; k < i; k++) {
            dDummy += m_apWELL[iT]->wrandd();
        }
    }
    
}


//----------------------------------------------------------------------------
// getUID
//  thread i gives IDs i, numthr+i, 2*NumThr+i etc
//
template<typename T>
idtype SPopulation<T>::getUID() {
    int iThread = omp_get_thread_num();
    return m_apIDGen[iThread]->getID();
}


/*
/@@ to be rempved
//----------------------------------------------------------------------------
// getNumAgents
//
template<typename T>
ulong SPopulation<T>::getNumEvents(int iEventMask) {
    ulong iNum = 0;
    if ((iEventMask & EVMSK_BIRTHS) != 0) {
        iNum += m_iNumBirths;
    }
    if ((iEventMask & EVMSK_MOVES) != 0) {
        iNum += m_iNumMoves;
    }
    if ((iEventMask & EVMSK_DEATHS) != 0) {
        iNum += m_iNumDeaths;
    }
    return iNum;
}
//@@t until here
*/


//----------------------------------------------------------------------------
// setPrioList
//
template<typename T>
int SPopulation<T>::setPrioList() {
    int iResult = 0;
    std::map<std::string, int>::const_iterator it;
    for (it = m_mPrioInfo.begin(); it != m_mPrioInfo.end(); ++it) {
        // record priority information into the Prioritizer class m_prio
        iResult += m_prio.setPrio(it->second, it->first);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// setAgentDataType
//
template<typename T>
void SPopulation<T>::setAgentDataType() {
    stdprintf("[SPopulation<T>::setAgentDataType] setting agent data type\n");
    m_hAgentDataType = createAgentDataTypeQDF();
}


//----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int SPopulation<T>::preLoop() {

   int iResult = 0;
   //showWELLStates("preLoop", true);
   setAgentDataType();
   
   if (!m_prio.empty()) {
       for (uint i = 0; (iResult == 0) && (i <= m_prio.getMaxPrio()); i++) {
           for (uint j = 0; (iResult == 0) && (j < m_prio.getNumActionsForPrio(i)); j++) { 
               Action<T> *pA = m_prio.getAction(i,j);
               iResult = pA->preLoop();
           }
       }
   }

   updateTotal();
   updateNumAgentsPerCell();

   return iResult;
}


//----------------------------------------------------------------------------
// postLoop
//
template<typename T>
int SPopulation<T>::postLoop() {

   int iResult = 0;
 
   if (!m_prio.empty()) {
       for (uint i = 0; (iResult == 0) && (i <= m_prio.getMaxPrio()); i++) {
           for (uint j = 0; (iResult == 0) && (j < m_prio.getNumActionsForPrio(i)); j++) { 
               Action<T> *pA = m_prio.getAction(i,j);
               iResult = pA->postLoop();
           }
       }
   }
   return iResult;
}


//----------------------------------------------------------------------------
// preWrite
//
template<typename T>
int SPopulation<T>::preWrite(float fTime) {

   int iResult = 0;
 
   if (!m_prio.empty()) {
       for (uint i = 0; (iResult == 0) && (i <= m_prio.getMaxPrio()); i++) {
           for (uint j = 0; (iResult == 0) && (j < m_prio.getNumActionsForPrio(i)); j++) { 
               Action<T> *pA = m_prio.getAction(i,j);
               iResult = pA->preWrite(fTime);
           }
       }
   }
   return iResult;
}


//----------------------------------------------------------------------------
// showWELLStates
//
template<typename T>
    void SPopulation<T>::showWELLStates(const std::string sCaption, bool bNice) {
    for (int iThread = 0; iThread < m_iNumThreads; ++iThread) {
        stdprintf("%s State of thread %02d:", sCaption,  iThread);
        if (bNice) {
            stdprintf("\n");
        } else {
            stdprintf("  ");
        }
        const uint32_t *pCurState = m_apWELL[iThread]->getState();
        char sState[128];
        for (uint i = 0; i < STATE_SIZE/4; i++) {
            *sState = '\0';                
            for (uint j = 0; j < 4; j++) {
                char sDig[16];
                sprintf(sDig, " %08x", pCurState[4*i+j]);
                strcat(sState, sDig);
            }
            stdprintf("    %s", sState);
            if (bNice) {
                stdprintf("\n");
            }
        }
        stdprintf("\n");
    }
}


//----------------------------------------------------------------------------
// hasAction
//
template<typename T>
bool SPopulation<T>::hasAction(const std::string sAction) {

    bool bResult = m_prio.hasAction(sAction);

   return bResult;
}


//----------------------------------------------------------------------------
// hasParam
//
template<typename T>
bool SPopulation<T>::hasParam(const std::string sParam) {

   bool bResult =  m_prio.hasParam(sParam);

   return bResult;
}


//----------------------------------------------------------------------------
// initializeStep
//
template<typename T>
int SPopulation<T>::initializeStep(float fTime) {
 
    m_fCurTime = fTime;

    int iResult = 0;

    if (!m_prio.empty()) {
        for (uint i = 0; i <= m_prio.getMaxPrio(); i++) {
            for (uint j = 0; j < m_prio.getNumActionsForPrio(i) && iResult == 0; j++) { 
                Action<T> *pA = m_prio.getAction(i,j);
                iResult = pA->initialize(fTime);
            }
        }
    }
    /*
    updateTotal();
    updateNumAgentsPerCell();
    initListIdx();
    stdprintf("%d %s agents ready for step\n", m_iTotal, m_sSpeciesName);
    */


    return iResult;
}


//----------------------------------------------------------------------------
// initLists
//
template<typename T>
void SPopulation<T>::initListIdx() {

#pragma omp parallel for
   for (int i = 0; i < m_iNumThreads; i++) {
        m_vBirthList[i]->clear();
        m_vDeathList[i]->clear();
        m_vMoveList[i]->clear();
    }
}


//----------------------------------------------------------------------------
// finalizeStep
//
template<typename T>
int SPopulation<T>::finalizeStep() {    

    int iResult = 0;


    if (!m_prio.empty()) {
        for (uint i = 0; i <= m_prio.getMaxPrio(); i++) {
            for (uint j = 0; j < m_prio.getNumActionsForPrio(i) && iResult == 0; j++) {
                Action<T> *pA = m_prio.getAction(i,j);
                iResult = pA->finalize(m_fCurTime);
                if (iResult != 0) {
                    stdprintf("Prio:%d, index:%d, act:%s\n", i, j, pA->getActionName());
	    }
            }
        }
    }

    if (iResult == 0) {
        if (m_bRecycleDeadSpace) {
            recycleDeadSpaceNew();
        } else {
	    performBirths();
            performDeaths();
        }
        performMoves();    
        // compacting at every step is too expeensive:
        //   compactData();
        
        updateTotal();
        updateNumAgentsPerCell();
        initListIdx();
        //stdprintf("after step %d total %lu %s agents\n", (int)m_fCurTime, m_iTotal - m_iNumPrevDeaths, m_sSpeciesName);
    } else {
        stdprintf("finalize result: %d!!!!\n", iResult);
    }


    return iResult;
}


//----------------------------------------------------------------------------
// compactData
//
template<typename T>
void SPopulation<T>::compactData() {
    if (m_pAgentController->getNumUsed() > 0) {
        m_pAgentController->compactData();
    }
    
    /*
    m_pAgentController->checkLists();
    fprintf(stderr,"after compact data: first index %d, last index %d, numagents %d\n",getFirstAgentIndex(),getLastAgentIndex(),m_iTotal);
    fprintf(stderr,"PASSIVE: first %d, last %d\n",m_pAgentController->getFirstIndex(LBController::PASSIVE), m_pAgentController->getLastIndex(LBController::PASSIVE));
    if (m_iTotal != getLastAgentIndex()+1) {
        for (int iA = getFirstAgentIndex(); iA <= getLastAgentIndex(); iA++) {
            if (m_aAgents[iA].m_iLifeState == 0) {
                fprintf(stderr,"agent %d born at %f is dead\n",iA,m_aAgents[iA].m_fBirthTime);
            }
        }
    }
    */
}


//----------------------------------------------------------------------------
// updateNumAgentsPerCell
//  recalculate number of agents in each cell
//
template <typename T>
void SPopulation<T>::updateNumAgentsPerCell() {
    bzero(m_aiNumAgentsPerCell, m_iNumCells * sizeof(ulong));
#pragma omp parallel 
    {
        int iT = omp_get_thread_num();
        bzero(m_aiTempAgentsPerCell[iT], m_iNumCells * sizeof(ulong));
    }

    int iFirstAgent = getFirstAgentIndex();
    if (iFirstAgent != LBController::NIL) {
        int iLastAgent = getLastAgentIndex();

#pragma omp parallel for
        for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
            int iT = omp_get_thread_num();
            if (m_aAgents[iAgent].m_iLifeState > 0) {
                m_aiTempAgentsPerCell[iT][m_aAgents[iAgent].m_iCellIndex]+=1;
            }
        }

        // cumulate
#pragma omp parallel for
        for (int iCell = 0; iCell < m_iNumCells; iCell++) {
            for (int iT = 0; iT < m_iNumThreads; iT++) {
                m_aiNumAgentsPerCell[iCell] +=  m_aiTempAgentsPerCell[iT][iCell];
            }
        }
    }
}


//----------------------------------------------------------------------------
// updateTotal
//  recalculate number of agents in each cell
//
template <typename T>
void SPopulation<T>::updateTotal() {
    m_iTotal = m_pAgentController->getNumUsed();
}


//----------------------------------------------------------------------------
// doActions 
// VERY IMPORTANT METHOD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
template<typename T>
int SPopulation<T>::doActions(uint iPrio, float fTime) {

    int iResult = 0;

    int iFirstAgent = getFirstAgentIndex();
    if (iFirstAgent !=  LBController::NIL) {
        int iLastAgent = getLastAgentIndex();
        for (uint iMethod = 0; iMethod < m_prio.getNumActionsForPrio(iPrio); iMethod++) {
            Action<T> *pA = m_prio.getAction(iPrio, iMethod);

            //            int iChunk = (uint)ceil((iLastAgent-iFirstAgent+1)/(double)m_iNumThreads);
            //#pragma omp parallel for schedule(static, iChunk) 
#pragma omp parallel for
            for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
                if (m_aAgents[iAgent].m_iLifeState > LIFE_STATE_DEAD) {
                    pA->execute(iAgent,fTime);
                }
            }
        }

    } 
    
    return iResult;
}


//----------------------------------------------------------------------------
// recycleDeadSpaceNew
//     use the space of dying agents to place a babies (no need for
//     unlinking and relinking; parallelizable)
//
//  - find out how many spaces can be reused (min(NumBirths,NumPrevDeaths))
//  - allocate arrays for holding birth data (m_pReuseB) for reused spaces
//  - allocate arrays for holding birth data (m_pNormalB) for normal birth
//  - collect iNumReuse birth infos from the m_vBirthList into m_pReuseB
//  - move the rest to m_pNormalB 
//  - create iNumReuse babies at indexes taken from m_pPrevD
//  - perform normal births for all in pNormalB
//  - perform normal death for all left overs in m_pPrevD
//  - collect all deaths for next step in m_pPrevD
//
template<class A>
int  SPopulation<A>::recycleDeadSpaceNew() {
    ulong iNumDeaths = 0;
    ulong iNumBirths = 0;
    // first count number of births and deaths
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        iNumDeaths += m_vDeathList[iThread]->size();
        iNumBirths += m_vBirthList[iThread]->size() / BQD_SIZE;
    }

    stdprintf("  %s  % 6zd births r\n", m_sSpeciesName, iNumBirths);
    m_iNumBirths += iNumBirths;
    stdprintf("  %s  % 6zd deaths r\n", m_sSpeciesName, iNumDeaths);
    m_iNumDeaths += iNumDeaths;

    // number of reusable dead spaces: min(iNumDeaths, iNumBirths)
    ulong iNumReuse = (m_iNumPrevDeaths<iNumBirths)?m_iNumPrevDeaths:iNumBirths;

    if (iNumReuse > 0) {
        // resize data vectors for dead space recycling if necessary
        if (iNumReuse > m_iMaxReuse) {
            m_iMaxReuse = 2 * iNumReuse;
            delete[] m_pReuseB;
            // data vectors for dead space recycling
            m_pReuseB = new int[m_iMaxReuse*BQD_SIZE];
        }
    }
    
    // we know: iNumBirths >= iNumReuse
    ulong iNumBNormal = iNumBirths - iNumReuse;
    // allocate space for indexes of "normally" born agents
    if (iNumBNormal > 0) {
        if (iNumBNormal > m_iMaxNormalB) {
            m_iMaxNormalB = 2 * iNumBNormal;
            delete[] m_pNormalB;
            m_pNormalB = new int[m_iMaxNormalB*BQD_SIZE];
        }
    }

    // calculate number of agents who have to die normally (unlinking etc)
    // we know: iNumPrevDeaths >= iNumReuse
    ulong iNumDNormal = m_iNumPrevDeaths - iNumReuse;

    
    // copy from birthlist to pReuseB until we have iNumReuse items;
    // copy rest to pNormalB
    ulong iDoneB = 0;
    ulong iDoneBN = 0;
    ulong iToDoB = iNumReuse*BQD_SIZE;
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        ulong iMax = m_vBirthList[iThread]->size();
        if (iMax > iToDoB) {
            iMax = iToDoB;
        }
        if (iMax > 0) {
            // we still need more data for the recycling
            std::copy(m_vBirthList[iThread]->begin(), 
                      m_vBirthList[iThread]->begin()+iMax, 
                      m_pReuseB+iDoneB);
            iToDoB -= iMax;
            iDoneB += iMax;
        } else {
            iMax = 0;
        } 

        if (iMax < m_vBirthList[iThread]->size()) {
            // rest of data is for normal births
            std::copy(m_vBirthList[iThread]->begin()+iMax,  
                      m_vBirthList[iThread]->end(), 
                      m_pNormalB+iDoneBN);
            iDoneBN +=  m_vBirthList[iThread]->size()-iMax;
        }
        
    }
    
    //    int iChunk = (int)ceil((iNumReuse+1)/(double)m_iNumThreads);
    //#pragma omp parallel for schedule(static, iChunk)
#pragma omp parallel for
    // recycling: create new baby at index of killed agent
    // only use iNumReuse of previous 
    for (uint i = 0; i < iNumReuse; i++) {
        if (m_pReuseB[BQD_SIZE*i] < 0) {
            stdprintf("[recycleDeadSpaceNew]Attention: agent %d has cellindex %d\n", m_pPrevD[i], m_pReuseB[BQD_SIZE*i]);
        }
        makeOffspringAtIndex(m_pPrevD[i],
                             m_pReuseB[BQD_SIZE*i],
                             m_pReuseB[BQD_SIZE*i+1],
                             m_pReuseB[BQD_SIZE*i+2]);
    }


    // now do normal births
    if (iNumBNormal > 0) {
        performBirths(iNumBNormal, m_pNormalB);
    }
    
    // now do normal deaths
    // the first iNumReuse places need not be unlinked,
    // they were overwritten with new agents
    if (iNumDNormal > 0) {
        performDeaths(iNumDNormal, m_pPrevD+iNumReuse);
    }
   
    // now prepare new deaths for reuse in next step
    // reallocate pPrevD if necessary
    if (iNumDeaths > m_iNumPrevDeaths) {
        delete[] m_pPrevD;
        m_pPrevD = new int[iNumDeaths];
    }
    // copy all new death info tom m_pPrevD
    ulong iCurOffs = 0;
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        std::copy(m_vDeathList[iThread]->begin(), 
                  m_vDeathList[iThread]->end(), 
                  m_pPrevD+iCurOffs);
        iCurOffs += m_vDeathList[iThread]->size();
    }
    // iCurOffs should be the same as iNumDeaths
    m_iNumPrevDeaths = iCurOffs;

    // loop through list and mark them as dead
    // this should not be needed since registerDeath already sets LIFE_STATE_DEAD

#pragma omp parallel for 
    for (uint i = 0; i < m_iNumPrevDeaths; i++) {
        m_aAgents[m_pPrevD[i]].m_iLifeState = LIFE_STATE_DEAD;
    }
    
    return 0;   
}


//----------------------------------------------------------------------------
// registerBirth
//  register mother, father (-1 if not needed) and cell index
//
template<typename T>
void SPopulation<T>::registerBirth(int iCellIndex, int iMotherIndex, int iFatherIndex) {

    int iThreadNum = omp_get_thread_num();

    m_vBirthList[iThreadNum]->push_back(iCellIndex);
    m_vBirthList[iThreadNum]->push_back(iMotherIndex);
    m_vBirthList[iThreadNum]->push_back(iFatherIndex);

}


//----------------------------------------------------------------------------
// performBirths
//
template<typename T>
int SPopulation<T>::performBirths() {
    int   iResult = 0;
    ulong iNumBirths = 0;
    int   iBirthQueueDataSize = 3;
    
    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        if (m_vBirthList[iThread]->size() > 0) {

            iNumBirths += m_vBirthList[iThread]->size() / iBirthQueueDataSize;

            for (unsigned int iIndex = 0; iIndex < m_vBirthList[iThread]->size() && iResult == 0; iIndex += iBirthQueueDataSize) {
                
                makeOffspring((*m_vBirthList[iThread])[iIndex], 
                              (*m_vBirthList[iThread])[iIndex+1], 
                              (*m_vBirthList[iThread])[iIndex+2]);

            }
        }
    }
    
    stdprintf("  %s  %6lu births p\n", m_sSpeciesName, iNumBirths);
    m_iNumBirths += iNumBirths;

    return iResult;
}


//----------------------------------------------------------------------------
// performBirths
//
template<typename T>
int SPopulation<T>::performBirths(ulong iNumBirths, int *piBirthData) {
    int iResult = 0;

    double fT0 = omp_get_wtime();
    bool bOldBirths = true;
    if ((bOldBirths) || ((int)iNumBirths < 10*m_iNumThreads)) {
        //#ifdef OLDBIRTHS
        // this loop must not be parallelized, because the linked list is manipulated    
        for (ulong iIndex = 0; iIndex < iNumBirths*BQD_SIZE; iIndex += BQD_SIZE) {
            //@@        stdprintf("makeOffSpring %d: %d + %d\n",  piBirthData[iIndex], piBirthData[iIndex+1], piBirthData[iIndex+2]);
            makeOffspring(piBirthData[iIndex], 
                          piBirthData[iIndex+1], 
                          piBirthData[iIndex+2]);
        }
        //stdprintf("normal births");fflush(stdout);
 
        //#else
    } else {
        int iStart = m_pAgentController->reserveSpace2(iNumBirths);
        //        checkLists();
        stdprintf("parallel births\n");fflush(stdout);

#pragma omp parallel for
        for (ulong iIndex0 = 0; iIndex0 < iNumBirths; iIndex0++) {
            ulong iIndex = iIndex0 * BQD_SIZE;
            makeOffspringAtIndex(iStart + iIndex0,
                                 piBirthData[iIndex], 
                                 piBirthData[iIndex+1], 
                                 piBirthData[iIndex+2]);
        }

    }
    fPerfBirthTime += (omp_get_wtime() - fT0);

    stdprintf("  %s  % 6zd births p\n", m_sSpeciesName, iNumBirths);

    return iResult;
}


///----------------------------------------------------------------------------
// makeOffspring
// crate a new agent wherever possible
//
template<typename T>
void SPopulation<T>::makeOffspring(int iCellIndex, int iMotherIndex, int iFatherIndex) {
    
    int iAgentIndex = createNullAgent(iCellIndex);

    makePopSpecificOffspring(iAgentIndex, iMotherIndex, iFatherIndex);
}


///----------------------------------------------------------------------------
// makeOffspringAtIndex
// crate a new agent at specific position
//
// ATTENTION: makeOffspringAtIndex is usually called inside a parallel for
//
template<typename T>
void SPopulation<T>::makeOffspringAtIndex(int iAgentIndex, int iCellIndex, int iMotherIndex, int iFatherIndex) {
    
    if (iCellIndex < 0) {
        stdprintf("[makeOffspringAtIndex] Attention: agent %d has cellindex %d\n", iAgentIndex, iCellIndex);
    }

    createAgentAtIndex(iAgentIndex, iCellIndex);
    
    makePopSpecificOffspring(iAgentIndex, iMotherIndex, iFatherIndex);
}


//----------------------------------------------------------------------------
// reserveAgentSpace
//
template<typename T>
int SPopulation<T>::reserveAgentSpace(int iNumAgents) {
    return m_pAgentController->reserveSpace2(iNumAgents);
}


//----------------------------------------------------------------------------
// creatNullAgent
//  find free index and reset data at that location
//
template<typename T>
int  SPopulation<T>::createNullAgent(int iCellIndex) {
    
    int iFreeIndex = m_pAgentController->getFreeIndex();
    
    createAgentAtIndex(iFreeIndex, iCellIndex);
    
    return iFreeIndex;
    
}


//----------------------------------------------------------------------------
// creatAgentAtIndex
//     reset data at that location
//
template<typename T>
int  SPopulation<T>::createAgentAtIndex(int iAgentIndex, int iCellIndex) {

    if (iCellIndex < 0) {
        stdprintf("[createAgentAtIndex] Attention: agent %d has cellindex %d\n", iAgentIndex, iCellIndex);
    }

    resetAgent(iAgentIndex);

    // set location
    m_aAgents[iAgentIndex].m_ulCellID = this->m_pCG->m_aCells[iCellIndex].m_iGlobalID;
    m_aAgents[iAgentIndex].m_iCellIndex = iCellIndex;
    m_aAgents[iAgentIndex].m_fBirthTime = m_fCurTime;  
    int iT = omp_get_thread_num();

    m_aAgents[iAgentIndex].m_iGender = (uchar)(2*m_apWELL[iT]->wrandd());
    if (m_aAgents[iAgentIndex].m_iGender == 0) {
        // by default females are fertile so they can be mated (RandomPair)
        m_aAgents[iAgentIndex].m_iLifeState = LIFE_STATE_FERTILE;
    }
    return iAgentIndex;
}


//----------------------------------------------------------------------------
// resetAgent
//  reset the agent data at specified index 
//  (called by createNullAgent)
//
template<typename T>
Agent *SPopulation<T>::resetAgent(int iAgentIndex) {

    // set ID
    m_aAgents[iAgentIndex].m_ulID = getUID(); 
    
    // set life state
    m_aAgents[iAgentIndex].m_iLifeState = LIFE_STATE_ALIVE;

    return &m_aAgents[iAgentIndex];
}


//----------------------------------------------------------------------------
// registerDeath
//  register agent
//
template<typename T>
void SPopulation<T>::registerDeath(int iCellIndex, int iAgentIndex) {

    if (m_aAgents[iAgentIndex].m_iLifeState == LIFE_STATE_DEAD) {
        stdprintf("\e[0;31mWARNING - trying to kill an already dead agent!\nCheck action priorities: all pairing actions should occur before killing actions\e[0m\n");
    }
    int iThreadNum = omp_get_thread_num();
    
    // mark agent as killed: not to be moved or otherwise used
    m_aAgents[iAgentIndex].m_iLifeState = LIFE_STATE_DEAD;
    
    m_vDeathList[iThreadNum]->push_back(iAgentIndex);

}


//----------------------------------------------------------------------------
// performDeaths
//  kill all the agents on the listz
//
template<typename T>
int SPopulation<T>::performDeaths() {
    int iResult = 0;
    ulong iNumDeaths = 0;

    for (int iThread = 0; iThread < m_iNumThreads; iThread++) {
        if (m_vDeathList[iThread]->size() > 0) {
            iNumDeaths += m_vDeathList[iThread]->size();

            for (unsigned int iIndex = 0; iIndex < m_vDeathList[iThread]->size() && iResult == 0; iIndex++) {
                makePopSpecificDeath((*m_vDeathList[iThread])[iIndex]);
                
                iResult = m_pAgentController->deleteElement((*m_vDeathList[iThread])[iIndex]);
            }
        }
    }

    stdprintf("  %s  %6lu deaths\n", m_sSpeciesName, iNumDeaths);
    m_iNumDeaths += iNumDeaths;

    return iResult;
}


//----------------------------------------------------------------------------
// performDeaths
//  kill all the agents on the listz
//
template<typename T>
int SPopulation<T>::performDeaths(ulong iNumDeaths, int *piDeathData) {
    int iResult = 0;

    // this loop must not be parallelized, because the linked list is manipulated    
    for (ulong iIndex = 0; (iIndex < iNumDeaths); iIndex++) {
        if (m_aAgents[piDeathData[iIndex]].m_iLifeState > 0) {
            fprintf(stderr,"deleting live agent %d!!!\n",piDeathData[iIndex]);
        }

        iResult = m_pAgentController->deleteElement(piDeathData[iIndex]);

    }

    stdprintf("  %s  % 6zd deaths p\n", m_sSpeciesName, iNumDeaths);
    return iResult;  
}


//----------------------------------------------------------------------------
// flushDeadSpace
//  kill all the agents on the recycling list 
//
template<typename T>
int SPopulation<T>::flushDeadSpace() {
    int iResult = performDeaths(m_iNumPrevDeaths, m_pPrevD);
    m_iNumPrevDeaths = 0;
    updateTotal();

    return iResult;
}


//----------------------------------------------------------------------------
// moveAgent
//  move agent to different cell
//  and
//  update the move statistics
//
//
template<typename T>
int SPopulation<T>::moveAgent(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo) {
    int iResult = 0;
    if (iCellIndexTo < 0) {
        stdprintf("[moveAgent] Attention: agent %d has cellindex %d\n", iAgentIndex, iCellIndexTo);
    }
    m_aAgents[iAgentIndex].m_iCellIndex = iCellIndexTo;
    m_aAgents[iAgentIndex].m_ulCellID = m_pCG->m_aCells[iCellIndexTo].m_iGlobalID;
    m_aAgents[iAgentIndex].m_iLifeState &= ~LIFE_STATE_MOVING;

    makePopSpecificMove(iCellIndexFrom, iAgentIndex, iCellIndexTo);
    
 
    return iResult;
}


//----------------------------------------------------------------------------
// registerMove
//  register move parameters
//
template<typename T>
void SPopulation<T>::registerMove(int iCellIndexFrom, int iAgentIndex, int iCellIndexTo) {
    
    //    stdprintf("[SPopulation<T>::registerMove] registering move: %i %i %i\n", iCellIndexFrom, iAgentIndex, iCellIndexTo);

    int iThreadNum = omp_get_thread_num();
    
    // LifeState should be reset after move is completed (moveAgent())
    m_aAgents[iAgentIndex].m_iLifeState |= LIFE_STATE_MOVING;

    if (iCellIndexTo < 0) {
        stdprintf("[registerMove] Attention: agent %d has cellindex %d\n", iAgentIndex, iCellIndexTo);
    }
    m_vMoveList[iThreadNum]->push_back(iCellIndexFrom);
    m_vMoveList[iThreadNum]->push_back(iAgentIndex);
    m_vMoveList[iThreadNum]->push_back(iCellIndexTo);
}


//----------------------------------------------------------------------------
// performMoves
//  do all the moves registered by registerMove()
//
template<typename T>
int SPopulation<T>::performMoves() {
    int iResult = 0;
    ulong iNumMoves = 0;  
    int iMoveQueueDataSize = 3;

#pragma omp parallel reduction(+:iNumMoves,iResult)
    {
        int iThreadNum = omp_get_thread_num();

        if (m_vMoveList[iThreadNum]->size() > 0) {
            
            //            iNumMoves += m_auMoveListIndex[iThreadNum]/3;
            iNumMoves += m_vMoveList[iThreadNum]->size() / iMoveQueueDataSize;

            for (unsigned int iIndex = 0; iIndex < m_vMoveList[iThreadNum]->size() && iResult == 0; iIndex += iMoveQueueDataSize) {
                
                //                if (m_aAgents[ (*m_vMoveList[iThreadNum])[iIndex+1] ].m_iLifeState > 0) {
                if ((*m_vMoveList[iThreadNum])[iIndex+2] < 0) {
                    stdprintf("[performMoves] Attention agent %d has cellindex %d\n", (*m_vMoveList[iThreadNum])[iIndex+1], (*m_vMoveList[iThreadNum])[iIndex+2]);
                }
                iResult = moveAgent((*m_vMoveList[iThreadNum])[iIndex], 
                                    (*m_vMoveList[iThreadNum])[iIndex+1], 
                                    (*m_vMoveList[iThreadNum])[iIndex+2]);
                //                }
            }
        }
        //        m_pMoveListController[iThreadNum]->clear();
    }

    stdprintf("  %s  %6lu moves\n", m_sSpeciesName, iNumMoves);
    m_iNumMoves += iNumMoves;

    
    return iResult;
}

//----------------------------------------------------------------------------
//   getNumAgentsMax
//
template<typename T>
ulong SPopulation<T>::getNumAgentsMax() { 
    return m_pAgentController->getNumLayers()*m_pAgentController->getLayerSize(); 
};


//----------------------------------------------------------------------------
// readSpeciesData
//   read data coming from xml file.
//
template<typename T>
int SPopulation<T>::readSpeciesData(ParamProvider2 *pPP) {
    int iResult = 0;
 
    // set class Attriutes
    m_sClassName   = pPP->getSelected();
    m_sSpeciesName = pPP->getSpeciesName();
                
    // read prios from XML (***new***)
    iResult = this->getPrioInfos(pPP->getClassInfo()->prios);
    if (iResult == 0) {
        // read action params
        iResult = m_prio.getActionAttributes(pPP->getClassInfo()->mods);

        if (iResult == 0) {
           // success
        } else {
           stdprintf("Couldn't extract action params\n");
        }
    } else {
        stdprintf("Couldn0t extract prios\n");
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// addAgent
//
template<typename T>
int  SPopulation<T>::addAgent(int iCellIndex, char *pData, bool bUpdateCounts) {
    int iResult = 0;
    
    uint iFreeIndex = m_pAgentController->getFreeIndex();
    m_aAgents[iFreeIndex].m_ulCellID = m_pCG->m_aCells[iCellIndex].m_iGlobalID;
    m_aAgents[iFreeIndex].m_iCellIndex = iCellIndex;

    iResult = addAgentData(iCellIndex, iFreeIndex, &pData);
    // remember highest ID
    if (m_aAgents[iFreeIndex].m_ulID > m_iMaxID) {
        m_iMaxID = m_aAgents[iFreeIndex].m_ulID;
    }
    if (bUpdateCounts) {
        updateTotal();
        updateNumAgentsPerCell();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// addAgentData
//
template<typename T>
int  SPopulation<T>::addAgentData(int iCellIndex, int iAgentIndex, char **ppData) {
    int iResult = 0;

    //    stdprintf("[SPopulation::addAgentData] got [%s]\n", *ppData);

    // must read
    //  uint   m_iLifeState;
    //  gridtype m_ulID;

    //  alerady set by in addAgent()
    //  int    m_iCellIndex;
    //  gridtype m_ulCellID;

    iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_iLifeState);
    if (iResult == 0) {
        iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_ulID);
    } else {
        stdprintf("[addAgentData] Couldn't read m_iLifeState from [%s]\n", *ppData);
    }
    if (iResult == 0) {
        iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fBirthTime);
    } else {
        stdprintf("[addAgentData] Couldn't read m_ulID from [%s]\n", *ppData);
    }
    if (iResult == 0) {
        iResult = this->addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_iGender);
    } else {
        stdprintf("[addAgentData] Couldn't read m_fBirthTime from [%s]\n", *ppData);
    }

    if (iResult == 0) {
        iResult = addPopSpecificAgentData(iAgentIndex,ppData);
    } else {
        stdprintf("[addAgentData] Couldn't read m_iGender from [%s]\n", *ppData);
    }

    if (iResult != 0) {
        stdprintf("[addAgentData] Couldn't read pop specific agent data from [%s]\n", *ppData);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// addAgentDataSingle 
// multi-use template of addAgentData basic functionality
//
template<typename T>
template<typename T1>
int  SPopulation<T>::addAgentDataSingle(char **ppData, T1 *pAgentDataMember) {
    int iResult = 0;

    char *pEnd;
    
    // LifeState
    if (**ppData != '\0') {
        //        char *p = nextWord(ppData, " ,;:");
        char *p = nextWord(ppData, " ,;:");
        T1 x = (T1) strtod(p, &pEnd);
        if (*pEnd == '\0') {
            *pAgentDataMember = x;
        } else {
            iResult = -1;
        }
    } else {
        iResult = -1;
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// setAgentBasic
//  set basic agent data (pBasic points at an agent structure)
//
template<typename T>
void  SPopulation<T>::setAgentBasic(int iAgentIndex, void *pBasic) {
    Agent *pa = static_cast<Agent*>(pBasic);
    Agent &newAgent = m_aAgents[iAgentIndex];
    newAgent.m_iLifeState  = pa->m_iLifeState;     
    newAgent.m_iCellIndex  = pa->m_iCellIndex;
    newAgent.m_ulID        = pa->m_ulID;
    newAgent.m_ulCellID    = pa->m_ulCellID;
    newAgent.m_fBirthTime  = pa->m_fBirthTime;
    newAgent.m_iGender     = pa->m_iGender;
}


//----------------------------------------------------------------------------
// shiftIDs
//  shift the agents' IDs by geven offset
//
template<typename T>
idtype  SPopulation<T>::shiftIDs(idtype iOffset) {
    idtype ulMax = 0;
    for (int iA = getFirstAgentIndex(); iA <= getLastAgentIndex(); iA++) {
        if (m_aAgents[iA].m_iLifeState != LIFE_STATE_DEAD) {
            m_aAgents[iA].m_ulID += iOffset;
            if (m_aAgents[iA].m_ulID > ulMax) {
                ulMax = m_aAgents[iA].m_ulID;
            }
        }
    }
    return ulMax;
}


//----------------------------------------------------------------------------
// mergePop
//  merge the specified pooulation with this, if
//    - ClassNames match
//    - SpeciesNames match
//    - relevant Actions match (isEqual with bStrict=false: only check relevant)
//    - agent data types match
//
template<typename T>
int  SPopulation<T>::mergePop(PopBase *pBPop) {
    int iResult = -1;
    SPopulation *pPop = static_cast<SPopulation *>(pBPop);
    stdprintf("[SPopulation<T>::mergePop]Trying to merge this (%s:%s) to self (%s:%s)\n",  pPop->getClassName(),  pPop->getSpeciesName(), m_sClassName, m_sSpeciesName);
    if ((m_sClassName   == pPop->getClassName()) && 
        (m_sSpeciesName == pPop->getSpeciesName()))  {
        if (m_prio.isEqual(&(pPop->m_prio), false)) {
            hid_t t1 = m_hAgentDataType;
            hid_t t2 = pPop->getAgentQDFDataType();
            iResult = qdf_compareDataTypes(t1, t2);
            if (iResult == 0) {
                // types match, we can copy the agents
                // pack agents of pPop
                pPop->compactData();
                pPop->updateTotal();
                idtype ulOtherMaxID = pPop->shiftIDs(getMaxLoadedID());
                //  get numer of agents of pPop
                int iCount = pPop->getNumAgentsTotal();
                // reserve space
                int iStart = m_pAgentController->reserveSpace2(iCount);
                stdprintf("[SPopulation<T>::mergePop] reserved %d spaces at pos %d\n", iCount, iStart);
                
                // copy block
                m_aAgents.copyBlock(iStart, &pPop->m_aAgents, 0, (uint)iCount);
                
                // now make sure popspecific data is copied as well
                copyAdditionalDataQDF(iStart, iCount, pPop);

                updateTotal();
                updateNumAgentsPerCell();
                stdprintf("[SPopulation<T>::mergePop] population now has %ld agents\n", getNumAgentsTotal());

                if (ulOtherMaxID > m_iMaxID) {
                    m_iMaxID = ulOtherMaxID;
                }
                iResult = 0;
            } else {
                stdprintf("[SPopulation<T>::mergePop] agent data types differ (res %d)\n", iResult);
                // same class and species but different rest is fatal
                iResult = -2;
            }            
        } else {
            stdprintf("[SPopulation<T>::mergePop] prios differ (res %d)\n", iResult);
            // same class and species but different rest is fatal
            iResult = -2;
        }
    } else {
        stdprintf("[SPopulation<T>::mergePop] class or species differ: not compatible\n");
        iResult = -1;
    }
    return iResult;
}



//----------------------------------------------------------------------------
// createAgentDataTypeQDF
//  Create the HDF5 datatype for Agent data
//
template<typename T>
hid_t  SPopulation<T>::createAgentDataTypeQDF() {

    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, agentRealSizeQDF());

    T ta;

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE.c_str(),  qoffsetof(ta, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX.c_str(),  qoffsetof(ta, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID.c_str(),     qoffsetof(ta, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID.c_str(),    qoffsetof(ta, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME.c_str(),  qoffsetof(ta, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER.c_str(),      qoffsetof(ta, m_iGender),    H5T_NATIVE_UCHAR);

    addPopSpecificAgentDataTypeQDF(&hAgentDataType);

    return hAgentDataType;
}


//----------------------------------------------------------------------------
// checkLists
//
template<typename T>
int SPopulation<T>::checkLists() { 
    int iResult = 0;
    stdprintf("m_pAgentController check:\n");
    int i1 = m_pAgentController->checkLists();
    if (i1 == 0) {
        stdprintf("ok\n");
    }

    iResult += i1;
    stdprintf("m_pWriteCopyController check:\n");
    int i2 = m_pWriteCopyController->checkLists();
    if (i2 == 0) {
        stdprintf("ok\n");
    }
    iResult += i2;
    
    fflush(stdout);

    return iResult;
}


//----------------------------------------------------------------------------
// writeAgentDataQDF
//
template<typename T>
int  SPopulation<T>::writeAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {
    int iResult = 0;

    stdprintf("[SPopulation<T>::writeAgentDataQD][%s] at step %d:\n", m_sSpeciesName, (int) m_fCurTime);
    stdprintf("  total number of births  %lu\n", m_iNumBirths);
    stdprintf("  total number of deaths  %lu\n", m_iNumDeaths);
    stdprintf("  total number of moves   %lu\n", m_iNumMoves);

    iResult = writeAgentDataQDFSafe(hDataSpace, hDataSet, hAgentType);

    return iResult;
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int SPopulation<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = m_prio.modifyAttributes(sAttrName, dValue);
    return iResult; 
}	


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------- I/O-------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//-- writeAgentDataQDFSafe
//-- readAgentDataQDF
//-- writeSpeciesDataQDF
//-- readSpeciesDataQDF
//-- writeStatArrsQDF
//-- readStatArrsQDF
//-- writeAdditionalDataQDF
//-- readAdditionalDataQDF
//-- readPrioInfo
//-- insertPrioDataAttribute
//-- extractPrioDataAttribute
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// writeAgentDataQDF
//  write data using hyperslabs
//  - create a file handle
//  - create a dataspace for the file (sized to hold entire array aData)
//  - create a dataspace for the slab (sized to hold one slab)
//  - create the data type for the structure
//  - create the data set
//  - write the data
//  This method copies each layer of m_aAgents to the first layer of m_aWriteCopy
//  The copied layer is then compacted and passed to  H5Dwrite
//  
template<typename T>
int  SPopulation<T>::writeAgentDataQDFSafe(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {
    int iResult = 0;
    stdprintf("[SPopulation::writeAgentDataQDFSafe]\n");
    fflush(stdout);
    m_pAgentController->calcHolyness();
    // make sure there is a  layer in WriteCopyController 
    //(it may have been freed in the previous call to writeAgentDataQDFSafe, if the layer was empty after killing the dead)
    if (m_pWriteCopyController->getNumLayers() == 0) {
    	m_pWriteCopyController->addLayer();
    }

    hsize_t dimsm = m_pAgentController->getLayerSize();
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

    hsize_t offset = 0;
    hsize_t count  = 0;  
    
    // step size when going through data (e.g. stride = 2: use every second element)
    hsize_t stride = 1;
    hsize_t block  = 1;
    
    // make sure there are no holes and no forgotten dead
    // m_vMergedDeadList is used by other objects (e.g. Genetics)
    m_vMergedDeadList.clear();
    m_vMergedDeadList.insert(m_vMergedDeadList.end(), m_pPrevD, m_pPrevD+m_iNumPrevDeaths);
    std::sort(m_vMergedDeadList.begin(), m_vMergedDeadList.end());

    int iTotalKill = 0;
    uint iNumWritten = 0;
    herr_t status = H5P_DEFAULT;
    uint iD = 0;
    int iLayerSize = m_pWriteCopyController->getLayerSize();
    
    if (m_pAgentController->getNumUsed() > 0) {
        
        for (uint j = 0; (iResult == 0) && (j < m_aAgents.getNumUsedLayers()); j++) {
            // empty layers will be removed in compactData, which leads to an undefinde state when used
            if (m_pAgentController->getNumUsed(j) > 0) {

                // write agents of layer j as hyperslab
                const T* pSlab0 = m_aAgents.getLayer(j);
            
                m_aWriteCopy.copyLayer(0, pSlab0);
                m_pWriteCopyController->setL2List(m_pAgentController->getL2List(j), 0);

                // remove dead in slab
                uint iNumKilled  = 0;
                while ((iD < m_vMergedDeadList.size()) && (m_vMergedDeadList[iD] < (int)(j+1)*iLayerSize)) {
                    
                    iResult = m_pWriteCopyController->deleteElement(m_vMergedDeadList[iD] - j*iLayerSize);

                    iD++;
                    iNumKilled++;
                }
                iTotalKill += iNumKilled;

                count =  m_pWriteCopyController->getNumUsed(0);
                if (count > 0) {
                    // here it is important not to have an empty slab
                    m_pWriteCopyController->compactData();
            
                    const T* pSlab = m_aWriteCopy.getLayer(0);
                    iNumWritten +=  count;

                    // adapt memspace if size of slab is different
                    if (count != dimsm) {
                        qdf_closeDataSpace(hMemSpace); 
                        dimsm = count;
                        hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
                    }
            
                    //			printf("selecting hyperslab:\n offset %d\n count %d\n", offset, count);
                
                    status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                             &offset, &stride, &count, &block);
            
                    status = H5Dwrite(hDataSet, hAgentType, hMemSpace,
                                  hDataSpace, H5P_DEFAULT, pSlab);
                    offset += count;
                } else {
                    stdprintf("[SPopulation<T>::writeAgentDataQDFSafe] ignored layer %d because it only contains dead\n", j);
                }
            } else {
                stdprintf("[SPopulation<T>::writeAgentDataQDFSafe] ignored layer %d because it's empty\n", j);
            }
        }
        
    } else {
        // no agents at all
    }
    
    qdf_closeDataSpace(hMemSpace); 

    stdprintf("[SPopulation<T>::writeAgentDataQDFSafe] written %u agents, killed %d\n", iNumWritten, iTotalKill); fflush(stdout);
    stdprintf("[SPopulation<T>::writeAgentDataQDFSafe] end with status %d\n", status); fflush(stdout);

    return (status >= 0)?iResult:-1;
}



//----------------------------------------------------------------------------
// readAgentDataQDF
//  read attributes to species data
//
template<typename T>
int  SPopulation<T>::readAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {

    int iResult = 0;
    hsize_t dims = 0;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    hsize_t iOffset = 0;
    hsize_t iCount  = 0;
    hsize_t iStride = 1;
    hsize_t iBlock  = 1;

    memset(m_pReadBuf, 0, ABUFSIZE*sizeof(T));
    // compacting seems ok, since we're changing the occupancy anyway
    compactData();

    updateTotal();

    while ((iResult == 0) && (dims > 0)) {
        if (dims > ABUFSIZE) {
            iCount = ABUFSIZE;
        } else {
            iCount = dims;
        }

        // read a buffer full
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, &iStride, &iCount, &iBlock);
        status = H5Dread(hDataSet, hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, m_pReadBuf);
        if (status >= 0) {

            uint iFirstIndex = m_pAgentController->reserveSpace2((uint)iCount);
            m_aAgents.copyBlock(iFirstIndex, m_pReadBuf, (uint)iCount);
            for (uint j =0; j < iCount; j++) {
                if (m_pReadBuf[j].m_ulID > m_iMaxID) {
                    m_iMaxID = m_pReadBuf[j].m_ulID;
                }
            }
 
            dims -= iCount;
            iOffset += iCount;

        } else {
            iResult = -1;
        }
    }
 
    updateTotal();

    updateNumAgentsPerCell();

    return iResult;
}

//----------------------------------------------------------------------------
// writeSpeciesDataQDF
//  write species data as attributes
//
template<typename T>
int  SPopulation<T>::writeSpeciesDataQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    if (iResult == 0) {
        iResult =  qdf_insertSAttribute(hSpeciesGroup, SPOP_ATTR_CLASS_NAME, m_sClassName);
    }
    if (iResult == 0) {
        iResult =  qdf_insertSAttribute(hSpeciesGroup, SPOP_ATTR_SPECIES_NAME, m_sSpeciesName);
    }
    if (iResult == 0) {
        iResult =  qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_NUM_CELL, 1, &m_iNumCells);
    }
    if (iResult == 0) {
        iResult =  qdf_insertAttribute(hSpeciesGroup, SPOP_QDF_VERSION, 1, &m_iQDFVersionOut);
    }

    if (iResult == 0) {
        iResult =  insertPrioDataAttribute(hSpeciesGroup);
    }

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_INIT_SEED, STATE_SIZE, m_aulInitialState); 
    }

    if (iResult == 0) {
        iResult = m_prio.writeActionParamsQDF(hSpeciesGroup);
    }

    return iResult;

}

//----------------------------------------------------------------------------
// readSpeciesDataQDF
//  read attributes to species data
//
template<typename T>
int  SPopulation<T>::readSpeciesDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, SPOP_ATTR_CLASS_NAME, m_sClassName);
    }

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, SPOP_ATTR_SPECIES_NAME, m_sSpeciesName);
    }        

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_NUM_CELL, 1, &m_iNumCells);
    }
 
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_QDF_VERSION, 1, &m_iQDFVersionIn);
        if (iResult != 0) {
            m_iQDFVersionIn = 3;
            iResult = 0;
        }
    }
    stdprintf("[SPopulation<T>::readSpeciesDataQDF] have QDF version %d\n", m_iQDFVersionIn);
    // now read action-specific parameters

    if (iResult == 0) {
        iResult = m_prio.extractActionParamsQDF(hSpeciesGroup);
    }

    // finally, read priority information

    if (iResult == 0) {
        iResult =  extractPrioDataAttribute(hSpeciesGroup);
    }
 
    return iResult;
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//  write additional data to the group 
//  (data to be stored separately from agent data, e.g. ancestors IDs)
//
template<typename T>
int  SPopulation<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    return 0;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//  read additional data from the group
//  (data stored separately from agent data)
//
template<typename T>
int  SPopulation<T>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    return 0;
}


//----------------------------------------------------------------------------
// getPrioInfos
//  Get priority info 
//  and fill m_mPrioInfo (map<string, uint>)
//
template<typename T>
int  SPopulation<T>::getPrioInfos(const stringmap &mPrios) {
   int iResult = 0;
    stringmap::const_iterator it;
    for (it = mPrios.begin(); (iResult == 0) && (it != mPrios.end()); ++it) {
        int iP = 1;
        if (strToNum(it->second, &iP)) {
            m_mPrioInfo[it->first] = iP;
        } else {
            stdprintf("invalid prio level [%s]\n",it->second);  
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// insertPrioDataAttribute
//   save the priority data as array of pairs (char[], int)
//
template<typename T>
int SPopulation<T>::insertPrioDataAttribute(hid_t hSpeciesGroup) {
    int iResult = -1;
 
  
    ulong iNum = m_mPrioInfo.size();
    // find length of longest string
    ulong iMaxLen = 0;
    std::map<std::string, int>::const_iterator it;
    for (it = m_mPrioInfo.begin(); it != m_mPrioInfo.end(); ++it) {
        ulong iL = it->first.length();
        if (iL > iMaxLen) {
            iMaxLen = iL;
        }
    }
    iMaxLen++; // account for terminating 0
    
    if (iMaxLen > MAX_FUNC_LEN) {
        stdprintf("Can't translate PrioInfo to array: function name too long (%lu > %u)\n", iMaxLen, MAX_FUNC_LEN);
    } else {
 
        // create an array to hold the map's data
        PrioData *paPD = new PrioData[iNum];
        PrioData *pCur = paPD;
        // bytes between string end and number are uninitialised
        // to prevent valgrind nagging: initialise all
        memset(paPD, 77, iNum*sizeof(PrioData));
        for (it = m_mPrioInfo.begin(); it != m_mPrioInfo.end(); ++it) {
            strcpy(pCur->m_sFunction, it->first.c_str());
            pCur->m_iPrioVal  = it->second;
            pCur++;
        }

        // string type used in PrioData
        hid_t strtype = H5Tcopy (H5T_C_S1);         /* Make a copy of H5T_C_S1 */
        hsize_t sizeMF = MAX_FUNC_LEN;
        herr_t status = H5Tset_size (strtype, sizeMF); 
        // define the data type for PrioData    
        hid_t hPrioDataType = H5Tcreate (H5T_COMPOUND, sizeof(PrioData));
        H5Tinsert(hPrioDataType, SPOP_AT_FUNCNAME.c_str(),  HOFFSET(PrioData, m_sFunction), strtype);
        H5Tinsert(hPrioDataType, SPOP_AT_FUNCPRIO.c_str(),  HOFFSET(PrioData, m_iPrioVal),  H5T_NATIVE_INT);

        // create and write the attribute
        hsize_t dims =  m_mPrioInfo.size();
        hid_t  hSpace = H5Screate_simple (1, &dims, NULL);
        hid_t  hAttr = H5Acreate (hSpeciesGroup, SPOP_ATTR_PRIO_INFO.c_str(), hPrioDataType, hSpace, 
                                  H5P_DEFAULT, H5P_DEFAULT);
        status = H5Awrite (hAttr, hPrioDataType, paPD);

        // close handles
        qdf_closeDataType(hPrioDataType);
        qdf_closeDataSpace(hSpace);
        qdf_closeAttribute(hAttr);
        iResult = (status >= 0)?0:-1;
        delete[] paPD;
    } 
    return iResult;
}


//----------------------------------------------------------------------------
// extractPrioDataAttribute
//  create an array suitable for HDF5 reading/writing
//
template<typename T>
int SPopulation<T>::extractPrioDataAttribute(hid_t hSpeciesGroup) {
    int iResult = -1;
    hsize_t dims[2];

    if (H5Aexists(hSpeciesGroup, SPOP_ATTR_PRIO_INFO.c_str())) {
        hid_t hAttribute = H5Aopen_name(hSpeciesGroup, SPOP_ATTR_PRIO_INFO.c_str());
        
        hid_t hAttrSpace = H5Aget_space(hAttribute);
        int rank = H5Sget_simple_extent_ndims(hAttrSpace);
        herr_t status = H5Sget_simple_extent_dims(hAttrSpace, dims, NULL);

       
        // define the data type for PrioData    
        hid_t hPrioDataType = H5Aget_type(hAttribute);

        if (rank == 1)  {
            PrioData *paPD = new PrioData[dims[0]];
            
            status = H5Aread(hAttribute, hPrioDataType, paPD);
            m_mPrioInfo.clear();
            if (status == 0) {
                for (uint i = 0; i < dims[0]; ++i) {
                    m_mPrioInfo[paPD[i].m_sFunction] = paPD[i].m_iPrioVal;
                }
                iResult = 0;

            } else {
                stdprintf("read priodata attribute err\n");
            } 


            delete[] paPD;
        } else {
            stdprintf("Bad Rank (%d)\n", rank);
        }
        qdf_closeDataSpace(hAttrSpace);
        qdf_closeAttribute(hAttribute);
        qdf_closeDataType(hPrioDataType);
    } else {
        stdprintf("Attribute [%s] does not exist\n", SPOP_ATTR_PRIO_INFO);
    }


    return iResult;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------- dump/restore ---------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//-- dumpAgentDataQDF
//-- restoreAgentDataQDF
//-- dumpSpeciesDataQDF
//-- restoreSpeciesDataQDF
//-- dumpStatArrsQDF
//-- restoreStatArrsQDF
//-- dumpAdditionalDataQDF
//-- restoreAdditionalDataQDF
//-- dumpController
//-- restoreController
//-- dumpDeadSpaces
//-- restoreDeadSpaces
//-- 
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// dumpAgentDataQDF
//   like writeAgentDataQDF, but writes *all* layers without compacting them
//   and without removing dead spaces
//   The normal readAgentDataQDF will restore the original layer layout when
//   reading this output.
//
template<typename T>
int  SPopulation<T>::dumpAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {
    int iResult = 0;
    stdprintf("[SPopulation::dumpAgentDataQDFSafe]\n");
    fflush(stdout);

    hsize_t dimsm = m_pAgentController->getLayerSize();
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

    int iFP = m_pAgentController->getFirstIndex( LBController::PASSIVE);
    stdprintf("[SPopulation<T>::dumpAgentDataQDF] first index passive=%d\n", iFP);


    hsize_t offset = 0;
    hsize_t count  = dimsm;  
    
    // step size when going through data (e.g. stride = 2: use every second element)
    hsize_t stride = 1;
    hsize_t block  = 1;

    // we don't remove dead spaces
  
    herr_t status = H5P_DEFAULT;
        
    for (uint j = 0; (iResult == 0) && (j < m_aAgents.getNumLayers()); j++) {
        // write agents of layer j as hyperslab
        const T* pSlab = m_aAgents.getLayer(j);
        
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &offset, &stride, &count, &block);
            
        status = H5Dwrite(hDataSet, hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, pSlab);
        offset += count;
    }
    
    qdf_closeDataSpace(hMemSpace); 

    stdprintf("[SPopulation<T>::dumpAgentDataQDFSafe written %d layers of size %d\n",  m_aAgents.getNumLayers(), m_pWriteCopyController->getLayerSize());

    return (status >= 0)?iResult:-1;
}


//----------------------------------------------------------------------------
// restoreAgentDataQDF
//  restoresead agent data from a fump
//
template<typename T>
int  SPopulation<T>::restoreAgentDataQDF(hid_t hDataSpace, hid_t hDataSet, hid_t hAgentType) {

    int iResult = 0;
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);


    T *pSlab = new T[dims];

    hsize_t dimsm = m_pAgentController->getLayerSize();
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

    hsize_t offset = 0;
    hsize_t count  = dimsm;  
    
    // step size when going through data (e.g. stride = 2: use every second element)
    hsize_t stride = 1;
    hsize_t block  = 1;

    // we don't remove dead spaces
  
    status = H5P_DEFAULT;
        
    for (uint j = 0; (iResult == 0) && (j < m_aAgents.getNumLayers()); j++) {
        // write agents of layer j as hyperslab
        
        
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &offset, &stride, &count, &block);
            
        status = H5Dread(hDataSet, hAgentType, hMemSpace,
                          hDataSpace, H5P_DEFAULT, pSlab);
        
        m_aAgents.copyLayer(j, pSlab);
        offset += count;
    }
    
    qdf_closeDataSpace(hMemSpace); 

 
    updateTotal();

    updateNumAgentsPerCell();

    // we must find highest ID
    idtype maxid = 0;

#pragma omp parallel for reduction(max:maxid)
    for (int iA = getFirstAgentIndex(); iA <=  getLastAgentIndex(); iA++) {
        if (m_aAgents[iA].m_ulID > maxid) {
            maxid = m_aAgents[iA].m_ulID;
        }
    }
    m_iMaxID = maxid;
    stdprintf("Found mac ID: %ld\n", m_iMaxID);
    delete[] pSlab;
    iResult = (status >= 0)?iResult:-1;
    return iResult;
}


//----------------------------------------------------------------------------
// dumpSpeciesDataQDF
//  dump species data as attributes plus full restore data
//
template<typename T>
int  SPopulation<T>::dumpSpeciesDataQDF(hid_t hSpeciesGroup, int iDumpMode) {
    stdprintf("dumping species data\n");

    int iResult = 0;

    // "normal" attributes output
    iResult = writeSpeciesDataQDF(hSpeciesGroup);

    if (iResult == 0) {
        iResult = m_prio.dumpActionStatesQDF(hSpeciesGroup);
    }
    // for full recovery
    
    if (iResult == 0) {
        iResult = dumpWELL(m_apWELL, m_iNumThreads, m_sSpeciesName, hSpeciesGroup);
    }
    /*
    // number of WELL states
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_NUM_STATES, 1, &m_iNumThreads); 
    }

    // current indexes of the states
    uint32_t *auiIndexes = new uint32_t[m_iNumThreads];
    if (iResult == 0) {
        for (int i = 0; i < m_iNumThreads; i++) {
            auiIndexes[i] = m_apWELL[i]->getIndex();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_CUR_INDEX, m_iNumThreads, auiIndexes); 
    }
    delete[] auiIndexes;

    // the WELL states
    if (iResult == 0) {
            
        uint32_t *pSuperState = new uint32_t[m_iNumThreads*STATE_SIZE];
        uint32_t *pCur = pSuperState;
        for (int i = 0; i < m_iNumThreads; i++) {
            memcpy(pCur, m_apWELL[i]->getState(), STATE_SIZE*sizeof(uint32_t));
            pCur +=  STATE_SIZE;
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_FINAL_WELL, m_iNumThreads*STATE_SIZE, pSuperState); 
        delete[] pSuperState;
    }
    */

    // probably not the correct place, because IDGen is app-global (not species specific)
    if (iResult == 0) {
        idtype *pIDGBuf = new idtype[m_iNumThreads];
        for (int i = 0; i < m_iNumThreads; i++) {
            pIDGBuf[i] = m_apIDGen[i]->getCur();
        }
        iResult = qdf_insertAttribute(hSpeciesGroup, SPOP_ATTR_IDGEN_STATE, m_iNumThreads, pIDGBuf);
        delete[] pIDGBuf;
    }

    if (iResult == 0) {
        iResult = dumpDeadSpaces(hSpeciesGroup);
    }

    if (iResult == 0) {
        iResult = dumpController(hSpeciesGroup, iDumpMode);
    }

    stdprintf("After dump\n");
    WELLUtils::showStates(m_apWELL, m_iNumThreads, true);

    return iResult;

}



//----------------------------------------------------------------------------
// restoreSpeciesDataQDF
//  read attributes to species data
//
template<typename T>
int  SPopulation<T>::restoreSpeciesDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    stdprintf("restoring species data\n");

    iResult = readSpeciesDataQDF(hSpeciesGroup);

    if (iResult == 0) {
        iResult = m_prio.restoreActionStatesQDF(hSpeciesGroup);
    }
    if (iResult == 0) {

        iResult = restoreWELL(m_apWELL, m_iNumThreads, m_sSpeciesName, hSpeciesGroup);
        if (iResult == 0) {
            stdprintf("WELL states after restore\n");
            WELLUtils::showStates(m_apWELL, m_iNumThreads, true);
        } else {
            // num states was missing, or states could not be read
            stdprintf("[readSpeciesDataQDF] num states was missing, or states could not be read. No full recovery possible\n");
            iResult = 0;
        }
     /*
        int iNumStates = 0;
        

        // number of WELL states dumped
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_NUM_STATES, 1, &iNumStates); 
        uint32_t *auiIndexes = new uint32_t[iNumStates];
    
        if (iResult == 0) {
            // curernt index values of the states
            iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_CUR_INDEX, iNumStates, auiIndexes); 
        }            

        if (iResult == 0) {
            // The WELL states themselves
            
            uint32_t *pSuperState = new uint32_t[iNumStates*STATE_SIZE];
            uint32_t *pCur = pSuperState;
            iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_FINAL_WELL, iNumStates*STATE_SIZE, pSuperState); 
            if (iResult == 0) {
                int iCopy = (iNumStates < m_iNumThreads)?iNumStates:m_iNumThreads;
                int i = 0;
                while (i < iCopy) {
                    m_apWELL[i]->seed(pCur, auiIndexes[i]);
                    pCur +=  STATE_SIZE;
                    i++;
                }
                // if there are more threads than saved states, the WELLs keep their states from the constructor
                // it doesn't matter, because continuing with a different number of threads changes everxthing
            }
            stdprintf("WELL states after restore\n");
            showStates();

            delete[] pSuperState;
            delete[] auiIndexes;
        }
*/
        if (iResult != 0) {
            // num states was missing, or states could not be read
            stdprintf("[readSpeciesDataQDF] num states was missing, or states could not be read. No full recovery possible\n");
            iResult = 0;
        }
    }
    
    // this is not really the place to store IDGen state, because it is app-global,
    // and not per species.
    // it should be ok though for multiplepops, because all would write the same values
    if (iResult == 0) {
        idtype *pIDGBuf = new idtype[m_iNumThreads];
        iResult = qdf_extractAttribute(hSpeciesGroup, SPOP_ATTR_IDGEN_STATE, m_iNumThreads, pIDGBuf);
        for (int i = 0; i < m_iNumThreads; i++) {
            m_apIDGen[i]->setCur(pIDGBuf[i]);
        }
        delete[] pIDGBuf;
    }

    if (iResult == 0) {
        iResult = restoreDeadSpaces(hSpeciesGroup);
    }
    
    if (iResult == 0) {
        iResult = restoreController(hSpeciesGroup);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// dumpAdditionalDataQDF
//  write additional data to the group 
//  (data to be stored separately from agent data, e.g. ancestors IDs or genes)
//
template<typename T>
int  SPopulation<T>::dumpAdditionalDataQDF(hid_t hSpeciesGroup) {
      
    return 0;
}


//----------------------------------------------------------------------------
// restoreAdditionalDataQDF
//  read additional data from the group
//  (data stored separately from agent data)
//
template<typename T>
int  SPopulation<T>::restoreAdditionalDataQDF(hid_t hSpeciesGroup) {
    return 0;
}




#define TEMP_SIZE 1048576
//----------------------------------------------------------------------------
// dumpController
//
template<typename T>
int  SPopulation<T>::dumpController(hid_t hSpeciesGroup, int iDumpMode) {
    stdprintf("dumping contoller\n");
    int iResult = 0;
    hsize_t iNumWritten = 0;
    herr_t status=-1;

    m_pAgentController->calcHolyness();
    // amount of space needed to save controller serialisation
    hsize_t dims = m_pAgentController->getBufSize(iDumpMode);
    uchar *pBuf = new uchar[dims];
    m_pAgentController->serialize(pBuf);
    
    uchar *pTempBuf = pBuf;
    hsize_t iCount = TEMP_SIZE;

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, CTRL_DATASET_NAME.c_str(), H5T_NATIVE_UCHAR, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
       
        if (hDataSet > 0) {

            while (iNumWritten < dims) {
                if ((dims - iNumWritten) > TEMP_SIZE) {
                    iCount = TEMP_SIZE; 
                } else {
                    iCount = dims - iNumWritten;
                    qdf_closeDataSpace(hMemSpace); 
                    hMemSpace = H5Screate_simple (1, &iCount, NULL); 
                }

                status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                             &iNumWritten, NULL, &iCount, NULL);
                
                status = H5Dwrite(hDataSet, H5T_NATIVE_UCHAR, hMemSpace,
                                  hDataSpace, H5P_DEFAULT, pTempBuf);
                
                iNumWritten += iCount;
                pTempBuf    += iCount;
            }
            
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }
    delete[] pBuf;

    return (status >= 0)?iResult:-1;
}

//----------------------------------------------------------------------------
// restoreController
//
template<typename T>
int  SPopulation<T>::restoreController(hid_t hSpeciesGroup) {
    int iResult = -1;
    stdprintf("[SPopulation<T>::restoreController] checking lists before\n");
    checkLists();
    
    if (qdf_link_exists(hSpeciesGroup, CTRL_DATASET_NAME)) {
        iResult = 0;

        // open the data set
        hid_t hDataSet = H5Dopen2(hSpeciesGroup, CTRL_DATASET_NAME.c_str(), H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        // get toal number of elements in dataset
        hsize_t dims;
        herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        stdprintf("Dataspace extent: %lld\n", dims);

        hsize_t iCount =  TEMP_SIZE;
        hsize_t iNumRead = 0;
        uchar *pBuf = new uchar[dims];
        uchar *pTempBuf = pBuf;
        
        
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 

        while (iNumRead < dims) {
             if ((dims - iNumRead) > TEMP_SIZE) {
                 iCount = TEMP_SIZE; 
             } else {
                 iCount = dims - iNumRead;
                 qdf_closeDataSpace(hMemSpace); 
                 hMemSpace = H5Screate_simple (1, &iCount, NULL); 
             }
             status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                          &iNumRead, NULL, &iCount, NULL);
             status = H5Dread(hDataSet, H5T_NATIVE_UCHAR, hMemSpace,
                              hDataSpace, H5P_DEFAULT, pTempBuf);
             iNumRead += iCount;
             pTempBuf += iCount;
        }

        if (status >= 0) {
            iResult = m_pAgentController->deserialize(pBuf);

            if (iResult == 0) {
                stdprintf("[SPopulation<T>::restoreController] checking lists after\n");
                checkLists();
            }

        }
        delete[] pBuf;
        
    } else {
        stdprintf("WARNING: no dataset [%s] found\n", CTRL_DATASET_NAME);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// dumpDeadSpaces
//
template<typename T>
int  SPopulation<T>::dumpDeadSpaces(hid_t hSpeciesGroup) {
    stdprintf("dumping dead spaces\n");
    int iResult = 0;
    uint iNumWritten = 0;
    herr_t status = 0; // status won't be changed if num desd spaces is 0

    // amount of elements we have to save
    hsize_t dims = m_iNumPrevDeaths;
    
    int    *pTempBuf = m_pPrevD;
    hsize_t iOffset = 0;
    hsize_t iCount = TEMP_SIZE;

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, DEAD_DATASET_NAME.c_str(), H5T_NATIVE_INT, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
       
        if (hDataSet > 0) {

            while (iNumWritten < dims) {
                if ((dims - iNumWritten) > TEMP_SIZE) {
                    iCount = TEMP_SIZE; 
                } else {
                    iCount = dims - iNumWritten;
                    qdf_closeDataSpace(hMemSpace); 
                    hMemSpace = H5Screate_simple (1, &iCount, NULL); 
                }

                status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                             &iOffset, NULL, &iCount, NULL);
                
                status = H5Dwrite(hDataSet, H5T_NATIVE_INT, hMemSpace,
                                  hDataSpace, H5P_DEFAULT, pTempBuf);
                
                
                iNumWritten += iCount;
                pTempBuf += iCount;
            }
            
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }

    return (status >= 0)?iResult:-1;
}

//----------------------------------------------------------------------------
// restoreDeadSpaces
//
template<typename T>
int  SPopulation<T>::restoreDeadSpaces(hid_t hSpeciesGroup) {
    int iResult = -1;
    stdprintf("restoring contoller\n");

    
    if (qdf_link_exists(hSpeciesGroup, DEAD_DATASET_NAME)) {
        iResult = 0;

        // open the data set
        hid_t hDataSet = H5Dopen2(hSpeciesGroup, DEAD_DATASET_NAME.c_str(), H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        // get toal number of elements in dataset
        hsize_t dims;
        herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        stdprintf("Dataspace extent: %lld\n", dims);
        m_iNumPrevDeaths = dims;
        hsize_t iCount =  TEMP_SIZE;
        hsize_t iOffset = 0;
        if (m_pPrevD != NULL) {
            delete[] m_pPrevD;
        }
        m_pPrevD = new int[dims];
        int *pTempBuf = m_pPrevD;
        uint iNumRead = 0;
        
        
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 

        while (iNumRead < dims) {
             if ((dims - iNumRead) > TEMP_SIZE) {
                 iCount = TEMP_SIZE; 
             } else {
                 iCount = dims - iNumRead;
                 qdf_closeDataSpace(hMemSpace); 
                 hMemSpace = H5Screate_simple (1, &iCount, NULL); 
             }
             status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                          &iOffset, NULL, &iCount, NULL);
             status = H5Dread(hDataSet, H5T_NATIVE_INT, hMemSpace,
                              hDataSpace, H5P_DEFAULT, pTempBuf);
             iNumRead += iCount;
             pTempBuf += iCount;
        }

        if (status >= 0) {
            iResult = 0;
        }
        
    } else {
        stdprintf("WARNING: no dataset [%s] found\n", DEAD_DATASET_NAME);
    }
    return iResult;

}


//----------------------------------------------------------------------------
// getFreeIndex
//
template<typename T>
int SPopulation<T>::getFreeIndex() {
    return m_pAgentController->getFreeIndex();
}


//----------------------------------------------------------------------------
// getFirstAgentIndex
//
template<typename T>
int SPopulation<T>::getFirstAgentIndex() {

    return m_pAgentController->getFirstIndex(LBController::ACTIVE);
}


//----------------------------------------------------------------------------
// getLastAgentIndex
//
template<typename T>
int SPopulation<T>::getLastAgentIndex() {

    return m_pAgentController->getLastIndex(LBController::ACTIVE);
}



/**********************************************************************************/
/**********************************************************************************/
/**NG STUFF*DEBUGGING STUFF*DEBUGGIN STUFF*DEBUGGING STUFF*DEBUGGING STUFF*DEBUGG**/
/**********************************************************************************/
/**********************************************************************************/

/*
//@@ to be remvoed
//----------------------------------------------------------------------------
// agentCheck (debugging)
//
template<typename T>
void SPopulation<T>::agentCheck() {
    stdprintf("Arrays:\n");
    m_pAgentController->displayArray(0, 0, m_pAgentController->getLayerSize());

    stdprintf("States:\n");    
    int iFirstAgent = m_pAgentController->getFirstIndex(LBController::ACTIVE);
    if (iFirstAgent != LBController::NIL) {
        int iLastAgent  = m_pAgentController->getLastIndex(LBController::ACTIVE);
        for (int iAgent = iFirstAgent; iAgent <= iLastAgent; iAgent++) {
            stdprintf(" %d:", iAgent);
            int s = m_aAgents[iAgent].m_iLifeState;
            switch (s) {
            case LIFE_STATE_ALIVE:
                stdprintf("L");
                break;
            case LIFE_STATE_DEAD:
                stdprintf("D");
                break;
            case LIFE_STATE_MOVING+LIFE_STATE_ALIVE:
                stdprintf("M");
                break;
            default:
                stdprintf("?");
                break;
            }
        }
        stdprintf("\n");
        
    }
    
}
//@@  until here
*/

//----------------------------------------------------------------------------
// showAgents (debugging only)
//   display a list of all agents by calling the pure virtual function showAgent
template<typename T>
void SPopulation<T>::showAgents() {
    stdprintf("-> %d agents\n", m_pAgentController->getNumUsed());
    int iCur = m_pAgentController->getFirstIndex(LBController::ACTIVE);
    while (iCur != LBController::NIL) {
        stdprintf("+ ");
        showAgent(iCur);
        stdprintf("\n");
        iCur = m_pAgentController->getNextIndex(LBController::ACTIVE, iCur);
    }
 
}


//----------------------------------------------------------------------------
// showAgent
//
template<typename T>
void SPopulation<T>::showAgent(int iAgentIndex) {
    Agent &a = m_aAgents[iAgentIndex];

    stdprintf(" [%d] ID [%ld] LS [%d] Loc [%d] ", iAgentIndex, a.m_ulID, a.m_iLifeState, a.m_ulCellID);
}


//----------------------------------------------------------------------------
// showControllerState
//
template<typename T>
void SPopulation<T>::showControllerState(const char *pCaption) {
    stdprintf("%s [%s]\n", pCaption, m_sSpeciesName);
    stdprintf("  layersize: %u\n", m_pAgentController->getLayerSize());
    stdprintf("  numlayers: %u\n", m_pAgentController->getNumLayers());
    stdprintf("  numused:   %u\n", m_pAgentController->getNumUsed());
    uint iSum1 = 0;
    for (uint i = 0; i <  m_pAgentController->getNumLayers(); i++) {
        iSum1 += m_pAgentController->getNumUsed(i);
    }
    stdprintf("  numused L: %d\n", iSum1);
    if (iSum1 != m_pAgentController->getNumUsed()) stdprintf("************** numused and layercount differ\n");
}

 
#endif
