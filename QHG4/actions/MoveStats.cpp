#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "geomutils.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "MoveStats.h"

template<typename T>
const char *MoveStats<T>::asNames[] = {
    ATTR_MOVESTATS_MODE_NAME};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
MoveStats<T>::MoveStats(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID) 
    : Action<T>(pPop,pCG,ATTR_MOVESTATS_NAME,sID),
      m_iNumCells(pCG->m_iNumCells),
      m_iMode(MoveStats::MODE_STAT_NONE),
      m_aiHops(NULL),
      m_adDist(NULL),
      m_adTime(NULL),
      m_aiHopsTemp(NULL),
      m_adDistTemp(NULL),
      m_adTimeTemp(NULL),
      m_asChanged(NULL),
      m_vMoveList(pPop->getMoveList()),
      m_fCalcDist(NULL),
      m_dDistScale(0)  {


    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

    if (pCG->m_pGeography != NULL) {
        if (pCG->isCartesian()) {
            m_fCalcDist = &cartdist;
            m_dDistScale = 1.0;
        } else { 
            m_fCalcDist = &spherdistDeg;
            m_dDistScale = pCG->m_pGeography->m_dRadius;
        }
    }
}


//-----------------------------------------------------------------------------
// destructor
//  delete all allocated arrays
//
template<typename T>
MoveStats<T>::~MoveStats() {

    if (m_aiHops != NULL) {
        delete[] m_aiHops;
    }

    if (m_adDist != NULL) {
        delete[] m_adDist;
    }

    if (m_adTime != NULL) {
        delete[] m_adTime;
    }

    if (m_aiHopsTemp != NULL) {
        for (int iIndex = 0; iIndex < omp_get_max_threads(); iIndex++) {
            delete[] m_aiHopsTemp[iIndex];
        }
        delete[] m_aiHopsTemp;
    }

    if (m_adDistTemp != NULL) {
        for (int iIndex = 0; iIndex < omp_get_max_threads(); iIndex++) {
            delete[] m_adDistTemp[iIndex];
        }
        delete[] m_adDistTemp;
    }

    if (m_adTimeTemp != NULL) {
        for (int iIndex = 0; iIndex < omp_get_max_threads(); iIndex++) {
            delete[] m_adTimeTemp[iIndex];
        }
        delete[] m_adTimeTemp;
    }

    if (m_asChanged != NULL) {
        delete[] m_asChanged;
    }

}

//-----------------------------------------------------------------------------
// preLoop
//  allocate all needed arrays & niitialize them
//
template<typename T>
int MoveStats<T>::preLoop() {

    m_aiHops = new int[m_iNumCells];
    m_adDist = new double[m_iNumCells];
    m_adTime = new double[m_iNumCells];

    memset(m_aiHops, 0xff, m_iNumCells*sizeof(uint));
   
    for (unsigned int k = 0; k < m_iNumCells; k++) {
        m_adDist[k]  =-1;
        m_adTime[k] = -1;
    }

    m_aiHopsTemp = new int*[omp_get_max_threads()];
    m_adDistTemp = new double*[omp_get_max_threads()];
    m_adTimeTemp = new double*[omp_get_max_threads()];

#pragma omp parallel for
    
    for (int iIndex = 0; iIndex < omp_get_max_threads(); iIndex++) {
        m_aiHopsTemp[iIndex] = new int[m_iNumCells];
        memset(m_aiHopsTemp[iIndex], 0xff, m_iNumCells*sizeof(uint));
        
        m_adDistTemp[iIndex] = new double[m_iNumCells];
        for (unsigned int k = 0; k < m_iNumCells; k++) {
            m_adDistTemp[iIndex][k] = -1;
        }   
       
        m_adTimeTemp[iIndex] = new double[m_iNumCells];
        for (unsigned int k = 0; k < m_iNumCells; k++) {
            m_adTimeTemp[iIndex][k] = -1;
        }

    }

    m_asChanged = new intset[omp_get_max_threads()];

    initializeOccupied();

    return 0;
}

