/*============================================================================
| EventConsts
| 
|  Various constants for events and their parameters
|  
|  Author: Jody Weissmann
\===========================================================================*/ 
#include <string>
#include "EventConsts.h"

bool hasInputFile(int iEventType) {
    return ((iEventType ==  EVENT_ID_ENV) ||
            (iEventType ==  EVENT_ID_ARR) ||
            (iEventType ==  EVENT_ID_POP));
}

bool hasInputFile(const std::string sEventType) {
    return ((sEventType == EVENT_TYPE_ENV) ||
            (sEventType == EVENT_TYPE_ARR) ||
            (sEventType == EVENT_TYPE_POP));
}



