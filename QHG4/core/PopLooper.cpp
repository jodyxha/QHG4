#include <cstdio>
#include <omp.h>

#include "types.h"
#include "xha_strutilsT.h"
#include "PopBase.h"
#include "PopLooper.h"


//----------------------------------------------------------------------------
// constructor
//
PopLooper::PopLooper() 
    : m_iMaxID(0),
      m_iCurIndex(0) {
    


}


//----------------------------------------------------------------------------
// destructor
//
PopLooper::~PopLooper() {
    popmap::iterator it;
    for (it = m_mP.begin(); it != m_mP.end(); ++it) {
        delete it->second;
    }

    for (uint i = 0; i < m_vExtinctPops.size(); i++) {
        delete m_vExtinctPops[i];
    }
}

/*
//----------------------------------------------------------------------------
// checkMerge
//  checks if the population pPop can be merged with an already present one
//
int PopLooper::checkMerge(PopBase *pPop) {
    int iResult = -1;

    popmap::iterator it;
    for (it = m_mP.begin(); (iResult < 0) && (it != m_mP.end()); ++it) {
        iResult = it->second->mergePop(pPop);
    }
    return iResult;
}
*/

//----------------------------------------------------------------------------
// tryMerge
//  checks if the population pPop can be merged with an already present one
//
int PopLooper::tryMerge() {
    int iResult = -1;
    
    std::vector<int> vMerged;
    popmap::iterator it1;
    for (it1 = m_mP.begin(); (iResult < 0) && (it1 != m_mP.end()); ++it1) {
        popmap::iterator it2=it1;
        it2++;
        for (; (iResult < 0) && (it2 != m_mP.end()); ++it2) {
            iResult = it1->second->mergePop(it2->second);
            if (iResult == 0) {
	        xha_printf("[PopLooper] merged pop #%di (%s)\n", it2->first,it2->second->getSpeciesName());
                vMerged.push_back(it2->first);

                // adjust highest id used
                idtype iCurMaxID = it1->second->getMaxLoadedID();
                if (iCurMaxID > m_iMaxID) {
                    m_iMaxID = iCurMaxID;
                }

            }
        }
    }


    // now remove the superfluous
    printf("Have to remove %zd pops\n", vMerged.size());
    for (uint i = 0; i < vMerged.size(); i++) {
        printf("removing pop #%d\n", vMerged[i]);
        PopBase *pP = removePopByIndex(vMerged[i], false);  // false: do not add to extincts
        delete pP;
    }
    return  vMerged.size();
}


