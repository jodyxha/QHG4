#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <omp.h>

#include <openssl/sha.h>

#include <vector>
#include <algorithm>
#include <hdf5.h>

#include "MessLoggerT.h"
#include "strutils.h"

#include "BinomialDist.h"
#include "ParamProvider2.h"
#include "Permutator.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "WELLUtils.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "WELLDumpRestore.h"
#include "PermDumpRestore.h"
#include "SequenceIOUtils.h"


#include "clsutils.cpp"
#include "Phenetics2.h"

// needed for BinomialDistribution
#define EPS 1e-6

static const uint  BUFSIZE_READ_PHENOMES=8192;

#define MAX_INIT_NAME 32

// this number must changed if the parameters change
template<typename T>
int Phenetics2<T>::NUM_PHENETIC_PARAMS = 8;

template<typename T>
const std::string Phenetics2<T>::asNames[] = {
    ATTR_PHENETICS_PHENOME_SIZE,
    ATTR_PHENETICS_MUTATION_RATE,
    ATTR_PHENETICS_MUTATION_SIGMA,
    ATTR_PHENETICS_INITIAL_SIGMA,
    ATTR_PHENETICS_CREATE_NEW_PHENOME,
    ATTR_PHENETICS_MIX_AVG,
    ATTR_PHENETICS_MUTATION_TYPE};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Phenetics2<T>::Phenetics2(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL)  
    : Action<T>(pPop, pCG, ATTR_PHENETICS_NAME,sID),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(apWELL),
      m_iPhenomeSize(-1),
      m_iTotalSize(-1),
      m_pBDist(NULL),
      m_apPerm(NULL),
      m_dMutationRate(-1),
      m_dMutationSigma(-1),
      m_dInitialSigma(0),
      m_bCreateNewPhenome(0),
      m_bMixAvg(true),
      m_iMutType(0),
      m_iNumSetParams(0),
      m_iPloidy(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_bOwnWELL(false),
      m_fCurTime(-1),
      m_pSeqIO(NULL),
      m_aiLocs(NULL),
      m_adMuts(NULL)   {

    m_fMixing = &Phenetics2<T>::mix_avg_haplo;
    m_fMutate = &Phenetics2<T>::mutate;

    m_iNumThreads = omp_get_max_threads();
 
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));

}


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Phenetics2<T>::Phenetics2(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed)  
    : Action<T>(pPop, pCG, ATTR_PHENETICS_NAME, sID),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(NULL),
      m_iPhenomeSize(-1),
      m_iTotalSize(-1),
      m_pBDist(NULL),
      m_apPerm(NULL),
      m_dMutationRate(-1),
      m_dMutationSigma(-1),
      m_bCreateNewPhenome(0),
      m_bMixAvg(true),
      m_iNumSetParams(0),
      m_iPloidy(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_bOwnWELL(true),
      m_pSeqIO(NULL),
      m_aiLocs(NULL),
      m_adMuts(NULL)  {

    m_fMixing = &Phenetics2<T>::mix_avg_haplo;
    m_fMutate = &Phenetics2<T>::mutateAll;

    m_iNumThreads = omp_get_max_threads();

    // we need to build our own WELLs    
    printf("[Phenetics::Phenetics] using %u as seed for WELLs\n", iSeed);
    m_apWELL = WELLUtils::buildWELLs(m_iNumThreads, iSeed);
 
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Phenetics2<T>::~Phenetics2() {
    deleteAllocated();

    if (m_bOwnWELL && (m_apWELL != NULL)) {
        WELLUtils::destroyWELLs(m_apWELL, m_iNumThreads);
    }

}


//-----------------------------------------------------------------------------
// deleteAllocated
//
template<typename T>
void Phenetics2<T>::deleteAllocated() {

    if (m_pBDist != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pBDist[iT] != NULL) {
                delete m_pBDist[iT];
            }
        }
        delete[] m_pBDist;
    }
    
    if (m_apPerm != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_apPerm[iT] != NULL) {
                delete m_apPerm[iT];
            }
        }
        delete[] m_apPerm;
    }
    
    if (m_pWriteCopyController != NULL) {
        delete m_pWriteCopyController;
    }

    if (m_bBufferAdded) {
        m_pAgentController->removeBuffer(static_cast<LBBase *>(&m_aPhenome));
        m_bBufferAdded = false;
    }

    if (m_pSeqIO != NULL) {
        delete m_pSeqIO;
    }

    if (m_aiLocs != NULL) {
        delete[] m_aiLocs;
    }

    if (m_adMuts != NULL) {
        delete[] m_adMuts;
    }
}


