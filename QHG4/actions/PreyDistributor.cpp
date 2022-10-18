#include <cstdio>
#include <cstring>

#include <omp.h>

#include "stdstrutilsT.h"
#include "WELL512.h"
#include "ArrayShare.h"

#include "PreyDistributor.h"

PreyDistributor *PreyDistributor::s_pPD = NULL;

//---------------------------------------------------------------------------
// createInstance
//
PreyDistributor *PreyDistributor::createInstance(int iNumCells, WELL512 **apWELL) {
    if (s_pPD == NULL) {
        s_pPD = new PreyDistributor(iNumCells, apWELL);
    }
    return s_pPD;
}

//---------------------------------------------------------------------------
// getInstance
//
PreyDistributor *PreyDistributor::getInstance() {
    return s_pPD;
}

//---------------------------------------------------------------------------
// freeInstance
//
void PreyDistributor::freeInstance() {
    if (s_pPD != NULL) {
        delete s_pPD;
        s_pPD = NULL;
    }
}


//---------------------------------------------------------------------------
// constructor
//
PreyDistributor::PreyDistributor(int iNumCells, WELL512 **apWELL)
    : m_iNumCells(iNumCells),
      m_apWELL(apWELL),
      m_fLastTime(-1)  {
    
      m_avNum = new std::map<std::string, std::vector<int> >[iNumCells];  
      m_iNumThreads =  1;

      m_iNumThreads = omp_get_max_threads();

      /*
      //@@ not really necessary: maps for statistics
      m_amUnmatchedBadHunting = new std::map<std::string, intset>[m_iNumThreads];
      m_amUnmatchedNoHunting  = new std::map<std::string, intset>[m_iNumThreads];
      */
}


//---------------------------------------------------------------------------
// destructor
//
PreyDistributor::~PreyDistributor() {
    if (m_avNum != NULL) {
        delete[] m_avNum;
    }

    std::map<std::string, assignmentmap* > ::const_iterator itf;
    for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {
        if (itf->second != NULL) {
	    delete[] itf->second;
	}
    }

    /*
    //@@ not really necessary
    delete[] m_amUnmatchedBadHunting;
    delete[] m_amUnmatchedNoHunting;
    */
}


//---------------------------------------------------------------------------
// registerPredator
//   expects shared preyratio array with name "<predname>_prey"
//
//   list of predator's prey species is added to m_mRelation struct
//
int PreyDistributor::registerPredator(const std::string sPredName) {
    int iResult = -1;


    std::string s1 = stdsprintf(ATTR_PD_TEMPLATE_PREY, sPredName);
    preyratio *pPredPrey = (preyratio*) ArrayShare::getInstance()->getArray(s1);
    if (pPredPrey != NULL) {
        stdprintf("[PreyDistributor::registerPredator] adding preyratio array [%s]\n", s1);

        int iNum = ArrayShare::getInstance()->getSize(s1);
        for (int i = 0; i < iNum; ++i) {
            m_mRelations[pPredPrey[i].first].push_back(preyratio(sPredName, pPredPrey[i].second));
        }
        iResult = 0;
    } else {
        stdprintf("[PreyDistributor::registerPredator] couldn't get preyratio array [%s]\n", s1);
    }
    
    // make sure there is an entry for this predator
    m_mbReady[sPredName] = false;
 
    return iResult;
}
    

