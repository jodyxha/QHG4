#include <cstdio>
#include <omp.h>

#include "MessLoggerT.h"
#include "EventConsts.h"
#include "WELL512.h"

#include "Action.h"
#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "Navigate2.h"


template<typename T>
const std::string Navigate2<T>::asNames[] = {
    ATTR_NAVIGATE2_DECAY_NAME,
    ATTR_NAVIGATE2_DIST0_NAME,
    ATTR_NAVIGATE2_PROB0_NAME,
    ATTR_NAVIGATE2_MINDENS_NAME,
    ATTR_NAVIGATE2_BRIDGE_PROB_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Navigate2<T>::Navigate2(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_NAVIGATE2_NAME,sID),
      m_apWELL(apWELL),
      m_dDecay(0),
      m_dDist0(0),
      m_dProb0(0),
      m_dMinDens(0),
      m_dA(0),
      m_bNeedUpdate(true),
      m_dBridgeProb(0),
      m_pGeography(pCG->m_pGeography),
      m_pNavigation2(pCG->m_pNavigation2) {

    pPop->addObserver(this);

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Navigate2<T>::~Navigate2() {
    // delete targdist arrays in map
    cleanup();

}


//-----------------------------------------------------------------------------
// cleanup
//
template<typename T>
void Navigate2<T>::cleanup() {
    jumpprobmap::iterator it;
    for (it = m_mJumpProbs.begin(); it != m_mJumpProbs.end(); ++it) {
        delete[] it->second.second;
    }
    m_mJumpProbs.clear();

    m_vCurBridges.clear();
}

//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void Navigate2<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if ((iEvent == EVENT_ID_GEO) || (iEvent == EVENT_ID_NAV)) { 
        m_bNeedUpdate = true;

    } else if (iEvent == EVENT_ID_FLUSH) {
        // no more events: recalculate
        recalculate();
    }
}


//-----------------------------------------------------------------------------
// recalculate
//
template<typename T>
void Navigate2<T>::recalculate() {

    if (m_bNeedUpdate) {
        printf("Navigate::recalculate\n");
        m_dA = m_dProb0/(exp(m_dDecay*m_dDist0));

        cleanup();
        int iResult = 0;
        waterwaymap::const_iterator it;
        Navigation2 *pNav2 = m_pNavigation2; 

        // HERE WE NEED NAVIGATION2 SPECIFIC CALCULATIONS

        for (it = pNav2->m_mWaterWays.begin(); (iResult == 0) && (it != pNav2->m_mWaterWays.end()); ++it) {
            targprob *pTP = new targprob[it->second.size()+1];
            destdistmap::const_iterator itl;

            // collect probabilitoes and sum them
            double dProbSum = 0;
            int i = 1;
            for (itl = it->second.begin(); itl != it->second.end(); ++itl) {
                double dProb = m_dA*exp(m_dDecay*itl->second);
                dProbSum += dProb;
                pTP[i++]=targprob(itl->first, dProb);
            }
            if (dProbSum < 1) {
                // -1: stay at home
                pTP[0] = targprob(-1, 1-dProbSum);

                m_mJumpProbs[it->first] = std::pair<int, targprob*>(it->second.size(), pTP);

                // accumulate probabilities
                for (uint j = 1; j < it->second.size()+1; j++) {
                    pTP[j].second += pTP[j-1].second;
                }
            } else {
                printf("[Navigate] probabilities for port [%d] add up to %f\n",  it->first, dProbSum);fflush(stdout);
                LOG_ERROR("[Navigate] probabilities for port [%d] add up to %f\n",  it->first, dProbSum);
                iResult = -1;
            }
        }

        // now manual bridges
        double *pAlt = m_pGeography->m_adAltitude;
        bridgelist::const_iterator itb;
        for (itb = pNav2->m_vBridges.begin(); itb != pNav2->m_vBridges.end(); ++itb) {
            if ((pAlt[itb->first] > 0) && (pAlt[itb->second] > 0)) {
                m_vCurBridges.push_back(*itb);
            }
        }

        m_bNeedUpdate = false;
    }    
}


