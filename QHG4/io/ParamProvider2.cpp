#include <cstdio>
#include <cstring>


#include "stdstrutilsT.h"
#include "ParamProvider2.h"

#define ATTR_NAME   "name"
#define ATTR_ID     "id"
#define ATTR_VALUE  "value"
#define ATTR_SPC_NAME "species_name"

#define ELEM_CLASS  "class"
#define ELEM_MODULE "module"
#define ELEM_PRIOS  "priorities"
#define ELEM_PRIO   "prio"
#define ELEM_PARAM  "param"


// ****************************************************************************
// ModuleComplex
// ****************************************************************************


//----------------------------------------------------------------------------
// constructor
//
ModuleComplex::ModuleComplex(const stringmap mParams, modulemap & mSubModules)
    : m_mParams(mParams),
      m_mSubModules(mSubModules) {
}

//----------------------------------------------------------------------------
// destructor
//
ModuleComplex:: ~ModuleComplex() {
    modulemap::iterator it;
    for (it = m_mSubModules.begin(); it != m_mSubModules.end(); ++it) {
        delete it->second;
    }
}

//----------------------------------------------------------------------------
// addParam
//
int ModuleComplex::addParam(const std::string sKey, const std::string &sVal) {
    int iResult = -1;
    stringmap::const_iterator it = m_mParams.find(sKey);
    if (it == m_mParams.end()) {
        m_mParams[sKey] = sVal;
        iResult = 0;
    } else {
        stdprintf("Already have parameter [%s]\n", it->first);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// addSubModule
//
int ModuleComplex::addSubModule(const std::string sKey, ModuleComplex *pSubModule) {
    int iResult = -1;
    modulemap::const_iterator it = m_mSubModules.find(sKey);
    if (it == m_mSubModules.end()) {
        m_mSubModules[sKey] = pSubModule;
        iResult = 0;
    } else {
        stdprintf("Already have submodule [%s]\n", it->first);
    }
    return iResult;
}

  
//----------------------------------------------------------------------------
// getParam
//
const std::string ModuleComplex::getParam(const std::string sKey) {
    std::string sRes = "";
    stringmap::const_iterator it = m_mParams.find(sKey);
    if (it != m_mParams.end()) {
        sRes = it->second;
    } else {
        stdprintf("Have no parameter [%s]\n", sKey);
    }
    return std::string(sRes);
}


//----------------------------------------------------------------------------
// getModule
//
ModuleComplex *ModuleComplex::getModule(const std::string sModuleName) {
    ModuleComplex *pSub = NULL;
    
    modulemap::const_iterator it = m_mSubModules.find(sModuleName);
    if (it != m_mSubModules.end()) {
        pSub = it->second;
    } else {
        stdprintf("Have no submodule [%s]\n", sModuleName);
    }
    return pSub;
}





// ****************************************************************************
// ParamProvider2
// ****************************************************************************


//----------------------------------------------------------------------------
// getAttribute
//  auxiliary tool to find an attribute by name in a ma of attributes.
//  return empty string if name does not exist
//
const std::string getAttribute(stringmap &mAttr, const std::string sAttrName) {
    std::string sValue = "";
    stringmap::const_iterator it = mAttr.find(sAttrName);
    if (it != mAttr.end()) {
        sValue = it->second;

    }
    return std::string(sValue);
}


//----------------------------------------------------------------------------
// createInstance
//
ParamProvider2 *ParamProvider2::createInstance(const std::string sXMLFile) {
    ParamProvider2 *pQXR = new ParamProvider2();
    int iResult = pQXR->init(sXMLFile);
    if (iResult != 0)  {
        delete pQXR;
        pQXR = NULL;
    }
    return pQXR;
}


//----------------------------------------------------------------------------
// constructor ParamProvider2
//
ParamProvider2::ParamProvider2()
    : m_pCurClassInfo(NULL),
      m_sCurClassName(""),
      m_sSpeciesName("") {
}


//----------------------------------------------------------------------------
// destructor
//
ParamProvider2::~ParamProvider2() {
    modulemap::iterator it;
    for (it = m_mModules.begin(); it != m_mModules.end(); it++) {
        delete it->second;
    }
}


//----------------------------------------------------------------------------
// getClass
//    
//
const modulemap *ParamProvider2::getClass(const std::string sClassName) {
    const modulemap *pM = NULL;
    classes::const_iterator it = m_mClasses.find(sClassName);
    if (it != m_mClasses.end()) {
        pM = &it->second.mods;
    }
    return pM;
}


//----------------------------------------------------------------------------
// getParams
//
const stringmap *ParamProvider2::getParams(const std::string sModuleName) {
    const stringmap *pP = NULL;
    if (m_pCurClassInfo != NULL) {
        modulemap::const_iterator itm = m_pCurClassInfo->mods.find(sModuleName);
        if (itm != m_pCurClassInfo->mods.end()) {
            pP = &itm->second->getAttributes();
        } else {
            stdprintf("Unknown module name [%s]\n", sModuleName);
        }
    } else {
        stdprintf("No class is selected\n");
    }
    return pP;
}


//----------------------------------------------------------------------------
// selectClass
//
int ParamProvider2::selectClass(const std::string sClassName) {
    int iResult = -1;
    m_pCurClassInfo = NULL;
    classes::const_iterator itc = m_mClasses.find(sClassName);
    if (itc != m_mClasses.end()) {
        m_pCurClassInfo = &itc->second;
        m_sCurClassName = itc->first;
        stringmap::const_iterator ita = itc->second.cattr.find(ATTR_SPC_NAME);
        if (ita != itc->second.cattr.end()) {
            m_sSpeciesName = ita->second;
            iResult = 0;
        } else {
            stdprintf("class [%s] has no [%s] attribute\n", sClassName, ATTR_SPC_NAME);
        }
    } else {
        stdprintf("class [%s] is not known\n", sClassName);
    }
    return iResult;
}
   


//----------------------------------------------------------------------------
// processParam
//    <subtag name=<name> value=<value> >
//
int ParamProvider2::processParam(qhgXMLNode *pParam, const std::string sSubtag,  stringmap &att_param) {
    int iResult = -1;
 
    if (pParam != NULL) {
        std::string sPName = pParam->getName();
        //stdprintf("[ParamProvider2::processParam] processing module [%s] (subtag [%s]\n", sPName, sSubtag);
        if (sPName == sSubtag) {
            stringmap &mAttr = pParam->getAttrs();
            const std::string sName  = getAttribute(mAttr, ATTR_NAME);
            const std::string sValue = getAttribute(mAttr, ATTR_VALUE);
            //stdprintf("[ParamProvider2::processParam]   got [%s] = [%s]\n", sName, sValue);
            if ((!sName.empty()) && (!sValue.empty())) {
                att_param[sName] = sValue;
                iResult = 0;
            } else {
                stdprintf("Couldn't find 'name' and/or 'value' in attribute tag\n");
            }
        } else {
            stdprintf("Expected Element to be '%s' not '%s'\n", sSubtag, sPName);
        }
    } else {
        stdprintf("Can't do NULL element'\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// processModule
//
int ParamProvider2::processModule(qhgXMLNode *pModule, modulemap &mModules) {
    int iResult = -1;
    if (pModule != NULL) {
        if (pModule->getName() == ELEM_MODULE) {
            stringmap att_param;
            modulemap subMods;

            stringmap &mAttr = pModule->getAttrs();
            const std::string sName = getAttribute(mAttr, ATTR_NAME);
            if (!sName.empty()) {
                const std::string sID = getAttribute(mAttr, ATTR_ID);
                //stdprintf("[ParamProvider2::processModule] processing module [%s] (id [%s])\n", sName, sID);

                iResult = 0;

                qhgXMLNode *pChild = pModule->getChild();
                while ((iResult == 0) && (pChild != NULL) && (pChild->getName() == ELEM_PARAM)) {
                    iResult = processParam(pChild, ELEM_PARAM, att_param);
                    pChild = pChild->getNext();
                }                    
                while ((iResult == 0) && (pChild != NULL) && (pChild->getName() == ELEM_MODULE)) {
                    iResult = processModule(pChild, subMods);
                    pChild = pChild->getNext();
                }                    
                    

                if (iResult == 0)  {
                    //stdprintf("[ParamProvider2::processModule] everytin ok; have name [%s] and id [%s]\n", sName, sID);
                    ModuleComplex *pMC = new ModuleComplex(att_param, subMods); 
                    std::string sName2 = "";
                    if (!sID.empty()) {
                        sName2 = stdsprintf("%s[%s]", sName, sID);
                    } else {
                        sName2 = sName;
                    }
                    //stdprintf("[ParamProvider2::processModule] added module [%s] to modulemap\n", sName2);
                    mModules[sName2] = pMC;
                }
            } else {
                stdprintf("Attribute 'name' of node '%s' does not exist\n", pModule->getName());
            }
        } else {
            stdprintf("Expected Element to be '%s' not '%s'\n", ELEM_MODULE, pModule->getName());
        }
    } else {
        stdprintf("Can't do NULL element'\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// processPriorities
//
int ParamProvider2::processPriorities(qhgXMLNode *pPrios, stringmap &pa) {
    int iResult = -1;
    if (pPrios != NULL) {
        if (pPrios->getName() == ELEM_PRIOS) {
            iResult = 0;
            //stdprintf("[ParamProvider2::processPriorities] processing priorities\n");
            qhgXMLNode *pChild = pPrios->getChild();
            while ((iResult == 0) && (pChild != NULL)) {
                iResult = processParam(pChild, ELEM_PRIO, pa);
                pChild = pChild->getNext();
            }
        } else {
            stdprintf("Expected Element to be '%s' not '%s'\n", ELEM_PRIOS, pPrios->getName());
        }
    } else {
        stdprintf("Can't do NULL element'\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// processClass
//
int ParamProvider2::processClass(qhgXMLNode *pClass) {
    int iResult = -1;
    if (pClass != NULL) {
        if (pClass->getName() == ELEM_CLASS) {
            stringmap &mAttr = pClass->getAttrs();
            const std::string sName = getAttribute(mAttr, ATTR_NAME);
            if (!sName.empty()) {
                stdprintf("[ParamProvider2::processClass] processing class [%s]\n", sName);
                stringmap attr_class;
                stringmap::const_iterator it;
                for (it = mAttr.begin(); it != mAttr.end(); ++it)   {
                    if (it->first != ATTR_NAME) {
                        attr_class[it->first] = it->second;
                    }
                }                    
                it = attr_class.find(ATTR_SPC_NAME);
                if (it != attr_class.end()) {
                    iResult = 0;
                    m_sSpeciesName = it->second;
                    stringmap attr_prios;
                    qhgXMLNode *pChild = pClass->getChild();
                    while ((iResult == 0) && (pChild != NULL)) {
                        if (pChild->getName() == ELEM_MODULE) {
                            iResult = processModule(pChild, m_mModules);
                            stdprintf("[ParamProvider2::processClass] class [%s] now has %zd  modules \n", sName, m_mModules.size());

                        } else if (pChild->getName() == ELEM_PRIOS) {
                            iResult = processPriorities(pChild, attr_prios);
                        } else {
                            stdprintf("unknown element:%s\n", pChild->getName());
                        } 
                        pChild = pChild->getNext();
                    }
                
                    classinfo ci;
                    ci.cattr = attr_class;
                    ci.mods  = m_mModules;
                    ci.prios = attr_prios; 
                    m_mClasses[sName] = ci;
                    m_vClassNames.push_back(sName);

                } else {
                    stdprintf("class attributes must include  [%s]\n", ATTR_SPC_NAME);
                }
            } else {
                stdprintf("Attribute '%s' of node '%s' does not exist\n", ATTR_NAME, pClass->getName());
            }
          

        } else {
            stdprintf("Expected Element to be '%s' not '%s'\n", ELEM_CLASS, pClass->getName());
        } 
    } else {
        stdprintf("Can't do NULL element'\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// init
//
int ParamProvider2::init(const std::string sXMLFile) {
    int iResult = -1;
    qhgXMLTree *pQXT = qhgXMLTree::createInstance(sXMLFile);
    if (pQXT != NULL) {
        qhgXMLNode  *pRoot = pQXT->getRoot(); 

        qhgXMLNode *pClass = pRoot->getChild();
        iResult = 0;
        
        while ((iResult == 0) && (pClass != NULL)) {
            iResult = processClass(pClass);
            pClass = pClass->getNext();
        }
        delete pQXT;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// showModule
//
void showModule(ModuleComplex *pModule, const std::string sIndent) {
    
    stringmap::const_iterator  itp;
    for (itp = pModule->getAttributes().begin(); itp != pModule->getAttributes().end(); ++itp) {
        stdprintf("%s%s -> %s\n", sIndent, itp->first, itp->second);
    }
    std::string sIndent2 = "    " + sIndent;    
    modulemap::const_iterator  itm;
    for (itm = pModule->getSubModules().begin(); itm != pModule->getSubModules().end(); ++itm) {
        stdprintf("%s%s: \n", sIndent, itm->first);
        showModule(itm->second, sIndent2);
    }        

}


//----------------------------------------------------------------------------
// showTree
//
void ParamProvider2::showTree() {
    classes::const_iterator itc;
    for (itc = m_mClasses.begin(); itc != m_mClasses.end(); ++itc) {
        stdprintf("------ class\n");
        stdprintf("class '%s'\n", itc->first);
        const stringmap &cattr = itc->second.cattr;
        stringmap::const_iterator  ita;
        for (ita = cattr.begin(); ita != cattr.end(); ++ita) {
            stdprintf("  %s: %s\n", ita->first, ita->second);
        }
        stdprintf("------ modules\n");
        const modulemap &mods = itc->second.mods;
        modulemap::const_iterator itm;
        for (itm = mods.begin(); itm != mods.end(); ++itm) {
            stdprintf("  module '%s'\n", itm->first);
            showModule(itm->second, "    ");
                        
        }
        stdprintf("------ priorities\n");
        const stringmap &pattr = itc->second.prios;
        stringmap::const_iterator  ita3;
        for (ita3 = pattr.begin(); ita3 != pattr.end(); ++ita3) {
            stdprintf("  %s: %s\n", ita3->first, ita3->second);
        }

    }

}