//---------------------------------------------------------------------------
// getFrequencies
//   fill avNum with the cumulated numbers of predators in each cell
//   e.g.:
//     avNum[iCell][k] = sum_0^k NP(k, iCell)
//   where
//     NP(k, iCell) : number of agents of predatorspecies #k in cell iCell
//
//   expects shared intlist array with name "<predname>_indexes"
//
int PreyDistributor::getFrequencies() {
    int iResult = 0;

    printf("[PreyDistributor::getFrequencies] started\n");
    printf("[PreyDistributor::getFrequencies] doing %zd prey types\n", m_mRelations.size()); 

#pragma omp parallel for reduction(+:iResult)
    for (int iCell = 0;  iCell < m_iNumCells; iCell++) {
        //    for (int iCell = 0; (iResult == 0) && (iCell < m_iNumCells); iCell++) {
        m_avNum[iCell].clear();
        std::map<std::string, relationvec>::const_iterator it;
        for (it = m_mRelations.begin(); (iResult == 0) && (it != m_mRelations.end()); ++it) {
            int iCum = 0;
            relationvec::const_iterator it2;
            for (it2 = it->second.begin(); (iResult == 0) && (it2 != it->second.end()); ++it2) {
                std::string s = stdsprintf(ATTR_PD_TEMPLATE_INDEXES, it2->first);
                if (iCell == 0) {
                    stdprintf("[PreyDistributor::getFrequencies] xxxShare looking at pred array [%s] for prey [%s]\n", s, it->first);
                }
                intlist *pArr = (intlist *)ArrayShare::getInstance()->getArray(s); 
                if (pArr != NULL) {
                    iCum += pArr[iCell].size(); 
                    m_avNum[iCell][it->first].push_back(iCum);
                } else {
                    stdprintf("[PreyDistributor::getFrequencies] required array [%s] not found\n", s);
                    iResult = -1;
                } 
            }
        }
        
        /* debug: show avNum
        if (m_avNum[iCell].size() == 0) {
            printf("Nothing for cell %d\n", iCell);
        } else {
            std::map<std::string, intlist>::const_iterator it2;
            for (it2 = m_avNum[iCell].begin(); it2 != m_avNum[iCell].end(); ++it2) {
                stdprintf("  C%03d[%s]: %zd: ", iCell, it2->first, it2->second.size());
                for (uint i = 0; i < it2->second.size(); i++) {
                    printf(" %d", it2->second[i]);
                }
                printf("\n");
            }
        }
        */
    }
    return iResult;
}