//-----------------------------------------------------------------------------
// init
//   must be called *after* params have been read, 
//   but *before* reading the phenome
//
template<typename T>
int Phenetics2<T>::init() {
    int iResult = -1;
    printf("init called\n");
    deleteAllocated();
    if ((m_iPhenomeSize > 0) && (m_dMutationRate <= 1)) {
        
        m_iTotalSize = m_iPloidy*m_iPhenomeSize;
        
        // initialize the buffer ...
        printf("initializing m_aPhenome with (%d, %d*%d)\n", m_pAgentController->getLayerSize(), m_iPloidy, m_iPhenomeSize);
        m_aPhenome.init(m_pAgentController->getLayerSize(), m_iTotalSize);
        
        // ... and add it to the AgentController
        iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aPhenome));
        
        
        // layeredBuffer and controller for writing a copy of the actual agent array
        if (iResult == 0) {
            
            printf("[Phenetics<T>::init()] writecopycontroller with real layer size of %d; array size %d\n", m_pAgentController->getLayerSize(), m_iTotalSize);
            
            m_pWriteCopyController = new LBController;
            
            m_aWriteCopy.init( m_pAgentController->getLayerSize(), m_iTotalSize);
            m_pWriteCopyController->init( m_pAgentController->getLayerSize());
            iResult = m_pWriteCopyController->addBuffer(static_cast<LBBase *>(&m_aWriteCopy));
            m_pWriteCopyController->addLayer();
        }

        if (iResult == 0) {
            
            m_bBufferAdded = true;

            m_pBDist = new BinomialDist*[m_iNumThreads];
            m_apPerm = new Permutator*[m_iNumThreads];

            // binomialDist and Permutator are currently only needed for mutateSel
            if ((m_dMutationRate >= 0) && (m_iMutType == MUT_TYPE_SEL)) {
                // bionomial distribution (to get number of mutations (maybe not needed )
#pragma omp parallel
                {
                    int iT = omp_get_thread_num();
                    m_pBDist[iT] = BinomialDist::create(m_dMutationRate, m_iTotalSize, EPS);
                    if (m_pBDist[iT] != NULL) {
                        iResult = 0;
                    } else {
                        printf("Couldn't create BinomialDistribution\n");
                    }

                    m_apPerm[iT]  = Permutator::createInstance(m_iTotalSize);
                }
            } else {
	    	memset(m_pBDist, 0, m_iNumThreads*sizeof(BinomialDist*));
	    	memset(m_apPerm, 0, m_iNumThreads*sizeof(Permutator*));

	    }
            
            m_pSeqIO = SequenceIOUtils<float>::createInstance(PHENOME_DATASET_NAME, H5T_NATIVE_FLOAT, &m_aPhenome, m_pAgentController, m_pvDeadList, m_iTotalSize/*m_iPhenomeSize*/);

        } else {
            printf("Couldn't add buffer to controller\n");
        }

        m_aiLocs = new int[m_iTotalSize];
        m_adMuts = new double[m_iTotalSize];
        
        // set function pointer for mixing to requested function
        if (m_bMixAvg) {
            if (m_iPloidy == 1) {
                m_fMixing = &Phenetics2<T>::mix_avg_haplo;
            } else if (m_iPloidy == 2) {
                m_fMixing = &Phenetics2<T>::mix_avg_diplo;
            } else {
                printf("Illegal ploidy: %d (should be 1 or 2)\n", m_iPloidy);
            }
        } else {
            if (m_iPloidy == 1) {
                m_fMixing = &Phenetics2<T>::mix_sel_haplo;
            } else if (m_iPloidy == 2) {
                m_fMixing = &Phenetics2<T>::mix_sel_diplo;
            } else {
                printf("Illegal ploidy: %d (should be 1 or 2)\n", m_iPloidy);
                iResult = -1;
            }
        }
        

        if (m_dMutationRate > 0) {
	    printf("MutRate > 0 %d -> mutateAll\n", m_iMutType);
            m_fMutate = &Phenetics2<T>::mutateSel;
        } else {

            switch(m_iMutType) {
            case MUT_TYPE_SEL:
                printf("without mutation rate we can't have mutateSel\n");
                iResult = -1;
                break;
            case MUT_TYPE_POS:
	        printf("MutType %d -> mutatePos\n", m_iMutType);
                m_fMutate = &Phenetics2<T>::mutatePos;
                break;
            case MUT_TYPE_INC:
	        printf("MutType %d -> mutateInc\n", m_iMutType);
                m_fMutate = &Phenetics2<T>::mutateInc;
                break;
            case MUT_TYPE_ALL:
	        printf("MutType %d -> mutateAll\n", m_iMutType);
                m_fMutate = &Phenetics2<T>::mutateAll;
                break;
	    default:
	        printf("Unknown mutation type %d\n", m_iMutType);
                iResult = -1;
            }
        }

    } else {
        printf("Bad values for phenome size or mutation rate\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// initialize
//
template<typename T>
int Phenetics2<T>::initialize(float fTime) { 
    m_fCurTime = fTime; 
    return 0; 
}


//----------------------------------------------------------------------------
// makeOffspring
//   actual creation of babies
//   (called from SPopulation::performBirths()
//
template<typename T>
int  Phenetics2<T>::makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex) {
    int iResult = 0;

    // we have to manipulate inside of a layer buf array
    // better to treat them as normal arrays
    // this is safe because a genome is alway inside  single layer
    phentype *pBabyPhenome  = &(m_aPhenome[iBabyIndex]);
    phentype *pPhenomeM     = &(m_aPhenome[iMotherIndex]);
    phentype *pPhenomeP     = &(m_aPhenome[iFatherIndex]);

    // clear baby genome
    memset(pBabyPhenome, 0, m_iTotalSize*sizeof(phentype));

    // average or parent1's phenome into baby's first strand
    (this->*m_fMixing)(pBabyPhenome, pPhenomeM, pPhenomeP);

    // mutations
    (this->*m_fMutate)(pBabyPhenome, m_dMutationSigma);

    return iResult;
}


//----------------------------------------------------------------------------
// mix_avg_haplo (only haploid sequences)
//   take the average of a strand of each parent
//   
template<typename T>
void Phenetics2<T>::mix_avg_haplo(phentype *pMixedPhenome, phentype *pPhenomeM, phentype *pPhenomeP) {
    for (int i = 0; i < m_iTotalSize; i++) {
        pMixedPhenome[i] = (pPhenomeM[i] + pPhenomeP[i])/2;
    }
}


//----------------------------------------------------------------------------
// mix_avg_diplo (only diploid sequences)
//  a sort of "crossing-over": take the average of a strand of each parent
//   
template<typename T>
void Phenetics2<T>::mix_avg_diplo(phentype *pMixedPhenome, phentype *pPhenomeM, phentype *pPhenomeP) {
    for (int i = 0; i < m_iPhenomeSize; i++) {
        pMixedPhenome[i] = (pPhenomeM[i] + pPhenomeM[i+m_iPhenomeSize])/2;
    }
    for (int i = 0; i < m_iPhenomeSize; i++) {
        pMixedPhenome[i+m_iPhenomeSize] = (pPhenomeP[i] + pPhenomeP[i+m_iPhenomeSize])/2;
    }
}


//----------------------------------------------------------------------------
// mix_sel_haplo  (only haploid sequences)
//  a sort of "free recombination": select elements randomly from parents
//
template<typename T>
void Phenetics2<T>::mix_sel_haplo(phentype *pMixedPhenome, phentype *pPhenomeM, phentype *pPhenomeP) {
    int j = 0;
    WELL512 *pWELL = m_apWELL[omp_get_thread_num()];
    while (j < m_iPhenomeSize) {
        //printf("j: %d\n", j);
        uint32_t k = pWELL->wrand();
        for (int i = 0; (i<32) && (j < m_iPhenomeSize); i++) {
            //printf("i: %d, k: %x\n", j, k);
             pMixedPhenome[j] = (k & 0x1) ? pPhenomeM[j] : pPhenomeP[j];
             k >>= 1;
             j++;
        }
    }
}


//----------------------------------------------------------------------------
// mix_sel_diplo 
//  a sort of "free recombination": select elements randomly from parents
//
template<typename T>
void Phenetics2<T>::mix_sel_diplo(phentype *pMixedPhenome, phentype *pPhenomeM, phentype *pPhenomeP) {
    // make gametes
    
    mix_sel_haplo(pMixedPhenome,                pPhenomeM, pPhenomeM+m_iPhenomeSize);
    mix_sel_haplo(pMixedPhenome+m_iPhenomeSize, pPhenomeP, pPhenomeP+m_iPhenomeSize);
}


//----------------------------------------------------------------------------
// mutateAll
//   mutation by adding normally distributed nudge to all traits
//   (ignores binomialDist and premutator)
//
template<typename T>
void Phenetics2<T>::mutateAll(phentype *pPhenome, double dSigma) {
    WELL512 *pWELL = m_apWELL[omp_get_thread_num()];
    for (int i = 0; i < m_iTotalSize; i++) {
        pPhenome[i] +=  pWELL->wgauss(dSigma);
    }
}


//----------------------------------------------------------------------------
// mutateSel
//   mutation by adding normally distributed nudge to a selection of traits
//
template<typename T>
void Phenetics2<T>::mutateSel(phentype *pPhenome, double dSigma) {
    WELL512 *pWELL = m_apWELL[omp_get_thread_num()];

    int iNumMutations = m_pBDist[omp_get_thread_num()]->getN(pWELL->wrandd());
    if (iNumMutations > 0) {
        if (iNumMutations == 1) {
            pPhenome[pWELL->wrandi(0, m_iTotalSize)] +=  pWELL->wgauss(dSigma);
        } else {
            const uint *piPermuted  = m_apPerm[omp_get_thread_num()]->permute(m_iTotalSize, iNumMutations, pWELL);

            for (int i = 0; i < iNumMutations; i++) {
                pPhenome[piPermuted[i]] +=  pWELL->wgauss(dSigma);
            }
        }
    }
}


//----------------------------------------------------------------------------
// mutatePos
//   mutation by adding the absolute value of a normally distributed
//   random number (i.e. all components are positive
//
template<typename T>
void Phenetics2<T>::mutatePos(phentype *pPhenome, double dSigma) {
    WELL512 *pWELL = m_apWELL[omp_get_thread_num()];
    for (int i = 0; i < m_iPhenomeSize; i++) {
        pPhenome[i] +=  fabs(pWELL->wgauss(dSigma));
    }
}


//----------------------------------------------------------------------------
// mutateInc
//   only add mutations if they increase distance to 0
//   otherwise, addtheir inverse
//
template<typename T>
void Phenetics2<T>::mutateInc(phentype *pPhenome, double dSigma) {
    WELL512 *pWELL = m_apWELL[omp_get_thread_num()];
    double dDot = 0;
    for (int i = 0; i < m_iTotalSize; i++) {
        m_adMuts[i] = pWELL->wgauss(dSigma);
        dDot += pPhenome[i]*m_adMuts[i];
    }
    if (dDot > 0) {
        for (int i = 0; i < m_iTotalSize; i++) {
            pPhenome[i] +=  m_adMuts[i];
        }
    } else {
        for (int i = 0; i < m_iPhenomeSize; i++) {
            pPhenome[i] -=  m_adMuts[i];
        }
    }
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//  write additional data to the group
//  Here: create a new Dataset for the genome.
//  Write genomes sequentially from all cells.
//  This method does not change the agent- or genome array
//  Go through all cells, and use the BufLayers of m_aGenome as slabs 
//
template<typename T>
int Phenetics2<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    printf("[Phenetics<T>::writeAdditionalDataQDFSafe] using sequence for group %ld wizh num %lu\n", hSpeciesGroup, this->m_pPop->getNumAgentsEffective()); fflush(stdout);
    iResult = m_pSeqIO->writeSequenceDataQDF(hSpeciesGroup,this->m_pPop->getNumAgentsEffective());
 
    return iResult;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//  read additional data from the group
//  Read phenomes sequentially from all cells which contain agents (1 per agent)
//
// Ass: agents have been read
//      There is exactly one phenome per agent
//      phenomes ordered as agents
//  
template<typename T>
int Phenetics2<T>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
 
    iResult = m_pSeqIO->readSequenceDataQDF(hSpeciesGroup,BUFSIZE_READ_PHENOMES);
    return iResult;
}


//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_PHENETICS_PHENOME_SIZE, 
//    ATTR_PHENETICS_MUTATION_RATE
//    ATTR_PHENETICS_MUTATION_SIGMA
//    ATTR_PHENETICS_INITIAL_SIGMA
//    ATTR_PHENETICS_CREATE_NEW_PHENOME
//    ATTR_PHENETICS_MIX_AVG
//    ATTR_PHENETICS_MUTATION_TYPE
//    ATTR_PHENETICS_PLOIDY
//
template<typename T>
int Phenetics2<T>::writeAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_PHENOME_SIZE,  1, (int *) &m_iPhenomeSize);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_RATE,  1, &m_dMutationRate);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_SIGMA,  1, &m_dMutationSigma);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_INITIAL_SIGMA,  1,  &m_dInitialSigma);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_CREATE_NEW_PHENOME, 1, &m_bCreateNewPhenome);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_MIX_AVG,  1,  &m_bMixAvg);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_TYPE,  1, (int *) &m_iMutType);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_PLOIDY,  1, (int *) &m_iPloidy);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_PHENETICS_PHENOME_SIZE, 
//    ATTR_PHENETICS_MUTATION_RATE
//    ATTR_PHENETICS_MUTATION_SIGMA
//    ATTR_PHENETICS_INITIAL_SIGMA
//    ATTR_PHENETICS_CREATE_NEW_PHENOME
//    ATTR_PHENETICS_MIX_AVG
//    ATTR_PHENETICS_MUTATION_TYPE
//    ATTR_PHENETICS_PLOIDY
//
template<typename T>
int Phenetics2<T>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_PHENOME_SIZE,  1, (int *) &m_iPhenomeSize);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_PHENOME_SIZE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_RATE,  1, &m_dMutationRate);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_MUTATION_RATE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_SIGMA,  1, &m_dMutationSigma);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_MUTATION_SIGMA);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_INITIAL_SIGMA, 1,  &m_dInitialSigma);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_INITIAL_SIGMA);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_CREATE_NEW_PHENOME, 1, &m_bCreateNewPhenome);
        if (iResult != 0) {
            LOG_WARNING("[Phenetics] couldn't read attribute [%s]; setting value to false", ATTR_PHENETICS_CREATE_NEW_PHENOME);
            m_bCreateNewPhenome = 0;
            iResult = 0;
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_MIX_AVG, 1, &m_bMixAvg);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_MIX_AVG);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_TYPE,  1, (int *) &m_iMutType);
        if (iResult != 0) {
            LOG_WARNING("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_MUTATION_TYPE);
        }   
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_PLOIDY,  1, (int *) &m_iPloidy);
        if (iResult != 0) {
            LOG_WARNING("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_PLOIDY);
        }   
    }


    
    printf("[Phenetics] ExtractParamsQDF:res %d\n", iResult);
    
    if (iResult == 0) {
        // we must call init() before callnig readAdditionalDataQDF()
        iResult = init();
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// dumpAdditionalDataQDF
//
template<typename T>
int Phenetics2<T>::dumpAdditionalDataQDF(hid_t hSpeciesGroup) {
    
    int iResult = m_pSeqIO->dumpSequenceDataQDF(hSpeciesGroup);
    return iResult;
}


//-----------------------------------------------------------------------------
// restoreAdditionalDataQDF
//
template<typename T>
int Phenetics2<T>::restoreAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pSeqIO->restoreSequenceDataQDF(hSpeciesGroup);
    return iResult;
}