//-----------------------------------------------------------------------------
// initializeOccupied
//   Reset the stats for cells that are initially occupied
//
template<typename T>
int MoveStats<T>::initializeOccupied() {
    int iResult = 0;

    bool **aaHasAgents = new bool*[omp_get_max_threads()];
    for (int i = 0; i < omp_get_max_threads(); i++) {
        aaHasAgents[i] = new bool[m_iNumCells];
        memset(aaHasAgents[i], 0, m_iNumCells*sizeof(bool));
    }

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();

    // mark occupied cells

#pragma omp parallel for 
    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        int iT = omp_get_thread_num();
        int iOccCell = this->m_pPop->m_aAgents[iA].m_iCellIndex;
        aaHasAgents[iT][iOccCell] = true;
        
    }

    for (int iT = 0; iT < omp_get_max_threads(); iT++) {
#pragma omp parallel for
        for (uint iCell = 0; iCell < m_iNumCells; iCell++) {
            if (aaHasAgents[iT][iCell]) {
                m_aiHops[iCell] = 0;
                m_adDist[iCell] = 0;
                m_adTime[iCell] = 0;
            }
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//   In SPopulation tge actions' finaize is called before the moves are performed.
//   I.e., the move list is till valid.
//
template<typename T>
int MoveStats<T>::finalize(float fT) {


#pragma omp parallel 
    {
        int iT = omp_get_thread_num();

        m_asChanged[iT].clear();

        for (unsigned int iIndex = 0; iIndex < m_vMoveList[iT]->size(); iIndex += 3) {
            int iCellFrom = (*m_vMoveList[iT])[iIndex];
            int iCellTo = (*m_vMoveList[iT])[iIndex+2];

            // calculate stats for new cell
            int iNewHops = m_aiHops[iCellFrom] + 1;

            double dOldDist = m_adDist[iCellFrom];
            double dLon1 = this->m_pCG->m_pGeography->m_adLongitude[iCellFrom];
            double dLat1 = this->m_pCG->m_pGeography->m_adLatitude[iCellFrom];
            double dLon2 = this->m_pCG->m_pGeography->m_adLongitude[iCellTo];
            double dLat2 = this->m_pCG->m_pGeography->m_adLatitude[iCellTo];
            double dNewDist = dOldDist + m_fCalcDist(dLon1, dLat1, dLon2, dLat2, m_dDistScale);

            double dNewTime = fT;

            if ((m_aiHopsTemp[iT][iCellTo] < 0) || (m_iMode == MoveStats::MODE_STAT_LAST)) {
                // cell has not been touched yet (or we always overwrite) - all values can be set
                m_aiHopsTemp[iT][iCellTo] = iNewHops;
                m_adDistTemp[iT][iCellTo] = dNewDist;
                m_adTimeTemp[iT][iCellTo] = dNewTime;
                m_asChanged[iT].insert(iCellTo);
            } else {
                switch (m_iMode) {
                case MoveStats::MODE_STAT_FIRST:
                    // nothing to do
                    break;
                case MoveStats::MODE_STAT_MIN:
                    if (iNewHops < m_aiHopsTemp[iT][iCellTo]) {
                        m_aiHopsTemp[iT][iCellTo] = iNewHops;
                    }
                    if (dNewDist < m_adDistTemp[iT][iCellTo]) {
                        m_adDistTemp[iT][iCellTo] = dNewDist;
                    }
                    m_adTimeTemp[iT][iCellTo] = dNewTime;
                    m_asChanged[iT].insert(iCellTo);
                    break;
                }
            }

        }
    } // end parallel


    for (int i = 1; i < omp_get_max_threads(); ++i) {
        m_asChanged[0].insert(m_asChanged[i].begin(), m_asChanged[i].end());
    }

    for (int iT = 0; iT < omp_get_max_threads(); ++iT) {
        intset::const_iterator itCell;
        for (itCell = m_asChanged[0].begin(); itCell != m_asChanged[0].end(); ++itCell) {
            if ((m_aiHops[*itCell] < 0) || (m_iMode == MoveStats::MODE_STAT_LAST)) {
                m_aiHops[*itCell] = m_aiHopsTemp[iT][*itCell];
                m_adDist[*itCell] = m_adDistTemp[iT][*itCell];
                m_adTime[*itCell] = m_adTimeTemp[iT][*itCell];
            } else {
                switch (m_iMode) {
                case MoveStats::MODE_STAT_FIRST:
                    // nothing to do
                    break;
                case MoveStats::MODE_STAT_MIN:
                    if (m_aiHopsTemp[iT][*itCell] < m_aiHops[*itCell]) {
                        m_aiHops[*itCell] = m_aiHopsTemp[iT][*itCell];
                        m_asChanged[iT].insert(*itCell);
                    }
                    if (m_adDistTemp[iT][*itCell] < m_adDist[*itCell]) {
                        m_adDist[*itCell] = m_adDistTemp[iT][*itCell];
                        m_asChanged[iT].insert(*itCell);
                    }
                    if (m_adTimeTemp[iT][*itCell] < m_adTime[*itCell]) {
                        m_adTime[*itCell] = m_adTimeTemp[iT][*itCell];
                        m_asChanged[iT].insert(*itCell);
                    }
                    break;
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
//    ATTR_MOVESTATS_NUMCELLS_NAME
//    ATTR_MOVESTATS_MODE_NAME
//
template<typename T>
int MoveStats<T>::extractAttributesQDF(hid_t hSpeciesActionGroup) {
    
     int iResult = 0;


     /*
     if (iResult == 0) {
         uint iTempNum = 0;
         iResult = qdf_extractAttribute(hSpeciesActionGroup, ATTR_MOVESTATS_NUMCELLS_NAME, 1, &iTempNum);
         if (iResult == 0) {
             if ((iTempNum == m_iNumCells) || (m_iNumCells == 0)) {
                 m_iNumCells = iTempNum;
             } else {
                 LOG_ERROR("[MoveStats] loaded number of cells does not match current number of cells");
                 iResult = -1;
             }
         } else {
             LOG_ERROR("[MoveStats] couldn't read attribute [%s]", ATTR_MOVESTATS_NUMCELLS_NAME);
         }
     }
     */
     if (iResult == 0) {
         iResult = qdf_extractAttribute(hSpeciesActionGroup, ATTR_MOVESTATS_MODE_NAME, 1, &m_iMode);
         if (iResult != 0) {
             LOG_ERROR("[MoveStats] couldn't read attribute [%s]", ATTR_MOVESTATS_MODE_NAME);
         }
     }
    
     return iResult; 
}

//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_MOVESTATS_NUMCELLS_NAME
//    ATTR_MOVESTATS_MODE_NAME
//
template<typename T>
int MoveStats<T>::writeAttributesQDF(hid_t hSpeciesActionGroup) {
   
printf("[MoveStats<T>::writeAttributesQDF] starting\n");
    int iResult = 0;
    
//    iResult += qdf_insertAttribute(hSpeciesActionGroup, ATTR_MOVESTATS_NUMCELLS_NAME,  1, &m_iNumCells);
    iResult += qdf_insertAttribute(hSpeciesActionGroup, ATTR_MOVESTATS_MODE_NAME,      1, &m_iMode);
    
printf("[MoveStats<T>::writeAttributesQDF] finishing with %d\n", iResult);
    return iResult; 
}

//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int MoveStats<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    
    int iResult = 0;
    if (sAttrName == ATTR_MOVESTATS_MODE_NAME) {
        m_iMode = static_cast<int>( dValue);
    } else {
        iResult = -1;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
template<typename T>
int MoveStats<T>::writeAdditionalDataQDF(hid_t hSpeciesActionGroup) {
    int iResult = -2;
printf("[MoveStats<T>::writeAdditionalDataQDF] starting\n");
    iResult = qdf_writeArray(hSpeciesActionGroup, MOVESTAT_DS_HOPS, m_iNumCells, m_aiHops, H5T_NATIVE_INT);
    if (iResult != 0) {
        LOG_ERROR("[MoveStats<T>::writeAdditionalDataQDF] couldn't write hops data");
    } else {
        iResult = qdf_writeArray(hSpeciesActionGroup, MOVESTAT_DS_DIST, m_iNumCells, m_adDist, H5T_NATIVE_DOUBLE);
        if (iResult != 0) {
            LOG_ERROR(" MoveStats<T>::writeAdditionalDataQDF] couldn't write dist array");
        } else {
            iResult = qdf_writeArray(hSpeciesActionGroup, MOVESTAT_DS_TIME, m_iNumCells, m_adTime, H5T_NATIVE_DOUBLE);
            if (iResult != 0) {
                LOG_ERROR("[MoveStats<T>::writeAdditionalDataQDF] couldn't write time array data");
            }
        }
    }
printf("[MoveStats<T>::writeAdditionalDataQDF] finishing with %d\n", iResult);
    return iResult;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//  MaxAmount should be handled by vegetation (mis-use NPP)
//  Actual amount should be saved
//
template<typename T>
int MoveStats<T>::readAdditionalDataQDF(hid_t hSpeciesActionGroup) {
    int iResult = -1;
    
    iResult = qdf_readArray(hSpeciesActionGroup, MOVESTAT_DS_HOPS, m_iNumCells, m_aiHops);
    if (iResult == 0) {
        iResult = qdf_readArray(hSpeciesActionGroup, MOVESTAT_DS_DIST, m_iNumCells, m_adDist);
        if (iResult == 0) {
            iResult = qdf_readArray(hSpeciesActionGroup, MOVESTAT_DS_TIME, m_iNumCells, m_adTime);
            if (iResult == 0) {
                // success
            } else  {
                LOG_ERROR("[MoveStats<T>::readAdditionalDataQDF] couldn't read time array");
            }
        } else  {
            LOG_ERROR("[MoveStats<T>::readAdditionalDataQDF] couldn't read dist array");
        }
    } else  {
        LOG_ERROR("[MoveStats<T>::readAdditionalDataQDF] couldn't read hops array");
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int MoveStats<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
//        uint iTempNum = 0;
//        iResult += getAttributeVal(mParams, ATTR_MOVESTATS_NUMCELLS_NAME, &iTempNum); 
        iResult += getAttributeVal(mParams, ATTR_MOVESTATS_MODE_NAME,     &m_iMode); 
/*
        if ((iResult == 0) &&((iTempNum == m_iNumCells) || (m_iNumCells == 0))) {
            m_iNumCells = iTempNum;
        } else {
            LOG_ERROR("[MoveStats] loaded number of cells does not match current number of cells");
            iResult = -1;
        }
*/
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool MoveStats<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    MoveStats<T>* pA = static_cast<MoveStats<T>*>(pAction);
    if ((m_iNumCells == pA->m_iNumCells) &&
        (m_iMode     == pA->m_iMode)) {
        bEqual = true;
    } 
    return bEqual;
}

