#include <omp.h>
#include <cmath>
#include <algorithm>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GeneUtils.h"
#include "SelPair.h"

template<typename T>
const std::string SelPair<T>::asNames[] = { 
    ATTR_SELPAIR_PROB_NAME,};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
SelPair<T>::SelPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_SELPAIR_NAME,sID),
      m_apWELL(apWELL),
      m_pGenome(NULL),
      m_iNumBlocks(0) {
 
    int iNumCells = this->m_pCG->m_iNumCells;

    m_vLocFemalesID = new std::vector<int>[iNumCells];
    m_vLocMalesID = new std::vector<int>[iNumCells];
 
    m_aFLocks = new omp_lock_t[iNumCells];
    m_aMLocks = new omp_lock_t[iNumCells];
    for (int i = 0; i < iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
        omp_init_lock(&m_aMLocks[i]);
    }

    this->m_vNames.push_back(ATTR_SELPAIR_PROB_NAME);
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
SelPair<T>::~SelPair() {
    
    if (m_vLocFemalesID != NULL) {
        delete[] m_vLocFemalesID;
    }

    if (m_vLocMalesID != NULL) {
        delete[] m_vLocMalesID;
    }
    if (m_pPLDistValues != NULL) {
        delete m_pPLDistValues;
    }

    if (m_aFLocks != NULL) {
        delete[] m_aFLocks;
    }
    if (m_aMLocks != NULL) {
        delete[] m_aMLocks;
    }
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int SelPair<T>::initialize(float fT) {
    
    int iResult = 0;

#pragma omp parallel for 
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        m_vLocFemalesID[iC].clear();
        m_vLocMalesID[iC].clear();
    }

   int iFirstAgent = this->m_pPop->getFirstAgentIndex();
   int iLastAgent  = this->m_pPop->getLastAgentIndex();

#pragma omp parallel for 
   for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        this->m_pPop->m_aAgents[iA].m_iMateIndex = -3;
   }


    iResult = findMates();

    return iResult;
}

//-----------------------------------------------------------------------------
// finalize
// reset 
//
template<typename T>
int SelPair<T>::finalize(float fT) {
    
    return 0;

}

//-----------------------------------------------------------------------------
// swap
//
void swap(std::vector<int> &V, int i1, int i2) {
    int iT = V[i1];
    V[i1] = V[i2];
    V[i2] = iT;
}