//-----------------------------------------------------------------------------
// dumpStateQDF
//
template<typename T>
int Phenetics2<T>::dumpStateQDF(hid_t hSpeciesGroup) {
    printf("Phenetics WELLState before dump\n");
    showWELLStates();

    int iResult = 0;
    if (m_bOwnWELL) {
        iResult = dumpWELL(m_apWELL, m_iNumThreads, ATTR_PHENETICS_NAME, hSpeciesGroup);
    }
    
    if (iResult == 0) {
        iResult = dumpPerm(m_apPerm, m_iNumThreads, ATTR_PHENETICS_NAME, hSpeciesGroup);
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// restoreStateQDF
//
template<typename T>
int Phenetics2<T>::restoreStateQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    if (m_bOwnWELL) {
        iResult = restoreWELL(m_apWELL, m_iNumThreads, ATTR_PHENETICS_NAME, hSpeciesGroup);
    }

    if (iResult == 0) {
        iResult = restorePerm(m_apPerm, m_iNumThreads, ATTR_PHENETICS_NAME, hSpeciesGroup);
    }
     
    printf("Phenetics WELLState after restore\n");
    showWELLStates();

   return iResult;
        
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T>
int Phenetics2<T>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_PHENOME_SIZE,       &m_iPhenomeSize);      
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_MUTATION_RATE,      &m_dMutationRate);     
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_MUTATION_SIGMA,     &m_dMutationSigma);    
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_INITIAL_SIGMA,      &m_dInitialSigma);     
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_CREATE_NEW_PHENOME, &m_bCreateNewPhenome); 
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_MIX_AVG,            &m_bMixAvg);           
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_MUTATION_TYPE,      &m_iMutType);          
        iResult += getAttributeVal(mParams, ATTR_PHENETICS_PLOIDY,             &m_iPloidy);           
    }
    return iResult;
}




