#include <cstdio>
#include <cstring>

//#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "LineReader.h"
#include "MessLoggerT.h"
#include "EventManager.h"
#include "EventConsts.h"
#include "EventData.h"
#include "PopBase.h"
#include "PopLooper.h"
#include "ParamProvider2.h"
#include "QDFUtils.h"
#include "EventChecker.h"

stringvec vEventTypes = {
    EVENT_TYPE_WRITE,
    EVENT_TYPE_ENV,
    EVENT_TYPE_POP,
    EVENT_TYPE_ARR,
    EVENT_TYPE_COMM,
    EVENT_TYPE_DUMP,
    EVENT_TYPE_INTERPOL,
    EVENT_TYPE_SCRAMBLE,
    EVENT_TYPE_CHECK,
    EVENT_TYPE_USER,
};

stringvec vWriteParams = {
    EVENT_PARAM_WRITE_GRID,
    EVENT_PARAM_WRITE_GEO,
    EVENT_PARAM_WRITE_CLIMATE,
    EVENT_PARAM_WRITE_VEG,
    EVENT_PARAM_WRITE_STATS,
    EVENT_PARAM_WRITE_NAV,
    EVENT_PARAM_WRITE_OCC,
    EVENT_PARAM_WRITE_POP,
};
/*
const char *asEventTypes[] = {
    EVENT_TYPE_WRITE,
    EVENT_TYPE_ENV,
    EVENT_TYPE_POP,
    EVENT_TYPE_ARR,
    EVENT_TYPE_COMM,
    EVENT_TYPE_DUMP,
    EVENT_TYPE_INTERPOL,
    EVENT_TYPE_SCRAMBLE,
    EVENT_TYPE_CHECK,
    EVENT_TYPE_USER,
};

const char *asWriteParams[] = {
    EVENT_PARAM_WRITE_GRID,
    EVENT_PARAM_WRITE_GEO,
    EVENT_PARAM_WRITE_CLIMATE,
    EVENT_PARAM_WRITE_VEG,
    EVENT_PARAM_WRITE_STATS,
    EVENT_PARAM_WRITE_NAV,
    EVENT_PARAM_WRITE_OCC,
};
*/

#define logstdprintf stdprintf

//-----------------------------------------------------------------------------
// createInstance
//
EventChecker *EventChecker::createInstance(char *pEventDescription, PopLooper *pPopLooper, std::vector<std::string> &vDataDirs, int iNumIters, float fStartTime, bool bUpdateEventList) {
    EventChecker *pEC = new EventChecker(pPopLooper, vDataDirs, iNumIters, fStartTime, bUpdateEventList);
    int iResult = pEC->init(pEventDescription);
    if (iResult  != 0) {
        delete pEC;
        pEC = NULL;
    }
    return pEC;
}


//-----------------------------------------------------------------------------
// checkEvents
//
int EventChecker::checkEvents(char *pEventDescription, PopLooper *pPopLooper, std::vector<std::string> &vDataDirs,int iNumIters, float fStartTime, bool bUpdateEventList) {
    EventChecker *pEC = new EventChecker(pPopLooper, vDataDirs, iNumIters, fStartTime, bUpdateEventList);
    int iResult = pEC->init(pEventDescription);
    delete pEC;
    return iResult;
}


//-----------------------------------------------------------------------------
// checkEvents
//
EventManager *EventChecker::createEventManager(char *pEventDescription, PopLooper *pPopLooper, std::vector<std::string> &vDataDirs,int iNumIters, float fStartTime, bool bUpdateEventList) {
    EventManager *pEM = NULL;
    EventChecker *pEC = new EventChecker(pPopLooper, vDataDirs, iNumIters, fStartTime, bUpdateEventList);
    int iResult = pEC->init(pEventDescription);
    if (iResult == 0) {
        pEM = pEC->getEM();
    } 
    delete pEC;
    return pEM;
}


//-----------------------------------------------------------------------------
// constructor
//
EventChecker::EventChecker(PopLooper *pPopLooper, std::vector<std::string> &vDataDirs, int iNumIters, float fStartTime, bool bUpdateEventList)
    : m_pPopLooper(pPopLooper),
      m_vDataDirs(vDataDirs),
      m_pEM(new EventManager(fStartTime)),
      m_iNumIters(iNumIters),
      m_fStartTime(fStartTime),
      m_bUpdateEventList(bUpdateEventList) {
    
    m_vEvents.clear();
      
}


