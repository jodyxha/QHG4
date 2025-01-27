#ifndef __PRIORITIZER_CPP__
#define __PRIORITIZER_CPP__

#include <map>
#include <vector>
#include <set>
#include <string>

#include "xha_strutilsT.h"
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
        xha_printf("Prioritizer set Prio for [%s] to %u\n", name, iPrio);
    } else {
        iResult = -1;
        xha_printf("[Prioritizer<A>::setPrio] WARNING: Prioritizer tried to add non-existing action '%s'\n",name);
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
        xha_printf("[Prioritizer<A>::addAction] WARNING: there already is an action with name [%s]\n", sActionName);
    } else {
        m_names[sActionName] = act;
        xha_printf("[Prioritizer<A>::addAction] Prioritizer added actions [%s]\n", sActionName);
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

        bool bRemoved = false;
        std::set<uint> sDeletable;
        Action<A> *pAction = itN->second;

        // loop through action list  erasing action from every vector
        typename actlist::iterator itM;
        for (itM = m_prios.begin(); !bRemoved && (itM != m_prios.end()); ++itM) {
            typename std::vector<Action<A> *>::iterator itV;
            for (itV = itM->second.begin(); !bRemoved && (itV != itM->second.end()); itV++) {
                if (*itV == pAction) {
                    itM->second.erase(itV);
                    bRemoved = true;
                }
            }
            if (itM->second.size()==0) {
                // if vector is empty, this entry may be deleted
                sDeletable.insert(itM->first);
            }
                
        }
        if (bRemoved) {

            // delete entries with empty vectors
            std::set<uint>::const_iterator itS;
            for (itS = sDeletable.begin(); itS != sDeletable.end(); itS++) {
                m_prios.erase(*itS);
            }
            m_names.erase(itN);
            iResult = 0;
        } else {
            xha_printf("[Prioritizer<A>::removeAction] couldn't remove Action [%s] /should not happen\n", name);
        }
    } else {
        xha_printf("[Prioritizer<A>::removeAction] Action [%s] not found\n", name);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// disableAction
//
template<typename A>
int Prioritizer<A>::disableAction(std::string name) {
    int iResult = -1;
    // find the action for the name
    typename namelist::iterator itN = m_names.find(name);
    if (itN != m_names.end()) {

        bool bDisabled = false;
        Action<A> *pAction = itN->second;

        // loop through action list  erasing action from every vector
        typename actlist::iterator itM;
        for (itM = m_prios.begin(); !bDisabled && (itM != m_prios.end()); ++itM) {
            typename std::vector<Action<A> *>::iterator itV;
            for (itV = itM->second.begin(); !bDisabled && (itV != itM->second.end()); itV++) {
                if (*itV == pAction) {
                    m_disabled[itM->first].push_back(pAction);
                    itM->second.erase(itV);
                    bDisabled = true;
                }
            }
        }

        if (bDisabled) {
            iResult = 0;
        } else {
            xha_printf("[Prioritizer<A>::disableAction] couldn't diaable Action [%s] /should not happen\n", name);
        }
    } else {
        xha_printf("[Prioritizer<A>::disableAction] Action [%s] not found\n", name);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// enableAction
//
template<typename A>
int Prioritizer<A>::enableAction(std::string name) {
    int iResult = -1;
    // find the action for the name
    typename namelist::iterator itN = m_names.find(name);
    if (itN != m_names.end()) {

        bool bEnabled = false;
        Action<A> *pAction = itN->second;
        
        // loop through disables list  erasing action from every vector
        typename actlist::iterator itM;
        for (itM = m_disabled.begin(); !bEnabled && (itM != m_prios.end()); ++itM) {
            typename std::vector<Action<A> *>::iterator itV;
            for (itV = itM->second.begin(); !bEnabled && (itV != itM->second.end()); itV++) {
                if (*itV == pAction) {
                    m_prios[itM->first].push_back(pAction);
                    itM->second.erase(itV);
                    bEnabled = true;
                }
            }
        }

     
        iResult = 0;
    } else {
        // this should never happen
        xha_printf("[Prioritizer<A>::removeAction] Action [%s] not found\n", name);
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
        xha_printf("[Prioritizer<A>::extractActionParamsQDF3] couldn't extract params 3 for [%s]\n", it->first);
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
        // xha_printf("adding parameters for action %s\n",it->first);
        iResult = it->second->writeAttributesQDF(hSpeciesGroup);
    }
    if (iResult != 0) {
        xha_printf("[Prioritizer<A>::writeActionParamsQDF3] couldn't write params 3 for [%s]\n", it->first);
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
                     xha_printf("[Prioritizer<A>::extractActionParamsQDF4] ERROR: extracting additional data failed\n");
                 }
            } else {
                xha_printf("[Prioritizer<A>::extractActionParamsQDF4] ERROR: extracting attribute data failed\n");
            }
            qdf_closeGroup(hActionGroup);
        } else {
            iResult = -1;
            xha_printf("[Prioritizer<A>::extractActionParamsQDF4] WARNING: couldn't open action group [%s] for writing\n", it->first);
        }       
    }
    if (iResult != 0) {
        xha_printf("[Prioritizer<A>::extractActionParamsQDF4] couldn't extract params for [%s]\n", it->first);
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
        // xha_printff(stderr,"adding parameters for action %s\n",it->first);
        hid_t hActionGroup = qdf_opencreateGroup(hSpeciesGroup, it->first);
        if (hActionGroup != H5P_DEFAULT) {
            iResult = it->second->writeAttributesQDF(hActionGroup);
            if (iResult == 0) {
                iResult = it->second->writeAdditionalDataQDF(hActionGroup);
                if (iResult == 0) {
                    //success
                } else {
                    xha_printf("[Prioritizer<A>::writeActionParamsQDF4] ERROR: writing additional data failed\n");
                }
            } else {
                xha_printf("[Prioritizer<A>::writeActionParamsQDF4] ERROR: writiong attribute data failed\n");
           }
            qdf_closeGroup(hActionGroup);
        } else {
            iResult = -1;
            xha_printf("[Prioritizer<A>::writeActionParamsQDF4] WARNING: couldn't open action group [%s] for reading\n", it->first);
        }
    }
    if (iResult != 0) {
        xha_printf("[Prioritizer<A>::writeActionParamsQDF4] couldn't write params for [%s]\n", it->first);
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
        xha_printf("[Prioritizer<A>::dumpActionStatesQD] dumping state for action %s\n",it->first);
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
        xha_printf("[Prioritizer<A>::modifyAttributes] adding parameters for action %s\n",it->first);
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
                xha_printf("[Prioritizer<A>::getActionParams] Couldn't get values for [%s]\n", it->first);
            }
        } else {
            if (it->second->getNumAttributes() != 0) {
                xha_printf("[Prioritizer<A>::getActionParams] couldn't find params for module [%s] (%s)\n", it->first, it->second->getActionName()); 
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
        
        xha_printf("prioritizer num actions %zd\n", m_names.size());

        
        
        

        typename namelist::iterator it;
        typename namelist::iterator it2;
        // we cycle through all actions until the parameter has changed
        for (it = m_names.begin(); (bIsEqual) && (it != m_names.end()); ++it) {
            it2 = pP->m_names.find(it->first);
            if (it2 != pP->m_names.end()) {
                bIsEqual &= it->second->isEqual(it2->second, bStrict);
                xha_printf("[Prioritizer<A>::isEqual] prioritizer equal for [%s]: %s\n", it->first, (bIsEqual ? "yes":"no")); 
                if (!bIsEqual) {
                    xha_printf("[Prioritizer<A>::isEqual] difference for [%s]: [%s] - [%s]\n", it->first, it->second->getActionName(), it2->second->getActionName());
                }
            } else {
                xha_printf("[Prioritizer<A>::isEqual] prioritizer: action not found in other: [%s]\n", it->first); 
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
        xha_printf("[%s]\n", it->first);
        it->second->showAttributes();
    }
    printf("++++++++++++++++++++++++++++++++++++++++\n");
}


#endif

