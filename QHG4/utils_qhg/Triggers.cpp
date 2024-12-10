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
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>


#include "qhg_consts.h"
#include "stdstrutilsT.h"
#include "Triggers.h"

bool g_bVerbose = false;

//-----------------------------------------------------------------------------
// class Trigger
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor
//
Trigger::Trigger(double fFirstTime, double fLastTime, double fStep) 
    : m_fNextTime(fFirstTime),
      m_fLastTime(fLastTime),
      m_fStep(fStep),
      m_fPrevNextTime(fFirstTime),
      m_fOrigStep(fStep) {
}


//-----------------------------------------------------------------------------
// getNextTime
//
void Trigger::calcNextTime() {
    m_fPrevNextTime = m_fNextTime;
    m_fNextTime += m_fStep;
    if (m_fNextTime > m_fLastTime) {
        m_fNextTime = fPosInf;
        m_fStep = fPosInf;
    }
}

//-----------------------------------------------------------------------------
// revert
//
void Trigger::revert() {
    m_fNextTime = m_fPrevNextTime;
    m_fStep     = m_fOrigStep;
}


//-----------------------------------------------------------------------------
// show
//
std::string Trigger::toString() const {
    return stdsprintf("[%f:%f]%f{%p}", m_fNextTime, m_fLastTime, m_fStep, this);
}



//-----------------------------------------------------------------------------
// class Triggers 
//   IMPORTANT: Triggers now use simulation time, not steps!
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor
//
Triggers::Triggers(double dStartTime, double dEndTime) 
    : m_fNextTriggerTime(fPosInf),
      m_dStartTime(dStartTime),
      m_dEndTime(dEndTime),
      m_bFinal(false) {
}


//-----------------------------------------------------------------------------
// destructor
//
Triggers::~Triggers() {
    for (uint i = 0; i < m_vAllTriggers.size(); i++) {
        delete m_vAllTriggers[i];
    }
}


//-----------------------------------------------------------------------------
// addTrigger
//
uint Triggers::addTrigger(Trigger *pTrigger) {
    m_vAllTriggers.push_back(pTrigger);
    return (uint) m_vAllTriggers.size();
}


//-----------------------------------------------------------------------------
// calcNextTriggerTime
//   find next trigger time.
//   if calcNextTriggerTime is called because of a newly added event,
//   no new set is created if the new events time is greater than m_fNextTriggerTime
//
double Triggers::calcNextTriggerTime(bool bAdditional) {
    double fNext = fPosInf;
 
    // determine new smallest time value
    for (uint i = 0; i < m_vAllTriggers.size(); i++) {
        if (m_vAllTriggers[i]->showNext() < fNext) {
            fNext = m_vAllTriggers[i]->showNext();
        }
    }
    
    if (g_bVerbose) printf("[Triggers::calcNextTriggerTime]{%p} have new next %f, old %f\n", this, fNext,m_fNextTriggerTime);

    // new time earlier: revert triggers for previous set of smallest times
    if (fNext < m_fNextTriggerTime) {
        // revert all in set
        intset::const_iterator it;
        for (it = m_sChanged.begin(); it != m_sChanged.end(); ++it) {
            m_vAllTriggers[*it]->revert();
            if (g_bVerbose) printf("[Triggers::calcNextTriggerTime]{%p} reverting Trigger %d:{%p}\n", this, *it, m_vAllTriggers[*it]);
        }
    }

    m_sChanged.clear();

    if (!bAdditional || (fNext < m_fNextTriggerTime)) {
        // select all with new triggertime
        for (uint i = 0; i < m_vAllTriggers.size(); i++) {
            if (m_vAllTriggers[i]->showNext() == fNext) {
                m_vAllTriggers[i]->calcNextTime();
                // reember them in case of needed revert
            m_sChanged.insert(i);
            if (g_bVerbose) printf("[Triggers::calcNextTriggerTime]{%p} adding Trigger %d:{%p}\n", this, i, m_vAllTriggers[i]);
            
            }
        }
        m_fNextTriggerTime = fNext;
    }
    if (g_bVerbose) printf("[Triggers::calcNextTriggerTime] {%p} %f\n", this, m_fNextTriggerTime);
    return m_fNextTriggerTime;
}