//-----------------------------------------------------------------------------
// destructor
//
EventChecker::~EventChecker() {
    std::vector<eventstrings*>::iterator it;
    for (it = m_vEvents.begin(); it != m_vEvents.end(); ++it) {
        delete *it;
    }

    // so not delete m_pEM!
}


//-----------------------------------------------------------------------------
// init
//
int EventChecker::init(char *pEventDescription) {
    int iResult = 0;
   
    if (iResult == 0) {
        iResult = readEvents(pEventDescription);
    }
    
    if (iResult == 0) {
        iResult = checkEvents();
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readEvents
//
int EventChecker::readEvents(const std::string sEventDescription) {
    int iResult = 0;
    m_vEvents.clear();

    stringvec vParts;
    uint iNum = splitString(sEventDescription, vParts, ",", false); 
    if (iNum > 0) {
        for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
            stdprintf("event: [%s]\n", vParts[i]);
            iResult = readEventFromString(vParts[i]);
        }

    } else {
        stdprintf("[readEvents] Empty event description\n");
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readEventFromString
//
int EventChecker::readEventFromString(const std::string sEventString) {
    int iResult = 0;

    std::string sEvent;
    std::string sEventParams;
    std::string sTriggerTimes;
    size_t iEvParStart = sEventString.find('|');
    size_t iEvTrigStart = sEventString.find('@');
    if ((iEvParStart != std::string::npos) && (iEvTrigStart != std::string::npos)) {
        sEvent        = sEventString.substr(0, iEvParStart);
        sEventParams  = sEventString.substr(iEvParStart+1, iEvTrigStart-iEvParStart-1);
        sTriggerTimes = sEventString.substr(iEvTrigStart+1);

        if (sEvent == EVENT_TYPE_FILE) {
            iResult = readEventsFromFile(sEventParams);
        } else {
            m_vEvents.push_back(new eventstrings(sEvent, sEventParams, sTriggerTimes));
           
        }
    } else {
        if (iEvParStart != std::string::npos) {
            stdprintf("[readEventFromString] Bad event description: expected '|' in [%s]\n", sEventString);
            iResult = -1;
        }
        
        if (iEvTrigStart != std::string::npos) { 
            stdprintf("[readEventFromString] Bad event description: expected '@' after [%s]\n", sEventString);
            iResult = -1;
        }
    }


    
    return iResult;
}


//-----------------------------------------------------------------------------
// readEventsFromFile
//
int EventChecker::readEventsFromFile(const std::string sEventFile) {
   int iResult = 0;

   LineReader *pLR = LineReader_std::createInstance(sEventFile, "rt");
    if (pLR != NULL) {

        char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
            iResult = readEventFromString(trim(pLine));
            
            pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        }

        delete pLR;
    } else {
        stdprintf("[readEventsFromFile] Couldn't open file [%s]\n", sEventFile);
        iResult = -1;
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// checkEvents
//
int EventChecker::checkEvents() {
    int iResult = 0;
    std::vector<eventstrings*>::const_iterator it;
    for (it = m_vEvents.begin(); (iResult == 0) && (it != m_vEvents.end()); ++it) {
        const std::string sEventType   = (*it)->sEventType;
        const std::string sEventParams = (*it)->sEventParams;
        const std::string sEventTimes  = (*it)->sEventTimes;
        int iID = checkEvent(sEventType, sEventParams);
        if (iID != EVENT_ID_NONE) {
            iResult = addEventToManager(iID, sEventParams, sEventTimes);
        } else {
            stdprintf("[checkEvents] unknown event type [%s,%s]\n", sEventParams, sEventTimes);
            iResult = -1;
        }
    }
    if (iResult != 0) {
        delete m_pEM;
        m_pEM = NULL;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// checkEvent
//
int EventChecker::checkEvent(const std::string sEventType, const std::string sEventParams) {
    int iResult = 0;
    int iEventType = EVENT_ID_NONE;
    if (sEventType == EVENT_TYPE_WRITE) {
        iEventType = EVENT_ID_WRITE;
        iResult = checkWriteParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_ENV) {
        iEventType = EVENT_ID_ENV;
        iResult = checkEnvParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_POP) {
        iEventType = EVENT_ID_POP;
        iResult = checkPopParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_ARR) {
        iEventType = EVENT_ID_ARR;
        iResult = checkArrParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_COMM) {
        iEventType = EVENT_ID_COMM;
        iResult = checkCommParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_DUMP) {
        iEventType = EVENT_ID_DUMP;
        iResult = checkDumpParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_INTERPOL) {
        iEventType = EVENT_ID_INTERPOL;
        iResult = checkInterpolParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_SCRAMBLE) {
        iEventType = EVENT_ID_SCRAMBLE;
        iResult = checkScrambleParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_CHECK) {
        iEventType = EVENT_ID_CHECK;
        iResult = checkCheckParams(sEventParams);
    } else if (sEventType == EVENT_TYPE_USER) {
        iEventType = EVENT_ID_USER;
        iResult = checkUserParams(sEventParams);
    } else {
        stdprintf("[checkEvents] Unknown event type: [%s]\n", sEventType);
        stdprintf("[checkEvents] Known event types:\n");
        for (uint i = 0; i < vEventTypes.size(); i++) {
            stdprintf("[checkEvents]   \"%s\"\n", vEventTypes[i]);
        }
        iEventType = EVENT_ID_NONE;
    }
     
    if (iResult != 0) {
        iEventType = EVENT_ID_NONE;
    }
    
    return iEventType;
}


//-----------------------------------------------------------------------------
// checkWriteParams
//    event-params ::= <type> ["+" <type>]*
//    type         ::= "grid" | "geo"   | "climate " |
//                     "veg"  | "stats" |  "nav" | "pop:"<speciesname>[<filter>]
//
int EventChecker::checkWriteParams(const std::string sParams) {
    int iResult = 0;
    
    stringvec vTerms;
    uint iNum = splitString(sParams, vTerms, "+", false);
    if (iNum > 0) {
        for (uint i = 0; (iResult == 0) && (i < iNum); i++) {
            size_t iC = vTerms[i].find(':');
            std::string sWPar = vTerms[i].substr(0, iC);
            std::string sPop = vTerms[i].substr(iC+1);
            stringvec::const_iterator it = std::find(vWriteParams.begin(), vWriteParams.end(), sWPar);
            if (it != vWriteParams.end()) {
                if (sWPar == "pop") {
                    if (iC != std::string::npos) {
                        //check popname
                        if (m_pPopLooper != NULL) {
                            // ignore trailing  '#', '%' or '~'
                            size_t iTrail = sPop.find_last_of("#%~");
                            sPop = sPop.substr(0, iTrail);
                            PopBase * pPB = m_pPopLooper->getPopByName(sPop);
                            if (pPB == NULL) {
                                stdprintf("[checkWriteParams] No species found with name [%s]\n", sPop);
                                iResult = -1;
                            } else {
                                iResult = 0;
                            }
                        } else {
                            stdprintf("[checkWriteParams] No PopLooper: didn't check pop [%s]\n", sPop);
                            iResult = 0;                        
                        }  
   
                    } else {
                        // expected a':'
                        stdprintf("[checkWriteParams] Expected ':' after 'pop' [%s]\n", vTerms[i]);
                        iResult = -1;                        
                    }
                } else {
                    // no ':' ok if it isn't a pop
                    if (iC == std::string::npos) {
                        iResult = 0;
                    } else {
                        iResult = -1;
                    }
                }
            } else {
                stdprintf("[checkWriteParams] Unknown write param: [%s]\n", vTerms[i]);
                stdprintf("[checkWriteParams] Known write prams:\n");
                for (uint i = 0; i < vWriteParams.size(); i++) {
                   stdprintf("[checkWriteParams]   \"%s\"\n", vWriteParams[i]);
                }
                iResult = -1;
            }
        }


    } else {
        stdprintf("[checkWriteParams] No Parameters on param string: [%s]\n", sParams);
        iResult = -1;
    }
    return iResult;

    return iResult;
}


//-----------------------------------------------------------------------------
// checkEnvParams
//    event-params ::= <type> ["+" <type>]* ":" <qdf_file>
//    type         ::= "geo"  | "climate " | "veg" | "nav" 
//
int EventChecker::checkEnvParams(const std::string sParams) {
    int iResult = 0;
    
    size_t iC = sParams.find(':');
    if (iC != std::string::npos) {
        std::string sQDF = sParams.substr(iC+1);
        if (!sQDF.empty()) {
            stringvec vTypes;
            uint iNum = splitString(sParams.substr(0, iC), vTypes, "+", false);
            if (iNum > 0) {
                for (uint i = 0; (iResult == 0) && (i < iNum); i++) {
                    std::string sGroup = "";
                    if (vTypes[i] == "geo") {
                        sGroup = GEOGROUP_NAME;
                    } else if (vTypes[i] == "climate") {
                        sGroup = CLIGROUP_NAME;
                    } else if (vTypes[i] == "veg") {
                        sGroup = VEGGROUP_NAME;
                    } else if (vTypes[i] == "nav") {
                        sGroup = NAVGROUP_NAME;
                    } else {
                        stdprintf("[checkEnvParams] invalid env type [%s]\n", vTypes[i]);
                        iResult = -1;
                    }
                    if (!sGroup.empty()) {

                        std::string sRealName;
                        if (checkFileExists(sQDF, sRealName)) {
                            if (checkQDFContainsGroup(sRealName, sGroup)) {
                                iResult = 0;
                            } else {
                                stdprintf("[checkEnvParams] group [%s] not found in [%s]\n", sGroup, sQDF);
                                iResult = -1;
                            }
                        } else {
                            stdprintf("[checkEnvParams] File does not exist: [%s]\n", sQDF);
                            iResult = -1;
                        }
                    } else {
                    }       

                }
            } else {
                // expected types before ':'
                stdprintf("[checkEnvParams] Expected types before ':': [%s]\n", sParams);
                iResult = -1;
            }
        } else {
            stdprintf("[checkEnvParams] Expected file name after ':': [%s]\n", sParams);
            iResult = -1;
        }
    } else {
        stdprintf("[checkEnvParams] Expected a':': [%s]\n", sParams);
        iResult = -1;
    }

    return iResult;

}


//-----------------------------------------------------------------------------
// checkArrParams
//    event-params ::= <type> ":" <arrayname> ":" <qdf_file>
//    type         ::= "geo"  | "climate " | "veg"
//
int EventChecker::checkArrParams(const std::string sParams) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sParams, vParts, ":", true);
    if (iNum == 3) {
        const std::string &sType     = vParts[0];
        const std::string &sArrName  = vParts[1];
        const std::string &sQDFFile  = vParts[2];

        std::string sGroup;

        if (sType == "geo") {
            sGroup = GEOGROUP_NAME;
        } else if (sType == "climate") {
            sGroup = CLIGROUP_NAME;
        } else if (sType == "veg") {
            sGroup = VEGGROUP_NAME;
        } else {
            stdprintf("[checkArrParams] Bad type: [%s]\n", sType);
            iResult = -1;
        }
        
        if (iResult == 0) {
            std::string sRealName;
            if (checkFileExists(sQDFFile, sRealName)) {
                if (checkQDFContainsArr(sRealName, sGroup, sArrName)) {
                    iResult = 0;
                } else {
                    stdprintf("[checkArrParams] The array [%s] does not exist in group [%s]\n", sArrName, sQDFFile);
                    iResult = -1;
                }
            } else {
                stdprintf("[checkArrParams] The file [%s] does not exist\n", sQDFFile);
                iResult = -1;
            }
        }
   } else {
        stdprintf("[checkArrParams] expected string of the form  \"<type>:<arrayname>:<qdf_file>\": [%s]\n", sParams);
        iResult = -1;
    }
    iResult = 0;
    return iResult;
}


//-----------------------------------------------------------------------------
// checkPopParams
//    event-params ::= <speciesname> ["+" <speciesname>]* ":" <qdf_file>
//
int EventChecker::checkPopParams(const std::string sParams) {
    int iResult = 0;
    
    size_t iC = sParams.find(":");
    if (iC != std::string::npos) {
        std::string sPopFile = sParams.substr(iC+1);
        if (!sPopFile.empty()) {
            std::string sRealName;
            if (checkFileExists(sPopFile, sRealName)) {
                std::string sPops = sParams.substr(0, iC);
                stringvec vParts;
                uint iNum = splitString(sPops, vParts, "+", true);
                for (uint i = 0; (iResult == 0) && (i < iNum); i++) {
                    if (!vParts[i].empty()) {
                        if (checkQDFContainsGroup(sRealName, vParts[i])) {
                            iResult = 0;
                        } else {
                            stdprintf("[checkPoipParams] The file [%s] does not contain the population [%s]\n", sPopFile, vParts[i]);
                            iResult = -1;
                        }
                    } else {
                        stdprintf("[checkPopParams] Empty pop: [%s]\n", vParts[i]);
                        iResult = -1;
                    }
                }
            } else {
                // check for xml:dat
                std::string sPops = sParams.substr(0, iC);
                iC = sPopFile.find(":");
                if (iC != std::string::npos) {
                    std::string sXML = sPopFile.substr(0, iC);
                    std::string sDat = sPopFile.substr(iC+1);
                    std::string sRealXMLName;
                    std::string sRealDATName;
                    if (checkFileExists(sXML, sRealXMLName)) {
                        if (checkFileExists(sDat, sRealDATName)) {
                            iResult = 0;
                        } else {
                            stdprintf("[checkPopParams] The file [%s] does not exist\n", sDat);
                            iResult = -1;
                        }
                    } else {
                        stdprintf("[checkPopParams] The file [%s] does not exist\n", sXML);
                        iResult = -1;
                    }

                    if (iResult == 0) {
                        ParamProvider2 *pPP = ParamProvider2::createInstance(sXML);
                        if (pPP != NULL) {
                            std::string sSpeciesName = pPP->getSpeciesName();
                            if (sPops == sSpeciesName) {
                                iResult = 0;
                            } else {
                                stdprintf("[checkPopParams] The species name in the event [%s] does not match the species name in the XML file  [%s]\n", sPops, sSpeciesName);
                                iResult = -1;
                            }
                            delete pPP;
                        } else {
                            stdprintf("[checkPopParams] The file [%s] is not a valid xml file\n", sXML);
                            iResult = -1;
                        }
                    }
                } else {
                    stdprintf("[checkPopParams] The file [%s] does not exist\n", sPopFile);
                    iResult = -1;
                }
            }

        } else {
            stdprintf("[checkPopParams] Expected file name after ':': [%s]\n", sParams);
            iResult = -1;
        }
    } else {
        stdprintf("[checkPopParams] Expected a ':': [%s]\n", sParams);
         iResult = -1;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// checkDumpParams
//    event-params   ::= "flat" | "smart" | "free"
//
int EventChecker::checkDumpParams(const std::string sParams) {
    int iResult = 0;
    if ((sParams == "flat") ||
        (sParams == "smart") ||
        (sParams == "free")) {
        iResult = 0;
    } else {
        stdprintf("[checkDumpParams] Unknown dump style [%s]\n", sParams);
        iResult = -1;
    }
    return iResult;

}


//-----------------------------------------------------------------------------
// checkInterpolParams
//    event-params   ::= "file":<filename> | "cmd":<command>
//    command        ::= "stop" | "start"
//
int EventChecker::checkInterpolParams(const std::string sParams) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sParams, vParts, ":", true);
    if (iNum == 2) {
        if (vParts[0] == "file") {
            std::string  sRealName;
            if (checkFileExists(vParts[1], sRealName)) {
                hid_t hFile = qdf_openFile(sRealName);
                if (hFile != H5P_DEFAULT) {
                    std::string sAttr;
                    iResult = qdf_extractSAttribute(hFile, "Targets", sAttr); 
                    if (iResult == 0) {
                        stringvec vSubs;
                        uint iNum = splitString(sAttr, vSubs, "#");
                        if (iNum > 0) {
                            for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
                                if ((vSubs[i] == "Vegetation/BaseNPP") ||
                                    (vSubs[i] == "Geography/Altitude")) {
                                    iResult = 0;
                                } else {
                                    stdprintf("[checkInterpolParams] Unsupported interpolation array: [%s]\n", vSubs[i]);
                                    iResult = -1;
                                }                                        
                                    
                            }
                        
                        } else {
                            stdprintf("[checkInterpolParams] The file [%s] seems not to be a qdf interpolation file (empty \"Targets\" attribute)\n", vParts[1]);
                            iResult = -1;
                        }
                    } else {
                        stdprintf("[checkInterpolParams] The file [%s] seems not to be a qdf interpolation file (no attribute \"Targets\")\n", vParts[1]);
                        iResult = -1;
                    }
                        
                } else {
                    stdprintf("[checkInterpolParams] couldn't open [%s] as QDF file\n", vParts[1]);
                    iResult = -1;
                }
                        
            } else {
                stdprintf("[checkInterpolParams] file [%s] does not exist\n", vParts[1]);
                iResult = -1;
            }

        } else if (vParts[0] == "cmd"){
            if ((vParts[1] == "start") || (vParts[1] == "stop")) {
                iResult = 0;
            } else {
                stdprintf("[checkInterpolParams] Unknown interpolation command [%s]\n", vParts[1]);
                iResult = -1;
            }
        } else {
            stdprintf("[checkInterpolParams] Unknown interpolation parameter [%s]\n", vParts[0]);
            iResult = -1;
        }
    } else {
        stdprintf("[checkInterpolParams] Expected a ':' [%s]\n", sParams);
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// checkScrambleParams
//    event-params   ::= <what> ["+" <what>]*
//    what           ::= "all"
//
int EventChecker::checkScrambleParams(const std::string sParams) {
    int iResult = 0;

    stringvec vWhats;
    uint iNum = splitString(sParams, vWhats, "+", true);
    if (iNum > 0) {
        for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
            if ((vWhats[i] == EVENT_PARAM_SCRAMBLE_CONN) ||
                (vWhats[i] == EVENT_PARAM_SCRAMBLE_ALL)) {
                iResult = 0;
            } else {
                iResult = -1;
            }
        } 
    } else {
        stdprintf("[checkScrambleParams] Empty params\n");
        iResult = -1;
    }
    return iResult;
}




//-----------------------------------------------------------------------------
// checkCheckParams
//    event-params   ::= <what> ["+" <what>]*
//    what           ::= "lists"
//
int EventChecker::checkCheckParams(const std::string sParams) {
    int iResult = 0;

    stringvec vWhats;
    uint iNum = splitString(sParams, vWhats, "+", true);
    if (iNum > 0) {
        for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
            if (vWhats[i] != "lists") {
                iResult = -1;
            }
        } 
    } else {
        stdprintf("[checkCheckParams] Empty params\n");
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// checkCommParams
//    event-params   ::= <cmd-file>  | <command>
//    command        ::= <iter_cmd>       | <del_action_cmd> | 
//                       <dis_action_cmd> | <ena_action_cmd> | 
//                       <mod_pop_cmd>    | <event>
//    iter_cmd       ::= "SET ITERS:"<num_iters>
//    del_action_cmd ::= "REMOVE ACTION:"<population>:<action_name>
//    ena_action_cmd ::= "DISABLE ACTION:"<population>:<action_name>
//    dus_action_cmd ::= "ENEABLE ACTION:"<population>:<action_name>
//    mod_pop_cmd    ::= "MOD POP:"<population>:<param_name>:<value>
//    event          : any event description; see definition above
//
int EventChecker::checkCommParams(const std::string sParams) {
    int iResult = 0;
    
    std::string sRealName;
    if (checkFileExists(sParams, sRealName)) {
        iResult = checkCommFile(sRealName);
    } else {
        iResult = checkCommLine(sParams);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// checkCommFile
//
int EventChecker::checkCommFile(const std::string sEventFile) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(sEventFile, "rt");
    if (pLR != NULL) {

        char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
            iResult = checkCommLine(trim(pLine));
            
            pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        }

        delete pLR;
    } else {
        stdprintf("[checkCommFile] Couldn't open file [%s]\n", sEventFile);
        iResult = -1;
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// checkCommLine
// Currently supported:
//   SET ITERS:<numiters>
//   REMOVE ACTION:<population>:<actionname>
//   MOD POP:<population>:<paramname>:<value>
//   (normal event)
//
int EventChecker::checkCommLine(const std::string sLine) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sLine, vParts, ":", false);
    
    if (iNum > 0) {
        if (vParts[0] == CMD_SET_ITERS) {
            if (iNum == 2) {
                int iVal = 0;
                if (strToNum(vParts[1], &iVal)) {
                    iResult = 0;
                } else {
                    stdprintf("[checkCommLine] Expected a number, not [%s]\n", vParts[1]);
                    iResult = -1;
                }
            } else {
                //SET_ITER need 1 param
                iResult = -1;
            }
        } else if ((vParts[0] == CMD_REMOVE_ACTION) |
                   (vParts[0] == CMD_DISABLE_ACTION) |
                   (vParts[0] == CMD_ENABLE_ACTION)){
            if (iNum == 3) {
                std::string &sPopName    = vParts[1];
                std::string &sActionName = vParts[2];
                if (m_pPopLooper != NULL) {
                    PopBase *pPop = m_pPopLooper->getPopByName(sPopName);
                    if (pPop != NULL) {
                        if (pPop->hasAction(sActionName)) {
                            iResult = 0;
                        } else {
                            stdprintf("[checkCommLine] Population [%s] has no action [%s]\n", sPopName, sActionName);
                            iResult = -1;
                        }
                    } else {
                        stdprintf("[checkCommLine] Couldn't find population [%s]\n", sPopName);
                        iResult = -1;
                    }
                } else {
                    stdprintf("[checkCommLine] (Warning) No PopLooper: didn't check pop [%s] and action [%s]\n", sPopName, sActionName);
                    iResult = 0;
                }
            } else {
                stdprintf("[checkCommLine] REMOVE_ACTION needs 2 parameters (pop-name, action-name): [%s]\n", sLine);
                iResult = -1;
            }
        } else if (vParts[0] == CMD_MOD_POP) {
            if (iNum == 4) {
                std::string &sPopName    = vParts[1];
                std::string &sParamName  = vParts[2];
                std::string &sParamValue = vParts[3];
                
                double dValue = 0;
                if (strToNum(sParamValue, &dValue)) {
                    if (m_pPopLooper != NULL) {
                        PopBase *pPop = m_pPopLooper->getPopByName(sPopName);
                        if (pPop != NULL) {
                            if (pPop->hasParam(sParamName)) {
                                iResult = 0;
                            } else {
                                stdprintf("[checkCommLine] No Population [%s] has no parameter [%s]\n", sPopName, sParamName);
                                iResult = -1;
                            }
                        } else {
                            stdprintf("[checkCommLine] No Population found with name [%s]\n", sPopName);
                            iResult = -1;
                        }
                    } else {
                        stdprintf("[checkCommLine] (Warning)No PopLooper: didn't check population [%s] and param [%s]\n", sPopName, sParamName);
                        iResult = 0;
                    }
                } else {
                    stdprintf("[checkCommLine] Expected numerical value: [%s]\n", sParamValue);
                    iResult = -1;
                }
            } else {
                stdprintf("[checkCommLine] MOD_POP needs 3 parameters (pop-name, param-name,param-value): [%s]\n", sLine);
                iResult = -1;
                iResult = -1;
            }

        } else {
            // try normal event
            size_t iB = sLine.find("|");
            if (iB != std::string::npos) {
                std::string sCmd = sLine.substr(0, iB);
                std::string sPar = sLine.substr(iB+1);
                iResult = checkEvent(sCmd, sPar);
            } else {
                stdprintf("[checkCommLine] unknown command [%s]\n", sLine);
                iResult = -1;
            }
        }
    } else {
        stdprintf("[checkCommLine] empty param list [%s]\n", sLine);
        iResult = -1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// checkUserParams
//    event-params   ::= <eventid>:<stringdata>
//    eventid        : an event id in [1000, 9999]
//    stringdata     : a string
//
int EventChecker::checkUserParams(const std::string sLine) {
    int iResult = 0;
    
    size_t iC = sLine.find(":");
    if (iC != std::string::npos) {
        std::string sID   = sLine.substr(0, iC-1);
        std::string sData = sLine.substr(iC);
        int iID = -1;
        if (strToNum(sID, &iID)) {
            if ((iID >= EVENT_ID_USR_MIN) && (iID <= EVENT_ID_USR_MAX)) {
                iResult =0;
            } else {
                stdprintf("[checkUserParams] bad user event ID [%d] (shoud be in [%d, %d])\n", iID, EVENT_ID_USR_MIN, EVENT_ID_USR_MAX);
                iResult = -1;
            }
        } else {
            stdprintf("[checkUserParams] not a number: [%s] in [%s]\n", sID, sLine);
            iResult = -1;
        }
        
    } else {
        stdprintf("[checkUserParams] missing ':' in [%s]\n", sLine);
        iResult = -1;
    }

    return iResult;
}
   

//----------------------------------------------------------------------------
// addEventToManager
//
int EventChecker::addEventToManager(int iEventType, std::string sEventParams, std::string sEventTimes) {
    int iResult = 0;
   
    EventData *pED = new EventData(iEventType, sEventParams);
    if (pED != NULL) {
        std::string sTimes = sEventTimes;
                
        if (iEventType == EVENT_ID_WRITE) {
            // force a write at the last time
            sEventTimes = stdsprintf("%s+[%d]+[%d]", sEventTimes, 0, m_iNumIters);
        }

        //printf("Setting triggers [%s]\n", pNewInts);
        Triggers *pT = Triggers::createTriggers(sEventTimes, m_fStartTime, m_fStartTime+m_iNumIters);
        if (pT != NULL) {
            m_pEM->loadEventItem(pED, pT, m_bUpdateEventList);
            
        } else {
            stdprintf("[addEventToManager] Bad trigger definition: [%s]\n", sEventTimes);
            iResult = -1;
        }
        
    } else {
        stdprintf("[addEventToManager] Bad Event Data [%s]\n", sEventParams);
        iResult = -1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// checkFileExists
//  if pFile exists in current directory, 
//    true is returned 
//  otherwise, if data dirs are given, and pFile exists in a data dir,
//    true is returned 
//  otherwise,
//    false is returned  
//
bool EventChecker::checkFileExists(const std::string sFile, std::string &sExists) {
    //@@    struct stat statbuf;
    bool bSearching = true;
    
    if (!fileExists(sFile))  {

        if (!m_vDataDirs.empty()) {
                       
            for (uint i = 0; bSearching && (i < m_vDataDirs.size()); i++) {
                std::string sTest = stdsprintf("%s%s%s", m_vDataDirs[i], (m_vDataDirs[i].back() != '/')?"/":"", sFile);
                if (fileExists(sTest)) {
                    sExists = sTest;
                    bSearching = false;
                }
            }
        } else {
            // pFile doesn't exist, and we don't have a datadir
            // so that's it
        } 
    } else {
        sExists = sFile;
        bSearching = false;
    }
    if (bSearching) {
        sExists = "";
        stdprintf("[exists] [%s] not found\n", sFile);
    }
    
    return !bSearching;
}


//----------------------------------------------------------------------------
// checkQDFContainsGroup
//  if pFile exists in current directory, 
 //    true is returned 
//  otherwise, if data dirs are given, and pFile exists in a data dir,
//    true is returned 
//  otherwise,
//    false is returned  
//
bool EventChecker::checkQDFContainsGroup(const std::string sFile, const std::string sGroupName) {
    bool bResult = false;
    hid_t hFile =  qdf_openFile(sFile);
    if (hFile != H5P_DEFAULT) {
        if (!sGroupName.empty()) {
            if (qdf_link_exists(hFile, sGroupName)) {
                bResult = true;
            } else {
                std::string sPath = stdsprintf("%s/%s", POPGROUP_NAME, sGroupName); 
                if (qdf_link_exists(hFile, sPath)) {
                    bResult = true;
                } else {
                    stdprintf("[checkQDFContains] group [%s] not found in [%s]\n", sGroupName, sFile);
                    bResult = false;
                }
            }
        } else {
            bResult = true;
        }
        qdf_closeFile(hFile);
    } else {
        stdprintf("[checkQDFContains] couldn't open [%s] as QDF file\n", sFile);
        bResult = false;
    }
    return bResult;
}


//----------------------------------------------------------------------------
// checkQDFContainsArr
//
bool EventChecker::checkQDFContainsArr(const std::string sFile, const std::string sGroupName, const std::string sArrName) {
    bool bResult = false;
    hid_t hFile =  qdf_openFile(sFile);
    if (hFile != H5P_DEFAULT) {
        hid_t hGroup = qdf_openGroup(hFile, sGroupName);
        if (hGroup != H5P_DEFAULT) {
    
            if (qdf_link_exists(hGroup, sArrName)) {
                bResult = true;
            } else {
                stdprintf("[checkQDFContains] array [%s] not found in group [%s] of [%s]\n", sArrName, sGroupName, sFile);
                bResult = false;
            }
            qdf_closeGroup(hGroup);
        } else {
            stdprintf("[checkQDFContains] couldn't open group [%s] in [%s]\n", sGroupName, sFile);
            bResult = false;
        }
        qdf_closeFile(hFile);
    } else {
        stdprintf("[checkQDFContains] couldn't open [%s] as QDF file\n", sFile);
        bResult = false;
    }
    return bResult;
}
