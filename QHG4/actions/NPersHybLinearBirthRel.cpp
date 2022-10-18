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
#include "NPersHybLinearBirthRel.h"

template<typename T>
const char *NPersHybLinearBirthRel<T>::asNames[] = {
    ATTR_NPERSHYBLINBIRTHREL_HYBMINPROB_NAME,
};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
NPersHybLinearBirthRel<T>::NPersHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths) 
    : Action<T>(pPop,pCG,ATTR_NPERSHYBLINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dHybMinProb(1.0),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0),
      m_pNumAgentsPerCell(NULL),
      m_fHybScale(2),
      m_fHybShift(0.5) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// constructor for use with VerhulstVarK action
//
template<typename T>
NPersHybLinearBirthRel<T>::NPersHybLinearBirthRel(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, int **aiNumBirths, double dHybMinProb) 
    : Action<T>(pPop,pCG,ATTR_NPERSHYBLINBIRTHREL_NAME,sID),
      m_apWELL(apWELL),
      m_dHybMinProb(dHybMinProb),
      m_iNumThreads(omp_get_max_threads()),
      m_aiNumBirths(aiNumBirths),
      m_iWhich(0) ,
      m_pNumAgentsPerCell(NULL),
      m_fHybScale(2),
      m_fHybShift(0.5) {
    
    m_aaiNumTemp = new int *[m_iNumThreads];
    for (int i = 0; i < m_iNumThreads; i++) {
        m_aaiNumTemp[i] = new int[this->m_pCG->m_iNumCells];
    }

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
NPersHybLinearBirthRel<T>::~NPersHybLinearBirthRel() {
    if (m_aaiNumTemp != NULL) {
        for (int i = 0; i < m_iNumThreads; i++) {
            if (m_aaiNumTemp != NULL) {
                delete[] m_aaiNumTemp[i];
            }
        }
        delete[] m_aaiNumTemp;
    }

    // we don't delete m_aiNumBirths because it comes from outside
}



//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int NPersHybLinearBirthRel<T>::preLoop() {
    int iResult = -1;

    m_fHybScale = 2*(1 - m_dHybMinProb);
    m_fHybShift = 1 - (1 - m_dHybMinProb)/2;
  
    if (m_pNumAgentsPerCell == NULL) {
        m_pNumAgentsPerCell = this->m_pPop->getNumAgentsArray();
    }

    if ((m_aiNumBirths != NULL)) {
#pragma omp parallel for
        for (int i = 0; i < m_iNumThreads; i++) {
            memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
        }       
        memset(m_aiNumBirths[0], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        memset(m_aiNumBirths[1], 0, sizeof(int) * (this->m_pCG->m_iNumCells));
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// preLoop
//
template<typename T>
int NPersHybLinearBirthRel<T>::postLoop() {
    int iResult = 0;

    return iResult;
 }

//-----------------------------------------------------------------------------
// initialize
//
template<typename T>
int NPersHybLinearBirthRel<T>::initialize(float fT) {
printf("initalize %fn", fT);
#pragma omp parallel for
    for (int i = 0; i < m_iNumThreads; i++) {
        memset(m_aaiNumTemp[i],0,sizeof(int) * (this->m_pCG->m_iNumCells));
    }
    //    printf("Num agents in cell 0: %lu\n", this->m_pPop->getNumAgents(0));
    // set 
    //    int iChunk = (int)ceil((this->m_pCG->m_iNumCells+1)/(double)omp_get_max_threads());
    //#pragma omp parallel for schedule(static,iChunk)

    
    return 0;
}


//-----------------------------------------------------------------------------
// action: execute
//
template<typename T>
int NPersHybLinearBirthRel<T>::execute(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);
    
    if (pa->m_iLifeState > 0) {
	// note: if this doesn't compile, the agent has no field m_iMateIndex, 
        // and therefore the population should not use this action class.
    	int iMateIndex = pa->m_iMateIndex;
    	int iCellIndex = pa->m_iCellIndex;
        int iThread    = omp_get_thread_num();
        // we'll need a random  number in any case
        double dR      = this->m_apWELL[iThread]->wrandd();

    	// if reproduction is in couples, use mate index, 
    	// otherwise use agent index also as father
	
   

        if (pa->m_dCC > 0) {
            
            pa->m_dBReal =  pa->m_dB0 + ((pa->m_dTheta - pa->m_dB0)*this->m_pPop->getNumAgents(iCellIndex))/pa->m_dCC;
            /*
            if (iCellIndex == 0) {
	    
#pragma omp critical
                {
                    printf("T:%f: ag %d: b0 %f, theta %f, numags %lu, cc %f -> bReal %f\n", fT, iAgentIndex, pa->m_dB0, pa->m_dTheta, this->m_pPop->getNumAgents(iCellIndex), pa->m_dCC,  pa->m_dBReal);
                }
	    
            }               
            */
            if (pa->m_dBReal > 0) { // positive birth prob
                if ((pa->m_iGender == 0) && (iMateIndex >= 0)) {
                    // calculate the hybridization factor
                    T *pa2 =  &(this->m_pPop->m_aAgents[iMateIndex]);
                    float h1 = pa->m_fHybridization;
                    float h2 = pa2->m_fHybridization;
                    /* old version
                    // this function returns 1 if both h1 and h2 are equal, and minHybProb if h1 = 0 and h2 = 1 and vice versa
                    float fH = (m_fHybShift+m_fHybScale * (h1 - 0.5)*(h2 - 0.5)); 
                    */
                    float f1 = m_dHybMinProb + 2*(1-m_dHybMinProb)*fabs(h1-0.5); 
                    float f2 = m_dHybMinProb + 2*(1-m_dHybMinProb)*fabs(h2-0.5); 
                    //printf("cell %d: M(%d)H:%f, F(%d)h:%f -> fH_%f\n", iCellIndex, iAgentIndex, h1, iMateIndex, h2, fH);
                    if (dR < pa->m_dBReal*f1*f2) {
                        
                        this->m_pPop->registerBirth(iCellIndex, iAgentIndex, iMateIndex);
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
int NPersHybLinearBirthRel<T>::finalize(float fT) {

printf("finalize %fn", fT);
#pragma omp parallel for
    for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
        m_aiNumBirths[m_iWhich][i] = 0;
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            m_aiNumBirths[m_iWhich][i] += m_aaiNumTemp[iT][i];
        }
    }
    //    printf("Num births in cell 0: %d\n", m_aiNumBirths[m_iWhich][0]);

    m_iWhich = 1 - m_iWhich;
    return 0;
}


//-----------------------------------------------------------------------------
// extractAttributesQDF
//
//  tries to read the attributes
//    ATTR_NPERSLINBIRTHREL_HYBMINPROB_NAME
//
template<typename T>
int NPersHybLinearBirthRel<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NPERSHYBLINBIRTHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);
        if (iResult != 0) {
            LOG_ERROR("[NPersHybLinearBirthRel] couldn't read attribute [%s]", ATTR_NPERSHYBLINBIRTHREL_HYBMINPROB_NAME);
        }
    }
    
    return iResult; 
}


//-----------------------------------------------------------------------------
// writeAttributesQDF
//
//  tries to write the attributes
//    ATTR_NPERSLINBIRTHREL_HYBMINPROB_NAME
//
template<typename T>
int NPersHybLinearBirthRel<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NPERSHYBLINBIRTHREL_HYBMINPROB_NAME, 1, &m_dHybMinProb);

    return iResult; 
}


//-----------------------------------------------------------------------------
// modifyAttributes
//
//
template<typename T>
int NPersHybLinearBirthRel<T>::modifyAttributes(const std::string sAttrName, double dValue) {
    int iResult = 0;
    if (sAttrName == ATTR_NPERSHYBLINBIRTHREL_HYBMINPROB_NAME) {
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
int NPersHybLinearBirthRel<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_NPERSHYBLINBIRTHREL_HYBMINPROB_NAME, &m_dHybMinProb);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool NPersHybLinearBirthRel<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    NPersHybLinearBirthRel<T>* pA = static_cast<NPersHybLinearBirthRel<T>*>(pAction);
    if  (m_dHybMinProb == pA->m_dHybMinProb) {
        bEqual = true;
    } 
    return bEqual;
}