//---------------------------------------------------------------------------
// calcAssignments
//
//   m_Ass[predName][iCell][preyName] : vector of pairs (iPredID, iPreyID)
//                                      iPredID: id of agent type predName
//                                      iPreyID: id of agent type preyName
//   e.g., list of assignments to agents of type predName in cell iCell,
//   listed separately for each prey type.
//   
//   expects shared intlist array with name "<preyname>_indexes"
//   expects shared intlist array with name "<predname>_indexes"
//
int PreyDistributor::calcAssignments() {
    int iResult = 0;

    // properly clean up
    std::map<std::string, assignmentmap* > ::const_iterator itf;
    for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {

#pragma omp parallel for
        for (int iCell = 0; iCell < m_iNumCells; iCell++) {
            itf->second[iCell].clear();
        }
        delete[] itf->second;
        
        std::string s = stdsprintf(ATTR_PD_TEMPLATE_ASSMAP, itf->first);
        ArrayShare::getInstance()->removeArray(s);

    }
    
    m_Ass.clear();
    

    double t0 = omp_get_wtime();
    
    /*
    //@@ not really necessary
#pragma omp parallel
    {
        m_amUnmatchedBadHunting[omp_get_thread_num()].clear();
        m_amUnmatchedNoHunting[omp_get_thread_num()].clear();
    }
    */

    omp_lock_t lock0;
    omp_init_lock(&lock0);
    
    printf("[PreyDistributor::calcAssignments] doing %zd prey types\n", m_mRelations.size()); 
    std::map<std::string, relationvec>::const_iterator it;
    
    for (it = m_mRelations.begin(); (iResult == 0) && (it != m_mRelations.end()); ++it) {
        // get array of prey indexes for current type
        std::string s = stdsprintf( ATTR_PD_TEMPLATE_INDEXES, it->first);
        int iNumPrey = ArrayShare::getInstance()->getSize(s); 
        if (iNumPrey > 0) {
            intlist *pPreyIdx = (intlist *) ArrayShare::getInstance()->getArray(s);
            stdprintf("[PreyDistributor::calcAssignments] xxxShare Getting indexes for prey [%s]: %p\n", s, pPreyIdx);

#pragma omp parallel for
            for (int iCell = 0; iCell < m_iNumCells; iCell++) {
                int iThread = omp_get_thread_num();
                
                intlist &v = m_avNum[iCell][it->first];
                if (v.back() > 0) { 

                    for (uint iPreyIndex = 0; iPreyIndex < pPreyIdx[iCell].size(); iPreyIndex++) {
                        int iPreyID = pPreyIdx[iCell][iPreyIndex];
                        
                        // assign a predator agent to prey #iPreyIndex of species s
                        
                        // pick one from total number of predators
                        int iPredIndex = (int) m_apWELL[iThread]->wrandr(0, v.back());
                        uint k = 0;
                        
                        
                        // find index of pred type in this cell containing picked agent
                        while ((k < v.size()) && (iPredIndex >= v[k])) {
                            k++;
                        }
                        if (k > 0) {
                            iPredIndex -= v[k-1];
                        }
                        
                        // find prey ratio (hunt efficiency) in m_mRelations
                        float fPreyRatio = -1;
                        relationvec &vR = m_mRelations[it->first];
                        for (uint j = 0; (fPreyRatio < 0) && (j < vR.size()); j++) {
                            if (it->second[k].first == vR[j].first) {
                                fPreyRatio = vR[j].second;
                            }    
                        }
                        
                        if (fPreyRatio >= 0) {
                            double dR0 = m_apWELL[iThread]->wrandd();
                            //@@@                            stdprintf("[%s]id %d c %d r:%f/%f\n", it->first, pPreyIdx[iCell][iPreyIndex], iCell, dR0, fPreyRatio);
                            
                            if (dR0 < fPreyRatio) {
                                std::string sPredName = stdsprintf(ATTR_PD_TEMPLATE_INDEXES, it->second[k].first);
                                intlist *pPredIdx = (intlist*)ArrayShare::getInstance()->getArray(sPredName);
                                
                                if (pPredIdx != NULL) {
                                    if (pPredIdx[iCell].size() > 0) {
                                        // if no assignment has been made for this predator type,
                                        // we need to create array (ATTENTION more than one thread might do this)
                                        std::map<std::string, assignmentmap* > ::const_iterator itg;
                                        itg = m_Ass.find(it->second[k].first);
                                        if (itg == m_Ass.end()) {
                                            // this block only happens a few times (mostly at the beginning)

                                            omp_set_lock(&lock0); 
                                            // repeat the find() in case somthing has changed in the mean time
                                            itg = m_Ass.find(it->second[k].first);
                                            if (itg == m_Ass.end()) {

                                                m_Ass[it->second[k].first] = new assignmentmap[m_iNumCells];
                                                //                                                stdprintf("created for pred[%s]: %p (T:%f)\n", it->second[k].first, m_Ass[it->second[k].first], m_fLastTime);

                                            }
                                            omp_unset_lock(&lock0);
                                        }

                                        // get id from index
                                        int iPredID = pPredIdx[iCell][iPredIndex];
                                        // do the assignment
                                        m_Ass[it->second[k].first][iCell][it->first].insert(intpair(iPredID, iPreyID));
                                        //                                        stdprintf("added (%d,%d) to ass[%s][%d][%s]\n", iPredID, iPreyID, it->second[k].first, iCell, it->first);
                                    } else {
                                        // shouldn't happen
                                        stdprintf("o-oh: no agents of [%s] in cell %d\n", it->second[k].first, iCell);
                                        iResult = -1;
                                    }
                                } else {
                                    // shouldn't happen
                                    stdprintf("o-oh: array [%s] not found\n", sPredName);
                                    iResult = -1;
                                }
                            } else {
                                //stdprintf("Failed hunt of [%s] on [%s] in cell %d\n", it->second[k].first, it->first, iCell); 
                                //@@ not really necessary
                                // for statistics: number of failed hunts
                                // m_amUnmatchedBadHunting[iThread][it->first].insert(iPreyID);
                            }
                        } else {
                            // shouldn't happen
                            stdprintf("o-oh: ratio for pred [%s] prey [%s] not found\n", it->second[k].first,it->first);
                            iResult = -1;
                        }
                    }
                } else {
                    /* debug: report unmatched
                    stdprintf("Cell %d: unmatched [%s]: ", iCell, it->first);
                    for (uint i = 0; i < v.size(); ++i) {
                        stdprintf(" %d", v[i]);
                    }
                    printf("\n");
                    */
                    //@@ not really necessary
                    // for statistics: number of preys in predator-free cells
                    //                    m_amUnmatchedNoHunting[iThread][it->first].insert(iPreyID);
                }  
                
            }
        } else {
            stdprintf("No prey indexes [%s]\n", s);
            iResult = -1;
        }
    }

    omp_destroy_lock(&lock0);
    
    printf("Used %fs\n", omp_get_wtime() - t0);
    
    return iResult;
}