//----------------------------------------------------------------------------
// addPop
//  calls the population's setPrioList() method
//  collects the prio levels
//  and adds it to the vector
//
int PopLooper::addPop(PopBase *pPop) {
    int iResult = 0;
 
    iResult = pPop->setPrioList();
    if (iResult == 0) {
        pPop->getPrios(m_vPrioLevels);
        m_mP[m_iCurIndex] = pPop;
        m_iCurIndex++;
        xha_printf("[PopLooper::addPop] added pop [%s](%p)\n", pPop->getSpeciesName(), pPop);
        idtype iCurMaxID = pPop->getMaxLoadedID();
        if (iCurMaxID > m_iMaxID) {
            m_iMaxID = iCurMaxID;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// preLoop
//  call the preLoop method of all populations
//
int PopLooper::preLoop() {
    int iResult = 0;
    // before step: call initializeStep for all pops
    popmap::iterator it;
    for (it = m_mP.begin(); it != m_mP.end(); ++it) {
        iResult += it->second->preLoop();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// postLoop
//  call the preLoop method of all populations
//
int PopLooper::postLoop() {
    int iResult = 0;
    // before step: call initializeStep for all pops
    popmap::iterator it;
    for (it = m_mP.begin(); it != m_mP.end(); ++it) {
        iResult += it->second->postLoop();
    }
    return iResult;
}

//----------------------------------------------------------------------------
// preWrite
//  call the preWrite method of all populations
//
int PopLooper::preWrite(float fTime) {
    int iResult = 0;
    // before step: call initializeStep for all pops
    popmap::iterator it;
    for (it = m_mP.begin(); it != m_mP.end(); ++it) {
        iResult += it->second->preWrite(fTime);
    }
    return iResult;
}



//----------------------------------------------------------------------------
// doStep
//  this method calls PopBase methods in the required order to perform
//  a single step for all poulations in the population vector
//
int PopLooper::doStep(float fStep) {
    int iResult = 0;
    // before step: call initializeStep for all pops
    popmap::iterator it;
    for (it = m_mP.begin(); it != m_mP.end(); ++it) {
        iResult += it->second->initializeStep(fStep);
    }

    // loop through prio levels and execute every population's
    // functions for this level in each cell
    std::set<uint>::iterator itp;
    // all priority levels
    double dTime = omp_get_wtime();

    for (itp = m_vPrioLevels.begin(); itp != m_vPrioLevels.end(); itp++) {

        // all populations
        for (it = m_mP.begin(); it != m_mP.end(); ++it) {
            
            iResult += it->second->doActions(*itp, fStep);
        }
    }

    dTimeActions += omp_get_wtime() - dTime;
    dTime = omp_get_wtime();

    // end of step: call finalizeStep for all pops
    for (it = m_mP.begin(); it != m_mP.end(); ++it) {

        iResult += it->second->finalizeStep();
    }
    
    dTimeFinalize += omp_get_wtime() - dTime;

    return iResult;

}


//----------------------------------------------------------------------------
// removePopByID
//  search for population with given name
//  if found, erase it from vector, optionally add it to extinct vector
//  and return it
//
PopBase *PopLooper::removePopByName(const std::string sSpeciesName, bool bAddToExtinct) {
    PopBase *pPop = NULL;
    popmap::iterator it;
    for (it = m_mP.begin(); (pPop == NULL) && (it != m_mP.end()); ++it) {
        if (it->second->getSpeciesName() ==  sSpeciesName) {
            //            printf("found it\n");
            // don't delete the population
            pPop = it->second;
            // remove it from vector
            m_mP.erase(it);
            
            if (bAddToExtinct) {
                m_vExtinctPops.push_back(pPop);
            }
        }
    }
    return pPop;
}


//----------------------------------------------------------------------------
// removePopByIndex
//  find Population with given PopLooper-Index
//  if found, erase it from vector, optionally add it to extinct vector
//  and return it
//
PopBase *PopLooper::removePopByIndex(int iIndex, bool bAddToExtinct) {
    PopBase *pPop = NULL;
    popmap::iterator it = m_mP.find(iIndex);
    if (it != m_mP.end()) {
        // don't delete the population
        pPop = it->second;
        // remove it from vector
        m_mP.erase(it);

	if (bAddToExtinct) {
            m_vExtinctPops.push_back(pPop);
	}    
    }
    return pPop;

}

/*
//----------------------------------------------------------------------------
// getPopByID
//   PopFinder implementation
//
PopBase *PopLooper::getPopByID(idtype iSpeciesID) {
    PopBase *pPop = NULL;
    popmap::iterator it;
    for (it = m_mP.begin(); (pPop == NULL) && (it != m_mP.end()); ++it) {
        if (it->second->getSpeciesID() == iSpeciesID) {
            pPop = it->second;
        }
    }
    return pPop;
}
*/

//----------------------------------------------------------------------------
// getPopByName
//   PopFinder implementation
//
PopBase *PopLooper::getPopByName(const std::string sSpeciesName) {
    PopBase *pPop = NULL;
    xha_printf("[PopLooper] Searching for [%s]\n", sSpeciesName);
    popmap::iterator it;
    for (it = m_mP.begin(); (pPop == NULL) && (it != m_mP.end()); ++it) {
        xha_printf("  found [%s]\n", it->second->getSpeciesName());
        if (it->second->getSpeciesName() == sSpeciesName) {
            pPop = it->second;
        }
    }
    return pPop;
}

/*
//----------------------------------------------------------------------------
// updatePopVec
//
void PopLooper::updatePopVec() {
    m_vP.clear();
    popmap::iterator it;
    for (it = m_mP.begin(); it != m_mP.end(); ++it) {
        m_vP.push_back(it->second);
    }
}
*/
