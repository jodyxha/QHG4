#include <cstring>
#include <omp.h>
#include <cmath>
#include <algorithm>

#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "Genetics.h"
#include "AssortativePair.h"

#include "MessLoggerT.h"
#include "RankTable.h"
#include "ProteinBuilder.h"
#include "ProteomeComparator.h"

template<typename T, class U>
const std::string AssortativePair<T,U>::asNames[] = {
    ATTR_ASSPAIR_PERMUTE,
    ATTR_ASSPAIR_CUTOFF};


//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
AssortativePair<T,U>::AssortativePair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, Genetics<T,U> *pGenetics, WELL512 **apWELL) 
    : Action<T>(pPop,pCG,ATTR_ASSPAIR_NAME,sID),
      m_pGenetics(pGenetics),
      m_apWELL(apWELL),
      m_fCutOff(0.0),
      m_bPermute(0) {
   
    // Genetic's genome size is probably not set yet - get it later
    m_iNumCells     = this->m_pCG->m_iNumCells;
    m_vLocFemalesID = new std::vector<int>[m_iNumCells];
    m_vLocMalesID   = new std::vector<int>[m_iNumCells];

    m_aFLocks = new omp_lock_t[m_iNumCells];
    m_aMLocks = new omp_lock_t[m_iNumCells];
    for (int i = 0; i < m_iNumCells; i++) {
        omp_init_lock(&m_aFLocks[i]);
        omp_init_lock(&m_aMLocks[i]);
    }

    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T, class U>
AssortativePair<T,U>::~AssortativePair() {
    
    if (m_vLocFemalesID != NULL) {
        delete[] m_vLocFemalesID;
    }

    if (m_vLocMalesID != NULL) {
        delete[] m_vLocMalesID;
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
template<typename T, class U>
int AssortativePair<T,U>::preLoop() {
    m_iNumCells   = this->m_pCG->m_iNumCells;
    m_iGenomeSize = m_pGenetics->getGenomeSize();
    printf("[AssortativePair<T,U>::preLoop] numcells %d, genomesize %d\n", m_iNumCells, m_iGenomeSize);
    return 0;
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T, class U>
int AssortativePair<T,U>::initialize(float fT) {
  
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
template<typename T, class U>
int AssortativePair<T,U>::finalize(float fT) {
    
    return 0;

}


//-----------------------------------------------------------------------------
// findMates
//
template<typename T, class U>
int AssortativePair<T,U>::findMates() {
    int iResult = 0;
    int iFirstAgent = this->m_pPop->getFirstAgentIndex();
    int iLastAgent  = this->m_pPop->getLastAgentIndex();
    float f1 = 0;
    float f2 = 0;
    float f3 = 0;
    float f4 = 0;
    float *fBuildMatch = new float[omp_get_max_threads()];
    memset(fBuildMatch, 0, omp_get_max_threads()*sizeof(float));
    float *fRanking = new float[omp_get_max_threads()];
    memset(fRanking, 0, omp_get_max_threads()*sizeof(float));
    float *fCreation = new float[omp_get_max_threads()];
    memset(fCreation, 0, omp_get_max_threads()*sizeof(float));
    float *fCompare = new float[omp_get_max_threads()];
    memset(fCompare, 0, omp_get_max_threads()*sizeof(float));
    float *fTranslate = new float[omp_get_max_threads()];
    memset(fTranslate, 0, omp_get_max_threads()*sizeof(float));

    f1 = omp_get_wtime();

#pragma omp for schedule(dynamic)
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
    

    f2 = omp_get_wtime();
    // now we have male and female vectors of agent indexes for each cell
#pragma omp parallel for schedule(dynamic)
    for (int iC = 0; iC < m_iNumCells; iC++) {
        float f2a = omp_get_wtime();
        int iT = omp_get_thread_num();

        if ((m_vLocFemalesID[iC].size() > 0) && (m_vLocMalesID[iC].size() > 0)) {
            //            printf("%d:Cell %d: %zd females, %zd males\n", iT,iC, m_vLocFemalesID[iC].size(), m_vLocMalesID[iC].size());
            // allocat match array
            float **ppMatch = new float*[m_vLocFemalesID[iC].size()];
            for (uint iF = 0; (iResult == 0) && (iF < m_vLocFemalesID[iC].size()); ++iF) {
                ppMatch[iF] = new float[m_vLocMalesID[iC].size()];
                memset(ppMatch[iF], 0, m_vLocMalesID[iC].size()*sizeof(float));
            }

            std::vector<std::pair<ProteinBuilder *, const prothash*> > vMaleProteomes;
            float fc1 = omp_get_wtime();
            for (uint iM = 0; (iResult == 0) && (iM < m_vLocMalesID[iC].size()); ++iM) {
                ProteinBuilder *pPB2 = ProteinBuilder::createInstance(m_pGenetics->getGenome(m_vLocMalesID[iC][iM]), m_iGenomeSize);
                if (pPB2 != NULL) {
                    iResult = pPB2->translateGenomeHash();
                    if (iResult == 0) {
                        vMaleProteomes.push_back(std::pair<ProteinBuilder *, const prothash*>(pPB2, &(pPB2->getProtHash())));
                    }
                }
            }            
            fCreation[iT] += omp_get_wtime()-fc1;

            // fill the match array using ProteinBuilder and ProteomeComparator
            for (uint iF = 0; (iResult == 0) && (iF < m_vLocFemalesID[iC].size()); ++iF) {
                float fc11 = omp_get_wtime();
                ProteinBuilder *pPB1 = ProteinBuilder::createInstance(m_pGenetics->getGenome(m_vLocFemalesID[iC][iF]), m_iGenomeSize);
                fCreation[iT] += omp_get_wtime()-fc11;
                if (pPB1 != NULL) {
                    iResult = pPB1->translateGenomeHash();
                    fCreation[iT] += omp_get_wtime()-fc1;

                    if (iResult == 0) {
                        const prothash &vProtHash1 = pPB1->getProtHash();
                        
                        float fc2 = omp_get_wtime();
                        for (uint iM = 0; (iResult == 0) && (iM < m_vLocMalesID[iC].size()); ++iM) {
                            const prothash &vProtHash2 = *(vMaleProteomes[iM].second);
                            ppMatch[iF][iM] =  ProteomeComparator::countProtHashMatches(vProtHash1, vProtHash2);
                        }
                        fCompare[iT] += omp_get_wtime()-fc2;
                        

                    } else {
                        //couldn't translate
                    }
                    delete pPB1;
                } else {
                    //couldn't create
                }
            }

            for (uint iM = 0; (iResult == 0) && (iM < m_vLocMalesID[iC].size()); ++iM) {
                delete vMaleProteomes[iM].first;
            }

            f3 = omp_get_wtime();
            fBuildMatch[iT] += (f3 -f2a);
            // use RankTable to find pairings
            
            RankTable *pRT = RankTable::createInstance( m_vLocFemalesID[iC].size(), m_vLocMalesID[iC].size(), m_apWELL, ppMatch);
            if (pRT != NULL) {
                pRT->setVerbosity(false);
                
                
                //            pRT->display();
                iResult = pRT->makeAllPairings(m_fCutOff, m_bPermute);
                
                const couples &vc = pRT->getPairs();
                for (uint i = 0; i < vc.size(); ++i) {
                    int iIDF = m_vLocFemalesID[iC][vc[i].first];
                    int iIDM = m_vLocMalesID[iC][vc[i].second];
                    
                    this->m_pPop->m_aAgents[iIDF].m_iMateIndex = iIDM;
                    this->m_pPop->m_aAgents[iIDM].m_iMateIndex = iIDF;
                    
                }
                delete pRT;    
            } else {
                printf("Xouldn't create RankTable\n");
            }
            
            // clean up ppMatch
            for (uint i = 0; i < m_vLocFemalesID[iC].size(); ++i) {
                delete[] ppMatch[i];
            }
            delete[] ppMatch;
            f4 = omp_get_wtime();
            fRanking[iT] += (f4 - f3);
        }

    }
    float f5 = omp_get_wtime();
    for (int i  = 1; i < omp_get_max_threads(); i++) {
        fBuildMatch[0] += fBuildMatch[i];
        fRanking[0]    += fRanking[i];
        fCreation[0]   += fCreation[i];
        fCompare[0]    += fCompare[i];
    }
    printf("Creating cell-based vectors: %f\n", f2 - f1);
    printf("Creating match tables:       %f\n", fBuildMatch[0]);
    printf("Actual pairing:              %f\n", fRanking[0]);
    printf("creating ProteinBuilders:    %f\n", fCreation[0]);
    printf("translating genomes:         %f\n", fTranslate[0]);
    printf("comparing Proteomes:         %f\n", fCompare[0]);
    printf("Total: %f\n", f5 - f1);
    
    delete[] fBuildMatch;
    delete[] fRanking;
    delete[] fCreation;
    delete[] fTranslate;
    delete[] fCompare;

    return iResult;
}





//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_ASSPAIR_CUTOFF 
//    ATTR_ASSPAIR_PERMUTE
//
template<typename T, class U>
int AssortativePair<T,U>::writeAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ASSPAIR_CUTOFF,  1, (int *) &m_fCutOff);
    }
    if (iResult == 0) {
        int iPermute = m_bPermute;
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_ASSPAIR_PERMUTE,  1, &iPermute);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_ASSPAIR_CUTOFF, 
//    ATTR_ASSPAIR_PERMUTE
//
template<typename T, class U>
int AssortativePair<T,U>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ASSPAIR_CUTOFF,  1, (float *) &m_fCutOff);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ASSPAIR_CUTOFF);
        }
    }

 
    if (iResult == 0) {

        int iPermute = 0;
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_ASSPAIR_PERMUTE,  1, &iPermute);
        m_bPermute = iPermute;
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_ASSPAIR_PERMUTE);
        }
    }
    
    
    return iResult;
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T, class U>
int AssortativePair<T,U>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_ASSPAIR_CUTOFF,         &m_fCutOff);
        int iPermute = 0;
        iResult += getAttributeVal(mParams, ATTR_ASSPAIR_PERMUTE,        &iPermute);
        if (iPermute != 0) {
            m_bPermute = true;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T, class U>
bool AssortativePair<T,U>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    AssortativePair<T,U>* pA = static_cast<AssortativePair<T,U>*>(pAction);
    if ((m_fCutOff  == pA->m_fCutOff) &&
        (m_bPermute == pA->m_bPermute)) {
        bEqual = true;
    } 
    return bEqual;
}