//---------------------------------------------------------------------------
// buildAssignments
//  cgets frequencies, calculates assigments and shares assignments
//
//   shares assignmentmap arrays with names "<predname>_ass"
//
int PreyDistributor::buildAssignments(const std::string sPredName, float fTime) {
    int iResult = 0;
    m_mbReady[sPredName] = true;
    flagmap::iterator itr;
    bool bReady = true;
    for (itr=m_mbReady.begin(); itr != m_mbReady.end(); itr++) {
        bReady = bReady && itr->second;
    }

    if (bReady && (fTime > m_fLastTime)) {
        for (itr=m_mbReady.begin(); itr != m_mbReady.end(); itr++) {
            itr->second = false;
        }

        m_fLastTime = fTime;

        iResult = getFrequencies();
        if (iResult == 0) {
            iResult = calcAssignments();
         
            if (iResult == 0) {
                // share arrays for each predator
                std::map<std::string, assignmentmap* > ::const_iterator itf;
                for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {
                    std::string s = stdsprintf(ATTR_PD_TEMPLATE_ASSMAP, itf->first);
                    /*
                    assignmentmap* pAss = (assignmentmap *)ArrayShare::getInstance()->getArray(s);
                    if (pAss != NULL) {
                        stdprintf("c deleted for pred[%s]: %p\n", itf->first, pAss);
                        for (int i  = 0; i < m_iNumCells; i++) {
                            pAss[i].clear();
                        }
                        delete[] pAss;
                    }
                    */
                    stdprintf("[PreyDistributor::buildAssignments] xxShare sharing array for [%s]:%p\n", s, itf->second);
                    ArrayShare::getInstance()->removeArray(s);
                    ArrayShare::getInstance()->shareArray(s, m_iNumCells, itf->second);
                }
            }   
        }
    }

    return iResult;
}


//---------------------------------------------------------------------------
// showRelations
//
void PreyDistributor::showRelations() {
   std::map<std::string, relationvec>::const_iterator it;
   for (it = m_mRelations.begin(); it != m_mRelations.end(); it++) {
       stdprintf("  %s : ", it->first);
       relationvec::const_iterator it2;
       for (it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
       	   stdprintf(" %s:%.02f", it2->first, it2->second);
       }
       stdprintf("\n");
   }
}


//---------------------------------------------------------------------------
// showFrequencies
//
void PreyDistributor::showFrequencies() {

    for (int iCell = 0; iCell < m_iNumCells; iCell++) {
        stdprintf("C%02d:\n", iCell);
        std::map<std::string, intlist>::const_iterator it;
	for (it = m_avNum[iCell].begin(); it != m_avNum[iCell].end(); ++it) {
	    stdprintf("  [%s]: ", it->first);
	    for (uint j = 0; j < m_avNum[iCell][it->first].size(); j++) {
	        stdprintf(" %d", m_avNum[iCell][it->first][j]);
	    }
	    stdprintf("\n");
	}
    }
}


