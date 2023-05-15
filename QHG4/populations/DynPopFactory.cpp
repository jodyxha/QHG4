#include <cstdio>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "PopBase.h"
#include "ParamProvider2.h"
#include "DynPopFactory.h"
#include "ArrayShare.h"

#define GETINFO_NAME    "getInfo"
#define CREATEPOP_NAME  "createPop"

typedef const std::string  (*getInfoFunc)();
typedef PopBase           *(*createPopFunc)(ArrayShare *pAS, SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);


//----------------------------------------------------------------------------
// createInstance
//
DynPopFactory *DynPopFactory::createInstance(stringvec vSODirs, SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) {
    DynPopFactory *pPC = new DynPopFactory(pCG, pPopFinder, iLayerSize, apIDG, aulState, aiSeeds);
    int iResult = pPC->init(vSODirs);
    if (iResult != 0) {
        delete pPC;
        pPC = NULL;
    }
    return pPC;
}


//----------------------------------------------------------------------------
// destructor
//
DynPopFactory::~DynPopFactory() {
    for (unsigned int i = 0; i < m_vLibHandles.size(); i++) {
        dlclose(m_vLibHandles[i]);
    }
}
      

//----------------------------------------------------------------------------
// init
//
int DynPopFactory::init(stringvec vSODirs) {
    int iResult = 0;

    m_vSODirs = vSODirs;
    m_mNameFiles.clear();

    iResult = collectPlugins();

    return iResult;
}


//----------------------------------------------------------------------------
// collectPlugins
//
int DynPopFactory::collectPlugins() {
    int iResult = 0;
    
    for (uint i = 0; (iResult == 0) && (i < m_vSODirs.size()); i++) {
        iResult = collectPluginsInDir(m_vSODirs[i]);
    }
    printf("[DynPopFactory::collectPlugins] Found %zd libar%s%s\n", m_mNameFiles.size(), (m_mNameFiles.size() == 1)?"y":"ies", (m_mNameFiles.size() > 0)?":":".");
    stringmap::const_iterator it;
    for (it = m_mNameFiles.begin(); it != m_mNameFiles.end(); ++it) {
        stdprintf("[DynPopFactory::collectPlugins]  [%s] found in [%s]\n", it->first, it->second);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// collectPluginsInDir
//
int DynPopFactory::collectPluginsInDir(const std::string sPath) {
    int iResult = -1;
    DIR *hDir = opendir(sPath.c_str());
    if (hDir != NULL) {
        struct dirent *hFileEnt = readdir(hDir);
        while (hFileEnt != NULL) {
            if ((hFileEnt->d_type == DT_REG) || (hFileEnt->d_type == DT_LNK)) {
                std::string sFileName = hFileEnt->d_name;
                    if (endsWith(sFileName, "Wrapper.so")) {
                        iResult = -1;
                        std::string sFullName  = stdsprintf("%s/%s", sPath, sFileName);
                        void *hLibrary = dlopen(sFullName.c_str(), RTLD_LAZY);
                        if (hLibrary != NULL) {
                            getInfoFunc pGetInfo = (getInfoFunc) dlsym(hLibrary, GETINFO_NAME);
                            if (pGetInfo != NULL) {
                                std::string sPopName = pGetInfo();
                                if (!sPopName.empty()) {
                                    createPopFunc pCreatePop = (createPopFunc) dlsym(hLibrary, CREATEPOP_NAME);
                                    if (pCreatePop != NULL) {
                                        
                                        stdprintf("[DynPopFactory::collectPlugins] Registering [%s] from [%s]\n", sPopName, sFullName);
                                        m_mNameFiles[sPopName] = sFullName;
                                        iResult = 0;
                                    } else {
                                        stdprintf("[DynPopFactory::collectPlugins] Couldn't get function [%s] from [%s]\n", CREATEPOP_NAME, sFullName);
                                    }
                                } else {
                                    stdprintf("[DynPopFactory::collectPlugins] Function [%s] returned empty name\n", GETINFO_NAME);
                                }
                            } else {
                                stdprintf("[DynPopFactory::collectPlugins] Couldn't get function [%s] from [%s]\n", GETINFO_NAME, sFullName);
                            }
                            dlclose(hLibrary);
                        } else {
                            stdprintf("[DynPopFactory::collectPlugins] Couldn't open so [%s]: %s\n", sFullName, dlerror());
                        }
                    } else {
                        // doesn't end in ".so"
                    }
            } else {
                // not a file or link
            }
            hFileEnt = readdir(hDir);
        }
        closedir(hDir);
    } else {
        stdprintf("[DynPopFactory::collectPlugins] Couldn't open so-path [%s]\n", sPath);
        iResult = -1;
    }
        
    return iResult;
}


//----------------------------------------------------------------------------
// createPopulationByName
//
PopBase *DynPopFactory::createPopulationByName(const std::string sName) {
    PopBase *pPB = NULL;

    stringmap::const_iterator it = m_mNameFiles.find(sName);
    if (it != m_mNameFiles.end()) {
        void *hLibrary = dlopen(it->second.c_str(), RTLD_LAZY);
        if (hLibrary != NULL) {

            createPopFunc pCreatePop = (createPopFunc) dlsym(hLibrary, CREATEPOP_NAME);
            if (pCreatePop != NULL) {
	        // we have to 'inject' the ArrayShare singleton
                ArrayShare *pAS = ArrayShare::getInstance();
                m_vLibHandles.push_back(hLibrary);
                pPB = pCreatePop(pAS, m_pCG, m_pPopFinder, m_iLayerSize, m_apIDG, m_aulState);
            } else {
                stdprintf("Couldn't get function [%s] from [%s]\n", CREATEPOP_NAME, it->second);
                dlclose(hLibrary);
            }
        } else {
            stdprintf("[DynPopFactory::createPopulationByName] couldn't open library [%s] in [%s]\n", sName, it->second);
        }

    } else {
        stdprintf("[DynPopFactory::createPopulationByName] no entry for [%s]\n", sName);
    }
    return pPB;
}





//----------------------------------------------------------------------------
// readPopulation
//
PopBase *DynPopFactory::readPopulation(ParamProvider2 *pPP) {
    PopBase *pPop = NULL;

    pPop = createPopulationByName(pPP->getSelected());
    if (pPop != NULL) {
        int iResult = pPop->readSpeciesData(pPP);   //<-new
        if (iResult != 0) {
            stdprintf("[DynPopFactory::readPopulation} error while reading xml config file\n");
            delete pPop;
            pPop = NULL;
        } else {
           
        }
    } else {
        stdprintf("[DynPopFactory::readPopulation} Couldn't create class [%s]\n", pPP->getSelected());
        
    } 
    return pPop;
}
