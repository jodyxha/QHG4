/*============================================================================
| EventManager
| 
|  Manages QHG EventData associated with Trigger objects determining their
|  activation.
| 
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __EVENTMANAGER_H__
#define __EVENTMANAGER_H__

#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "Triggers.h"
#include "EventData.h"

typedef std::map<EventData *, Triggers *, EDCompare> evttrig;


class EventManager {
public:
    EventManager(double dStartTime);
   ~EventManager();

    void start();
    void clear();
    int   loadEventItem(EventData *pSD, Triggers *pT, bool bUpdateEventList);
    
    bool  hasNewEvent(int iCurStep);

    const std::vector<EventData*> &getEventData();
    const std::vector<EventData*> &force();
    
    void forwardTo(int iNewStep);
    void triggerAll(int iCurStep);
    void findFinalEvents();

    void toString(stringvec &vsCur) const;
protected:
    double findNextEvent();
    void   updateEventList();

    std::vector<EventData*> m_vEDSet;
    
    double m_dStartTime;
    double m_dNextTime;

    evttrig m_vEDT;
    bool m_bStarted;
};

#endif
