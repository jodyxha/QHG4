#ifndef __PRIORITIZER_CPP__
#define __PRIORITIZER_CPP__

#include <map>
#include <vector>
#include <set>
#include <string>

#include "stdstrutilsT.h"
#include "Prioritizer.h"
#include "QDFUtils.h"
#include "Action.h"


//-----------------------------------------------------------------------------
// setPrio
//
template<typename A>
int Prioritizer<A>::setPrio(uint iPrio, std::string name) {
    int iResult = 0;
    if (m_names.find(name) != m_names.end()) {
        m_prios[iPrio].push_back(m_names[name]);
        stdprintf("Prioritizer set Prio for [%s] to %u\n", name, iPrio);
    } else {
        iResult = -1;
        stdprintf("[Prioritizer<A>::setPrio] WARNING: Prioritizer tried to add non-existing action '%s'\n",name);
        showAttributes();
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// addAction
//
template<typename A>
int Prioritizer<A>::addAction(Action<A> *act) {
    int iResult = 0;
    const std::string sActionName = act->getActionName();
    if (m_names.find(sActionName) != m_names.end()) {
        iResult = -1;
        stdprintf("[Prioritizer<A>::addAction] WARNING: there already is an action with name [%s]\n", sActionName);
    } else {
        m_names[sActionName] = act;
        stdprintf("[Prioritizer<A>::addAction] Prioritizer added actions [%s]\n", sActionName);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// removeAction
//
template<typename A>
int Prioritizer<A>::removeAction(std::string name) {
    int iResult = -1;
    // find the action for the name
    typename namelist::iterator itN = m_names.find(name);
    if (itN != m_names.end()) {

        std::set<uint> sDeletable;
        Action<A> *pAction = itN->second;

        // loop through action list  erasing action from every vector
        typename actlist::iterator itM;
        for (itM = m_prios.begin(); itM != m_prios.end(); ++itM) {
            typename std::vector<Action<A> *>::iterator itV;
            for (itV = itM->second.begin(); itV != itM->second.end(); ) {
                if (*itV == pAction) {
                    itM->second.erase(itV);
                } else {
                    itV++;
                }
            }
            if (itM->second.size()==0) {
                // if vector is empty, this entry may be deleted
                sDeletable.insert(itM->first);
            }
        }

        // delete entries with empty vectors
        std::set<uint>::const_iterator itS;
        for (itS = sDeletable.begin(); itS != sDeletable.end(); itS++) {
            m_prios.erase(*itS);
        }
	m_names.erase(itN);
        iResult = 0;
    } else {
        stdprintf("[Prioritizer<A>::removeAction] Action [%s] not found\n", name);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// hasAction
//
template<typename A>
bool Prioritizer<A>::hasAction(std::string sAction) {
    bool bResult = false;
    // find the action for the name
    typename namelist::iterator itN = m_names.find(sAction);
    if (itN != m_names.end()) {
        bResult = true;
    }
    return bResult;
}


//-----------------------------------------------------------------------------
// hasParam
//
template<typename A>
bool Prioritizer<A>::hasParam(std::string sAttrName) {
    bool bResult = false;
    // find the action for the name
    typename namelist::iterator it;
    // we cycle through all actions until the parameter has changed
    for (it = m_names.begin(); (!bResult) && (it != m_names.end()); ++it) {
        bResult |= it->second->hasAttribute(sAttrName);
    }

    return bResult;
}


//-----------------------------------------------------------------------------
// getPrios
//
template<typename A>
uint Prioritizer<A>::getPrios( std::set<uint> &vPrios) {
    typename actlist::const_iterator it;
    for (it = m_prios.begin(); it != m_prios.end(); ++it) {
        vPrios.insert(it->first);
    }
    return (uint) vPrios.size();
}

/*
//-----------------------------------------------------------------------------
// extractActionParamsQDF
//
template<typename A>
int Prioritizer<A>::extractActionParamsQDF(hid_t hSpeciesGroupi/ *, int iVersion* /) {

    int iResult = -1;
    //    if (iVersion >= 4) {
        iResult = extractActionParamsQDF4(hSpeciesGroup);
        //    } else {
        //        iResult = extractActionParamsQDF3(hSpeciesGroup);
        //    }

    return iResult;
}


//-----------------------------------------------------------------------------
// writeActionParamsQDF
//
template<typename A>
int Prioritizer<A>::writeActionParamsQDF(hid_t hSpeciesGroup/ *, int iVersion* /) {
    int iResult = -1;
    //    if (iVersion >= 4) {
        iResult = writeActionParamsQDF4(hSpeciesGroup);
        //    } else {
        //        iResult = writeActionParamsQDF3(hSpeciesGroup);
        //    }

    return iResult;
}

/ *
//-----------------------------------------------------------------------------
// extractActionParamsQDF3
//
template<typename A>
int Prioritizer<A>::extractActionParamsQDF3(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        iResult = it->second->extractAttributesQDF(hSpeciesGroup);
    }
    if (iResult != 0) {
        stdprintf("[Prioritizer<A>::extractActionParamsQDF3] couldn't extract params 3 for [%s]\n", it->first);
        showAttributes();
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// writeActionParamsQDF3
//
template<typename A>
int Prioritizer<A>::writeActionParamsQDF3(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        // stdprintf("adding parameters for action %s\n",it->first);
        iResult = it->second->writeAttributesQDF(hSpeciesGroup);
    }
    if (iResult != 0) {
        stdprintf("[Prioritizer<A>::writeActionParamsQDF3] couldn't write params 3 for [%s]\n", it->first);
        showAttributes();
    }

    return iResult;
}
*/

//-----------------------------------------------------------------------------
// extractActionParamsQDF
//
template<typename A>
int Prioritizer<A>::extractActionParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        hid_t hActionGroup = qdf_openGroup(hSpeciesGroup, it->first);
        if (hActionGroup != H5P_DEFAULT) {
            iResult = it->second->extractAttributesQDF(hActionGroup);
            if (iResult == 0) {
                 iResult = it->second->readAdditionalDataQDF(hActionGroup);
                 if (iResult == 0) {
                     // success
                 } else {
                     stdprintf("[Prioritizer<A>::extractActionParamsQDF4] ERROR: extracting additional data failed\n");
                 }
            } else {
                stdprintf("[Prioritizer<A>::extractActionParamsQDF4] ERROR: extracting attribute data failed\n");
            }
            qdf_closeGroup(hActionGroup);
        } else {
            iResult = -1;
            stdprintf("[Prioritizer<A>::extractActionParamsQDF4] WARNING: couldn't open action group [%s] for writing\n", it->first);
        }       
    }
    if (iResult != 0) {
        stdprintf("[Prioritizer<A>::extractActionParamsQDF4] couldn't extract params for [%s]\n", it->first);
        showAttributes();
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// writeActionParamsQDF4
//
template<typename A>
int Prioritizer<A>::writeActionParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        // stdprintff(stderr,"adding parameters for action %s\n",it->first);
        hid_t hActionGroup = qdf_opencreateGroup(hSpeciesGroup, it->first);
        if (hActionGroup != H5P_DEFAULT) {
            iResult = it->second->writeAttributesQDF(hActionGroup);
            if (iResult == 0) {
                iResult = it->second->writeAdditionalDataQDF(hActionGroup);
                if (iResult == 0) {
                    //success
                } else {
                    stdprintf("[Prioritizer<A>::writeActionParamsQDF4] ERROR: writing additional data failed\n");
                }
            } else {
                stdprintf("[Prioritizer<A>::writeActionParamsQDF4] ERROR: writiong attribute data failed\n");
           }
            qdf_closeGroup(hActionGroup);
        } else {
            iResult = -1;
            stdprintf("[Prioritizer<A>::writeActionParamsQDF4] WARNING: couldn't open action group [%s] for reading\n", it->first);
        }
    }
    if (iResult != 0) {
        stdprintf("[Prioritizer<A>::writeActionParamsQDF4] couldn't write params for [%s]\n", it->first);
        showAttributes();
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// restoreActionStatesQDF
//
template<typename A>
int Prioritizer<A>::restoreActionStatesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        iResult = it->second->restoreStateQDF(hSpeciesGroup);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// dumpActionStatesQDF
//
template<typename A>
int Prioritizer<A>::dumpActionStatesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        stdprintf("[Prioritizer<A>::dumpActionStatesQD] dumping state for action %s\n",it->first);
        iResult = it->second->dumpStateQDF(hSpeciesGroup);
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// modifyAttributes
//
template<typename A>
int Prioritizer<A>::modifyAttributes(const std::string sAttrName, double dValue) {

    int iResult = -1;
    typename namelist::const_iterator it;

    // we cycle through all actions until the parameter has changed
    for (it = m_names.begin(); (iResult < 0) && (it != m_names.end()); ++it) {
        stdprintf("[Prioritizer<A>::modifyAttributes] adding parameters for action %s\n",it->first);
        iResult = it->second->modifyAttributes(sAttrName, dValue);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// getActionAttributes
//
template<typename A>
int Prioritizer<A>::getActionAttributes(const modulemap &mModules) {
    int iResult = 0;
    modulemap::const_iterator itM;    
    typename namelist::const_iterator it;
    for (it = m_names.begin(); (iResult == 0) && (it != m_names.end()); ++it) {
        itM = mModules.find(it->second->getActionName());
        if (itM != mModules.end()) { 
            iResult = it->second->tryGetAttributes(itM->second);  // in Action
            if (iResult != 0) {
                stdprintf("[Prioritizer<A>::getActionParams] Couldn't get values for [%s]\n", it->first);
            }
        } else {
            if (it->second->getNumAttributes() != 0) {
                stdprintf("[Prioritizer<A>::getActionParams] couldn't find params for module [%s] (%s)\n", it->first, it->second->getActionName()); 
                iResult = -1;
            } else {
                iResult = 0;
            }
       }
    }
    return iResult;

}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename A>
bool Prioritizer<A>::isEqual(Prioritizer<A> *pP, bool bStrict) {

    bool bIsEqual = false;
    if (m_names.size() == pP->m_names.size()) {
        bIsEqual = true;
        
        stdprintf("prioritizer num actions %zd\n", m_names.size());

        
        
        

        typename namelist::iterator it;
        typename namelist::iterator it2;
        // we cycle through all actions until the parameter has changed
        for (it = m_names.begin(); (bIsEqual) && (it != m_names.end()); ++it) {
            it2 = pP->m_names.find(it->first);
            if (it2 != pP->m_names.end()) {
                bIsEqual &= it->second->isEqual(it2->second, bStrict);
                stdprintf("[Prioritizer<A>::isEqual] prioritizer equal for [%s]: %s\n", it->first, (bIsEqual ? "yes":"no")); 
            } else {
                stdprintf("[Prioritizer<A>::isEqual] prioritizer: action not found in other: [%s]\n", it->first); 
                bIsEqual = false;
            }
        }
    }
    return bIsEqual;
}


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename A>
void Prioritizer<A>::showAttributes() {
    printf("++++++++++++++++++++++++++++++++++++++++\n");
    printf("Required attributes for this population:\n");
    typename namelist::const_iterator it;
    for (it = m_names.begin(); it != m_names.end(); ++it) {
        stdprintf("[%s]\n", it->first);
        it->second->showAttributes();
    }
    printf("++++++++++++++++++++++++++++++++++++++++\n");
}


#endif