//-----------------------------------------------------------------------------
// createInitialPhenomes
//   only do this if the buffer has been already added to the controller
//
template<typename T>
int Phenetics2<T>::createInitialPhenomes(int iNumPhenomes) {
    int iResult = -1;
    if (m_bBufferAdded) {
        if (m_bCreateNewPhenome != 0) {
            iResult = 0;
            m_bCreateNewPhenome = false;
            printf("Creating %d mutated variants with sigma %f\n", iNumPhenomes, m_dInitialSigma);
            phentype *pG0 = new phentype[m_iTotalSize];
            memset(pG0, 0,  m_iTotalSize*sizeof(phentype));
            for (int i = 0; i < iNumPhenomes; i++) {
                // go to position for next phenome
                phentype *pPhenome = &(m_aPhenome[i]); 
                memcpy(pPhenome, pG0, m_iTotalSize*sizeof(phentype));
                mutateAll(pPhenome, m_dInitialSigma);
            }
            delete[] pG0;
        } else {
            printf("[Genetics::createInitialGenomes] No new Phenome created (buffer has not been added\n");
            iResult = 0;
        }

    } else {
        printf("Phenetics has not been initialized\n");
    }   
    return iResult;
}


//-----------------------------------------------------------------------------
// showWELLStates
//
template<typename T>
void Phenetics2<T>::showWELLStates() {
    for (int i = 0; i < m_iNumThreads; i++) {
        printf("[%08x] ", m_apWELL[i]->getIndex());
        const uint32_t *p = m_apWELL[i]->getState();
        for (uint j = 0; j < STATE_SIZE;j++) {
            printf("%08x ", p[j]);
        }
        printf("\n");
    }
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool Phenetics2<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Phenetics2<T>* pA = static_cast<Phenetics2<T>*>(pAction);
    if ((m_iPhenomeSize   == pA->m_iPhenomeSize) &&
        (m_dMutationRate  == pA->m_dMutationRate) &&
        (m_dMutationSigma == pA->m_dMutationSigma) &&
        ((!bStrict) || (m_dInitialSigma     == pA->m_dInitialSigma)) &&
        ((!bStrict) || (m_bCreateNewPhenome == pA->m_bCreateNewPhenome)) &&
        (m_bMixAvg        == pA->m_bMixAvg) &&
        (m_iMutType       == pA->m_iMutType) &&
        (m_iPloidy        == pA->m_iPloidy)) {
        bEqual = true;
    } 
    return bEqual;
}

