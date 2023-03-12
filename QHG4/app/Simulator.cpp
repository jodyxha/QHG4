#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include <omp.h>

#include <hdf5.h>

#include "EventConsts.h"
#include "MessLoggerT.h"
#include "LineReader.h"
#include "ParamReader.h"
#include "LBController.h"
#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "gzutils.h"
#include "ArrayShare.h"
#include "GeoGroupReader.h"
#include "ClimateGroupReader.h"
#include "VegGroupReader.h"
#include "QDFUtils.h"
#include "StatusWriter.h"
#include "PopWriter.h"

#include "EnvInterpolator.h"
#include "AutoInterpolator.h"
#include "SCellGrid.h"

#include "PopBase.h"
#include "SPopulation.h"
#include "PopLooper.h"
#include "EventManager.h"
#include "OccTracker.h"
#include "GridScrambler.h"

#include "L2List.h" //@@ (for now) for LBController::DUMP_MODE_XXX

#include "SimParams.h"
#include "Simulator.h"

#define GENOMESIZE 20
const uint GZIP_BLOCK = 0x7fffffff;

//----------------------------------------------------------------------------
// constructor
//
Simulator::Simulator()
    : SimParams(),
      m_iLastCommTime(-1),
      m_bInterrupt(false),
      m_pgzCompressor(NULL),
      m_pOcc(NULL) {

}


//----------------------------------------------------------------------------
// destructor
//
Simulator::~Simulator() {

    if (m_pgzCompressor != NULL) {
        delete m_pgzCompressor;
    }

    if (m_pOcc != NULL) {
        delete m_pOcc;
    }
}


//----------------------------------------------------------------------------
// isReady
//   - make sure there is a grid
//   - make sure there is a poplooper
//   - make sure there ae populations
//   - create IDGens and pass them to the pops
//
bool Simulator::isReady() {
    bool bResult = false;
    if ((m_pCG != NULL) && (m_pPopLooper != NULL) && (m_pPopLooper->getNumPops() > 0)) {
        bResult = true;

        //@@ NOTE: for MPI apps we have to get:
        //         the global Max id, 
        //         the total number of threads,
        //         the total number of threads in the "preceding" nodes
        
        int    iNumThreads = omp_get_max_threads();
        int    iOffsBase   = 0;
        idtype iMaxID      = m_pPopLooper->getMaxID();
        printf("Setting Base for IDGens: %ld\n", iMaxID);
        printf("NOTE: for MPI apps we have to get:\n");
        printf("         the global Max id,\n"); 
        printf("         the total number of threads,\n");
        printf("         the total number of threads in the \"preceding\" nodes\n");

        printf("have %d threads\n", omp_get_max_threads());

        // only set IDGen data if we're not resuming
        // (restoreSpeciesDataQDF() sets the current values of the IDGens)
        if (!m_bResume) {
            for (int iT = 0; iT < iNumThreads; iT++) {
                m_apIDG[iT]->setData(iMaxID+1, iOffsBase+iT, iNumThreads);
            }      
        }

    } else {
        //        printf("No CellGrid or no Populations: can't start Simulation loop\n");
        LOG_ERROR2("No CellGrid or no Populations: can't start Simulation loop\n");
    }
    return bResult;
}


//----------------------------------------------------------------------------
// runSimulation
//
int Simulator::runSimulation() {
    int iResult = 0;
    
    iResult = preLoop();
    
    if (iResult == 0) {
        iResult = runLoop();

        postLoop();
    } else {
        //printf("[Simulator] Error during preLoop()\n");
        LOG_ERROR2("[Simulator] Error during preLoop()");
    }

    ArrayShare::freeInstance();
    return iResult;
}