//-----------------------------------------------------------------------------
// parseTrigger
//   a trigger definition has the form
//     trigger-def       ::=  ["S" | "T"] <trigger-def-sub>
//     trigger-def-sub   ::=  <normal-trigger> | <point-trigger> | <final-trigger>
//     normal-trigger    ::= [<trigger-interval>] <step-size>
//     trigger-interval  ::= "[" [<minval>] : [<maxval>] "]"
//     point-trigger     ::= "[" <time> "]"
//     final-trigger     ::= "final"
//  the parsing is most natural if we use char * 
//
int Triggers::parseTrigger(const std::string sTriggerDef) {
    int iResult = 0;


    // make a writeable copy of sTriggerDef
    char *pTriggerDef = new char[sTriggerDef.length()+1];
    char *pBackup     = pTriggerDef; // remember for delete[]
    strcpy(pTriggerDef, sTriggerDef.c_str());

    bool bTimeMode = false;
    // find mode: T=Time, S=Steps
    char c = *pTriggerDef;
    if ((c == 'T') || (c == 'S')) {
        if (c == 'T') {
            bTimeMode = true;
        }
        ++pTriggerDef;
    } else {
        stdprintf("[ Triggers::parseTrigger] WARNING: no time mode given for trigger [%s]. Step mode (S) is used\n", sTriggerDef);
    }

    double dMin  = dPosInf;
    double dMax  = dNegInf;
    double dStep = dNaN;
    char *p1 = strchr(pTriggerDef, '[');
    char *p2 = strchr(pTriggerDef, ':');
    char *p3 = strchr(pTriggerDef, ']');

    if ((p1 != NULL) && (p3 != NULL)) {
        // we have an expression with brackets
        // sanity:
        if (p1 < p3) {
            p1++;
            if (p2 != NULL) {
                if ((p1 <= p2) && (p2 <= p3))  {
                    *p2 = '\0';
                    p2++;
                } else {
                    
                    iResult = -1;
                }
            }
            *p3 = '\0';
            p3++;

            // p1 points to the first character after '[' (beginning of timepoint or min time)
            // p2 points to the first character after ':' (beginning of max time or NULL) 
            // p3 points to the first character after ']' (beginning of the step or NULL) 

            double d1;
            double d2;
            double d3;
            if (iResult == 0) {
                if (*p1 == '\0') {
                    // normal-trigger : minval = start time
                    d1 = 0;
                } else {
                    // normal-trigger : minval
                    if (!strToNum(p1, &d1)) {
                        iResult = -1;
                    } 
                }
                //d1 = d1 - (bTimeMode ? m_dStartTime : 0);
                d1 = d1 + (bTimeMode ? 0 : m_dStartTime);
            }

            if (p2 != NULL) {
                // there has been a ':'
                if (iResult == 0) {
                    // sanity check ':' between '[' and ']'
                    if ((p1 < p2) && (p2 < p3)) {
                        if (*p2 == '\0') {
                            // normal-trigger : maxval = posInf
                            d2 = dPosInf;
                        } else {
                            // normal-trigger : maxval
                            if (!strToNum(p2, &d2)) {
                                iResult = -1;
                            }
                        }
                        //d2 = d2 -(bTimeMode ? m_dStartTime : 0);
                        d2 = d2 + (bTimeMode ? 0 : m_dStartTime);
                    } else {
                        iResult = -1;
                    }
                }
            } else {
                // point-trigger 
                // [<number>] is the same as [<number>:<number>]
                d2 = d1;
            }
    
            if (iResult == 0) {
                // step size
                if (*p3 != '\0') {
                    if (!strToNum(p3, &d3)) {
                        iResult = -1;
                    }
                } else {
                    // no step -> 1
                    d3 = 1;
                }
            }
        
            if(iResult == 0) {
                dMin  = d1;
                dMax  = d2;
                dStep = d3;
            }
        } else {
            iResult = -1;
        }

    } else if ((p1 == NULL) && (p2 == NULL) && (p3 == NULL)) { 
        // single number: eternal step size
        double dV = 0;
        if (strToNum(pTriggerDef, &dV)) {
            dMin  = m_dStartTime;
            dMax  = dPosInf;
            dStep = dV;
            
        } else if (strcmp("final", pTriggerDef) == 0) {
            m_bFinal = true;
        } else {
            iResult = -1;
        }
    } else {
        stdprintf("[Triggers::parseTrigger] bad trigger \"%s\" (p1;\"%s\", p2:\"%s\", p3:\"%s\")\n", sTriggerDef, (p1==NULL)?"(null)":p1, (p2==NULL)?"(null)":p2, (p3==NULL)?"(null)":p3);
        iResult = -1;
    }

  
    // create the trigger
    if (iResult == 0) {
        // eliminate point triggers outside of out interval of interest 
        if ((dMin != dMax) || ((dMin >= m_dStartTime) && (dMax <= m_dEndTime))) {

            stdprintf("Setting trigger %f, %f, %f\n", dMin, dMax, dStep);
            Trigger *pT = new Trigger(dMin, dMax, dStep);
            addTrigger(pT);
        } else {
            // not an error - we just ignore it
            // stdprintf("Ignoring trigger %f, %f, %f\n", dMin, dMax, dStep);
        }
    }

    // delete the buffer
    delete[] pBackup;
    return iResult;

}


//-----------------------------------------------------------------------------
// merge
//
void Triggers::merge(Triggers *pTriggers) {
    m_vAllTriggers.insert(m_vAllTriggers.end(),  
                          pTriggers->m_vAllTriggers.begin(),
                          pTriggers->m_vAllTriggers.end());
}

 
//-----------------------------------------------------------------------------
// createTriggers
//
Triggers *Triggers::createTriggers(const std::string sTriggersDef, double dStartTime, double dEndTime) {
    Triggers *pT = NULL;
    int iResult = 0;
    stringvec vTriggerDefs;
    uint iNum = splitString(sTriggersDef, vTriggerDefs, "+", true);
    if (iNum > 0) {
        pT = new Triggers(dStartTime, dEndTime);
        for (uint i = 0; (iResult == 0) && (i < iNum); i++) {
            iResult = pT->parseTrigger(vTriggerDefs[i]);
        }
    } else {
        iResult = -1;
    }
    if (iResult != 0) {
        // emoty triggerdef
        pT = NULL;
    }
    return pT;
    /*
    char *p = strtok(pTriggersDef, "+");
    while ((iResult == 0) && (p != NULL)) {
        iResult = pT->parseTrigger(p);
        p = strtok(NULL, "+");
    }
    if (iResult != 0) {
        delete pT;
        pT = NULL;
    }
    return pT;
    */
}


//-----------------------------------------------------------------------------
// toString
//
std::string Triggers::toString() const {
    std::string sCur = stdsprintf("{{%p}}m[%f]", this, m_fNextTriggerTime);

    for (uint i = 0; i < m_vAllTriggers.size(); i++) {
        if (i > 0) {
            sCur += '+';
        }
        sCur += m_vAllTriggers[i]->toString();
    }
    return std::string(sCur);
}