//---------------------------------------------------------------------------
// showAssignments
//
void PreyDistributor::showAssignments() {

    std::map<std::string, assignmentmap* > ::const_iterator itf;
    for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {
        stdprintf("assignment [%s]\n", itf->first);
        for (int iCell = 0; iCell < m_iNumCells; iCell++) {
	    stdprintf("  C%02d\n", iCell);
            assignmentmap::const_iterator ita;
            for (ita = itf->second[iCell].begin(); ita != itf->second[iCell].end(); ++ita) {
                stdprintf("     [%s]: ", ita->first);
                std::set<intpair>::const_iterator itp;
                for (itp = ita->second.begin(); itp != ita->second.end(); ++itp) {
                    stdprintf(" (%d,%d)", itp->first, itp->second);
                }
                ///		for (uint j = 0; j < ita->second.size(); j++) {
                    ///                    printf(" (%d,%d)", ita->second[j].first, ita->second[j].second);
                    ///		}
		stdprintf("\n");
	    }
	}
    }
}

//---------------------------------------------------------------------------
// showAgentAssignments
//
void PreyDistributor::showAgentAssignments(int iCellID, const std::string sPredName, int iAgentIndex) {
    assignmentmap mAss = m_Ass[sPredName][iCellID];
    stdprintf("Assignments to id %d of species [%s] (cell %d)\n", iAgentIndex, sPredName, iCellID); 
    assignmentmap::const_iterator ita;
    for (ita = mAss.begin(); ita != mAss.end(); ++ita) {
        stdprintf("     [%s]: ", ita->first);
        std::set<intpair>::const_iterator itp;
        for (itp = ita->second.begin(); itp != ita->second.end(); ++itp) {
            if  (itp->first == iAgentIndex) {
                stdprintf(" %d", itp->second);
            }
            //        for (uint j = 0; j < ita->second.size(); j++) {
            //            if  (ita->second[j].first == iAgentIndex) {
            //                printf(" %d", ita->second[j].second);
            //            }
        }
        stdprintf("\n");
    }
}


/*
//---------------------------------------------------------------------------
// showUnmatched
//  
//@@ not really necessary
//
void PreyDistributor::showUnmatched() {
    
    // sum "over threads"
    for (int i = 1; i < m_iNumThreads; i++) {
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedBadHunting[i].begin(); it0 != m_amUnmatchedBadHunting[i].end(); ++it0) {
            m_amUnmatchedBadHunting[0][it0->first].insert(it0->second.begin(), it0->second.end());
        }
    }

    for (int i = 1; i < m_iNumThreads; i++) {
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedNoHunting[i].begin(); it0 != m_amUnmatchedNoHunting[i].end(); ++it0) {
            m_amUnmatchedNoHunting[0][it0->first].insert(it0->second.begin(), it0->second.end());
        }
    }


    if (m_amUnmatchedBadHunting[0].size() > 0) {
        printf("Bad Hunting\n");
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedBadHunting[0].begin(); it0 != m_amUnmatchedBadHunting[0].end(); ++it0) {
            stdprintf("  [%s]\n", it0->first);
            intset::const_iterator it1;
            for (it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
                printf(" %d", *it1);
            }
            printf("\n");
        }
    }

    if (m_amUnmatchedNoHunting[0].size() > 0) {
        printf("No Hunting\n");
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedNoHunting[0].begin(); it0 != m_amUnmatchedNoHunting[0].end(); ++it0) {
            stdprintf("  [%s]\n", it0->first);
            intset::const_iterator it1;
            for (it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
                printf(" %d", *it1);
            }
            printf("\n");
        }
    }
    
}
*/