//----------------------------------------------------------------------------
// preLoop
//   - create an instance of status writer
//
int Simulator::preLoop()  {
    int iResult = -1;

    if (isReady()) {

        if (m_bTrackOcc) {
            std::vector<int> vCellIDs;
            for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                vCellIDs.push_back(i);
            }
            m_pOcc = OccTracker::createInstance(vCellIDs, m_pPopLooper);
            m_pCG->setOccTracker(m_pOcc);
        }

        // fast forward events to current start time
        // one more in case of resume, because those things have been handled in the original run
        m_pEM->forwardTo(m_iStartStep+(m_bResume?1:0));

        // call the preloop methods of the pops
        iResult = 0;
        popvec vPops;
        // preloop for everybody
        m_pPopLooper->preLoop();
        
        if (iResult == 0) {
            // merge pops if required
            if (m_bMergePops) {
	        printf("trying to merge---\n");
                int iNumMerged = m_pPopLooper->tryMerge();
                printf("merged %d pops\n", iNumMerged);
            }

            // collect remaining pops in vPops
            popmap::const_iterator it_pop;
            for (it_pop = m_pPopLooper->begin(); (iResult == 0) && (it_pop != m_pPopLooper->end()); ++it_pop) {
                vPops.push_back(it_pop->second);
            }
                
            m_pSW = StatusWriter::createInstance(m_pCG, vPops);

            m_pgzCompressor = gzUtils::createInstance(GZIP_BLOCK/*m_iLayerSize*/);
        }

    } else {
        printf("Not ready to run:\n");
        if (m_pCG == NULL) {
            printf("  no Grid\n");
        }
        if ((m_pPopLooper == NULL) || (m_pPopLooper->getNumPops() == 0)) {
            printf("  no Populations\n");
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// checkEvents
//
void Simulator::checkEvents(int iCurStep) {
    // check for events
    if (m_pEM != NULL) {
        if (m_pEM->hasNewEvent(iCurStep)) {
            std::vector<EventData*> vEDSet;
            vEDSet = m_pEM->getEventData();
            for (unsigned int i = 0; i < vEDSet.size(); i++) {
                stdprintf("processing event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData); 
                int iOK = processEvent(vEDSet[i]);
                if (iOK != 0) {
                    //stdprintf("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData);
                    LOG_WARNING2("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData);
                }
            }
        }
    }
}


//----------------------------------------------------------------------------
// checkFinalEvents
//
void Simulator::checkFinalEvents() {
    // check for events
    stdprintf("[Simulator::checkFinalEvents] checking final events\n");
    if (m_pEM != NULL) {
        m_pEM->findFinalEvents();
        std::vector<EventData*> vEDSet;
        vEDSet = m_pEM->getEventData();
        printf("[Simulator::checkFinalEvents] have %zd final events\n", vEDSet.size());
        for (unsigned int i = 0; i < vEDSet.size(); i++) {
            int iOK = processEvent(vEDSet[i]);
            if (iOK != 0) {
                //stdprintf("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData);
                LOG_WARNING2("Couldn't process event (%d,[%s])\n", vEDSet[i]->m_iEventType, vEDSet[i]->m_sData);
            }
        }
    }
}


//----------------------------------------------------------------------------
// runLoop
//
int Simulator::runLoop() {
    int iResult = 0;

    double dEnd   = 0;
    printf("-------OMPA VERSION---------\n");
    printf("---------------------------\n");
    printf("------ starting loop ------\n");
    printf("------ %d iterations \n",m_iNumIters);
    printf("---------------------------\n");

    if (m_bResume) {
        m_iCurStep = m_iStartStep;
    } else {
        m_iCurStep = 0;
    }
    int iCompletedIterations = 0;

    checkEvents(m_iCurStep); // later maybe m_fCurTime
    
    printf("[Simulator::runLoop] AutoInterpolator: %p\n", m_pAutInt);
    if (m_pAutInt != NULL) {
        m_pAutInt->startInterpolations(m_iCurStep);
        m_pAutInt->calcNextDiff();
    }

    LOG_STATUS("Number of agents before starting\n");
    popmap::const_iterator it_pop;
    for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
        LOG_STATUS("  %s: %d\n", it_pop->second->getSpeciesName(), it_pop->second->getNumAgentsEffective());
    }
    
    LOG_STATUS("Starting Simulation ...\n");
    LOG_STATUS("-----\n");
    double dStart = omp_get_wtime();

    // start value for m_iCurStep set above
    while ((m_iCurStep < m_iNumIters) && (!m_bInterrupt)) {

        fprintf(stderr, "Step %03d ---------------------------\n", m_iCurStep);

        double dStart2 = omp_get_wtime();
        m_pPopLooper->doStep((float)m_iCurStep);
        double dEnd2 = omp_get_wtime();
        //        printf("step used: %f s\n", dEnd2-dStart2);

        // count total number of agents
        ulong iTotalTotal = 0;
        if (m_pPopLooper->getNumPops() > 0) {
            std::vector<int> viDeadIDs;
            
            for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
                PopBase *pPop = it_pop->second;
                if ( pPop->getNumAgentsEffective() == 0) {
                    stdprintf("Population [%s] has died out at step [%d] - removing it from action loop\n", pPop->getSpeciesName(), m_iCurStep);
                    // we must clean up the recyling buffers, otherwise attempted writes will cause a problem 
                    pPop->flushDeadSpace();
                    viDeadIDs.push_back(it_pop->first);
                } else {
                    iTotalTotal +=  pPop->getNumAgentsEffective();
                }
            }
            for (uint i = 0; i < viDeadIDs.size(); ++i) {
                m_pPopLooper->removePopByIndex(viDeadIDs[i], true);  // true: add to extincts
            }
        }
        
        printf("After step %d (%f s): total %8lu agents\n", m_iCurStep, dEnd2-dStart2, iTotalTotal);

        if (m_pCG->m_pOccTracker != NULL) {
            m_pCG->m_pOccTracker->updateCounts(m_iCurStep); // or m_iCurStep+starttime
        }

        m_iCurStep++;

        if (m_pVeg != NULL) {
            m_pVeg->resetUpdated();
        } 
        if (m_pCli != NULL) {
            m_pCli->resetUpdated();
        }
        if (m_pGeo != NULL) {
            m_pGeo->resetUpdated();
        }

        // check the events
        checkEvents(m_iCurStep);
        
        if (m_pAutInt != NULL) {
            // printf("checking for new interpolation calculation at %d\n", m_iCurStep); 
            iResult = m_pAutInt->checkNewInterpolation(m_iCurStep);
            if (iResult >= 0) {
                if (/*m_pAutInt->hasInterpolations() &&*/ ((m_iCurStep % m_iInterpolStep) == 0)) {
                    m_pAutInt->interpolate(m_iInterpolStep);
                    for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
                        // make the populations aware of the changes 
                        event_list::const_iterator it_ev;
                        for (it_ev = m_pAutInt->getEvents().begin(); it_ev != m_pAutInt->getEvents().end(); ++it_ev) {
                            iResult = it_pop->second->updateEvent(*it_ev, NULL, m_iCurStep);
                        }
                    }
                }
            }

        } else { 
            // (if m_pAUtInt works, we can get rid of m_pEnvInt)
            // perform interpolations if requested
            // this must happen *after* checkEvents and *before* flushEvents
            // for updates consistent with file-read updates
 
            if (m_pEnvInt->hasInterpolations() && ((m_iCurStep % m_iInterpolStep) == 0)) {
                m_pEnvInt->interpolate(m_iInterpolStep);
                for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
                    event_list::const_iterator it_ev;
                    for (it_ev = m_pEnvInt->getEvents().begin(); it_ev != m_pEnvInt->getEvents().end(); ++it_ev) {
                        iResult = it_pop->second->updateEvent(*it_ev, NULL, m_iCurStep);
                    }
                }
            }
         }

        // notify end of events
        for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
            it_pop->second->flushEvents((float)m_iCurStep);
        }
 
        iCompletedIterations = m_iCurStep;

        if (iTotalTotal == 0) {
            // avoid crash by stopping simulation if no agents survive
            checkFinalEvents();
            m_iCurStep = m_iNumIters;
        }
        
        fflush(stdout);

        if (m_bInterrupt) {
            printf("\n");
            printf("Creating dump\n");fflush(stdout);
            //@@            handleDumpEvent("SIG_INT");
            handleDumpEvent("flat");
            printf("Gracefully exiting\n");
        }
    }

    dEnd = omp_get_wtime();
    

    double dTimeUsed =  dEnd - dStart;
    int iHoursUsed = (int)(dTimeUsed/3600);
    dTimeUsed -= iHoursUsed*3600;
    int iMinutesUsed = (int)(dTimeUsed/60);
    dTimeUsed -= iMinutesUsed*60;
    int iSecondsUsed = (int)(dTimeUsed);

    printf("Used time: %f (%02d:%02d:%02d)\n", dEnd - dStart, iHoursUsed, iMinutesUsed, iSecondsUsed);
    
    LOG_STATUS2("-----\n");
    if (iCompletedIterations >= m_iNumIters) {
        LOG_STATUS2("Finished Simulation\n");
    } else {
        LOG_STATUS2("Stopped  Simulation\n");
    }

    LOG_STATUS2("Number of threads: %d\n", omp_get_max_threads());
    LOG_STATUS2("Number of iterations: %d\n", iCompletedIterations);
    LOG_STATUS2("Used time: %f (%02d:%02d:%02d)\n", dEnd - dStart, iHoursUsed, iMinutesUsed, iSecondsUsed);
    LOG_STATUS2("Number of agents after last step\n");
    int iFinalTotal = 0;
    int iMaxNameLen = strlen("total");
    // get total and maximal name length
    for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
        int iL = it_pop->second->getSpeciesName().length();
        if (iL > iMaxNameLen) {
            iMaxNameLen = iL;
        }
        iFinalTotal += it_pop->second->getNumAgentsEffective();
    }
    const popvec &vDeadPops = m_pPopLooper->getDead();
    for (uint i = 0; i < vDeadPops.size(); i++) {
        int iL = vDeadPops[i]->getSpeciesName().length();
        if (iL > iMaxNameLen) {
            iMaxNameLen = iL;
        }
	iFinalTotal += vDeadPops[i]->getNumAgentsEffective();
    }

    std::string sTemplate = stdsprintf("  %%%ds: %%9d\n", iMaxNameLen);
    // print it out nicely
    for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
        LOG_STATUS2(sTemplate, it_pop->second->getSpeciesName(), it_pop->second->getNumAgentsEffective());
    }
    for (uint i = 0; i < vDeadPops.size(); i++) {
        LOG_STATUS2(sTemplate, vDeadPops[i]->getSpeciesName(), vDeadPops[i]->getNumAgentsEffective());
    }    
    LOG_STATUS2(sTemplate, "total", iFinalTotal);
    
    return iResult;
}