//-----------------------------------------------------------------------------
// preloop
//
template<typename T>
int Navigate2<T>::preLoop() {
    int iResult = 0;
    // we need 
    if ((m_pGeography != NULL) && 
        (m_pNavigation2 != NULL)) {
        cleanup();
        recalculate();
    } else {
        iResult = -1;
        if (m_pGeography != NULL) {
            printf("[Navigate2] m_pGeography is NULL!\n");
            LOG_ERROR("[Navigate2] m_pGeography is NULL!\n");
            printf("  Make sure your gridfile has a geography group!\n");
        }
        if (m_pNavigation2 != NULL) {
            printf("[Navigate2] m_pNavigation2 is NULL!\n");
            LOG_ERROR("[Navigate2] m_pNavigation2 is NULL!\n");
            printf("  Make sure your gridfile has a navigation2 group!\n");
        }
    }


    return iResult;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int Navigate2<T>::execute(int iAgentIndex, float fT) {

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

    // we only ship alive and agents not already moving
    if ((pa->m_iLifeState > 0) && ((pa->m_iLifeState & LIFE_STATE_MOVING) == 0)) {

        int iThread = omp_get_thread_num();
        int iCellIndex = pa->m_iCellIndex;

        jumpprobmap::const_iterator it = m_mJumpProbs.find(iCellIndex);
        if (it != m_mJumpProbs.end()) {

            int iNumDests = it->first;
        
            double dR =  m_apWELL[iThread]->wrandd();
        
            int i = 0;
            /*@@@@ old
              while ((i < iNumDests) && (dR > it->second.second[i].second)) {
              dR -= it->second.second[i++].second;
              }
              old @@@*/
            while ((i < iNumDests) && (dR > it->second.second[i].second)) {
                i++;
            }


            // i = 0 means we hit the 'stay home' region
            if (i > 0) {
                int iNewCellIndex = it->second.second[i].first;
                if ((m_pGeography == NULL) || (!m_pGeography->m_abIce[iNewCellIndex])) {
                    
                    if (iNewCellIndex < 0) {
                        printf("[Navigate<T>::execute] Attention: agent %d has cellindex %d\n", iAgentIndex, iNewCellIndex);
                    }
                    this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                }
            }


        }
    

        // bridges can also only crossed by live agents which are not already on the move
        bridgelist::const_iterator itb;
        for (itb = m_vCurBridges.begin(); itb != m_vCurBridges.end(); ++itb) {
            int iNewCellIndex = -1;
            if (itb->first == iCellIndex) {
                iNewCellIndex = itb->second;
                //                printf("[Navigate<T>::execute] have candidate at first\n");
            } else if (itb->second == iCellIndex) {
                iNewCellIndex = itb->first;
                //                printf("[Navigate<T>::execute] have candidate at second\n");
            } else {
                iNewCellIndex = -1;
            }
        
            if (iNewCellIndex >= 0) {
                double dR = m_apWELL[iThread]->wrandd();
                if (dR < m_dBridgeProb) {
                    this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                    //printf("[Navigate<T>::execute] Sent %d over bridge from %d to %d at T%f\n", iAgentIndex, iCellIndex, iNewCellIndex, fT);
                }
            }
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_NAVIGATE2_DECAY_NAME
//    ATTR_NAVIGATE2_DIST0_NAME
//    ATTR_NAVIGATE2_PROB0_NAME
//    ATTR_NAVIGATE2_MINDENS_NAME
//    ATTR_NAVIGATE2_BRIDGE_PROB_NAME
//
template<typename T>
int Navigate2<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE2_DECAY_NAME, 1, &m_dDecay);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE2_DECAY_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE2_DIST0_NAME, 1, &m_dDist0);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE2_DIST0_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE2_PROB0_NAME, 1, &m_dProb0);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE2_PROB0_NAME);
        }
    }


    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE2_MINDENS_NAME, 1, &m_dMinDens);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE2_MINDENS_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE2_BRIDGE_PROB_NAME, 1, &m_dBridgeProb);
        if (iResult != 0) {
            m_dBridgeProb = 0;
            iResult = 0;
        }
    }

    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_NAVIGATE2_DECAY_NAME
//    ATTR_NAVIGATE2_DIST0_NAME
//    ATTR_NAVIGATE2_PROB0_NAME
//    ATTR_NAVIGATE2_MINDENS_NAME
//    ATTR_NAVIGATE2_BRIDGE_PROB_NAME
//
template<typename T>
int Navigate2<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE2_DECAY_NAME,   1, &m_dDecay);

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE2_DIST0_NAME,   1, &m_dDist0);

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE2_PROB0_NAME,   1, &m_dProb0);

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE2_MINDENS_NAME, 1, &m_dMinDens);

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE2_BRIDGE_PROB_NAME, 1, &m_dBridgeProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int Navigate2<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams,  ATTR_NAVIGATE2_DECAY_NAME,   &m_dDecay);  
        iResult += getAttributeVal(mParams,  ATTR_NAVIGATE2_DIST0_NAME,   &m_dDist0);  
        iResult += getAttributeVal(mParams,  ATTR_NAVIGATE2_PROB0_NAME,   &m_dProb0);  
        iResult += getAttributeVal(mParams,  ATTR_NAVIGATE2_MINDENS_NAME, &m_dMinDens);

        int iRes2 = getAttributeVal(mParams, ATTR_NAVIGATE2_BRIDGE_PROB_NAME, &m_dBridgeProb);
        if (iRes2 != 0) {
            //no problem; use 0
        }

    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool Navigate2<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Navigate2<T>* pA = static_cast<Navigate2<T>*>(pAction);
    if ((m_dDecay   == pA->m_dDecay) &&
        (m_dDist0   == pA->m_dDist0) &&
        (m_dProb0   == pA->m_dProb0) &&
        (m_dMinDens == pA->m_dMinDens) &&
        (m_dBridgeProb == pA->m_dBridgeProb)) {

        bEqual = true;
    } 
    return bEqual;
}

