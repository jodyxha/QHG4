/*============================================================================
| EventData
| 
|  Represents QHG events.
|  Consists of an event id and a std::string containing data.
|  The EDCompare struct is used to order compare EventData objects.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstdio>
#include <cstring>
#include <string>

#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "EventData.h"

//-----------------------------------------------------------------------------
// constructor
//
EventData::EventData(int iEventType,std::string sData) :
    m_iEventType(iEventType),
    m_sData(trim(sData)) {

}


//-----------------------------------------------------------------------------
// parseDef
//
EventData *EventData::parseDef(std::string sDef) {
    EventData *pED = NULL;
    int iEventType = -1;
    stringvec vParts;
    uint iNum = splitString(sDef, vParts, ";:,", false);
    if (iNum == 2) {
        if (strToNum(vParts[0], &iEventType)) {
            pED = new EventData(iEventType, vParts[1]);
        }    
    } else {
        // bad format
    }
    return pED;
}


//-----------------------------------------------------------------------------
// equals
//
bool EventData::equals(EventData *pED) {
    bool bEqual = false;
    if (m_iEventType == pED->m_iEventType) {
        if (m_sData.compare(pED->m_sData) == 0) {
            bEqual = true;
        }
    }
    return bEqual;
}


//-----------------------------------------------------------------------------
// toString
//
void EventData::toString(std::string &sCur) const {
    std::string sOut = stdsprintf("(%d)|%s", m_iEventType, m_sData);
    sCur += sOut;
}