//----------------------------------------------------------------------------
// postLoop
//  call postLoop for all populations, living and dead
//
int Simulator::postLoop() {
    int iResult = 0;

    m_pPopLooper->postLoop();

    // also do this for the extinct populations because some clean-up might be necessary.
    const popvec &vp2 = m_pPopLooper->getDead();
    for (uint i = 0; (iResult == 0) && (i < vp2.size()); i++) {
        iResult = vp2[i]->postLoop();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleWriteEvent
//
int Simulator::handleWriteEvent(const std::string sDesc, int iDumpMode) {
    int iResult = -1;
    //stdprintf("[Simulator::handleWriteEvent] Handling writeEvent [%s] at step %d\n", sDesc, m_iCurStep);
    LOG_STATUS2("[Simulator::handleWriteEvent] Handling writeEvent [%s] at step %d\n", sDesc, m_iCurStep);

    std::string sTemp  = sDesc;
    std::string sPops  = "";
    std::string sOther = "";
    std::vector<std::pair<std::string, popwrite_flags>> vSubs;

    int iWhat = WR_NONE;

    stringvec vEvents;
    uint iNum = splitString(sDesc, vEvents, "+");
    if (iNum > 0) {
        iResult = 0;
        for (uint i = 0; (iResult == 0) && (i < iNum); i++) {
            std::string sSub = "";
            const std::string &sEvent = vEvents[i];

            if (sEvent.substr(0, EVENT_PARAM_WRITE_POP.size()) == EVENT_PARAM_WRITE_POP) {
                stringvec vSub;
                uint iNumSub = splitString(sEvent, vSub, ":");
                if (iNumSub  >= 2) { 
                    iWhat += WR_POP;
                    sSub = vSub[1];
                } else {
                    stdprintf("Invalid event param [%s]\n", vEvents[i]);
                    iResult = -1;
                }
            } else if (sEvent ==  EVENT_PARAM_WRITE_GRID) {
                iWhat |= WR_GRID;
                iWhat = iWhat | WR_GRID;
                sOther += "S";
            } else if (sEvent == EVENT_PARAM_WRITE_GEO) {
                iWhat |= WR_GEO;
                sOther += "G";
            } else if (sEvent == EVENT_PARAM_WRITE_CLIMATE) {
                iWhat |= WR_CLI;
                sOther += "C";
            } else if (sEvent == EVENT_PARAM_WRITE_VEG) {
                iWhat |= WR_VEG;
                sOther += "V";
            } else if (sEvent == EVENT_PARAM_WRITE_NAV) {
                iWhat |= WR_NAV;
                sOther += "N";
            } else if (sEvent == EVENT_PARAM_WRITE_ENV) {
                iWhat |= WR_ALL;
                sOther += "env";
            } else if (sEvent == EVENT_PARAM_WRITE_OCC) {
                iWhat |= WR_OCC;
                sOther += "O";
            } else {
                iWhat = WR_NONE;
                iResult = -1;
                //stdprintf("Unknown output type [%s] (%s)\n", sEvent, sDesc);
                LOG_ERROR2("Unknown output type [%s] (%s)\n", sEvent, sDesc);
            }
    
            if  (iWhat != WR_NONE) {
                if (iResult == 0) {
                    popwrite_flags iWS = popwrite_flags::PW_NONE;
                
                    if  ((!sSub.empty()) && (iWhat >= WR_POP)) {
                        sPops += "_pop-";
 
                        size_t iPosSpecial = sSub.find_first_of("#%~*");
                        std::string sD = "_";
 
                        if (iPosSpecial != std::string::npos) {
                            size_t iStar = sSub.find_first_of('*', iPosSpecial);
                            if (iStar != std::string::npos) {
                                iWS = PW_ALL;
                                sD += "PMA";
                            } else {  

                                size_t iPos = sSub.find_first_of("#%~", iPosSpecial);
                                size_t iPos0 = iPos; 
                                while (iPos != std::string::npos) {
                                    char c = sSub.at(iPos);
                                    switch(c) {
                                    case '#':
                                        sD += "P";
                                        iWS |= popwrite_flags::PW_AGENTS_ONLY;
                                        break;
                                    case '%':
                                        sD += "M";
                                        iWS |= popwrite_flags::PW_STATS_ONLY;
                                        break;
                                    case '~':
                                        sD += "A";
                                        iWS |= popwrite_flags::PW_ADDITIONAL_ONLY;
                                        break;
                                    }
                                    iPos = sSub.find_first_of("#%~", iPos+1);
                                }
                                if (iPos0 != std::string::npos) {
                                    sSub = sSub.substr(0, iPos0);
                                }
                            }
        
                        } else {
                            iWS = popwrite_flags::PW_ALL;
                            sD = "";
                        }

                        sPops += sSub;
                        sPops += sD;
                        stdprintf("current sPops is [%s])\n", sPops);
                        stdprintf("pushing back (%s, %d)\n", sSub, iWS);
                
                        vSubs.push_back(std::pair<std::string, popwrite_flags>(sSub, iWS));
                    }
                
                    if (iWS == PW_NONE) {
                        iWS = PW_ALL;
                    }
                
                }
            } 
        }
    }
    
    if (iResult == 0) {
        printf("[Simulator::handleWriteEvent] vSubs has %zd elements\n", vSubs.size());
        for (uint i = 0; i < vSubs.size(); i++) {
            stdprintf("[Simulator::handleWriteEvent]  %s -> %d\n", vSubs[i].first, vSubs[i].second);
        }
        
        m_pPopLooper->preWrite(m_iCurStep);

        std::string sName = stdsprintf("%s%s%s%s_%s_%06d.qdf", m_sOutputDir, m_sOutputPrefix, (iDumpMode != LBController::DUMP_MODE_NONE)?"_dump":"", sPops, sOther, m_iCurStep);
        stdprintf("[Simulator::handleWriteEvent] writing file [%s] with %sdump\n", sName, (iDumpMode != LBController::DUMP_MODE_NONE)?"":"no ");
        iResult = writeState(sName, iWhat, vSubs, iDumpMode);
        if (iDumpMode != LBController::DUMP_MODE_NONE) {
            m_vDumpNames.push_back(sName);
        }
    
    } 
    return iResult;
}


//----------------------------------------------------------------------------
// handleDumpEvent
//
int Simulator::handleDumpEvent(const std::string sDesc) {
    int iResult = 0;
    int iDumpMode = LBController::DUMP_MODE_NONE;
    //stdprintf("Handling dumpEvent [%s] at step %d\n", sDesc, m_iCurStep);
    LOG_STATUS2("Handling dumpEvent [%s] at step %d\n", sDesc, m_iCurStep);
    if (sDesc == "flat") {
        iDumpMode = LBController::DUMP_MODE_FLAT;
    } else if (sDesc == "smart") {
        iDumpMode = LBController::DUMP_MODE_SMART;
    } else if (sDesc == "free") {
        iDumpMode = LBController::DUMP_MODE_FREE;
    } else {
        //stdprintf("Illegal DumpMode [%s]\n", sDesc);
        LOG_STATUS2("Illegal DumpMode [%s]\n", sDesc);
        iResult = -1;
    }
    
    if (iResult == 0) {

        m_vDumpNames.clear();
        // write a nice environment thingy
        iResult = handleWriteEvent(EVENT_PARAM_WRITE_ENV, iDumpMode);
        if (iResult == 0) {
            // concatenate all pop names for dump name
            std::string sTemp = EVENT_PARAM_WRITE_POP;
            popmap::const_iterator it_pop;
            for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
                sTemp += ":" + it_pop->second->getSpeciesName();
            }
            // writing pop dump
            stdprintf("sending off dump writeevent [%s]\n", sTemp); 
            iResult = handleWriteEvent(sTemp, iDumpMode);
        }
        // now write param config file for resume
        if (iResult == 0) {
            std::string sConfigOut = stdsprintf("%s%s_dump_%06d.cfg", m_sOutputDir, m_sOutputPrefix, m_iCurStep);
            stdprintf("Writing resume config file [%s]\n", sConfigOut);
            writeResumeConfig(sConfigOut);
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// handleEnvironmentEvent
//   Full format  "env"|<type>[+<type>]:<qdf-file>
//   Here we only look at the stuff following the '|'
//
int Simulator::handleEnvironmentEvent(const std::string sDesc) {
    int iResult = -1;

    //stdprintf("Handling environmentEvent [%s] at step %d\n", sDesc, m_iCurStep);
    LOG_STATUS2("Handling environmentEvent [%s] at step %d\n", sDesc, m_iCurStep);

    stringvec vParts;
    uint iNum = splitString(sDesc, vParts, ":");
    if (iNum == 2) {
        std::string &sFile = vParts[1];
        stringvec vTypes;
        uint iNumTypes = splitString(vParts[0], vTypes, "+");
        if (iNumTypes> 0) {

            double dStart = omp_get_wtime();
            std::string sExistingFile;
            if (exists(sFile, sExistingFile)) {
                hid_t hFile = qdf_openFile(sExistingFile);
                iResult = 0;

                std::set<int> vEvents;
                for (uint i = 0; (iResult == 0) && (i < vTypes.size()); i++) {
                    std::string &sCurType = vTypes[i];
                    if (sCurType == EVENT_PARAM_NAME_ALL)  {
                        vEvents.insert(EVENT_ID_GEO);
                        iResult = setGeo(hFile, true, true);  // true,true: isRequired, isUpdate
                        if (iResult == 0) {
                            vEvents.insert(EVENT_ID_CLIMATE);
                            iResult = setClimate(hFile, true, true);  // true,true: isRequired, isUpdate
                            if (iResult == 0) {
                                vEvents.insert(EVENT_ID_VEG);
                                iResult = setVeg(hFile, true, true);  // true,true: isRequired, isUpdate
                                if (iResult == 0) {
                                    vEvents.insert(EVENT_ID_NAV);
                                    iResult = setNav(hFile, true);  // true,true: isRequired, isUpdate
                                }
                            }
                        }
                    } else if (sCurType == EVENT_PARAM_NAME_GEO)  {
                        vEvents.insert(EVENT_ID_GEO);
                        iResult = setGeo(hFile, true, true);  // true,true: isRequired, isUpdate
                    } else if (sCurType == EVENT_PARAM_NAME_CLIMATE) {
                        vEvents.insert(EVENT_ID_CLIMATE);
                        iResult = setClimate(hFile, true, true);  // true,true: isRequired, isUpdate
                    } else if (sCurType == EVENT_PARAM_NAME_VEG) {
                        vEvents.insert(EVENT_ID_VEG);
                        iResult = setVeg(hFile, true, true);  // true,true: isRequired, isUpdate
                    } else if (sCurType == EVENT_PARAM_NAME_NAV) {
                        vEvents.insert(EVENT_ID_NAV);
                        iResult = setNav(hFile, true);  // true: isUpdate
                    } else {
                        // iResult = -1;
                        //stdprintf("ignoring unknown environment type: [%s]\n", sCurType);
                        LOG_ERROR2("ignoring unknown environment type [%s]\n", sCurType);                
                    }               
                }
                double dEnd = omp_get_wtime();
                qdf_closeFile(hFile);
                //printf("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
                LOG_WARNING2("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
                if (vEvents.size() > 0) {
                    popmap::const_iterator it_pop;
                    for (it_pop = m_pPopLooper->begin(); (iResult == 0) && (it_pop != m_pPopLooper->end()); ++it_pop) {
                        for (intset::iterator it = vEvents.begin(); it != vEvents.end(); ++it) {
                            iResult = it_pop->second->updateEvent(*it, NULL, m_iCurStep);
                        }
                    }
                }
            } else {
                //stdprintf("couldn't find file [%s]\n", sFile);
                LOG_ERROR2("couldn't find file [%s]\n", sFile);
            }

        } else {
            //stdprintf("expected at least 1 type\n");
            LOG_ERROR2("expected at least 1 type\n");
        }
        
    } else {
        //stdprintf("expected params of the form \"<type>[+<type>]:<qdf-file>\": [%s]\n", sDesc);
        LOG_ERROR2("expected params of the form \"<type>[+<type>]:<qdf-file>\": [%s]\n", sDesc);
    }


    return iResult;
}

//----------------------------------------------------------------------------
// handleEnvArrayEvent
//   Full format  "arr"|<type>:<arrname>:<qdf-file>
//   Here we only look at the stuff following the '|'
//
int Simulator::handleEnvArrayEvent(const std::string sDesc) {
    int iResult = -1;

    //stdprintf("Handling envArrayEvent [%s] at step %d\n", sDesc, m_iCurStep);
    LOG_STATUS2("Handling envArrayEvent [%s] at step %d\n", sDesc, m_iCurStep);
    stringvec vParts;
    uint iNum = splitString(sDesc, vParts, ":");
    if (iNum == 3) {
        std::string &sGroup   = vParts[0];
        std::string &sArrName = vParts[1];
        std::string &sFile    = vParts[2];

        double dStart = omp_get_wtime();
        std::string sExistingFile;
        if (exists(sFile, sExistingFile)) {
            hid_t hFile = qdf_openFile(sExistingFile);
            iResult = 0;
            int iEvent = 0;
            if (sGroup == EVENT_PARAM_NAME_GEO)  {
                iEvent = EVENT_ID_GEO;
                iResult = setGeoArray(hFile, sArrName);
            } else if (sGroup == EVENT_PARAM_NAME_CLIMATE) {
                iEvent = EVENT_ID_CLIMATE;
                iResult = setClimateArray(hFile, sArrName);
            } else if (sGroup == EVENT_PARAM_NAME_VEG) {
                iEvent = EVENT_ID_VEG;
                iResult = setVegArray(hFile, sArrName);
            } else if (sGroup == EVENT_PARAM_NAME_NAV) {
                //stdprintf("[handleEnvArrayEvent] no array setting  for [%s]\n", sGroup);
                LOG_ERROR2("[handleEnvArrayEvent] no array setting  for [%s]\n", sGroup);
            } else {
                // iResult = -1;
                //stdprintf("[handleEnvArrayEvent] ignoring unknown environment type: [%s]\n", sGroup);
                LOG_ERROR2("[handleEnvArrayEvent] ignoring unknown environment type [%s]\n", sGroup);                
            }                                   
                    
            double dEnd = omp_get_wtime();
            qdf_closeFile(hFile);
            
            //            printf("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
            LOG_WARNING2("[%d]environment load took %f s\n", m_iCurStep, dEnd -dStart);
            if (iResult == 0) {
                popmap::const_iterator it_pop;
                for (it_pop = m_pPopLooper->begin(); (iResult == 0) && (it_pop != m_pPopLooper->end()); ++it_pop) {
                    iResult = it_pop->second->updateEvent(iEvent, NULL, m_iCurStep);
                }
            } else {

                stdprintf("[handleEnvArrayEvent] Couldn't open array [%s]  for group [%s] in file [%s]\n", sArrName, sGroup, sExistingFile);
            }
        } else {
            LOG_ERROR2("[handleEnvArrayEvent] couldn't find file [%s]\n", sFile);
        }

    } else {
        LOG_ERROR2("[handleEnvArrayEvent] expected exactlyx three ':'-separted params [%s]\n", sDesc);
    }


    return iResult;
}


//----------------------------------------------------------------------------
// handlePopEvent
//   Full format  "pop"|<speciesname>[+<speciesname>]:<qdf-file>
//   Here we only look at the stuff following the '|'
//
int Simulator::handlePopEvent(const std::string sDesc) {
    int iResult = -1;

    LOG_STATUS2("[Simulator::handlePopEvent] Handling populationEvent [%s] at step %d\n", sDesc, m_iCurStep);
    stringvec vParts;
    uint iNum = splitString(sDesc, vParts, ":");
    double dStart = 0;
    double dEnd = 0;
    if (iNum == 2) {
        std::string &sFile = vParts[1];
        stringvec vSpecies;
        uint iNumSpecies = splitString(vParts[0], vSpecies, "+");
        if (iNumSpecies > 0) {
            dStart = omp_get_wtime();
            std::string sExistingFile;
            if (exists(sFile, sExistingFile))  {
                hid_t hFile = qdf_openFile(sExistingFile);
                iResult = 0;

                for (uint i = 0; (iResult == 0) && (i < vSpecies.size()); i++) {
                    iResult  = setPops(hFile, vSpecies[i],true);
                }
                
                dEnd = omp_get_wtime();
                qdf_closeFile(hFile);

            } else {
                LOG_ERROR2("[handlePopEvent] couldn't find file [%s]\n", sFile);
            }
            
        } else {
            LOG_ERROR2("[handlePopEvent] empty pop list? [%s]\n", sDesc);
        }
        
    } else if (iNum == 3) {
        std::string sPop = vParts[0];
        std::string sXML = vParts[1];
        std::string sDat = vParts[2];
        dStart = omp_get_wtime();
        iResult = setPopsFromXMLFile(sXML, sDat);
        dEnd = omp_get_wtime();
        if (iResult == 0) {
        } else {
            LOG_ERROR2("[handlePopEvent] couldn't ceate populaion from xml [%s] and dat [%s]\n", sXML, sDat);
        }
        
    } else {
        LOG_ERROR2("[handlePopEvent] expected either two or three  ':'-separted params [%s]\n", sDesc);
        iResult = -1;
    }

    if (iResult == 0) {
        if (m_bMergePops) {
            printf("[handlePopEvent] checking for mergeable pops\n");
            int iNumMerged = m_pPopLooper->tryMerge();
            if (iNumMerged >= 0) {
                printf("[handlePopEvent] merged %d pops\n", iNumMerged);
            } else {
                iResult = -1;
                LOG_WARNING2("[handlePopEvent] merge error\n");
            }
        }
        
        LOG_WARNING2("[%d]population load took %f s\n", m_iCurStep, dEnd -dStart);
        if (iResult == 0) {
            popmap::const_iterator it_pop;
            for (it_pop = m_pPopLooper->begin(); (iResult == 0) && (it_pop != m_pPopLooper->end()); ++it_pop) {
                iResult = it_pop->second->updateEvent(EVENT_ID_POP, NULL, m_iCurStep);
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleInterpolEvent
//   Full format  
//      "interpol|file:"<filename>
//   or
//      "interpol|cmd:"<command>
//   where
//      filename   name of a QDF interpolation file
//      command    "stop" or "start" (currently the only supported command)
//
int Simulator::handleInterpolEvent(const std::string sDesc) {
    int iResult = -1;
    
    LOG_STATUS2("[Simulator::handleInterpolEvent] Handling interpolEvent [%s] at step %d\n", sDesc, m_iCurStep);

    stringvec vParts;
    uint iNum = splitString(sDesc, vParts, ";");
    if (iNum == 2) {
        std::string &sCmd   = vParts[0];
        std::string &sParam = vParts[1];
        if (sCmd == EVENT_PARAM_INTERPOL_FILE) {
            std::string sExistingFile;
            if (exists(sParam, sExistingFile)) {
                iResult = m_pEnvInt->readInterpolationData(sParam);
            } else {
                LOG_ERROR2("[Simulator::handleInterpolEvent] couldn't find file [%s]\n", sParam);
            }
        } else if (sCmd == EVENT_PARAM_INTERPOL_CMD) {
            if (sParam == EVENT_PARAM_INTERPOL_START) {
                m_pEnvInt->setActive(true);
                iResult = 0;
            } else if (sParam == EVENT_PARAM_INTERPOL_STOP) {
                m_pEnvInt->setActive(false);
                iResult = 0;
            } else  {
                LOG_ERROR2("[Simulator::handleInterpolEvent] unknown command [%s] (only '%s' or '%s' supported\n", sCmd, EVENT_PARAM_INTERPOL_START,EVENT_PARAM_INTERPOL_STOP);
            }        
        } else {
            LOG_ERROR2("[Simulator::handleInterpolEvent] unknown mode [%s] (only '%s' or '%s' supported\n", sCmd, EVENT_PARAM_INTERPOL_FILE,EVENT_PARAM_INTERPOL_CMD);
                    
        }   

    } else {
        LOG_ERROR2("[Simulator::handleInterpolEvent] expected a ':' in interpol event [%s]\n", sDesc);
    }

    return iResult;
}



//----------------------------------------------------------------------------
// handleScrambleEvent
//   Full format  "scramble"|""<eventid>:<data>
//   Here we only look at the stuff following the '|'
//
int Simulator::handleScrambleEvent(const std::string sDesc) {
    int iResult = 0;
    // curretly, "all" and "connections" means the same thing
    // this will hold values for all conectivities (4, 6)

    m_pGridScrambler->scrambleConnections();
    return iResult;
}


//----------------------------------------------------------------------------
// handleUserEvent
//   Full format  "user"|<eventid>:<data>
//   Here we only look at the stuff following the '|'
//
int Simulator::handleUserEvent(const std::string sDesc) {

    int iResult = -1;
    
    stringvec vParts;
    uint iNum = splitString(sDesc, vParts, ":");
    if (iNum == 2) {
        LOG_STATUS2("[Simulator::handleUserEvent] Handling userEvent [%s] at step %d\n", sDesc, m_iCurStep);
        std::string &sID   = vParts[0];
        std::string &sData = vParts[1];
   
        int iUserEvent = -1;
        if (strToNum(sID, &iUserEvent) && ((iUserEvent >= EVENT_ID_USR_MIN) && (iUserEvent <= EVENT_ID_USR_MAX))) {
            stdprintf("[Simulator::handleUserEvent] we need to do something with id %d and data [%s]\n", iUserEvent, sData);
            iResult = 0;

        } else {
            LOG_ERROR2("[Simulator::handleUserEvent] the first part should be a user event id in [%d, %d]\n", EVENT_ID_USR_MIN, EVENT_ID_USR_MAX);
        }
           
    } else {
        LOG_ERROR2("[Simulator::handleUserEvent] expected ':' in user event [%s]\n", sDesc);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setGeoArray
//     an array event
//
int Simulator::setGeoArray(hid_t hFile, const std::string sArrName) {
    int iResult = -1;
    // try for array event
    GeoGroupReader *pGR = GeoGroupReader::createGeoGroupReader(hFile);
    if (pGR != NULL) {
        GeoAttributes geoatt;
        iResult = pGR->readAttributes(&geoatt);
        if (iResult == 0) {                
            if (m_pGeo != NULL) {
                m_pGeo = m_pCG->m_pGeography;
                iResult = pGR->readArray(m_pGeo, sArrName);
                if (iResult == 0) {
                    LOG_STATUS2("[Simulator::setGeoArray] Successfully read array [%s] form file\n", sArrName);
                } else {
                    LOG_ERROR2("[Simulator::setGeoArray] unknown geo array [%s]\n", sArrName);
                }
            } else {
                LOG_ERROR2("[Simulator::setGeoArray] A Geography object must exist to add array\n");
                iResult = -1;
            }
            
        } else {
            LOG_ERROR2("[Simulator::setGeoArray] Couldn't read attributes from file\n");
        }
        delete pGR;
    } else {
        LOG_ERROR2("[Simulator::setGeoArray] Couldn't open GeoGroupReader for file\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setClimateArray
//     an array event
//
int Simulator::setClimateArray(hid_t hFile, const std::string sArrName) {
    int iResult = -1;
    // try for array event
    ClimateGroupReader *pCR = ClimateGroupReader::createClimateGroupReader(hFile);
    if (pCR != NULL) {
        ClimateAttributes climatt;
        iResult = pCR->readAttributes(&climatt);
        if (iResult == 0) {                
            if (m_pCli != NULL) {
                iResult = pCR->readArray(m_pCli, sArrName);
                if (iResult == 0) {
                    LOG_STATUS2("[Simulator::setClimateArray] Successfully read array [%s] form [%s]\n", sArrName);
                } else {
                    LOG_ERROR2("[Simulator::setClimateArray] unknown geo arrayevent [%s]\n", sArrName);
                }
            } else {
                LOG_ERROR2("[Simulator::setClimateArray] A Climate object must exist to add array\n");
                iResult = -1;
            }
            
        } else {
            LOG_ERROR2("[Simulator::setClimateArray] Couldn't read attributes from file\n");
        }
        delete pCR;
    } else {
        LOG_ERROR2("[Simulator::setClimateArray] Couldn't open ClimateGroupReader for file\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setVegArray
//     an array event
//
int Simulator::setVegArray(hid_t hFile, const std::string sArrName) {
    int iResult = -1;
    // try for array event
    VegGroupReader *pVR = VegGroupReader::createVegGroupReader(hFile);
    if (pVR != NULL) {
        VegAttributes vegatt;
        iResult = pVR->readAttributes(&vegatt);
        if (iResult == 0) {                
            if (m_pVeg != NULL) {
                iResult = pVR->readArray(m_pVeg, sArrName);
                
                if (iResult == 0) {
                    LOG_STATUS2("[Simulator::setVegArray] Successfully read array [%s] form [%s]\n", sArrName);
                } else {
                    LOG_ERROR2("[Simulator::setVegArray] unknown geo arrayevent [%s]\n", sArrName);
                }
            } else {
                LOG_ERROR2("[Simulator::setVegArray] A Vegetation object must exist to add array\n");
                iResult = -1;
            }
            
        } else {
            LOG_ERROR2("[Simulator::setVegArray] Couldn't read attributes from file\n");
        }
        delete pVR;
    } else {
        LOG_ERROR2("[Simulator::setVegArray] Couldn't open VegGroupReader for file\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleCheckEvent
// Currently supported:
//   LISTS
//
int Simulator::handleCheckEvent(const std::string sChecks) {
    int iResult = 0;

    stringvec vChecks;
    uint iNum = splitString(sChecks, vChecks, "+");
    if (iNum > 0) {

        stringvec::const_iterator it;
        for (it = vChecks.begin(); it != vChecks.end(); ++it) {
            if (*it == EVENT_PARAM_CHECK_LISTS) {
                popmap::const_iterator it_pop;
                for (it_pop = m_pPopLooper->begin(); (iResult == 0) && (it_pop != m_pPopLooper->end()); ++it_pop) {
                    iResult = it_pop->second->checkLists();
                }
            } else {
                iResult = -1;
                LOG_ERROR2("unknown check type: [%s]\n", *it);
            }
        }
    } else {
        iResult = -1;
        LOG_ERROR2("empty check list?\n");
    }
       
 
    return iResult;
}


//----------------------------------------------------------------------------
// handleCommEvent
// Currently supported:
//   SET ITERS <numiters>
//   REMOVE ACTION <population>:<actionname>
//
int Simulator::handleCommEvent(const std::string sCommFile) {
    int iResult = 0;

    struct stat buf;
    iResult = stat(sCommFile.c_str(), &buf);
    if (iResult == 0) {
        int iNewTime = buf.st_mtim.tv_sec;
        if (iNewTime > m_iLastCommTime) {
            m_iLastCommTime = iNewTime;
            LineReader *pLR = LineReader_std::createInstance(sCommFile, "rt");
            if (pLR != NULL) {
                char *pLine = pLR->getNextLine();
                while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
                    iResult = handleCommLine(pLine);
                    iResult = 0;
                    pLine = pLR->getNextLine();
                }
                delete pLR;
            } else {
                stdprintf("Couldn'get open [%s]\n", sCommFile);
            }
            //
            stringvec vs;
            m_pEM->toString(vs);
            for (uint i = 0; i < vs.size(); ++i) {
                std::string &sCur = vs[i];
                stdprintf("%s\n", addEventName(sCur));
            }

        }
    } else {
        iResult = 0;
        stdprintf("Couldn'get stat for [%s]\n", sCommFile);
        stdprintf("Trying [%s] as command\n", sCommFile);
        handleCommLine(sCommFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleCommLine
// Currently supported:
//   SET ITERS:<numiters>
//   REMOVE ACTION <population>:<actionname>
//   MOD POP
//
int Simulator::handleCommLine(const std::string sLine) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sLine, vParts, ":");
    if (iNum > 0) {
        std::string &sCommand = vParts[0];
        if (sCommand == CMD_SET_ITERS) {
            if (iNum == 2) {
                int iNumIters = 0;
                if (strToNum(vParts[1], &iNumIters)) {
                    if (iNumIters < m_iCurStep) {
                        m_iNumIters = m_iCurStep+1;
                    } else {
                        m_iNumIters = iNumIters;
                    }
                    m_pEM->triggerAll(m_iNumIters);
                    iResult = 0;
                } else {
                    LOG_ERROR2("[Simulator::handleCommLine] Bad argument to command '%s': [%s]\n", CMD_SET_ITERS, sLine);
                    iResult = -1;
                }
            } else {
                LOG_ERROR2("[Simulator::handleCommLine] Expected number of iters [%s]\n", sLine);
                iResult = -1;
            }

        } else if ((sCommand == CMD_REMOVE_ACTION) ||
                   (sCommand == CMD_DISABLE_ACTION) ||
                   (sCommand == CMD_ENABLE_ACTION)) {
            if (iNum == 3) {
                std::string &sPopName = vParts[1];
                std::string &sAction  = vParts[2];

                PopBase *pPop = m_pPopLooper->getPopByName(sPopName);
                if (pPop != NULL) {
                    
                    if (sCommand == CMD_REMOVE_ACTION){
                        iResult = pPop->removeAction(sAction);
                        if (iResult == 0) {
                            stdprintf("[Simulator::handleCommLine] Successfully removed action [%s]\n", sAction);
                        }
                    } else if(sCommand == CMD_DISABLE_ACTION) {
                        iResult = pPop->disableAction(sAction);
                        if (iResult == 0) {
                            stdprintf("[Simulator::handleCommLine] Successfully disabled action [%s]\n", sAction);
                        }
                    } else if(sCommand == CMD_ENABLE_ACTION) {
                        iResult = pPop->enableAction(sAction);
                        if (iResult == 0) {
                            stdprintf("[Simulator::handleCommLine] Successfully enabled action [%s]\n", sAction);
                        }
                    }
                    iResult = 0;
                } else {
                    LOG_ERROR2("[Simulator::handleCommLine] Couldn't find population [%s]\n", sPopName);
                    iResult = -1;
                }
            } else {
                LOG_ERROR2("[Simulator::handleCommLine] Expected population name and action name\n");
                iResult = -1;
            }

        } else if (sCommand == CMD_MOD_POP) {
            if (iNum == 4) {
                std::string &sPopName = vParts[1];
                std::string &sAttr    = vParts[2];
                std::string &sValue   = vParts[3];
                double dValue = 0;
                if (strToNum(sValue, &dValue)) {
                    
                    PopBase *pPop = m_pPopLooper->getPopByName(sPopName);
                    if (pPop != NULL) {
                        pPop->modifyAttributes(sAttr, dValue);
                        iResult = 0;
                    } else {
                        LOG_ERROR2("[Simulator::handleCommLine] No Population found with name [%s]\n", sPopName);
                        iResult = -1;
                    }
                } else {
                    LOG_ERROR2("[Simulator::handleCommLine] Expected numerical value: [%s]\n", sValue);
                    iResult = -1;
                }
            } else {
                LOG_ERROR2("[Simulator::handleCommLine] Expected <popname>:<param>:<value>\n");
                iResult = -1;
            }
      
        } else {
            LOG_ERROR2("[Simulator::handleCommLine] unknown command [%s]\n", sCommand);
            iResult = -1;
        }
    } else {
        LOG_ERROR2("[Simulator::handleCommLine] empty command [%s]?\n", sLine);
        iResult= -1;
    }

  
    return iResult;
}



//----------------------------------------------------------------------------
// processEvent
//   EVENT_ID_WRITE   (output)
//   EVENT_ID_ENV     (geography,climate,veg,nav change)
//   EVENT_ID_POP     (add populations)
// 
//   EVENT_ID_GEO     (geography change)
//   EVENT_ID_CLIMATE (climate change)
//   EVENT_ID_VEG     (vegetation change)
//   EVENT_ID_NAV     (navigation change)
//
//   EVENT_ID_DUMP    (dump all data)
//
//   EVENT_ID_USER    (user-defined event+data)
//
int Simulator::processEvent(EventData *pData) {
    int iResult = 0;

    // look at the event type
    switch (pData->m_iEventType) {
    case EVENT_ID_WRITE:
        iResult = handleWriteEvent(pData->m_sData, LBController::DUMP_MODE_NONE);
        break;
    case EVENT_ID_ENV:
        iResult = handleEnvironmentEvent(pData->m_sData);
        break;
    case EVENT_ID_ARR:
        iResult = handleEnvArrayEvent(pData->m_sData);
        break;
    case EVENT_ID_POP:
        iResult = handlePopEvent(pData->m_sData);
        break;
        
        // more debuggy stuff
    case EVENT_ID_COMM:
        iResult = handleCommEvent(pData->m_sData);
        break;

    case EVENT_ID_CHECK:
        iResult = handleCheckEvent(pData->m_sData);
        break;
        
    case EVENT_ID_DUMP:
        iResult = handleDumpEvent(pData->m_sData);
        break;
        
    case EVENT_ID_INTERPOL:
        iResult = handleInterpolEvent(pData->m_sData);
        break;
        
    case EVENT_ID_SCRAMBLE:
        iResult = handleScrambleEvent(pData->m_sData);
        break;
        
    case EVENT_ID_USER:
        iResult = handleUserEvent(pData->m_sData);
        break;
        
    default:
        LOG_WARNING2("Unknown event type [%d]\n", pData->m_iEventType);
        iResult = -1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// writeState
//  write output
//
int Simulator::writeState(const std::string sQDFOut, int iWhat, std::vector<std::pair<std::string, popwrite_flags>> &vSub, int iDumpMode) {
    int iResult = 0;

    double dStartW = omp_get_wtime();

    iResult = m_pSW->write(sQDFOut, m_iCurStep, m_fStartTime, m_sInfoString, iWhat, vSub, iDumpMode);

    double dEndW = omp_get_wtime();
    LOG_WARNING2("[%d] Writing of [%s] took %fs\n", m_iCurStep, sQDFOut, dEndW - dStartW);
    
    if (iResult != 0) {
        stdprintf("StatusWriter [%s]\n", m_pSW->getError());
        if (iResult < 0) {
            LOG_ERROR("StatusWriter [%s]\n", m_pSW->getError());
        } else {
            LOG_WARNING("StatusWriter [%s]\n", m_pSW->getError());
        }
    }

    if (iResult >= 0) {
        if (m_bZipOutput) {
            double dStartZ = omp_get_wtime();

            std::string sQDFOutgz = stdsprintf("%s.gz", sQDFOut);
	    stdprintf("gzipping %s -> %s\n", sQDFOut, sQDFOutgz);
            iResult = m_pgzCompressor->do_gzip(sQDFOut, sQDFOutgz);
            double dEndZ = omp_get_wtime();
            if (iResult == 0) {
                if (m_bDeleteOrig) {
                    stdprintf("[%d] deleted original [%s]\n", m_iCurStep, sQDFOut);
                    remove(sQDFOut.c_str());
                } else {
                    stdprintf("[%d] keeping original [%s]\n", m_iCurStep, sQDFOut);
                }
            } else {
                LOG_ERROR2("[%d] gzip failed\n", m_iCurStep);
            }
         
            LOG_STATUS2("[%d] zipping [%s] took %fs\n", m_iCurStep, sQDFOut, dEndZ - dStartZ);
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// showInputs
//  write output
//
void Simulator::showInputs() {
    printf("--------Inputs--------\n"); 
    if (m_pCG != NULL) {
        printf("Grid %d cells\n", m_pCG->m_iNumCells);
    }
    if (m_pGeo != NULL) {
        printf("Geography\n");
    }
    if (m_pCli != NULL) {
        printf("Climate\n");
    }
    if (m_pVeg != NULL) {
        printf("Vegetation\n");
    }
    if (m_pPopLooper != NULL) {
        printf("Populations: %zd\n   ", m_pPopLooper->getNumPops());
        popmap::const_iterator it_pop;
        for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
            stdprintf("%s  ", it_pop->second->getSpeciesName());
        }
        printf("\n");
    }
    printf("----------------------\n"); 
}

//----------------------------------------------------------------------------
// rangeEliminate
//   modify the time range such that iStep is not contained anymore
//   <iStep>        remove
//   [<iStep>]      remove
//   [<iStep>:<max>]  -> [<iStep+1>:<max>]
//   [<min>:<iStep>]  -> [<min>:<iStep-1>]
//
bool rangeEliminate(std::string &sRange, int iStep) {
    bool bEliminated = false;
    bool bBrackets = false;
    size_t iPos = sRange.find("[");
    if (iPos != std::string::npos) {
        bBrackets=true;
        sRange[iPos] = ' ';
    }
    iPos = sRange.find("]");
    if (iPos != std::string::npos) {
        bBrackets=true;
        sRange[iPos] = ' ';
    }

    iPos = sRange.find(":");
    if (iPos == std::string::npos) {
        int r = 0;
        if (strToNum(sRange, &r)) {
            if (r == iStep) {
                bEliminated = true;
                sRange = "";
            }
        }
        if (!bEliminated) {
            sRange = trim(sRange);
            if (bBrackets) {
                sRange = "["+sRange+"]";
            }
        }
    } else {
        std::string s1 = trim(sRange.substr(0, iPos));
        std::string s2 = trim(sRange.substr(iPos+1, std::string::npos));
        int r1 = 0;
        int r2 = 0;
        if (strToNum(s1, &r1) && strToNum(s2, &r2)) {
            bEliminated = true;
            std::string sSub = stdsprintf("%d", iStep-1);
            std::string sSup = stdsprintf("%d", iStep+1);
            if ((r1 < iStep) && (iStep < r2) ) {
                sRange = "["+s1+":"+sSub+"]+["+sSup+":"+s2+"]";
            } else if (r1 == iStep) {
                sRange = "["+sSup+":"+s2+"]";
            } else if (r2 == iStep) {
                sRange = "["+s1+":"+sSub+"]";
            } else {
                bEliminated = false;
            }
        }
    }
    return bEliminated;
}


//----------------------------------------------------------------------------
// writeResumeConfig
//  write output
//
void Simulator::writeResumeConfig(const std::string sConfigOut) {
    
    // retrieve the dump file names
    std::string sPop  = "";
    std::string sGrid = "";
    
    for (uint i = 0; i < m_vDumpNames.size(); i++) {
        if (m_vDumpNames[i].find("pop") != std::string::npos) {
            if (sPop.size() > 0) {
                sPop = sPop + ",";
            }
            sPop  = sPop + m_vDumpNames[i];
        } else if (m_vDumpNames[i].find("env") != std::string::npos) {
            sGrid = m_vDumpNames[i];
        }
    }
    stringvec vsOptions;
    m_pPR->collectOptions(vsOptions);
    for (unsigned int j = 0; j < vsOptions.size(); j++) {
        std::string sCur = trim(vsOptions[j]);
        if (sCur.find("--grid=") == 0) {
            vsOptions[j] =  "--grid="+sGrid;
        } else if (sCur.find("--pops=") == 0) {
            vsOptions[j] = "--pops="+sPop;
        } else if (sCur.find("--log-file=") == 0) {
            size_t iPos = sCur.find("=");
            std::string sLog = sCur.substr(iPos+1, std::string::npos);
            if (!strReplace(sLog, "_o.log", "_r.log")) {
                if (!strReplace(sLog, ".log", "_r.log")) {
                    sLog = sLog+"_r.log";
                }
            }
            vsOptions[j] = "--log-file="+sLog;
        } else if (sCur.find("--data-dirs=") == 0) {
            size_t iPos = sCur.find("=");
            std::string sDDirs = sCur.substr(iPos+1, std::string::npos);
            int i0 = 0;
            int l = sDDirs.size()-1;
            if (sDDirs[l] == '\'') {
                l--;
            }
            if (sDDirs[i0] == '\'') {
                i0++;
                l--;
            }
            vsOptions[j] = "--data-dirs="+sDDirs.substr(i0, l);

        } else if (sCur.find("--events=") == 0) {
            std::string sOut = "--events=";
            stringvec vSingleOptions;
            size_t iPos =  vsOptions[j].find("=");
            iPos++;
            splitString(vsOptions[j].substr(iPos, std::string::npos), vSingleOptions, ","); 
            bool bFollower = false;
            for (unsigned int i = 0; i < vSingleOptions.size(); i++) {
                std::string sNew = "";
                if (vSingleOptions[i].find("dump") == std::string::npos) {
                    sNew = vSingleOptions[i];
                } else {
                    stringvec vSingleEvents;
                    size_t iPos2 = vSingleOptions[i].find("@");
                    if (iPos2 != std::string::npos) {
                        iPos2++;
                        std::string sFirst = vSingleOptions[i].substr(0, iPos2);
                        std::string sTimes = "";
                        
                        splitString(vSingleOptions[i].substr(iPos2, std::string::npos), vSingleEvents, "+"); 
                        bool bFollower2 = false;
                        for (unsigned int k = 0; k < vSingleEvents.size(); k++) {
                            if (!rangeEliminate(vSingleEvents[k], m_iCurStep)) {
                                if (bFollower2) {
                                    sTimes = sTimes+"+";
                                } else {
                                    bFollower2 = true;
                                }
                                sTimes = sTimes + vSingleEvents[k];
                            } else {
                                //stdprintf("%s is removed\n", vSingleEvents[k]);
                            }
                        }
                        if (sTimes != "") {
                            sNew = sFirst+sTimes;
                        }    
                    }
                }
                if (sNew != "") {
                    if (bFollower) {
                        sOut = sOut + ",";
                    } else {
                        bFollower = true;
                    }
                    sOut = sOut + sNew;
                }
            }
           
            vsOptions[j] = sOut;
        } else {
            vsOptions[j] = trim( vsOptions[j]);
        }
        
    }
    vsOptions.push_back("--resume");
    FILE *fOut = fopen(sConfigOut.c_str(), "wt");
    for (uint i = 0; i< vsOptions.size(); ++i) {
        stdfprintf(fOut, "%s\n", vsOptions[i]);
    }
    fclose(fOut);
}