//-----------------------------------------------------------------------------
// findCompatiblePartner
//
template<typename T>
int SelPair<T>::findCompatiblePartner(int iCur, std::vector<int> &vAvailable) {
    std::vector<double> vAccVals;
    vAccVals.push_back(0);
    ulong *pCurGenome  = &((*m_pGenome)[iCur]);
    for (uint i = 0; i < vAvailable.size(); i++) {
        ulong *pAvailableGenome =  &((*m_pGenome)[iCur]);
        double dGDist = GeneUtils::calcDist(pCurGenome, pAvailableGenome, m_iNumBlocks);
        double dVal = m_pPLDistValues->getVal(dGDist);
        vAccVals.push_back(dVal + vAccVals[i]);
    }

    double dMax = vAccVals.back();
    double dR = this->m_apWELL[omp_get_thread_num()]->wrandr(0,dMax);
    int iChoose = 0;
    while ((iChoose < (int) vAccVals.size()-1) && (dR >= vAccVals[iChoose+1])) {
        iChoose++;
    }
    iChoose--;
    return iChoose;
}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T>
int SelPair<T>::findMates() {

    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();



#pragma omp parallel 
    {
#pragma omp for
        for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {

            T* pA = &(this->m_pPop->m_aAgents[iA]);
            
            if (pA->m_iLifeState > 0) {

                int iC = pA->m_iCellIndex;

                if (pA->m_iLifeState == LIFE_STATE_FERTILE) {
                    if (pA->m_iGender == 0) { // FEMALE

                        omp_set_lock(&m_aFLocks[iC]);

                        m_vLocFemalesID[iC].push_back(iA);

                        omp_unset_lock(&m_aFLocks[iC]);

                    } else if (pA->m_iGender == 1) { // MALE

                        omp_set_lock(&m_aMLocks[iC]);

                        m_vLocMalesID[iC].push_back(iA);

                        omp_unset_lock(&m_aMLocks[iC]);

                    }
                }
            }
        }


#pragma omp for schedule(static,1)
        for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        
            if (this->m_pPop->getNumAgents(iC) > 1) {
                int iFNum = (int)m_vLocFemalesID[iC].size();
                int iMNum = (int)m_vLocMalesID[iC].size();
    
                if ((iFNum > 0) && (iMNum > 0)) {
                    int iMated = 0;
                    // sort to ensure reproducibility of simulation with same random seeds
                    // this should be faster than using STL sets 
                    // because insertions at the end of an STL vector are fast
                    std::sort(m_vLocFemalesID[iC].begin(),  m_vLocFemalesID[iC].end());
                    std::sort(m_vLocMalesID[iC].begin(), m_vLocMalesID[iC].end()); 
                    
                    // let's pair them up
    
                    if (iFNum <= iMNum) {
                     // indices after iLast are already handled
                        int iLast = iFNum-1;
                        int iF = 0;
                        while ((iF < iFNum) && (iLast >= 0)) {
                        
                            int iChoose = findCompatiblePartner(iF, m_vLocMalesID[iC]);
            
                            if (iChoose > 0) {
                                
                                int iIDF = m_vLocFemalesID[iC][iF];
                                int iIDM = m_vLocMalesID[iC][iChoose];
                                
                                this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                                this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;

				iMated++;
                            }
          
                            swap(m_vLocMalesID[iC], iF, iLast);
                            iLast--;
                            iF++;
                        }
                        
                    } else {
                        
                        // indices after iLast are already handled
                        int iLast = iFNum-1;

		        int iM = 0;
                        while ((iM < iMNum) && (iLast >= 0)) {
                        
                            int iChoose = findCompatiblePartner(iM, m_vLocFemalesID[iC]);
            
                            if (iChoose > 0) {
                                
                                int iIDF = m_vLocFemalesID[iC][iChoose];
                                int iIDM = m_vLocMalesID[iC][iM];
                                
                                this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                                this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;

				iMated++;
                            }
          
                            swap(m_vLocFemalesID[iC], iM, iLast);
                            iLast--;
                            iM++;
                        }


                    } 
		   // printf("C[%d] iF %d, iM %d, mated %d\n", iC, iFNum, iMNum, iMated);
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
//    ATTR_SELPAIR_PROB_NAME
//  and creates a PolyLine
//
template<typename T>
int SelPair<T>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = -1;


    m_pPLDistValues = qdf_createPolyLine(hSpeciesGroup, ATTR_SELPAIR_PROB_NAME);
    if (m_pPLDistValues != NULL) {
        if (m_pPLDistValues->m_iNumSegments == 0) {
            delete m_pPLDistValues;
            m_pPLDistValues = NULL;
        }
        iResult = 0;
    } else {
        LOG_ERROR("[SelPair] couldn't read attribute [%s]", ATTR_SELPAIR_PROB_NAME);
       iResult = -1;
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// writeAttributesQDF
//
template<typename T>
int SelPair<T>::writeAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    iResult = qdf_writePolyLine(hSpeciesGroup, m_pPLDistValues, ATTR_SELPAIR_PROB_NAME);
    if (iResult != 0) {
        printf("Couldn't write polyline [%s] to QDF\n", ATTR_SELPAIR_PROB_NAME.c_str());
    }

 
    return iResult;

}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int SelPair<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = 0;

    const stringmap &mParams = pMC->getAttributes();
    std::string sPolyName = "";
    iResult = getAttributeStr(mParams, ATTR_SELPAIR_PROB_NAME, sPolyName);
    if (iResult == 0) {
        m_pPLDistValues = PolyLine::readFromString(sPolyName);
        
        if (m_pPLDistValues != NULL) {
            iResult = 1;
        }  
    
    }

    return iResult;}




//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool SelPair<T>::isEqual(Action<T> *pAction, bool bStrict) {
    /*@@TODO:compare polylines
    bool bEqual = false;
    SelPair<T>* pA = static_cast<SelPair<T>*>(pAction);
    if (strcmp(m_pPLDistValues, pA->m_pPLDistValues) == 0) {
        bEqual = true;
    }
    */
    bool bEqual = true;
    return bEqual;
}





