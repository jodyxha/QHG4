/*============================================================================
| Trigger
| 
|  Represents a sequence of time points spaced at regular intervals
|  between a starting and end points (which may include -inf and +inf).
|  A single time point is the special case where start == end.
|  The method showNext() return the next trigger time for a Trigger object.
|
| Triggers
|  manages a collection of trigger objects:
|  - reading from string
|  - adding new Trigger objects
|  - finding next trigger time (i.e. the minimum of all showNext() values)
|
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __TRIGGERS_H__
#define __TRIGGERS_H__

#include <cstdio>
#include <vector>
#include <string>

#include "utils.h"

class Trigger {
public:
    Trigger(double fFirstTime, double fLastTime, double fStep); 
    double showNext() { return m_fNextTime;};
    void calcNextTime();
    std::string toString() const;
    void revert();
private:
    double m_fNextTime;
    double m_fLastTime;
    double m_fStep;
    double m_fPrevNextTime;
    double m_fOrigStep;
};



class Triggers {
public:
    Triggers(double dStartTime, double dEndTime);
    static Triggers *createTriggers(const std::string sTriggersDef, double dStartTime, double dEndTime);
    ~Triggers();
    double calcNextTriggerTime(bool bAdditional);
    uint addTrigger(Trigger *pTrigger);
    int parseTrigger(const std::string sTriggerDef);
    void merge(Triggers *pTriggers);
    double getNextTriggerTime() { return m_fNextTriggerTime;};
    std::string toString() const;

    double getStartTime() { return m_dStartTime;};
    double getEndTime() { return m_dEndTime;};
    bool hasFinal() { return m_bFinal; };
protected:
    std::vector<Trigger *> m_vAllTriggers;
    double m_fNextTriggerTime;
    intset m_sChanged;
    double m_dStartTime;
    double m_dEndTime;
    bool   m_bFinal;

};

#endif
