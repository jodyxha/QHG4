#include <omp.h>
#include <cmath>

#include "MessLoggerT.h"

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "WELL512.h"
#include "WELLUtils.h"  //@@debugging
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "PersHybLinearBirthRel.h"

template<typename T>
const char *PersHybLinearBirthRel<T>::asNames[] = {
    ATTR_PERSHYBLINBIRTHREL_HYBMINPROB_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PersHybLinearBirthRel<T>::PersHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths) 
    : Action<T>(pPop,pCG,ATTR_PERSHYBLINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dHybMinProb(1.0),
      m_adK(NULL),
      m_iStride(1),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0),
      m_pNumAgentsPerCell(NULL),
      m_fHybScale(2),
      m_fHybShift(0.5),
      m_adBBase(NULL) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }
    m_adBBase = new double[this->m_pCG->m_iNumCells];
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// constructor for use with VerhulstVarK action
//
template<typename T>
PersHybLinearBirthRel<T>::PersHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double dHybMinProb, double* adK, int iStride) 
    : Action<T>(pPop,pCG,ATTR_PERSHYBLINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dHybMinProb(dHybMinProb),
      m_adK(adK), 
      m_iStride(iStride),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0) ,
      m_pNumAgentsPerCell(NULL),
      m_fHybScale(2),
      m_fHybShift(0.5),
      m_adBBase(NULL) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }
    m_adBBase = new double[this->m_pCG->m_iNumCells];


    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PersHybLinearBirthRel<T>::~PersHybLinearBirthRel() {
    if (m_aaiNumTemp != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            if (m_aaiNumTemp != NULL) {
                delete[] m_aaiNumTemp[i];
            }
        }
        delete[] m_aaiNumTemp;
    }
    if (m_adBBase != NULL) {
        delete [] m_adBBase;
    }

    // we don't delete m_aiNumBirths because it comes from outside
}



//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int PersHybLinearBirthRel<T>::preLoop() {
    int iResult = -1;

    m_fHybScale = 2*(1 - m_dHybMinProb);
    m_fHybShift = 1 - (1 - m_dHybMinProb)/2;

    if (m_pNumAgentsPerCell == NULL) {
        m_pNumAgentsPerCell = this->m_pPop->getNumAgentsArray();
    }

    if ((m_adK != NULL) && 
        (m_aiNumBirths != NULL)) {
#pragma omp parallel for
        for (int i = 0; i < m_iNumThreads; i++) {
            memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
        }       
        memset(m_aiNumBirths[0], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        memset(m_aiNumBirths[1], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        memset(m_adBBase, 0, sizeof(double) * (this->m_pCG->m_iNumCells));
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int PersHybLinearBirthRel<T>::postLoop() {
    int iResult = 0;

    return iResult;
 }

//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int PersHybLinearBirthRel<T>::initialize(float fT) {

#pragma omp parallel for
    for (int i = 0; i < m_iNumThreads; i++) {
        memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
    }

    // set 
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
    //#pragma omp parallel for schedule(static,iChunk)
#pragma omp parallel for
    for (uint iC = 0; iC < this->m_pCG->m_iNumCells; iC++) {
        if ( m_adK[iC * m_iStride] <= 0) {
            m_adBBase[iC] = dNaN;
        } else {
            // case with space-dependent K
            m_adBBase[iC] =  this->m_pNumAgentsPerCell[iC] / m_adK[iC * m_iStride]; 
        }
    }
    
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int PersHybLinearBirthRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);
    
    if (pa->m_iLifeState > 0) {
	
    	int iCellIndex = pa->m_iCellIndex;
        int iThread    = omp_get_thread_num();
        // we'll need a random  number in any case
        double dR = this->m_apWELL[iThread]->wrandd();

    	// if reproduction is in couples, use mate index, 
    	// otherwise use agent index also as father
	
        // note: if this doesn't compile, the agent has no field m_iMateIndex, 
        // and therefore the population should not use this action class.
	
    	int iMate = pa->m_iMateIndex;

        
        if (!std::isnan(m_adBBase[iCellIndex])) {
            pa->m_dBReal =  pa->m_dB0 + (pa->m_dTheta - pa->m_dB0) *m_adBBase[iCellIndex];
         
            if (pa->m_dBReal > 0) { // positive birth prob
                if ((pa->m_iGender == 0) && (iMate >= 0)) {
                    // calculate the hybridization factor
                    T *pa2 =  &(this->m_pPop->m_aAgents[iMate]);
                    float h1 = pa->m_fHybridization;
                    float h2 = pa2->m_fHybridization;
                    // this function returns 1 if both h1 and h2 are equal, and minHybProb if h1 = 0 and h2 = 1 and vice versa
                    float fH = (m_fHybShift+m_fHybScale * (h1 - 0.5)*(h2 - 0.5)); 
                    if (dR < pa->m_dBReal*fH) {
                        
                        this->m_pPop->registerBirth(iCellIndex, iAgentIndex, iMate);
                        m_aaiNumTemp[iThread][iCellIndex]++;
                        iResult = 1;
                    } 
                }
            } else if (pa->m_dBReal < 0) {
                
                if (dR < -pa->m_dBReal) { // convert negative birth prob to death prob
                    this->m_pPop->registerDeath(iCellIndex, iAgentIndex);
                }
            }     
    	} else {
            pa->m_dBReal  = 0;
            // negative K : no births
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//
template<typename T>
int PersHybLinearBirthRel<T>::finalize(float fT) {

#pragma omp parallel for
    for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
        m_aiNumBirths[m_iWhich][i] = 0;
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            m_aiNumBirths[m_iWhich][i] += m_aaiNumTemp[iT][i];
        }
    }
    m_iWhich = 1 - m_iWhich;
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_PERSLINBIRTHREL_HYBMINPROB_NAME
//
template<typename T>
int PersHybLinearBirthRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PERSHYBLINBIRTHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
        if (iResult != 0) {
            LOG_ERROR("[PersHybLinearBirthRel] couldn't read attribute [%s]", ATTR_PERSHYBLINBIRTHREL_HYBMINPROB_NAME);
        }
    }
    
    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_PERSLINBIRTHREL_HYBMINPROB_NAME
//
template<typename T>
int PersHybLinearBirthRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_PERSHYBLINBIRTHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int PersHybLinearBirthRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_PERSHYBLINBIRTHREL_HYBMINPROB_NAME) {
        m_dHybMinProb = dValue;
    } else {
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int PersHybLinearBirthRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_PERSHYBLINBIRTHREL_HYBMINPROB_NAME, &m_dHybMinProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool PersHybLinearBirthRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    PersHybLinearBirthRel<T>* pA = static_cast<PersHybLinearBirthRel<T>*>(pAction);
    if  (m_dHybMinProb == pA->m_dHybMinProb) {
        bEqual = true;
    } 
    return bEqual;
}

