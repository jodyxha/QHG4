#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>

#include <openssl/sha.h>
#include <hdf5.h>

#include <vector>
#include <algorithm>

#include "MessLoggerT.h"
#include "strutils.h"
#include "xha_strutilsT.h"

#include "BinomialDist.h"
#include "ParamProvider2.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "WELLUtils.h"
#include "WELLDumpRestore.h"

#include "SequenceIOUtils.h"


#include "clsutils.h"
#include "Genetics.h"

// needed for BinomialDistribution
#define EPS 1e-6

static const uint  BUFSIZE_READ_GENOMES=1000;

#define MAX_INIT_NAME 32

// this number must changed if the parameters change
template<typename T, class U>
int Genetics<T,U>::NUM_GENETIC_PARAMS = 6;

template<typename T, class U>
const std::string Genetics<T,U>::asNames[] = {
    ATTR_GENETICS_GENOME_SIZE,
    ATTR_GENETICS_NUM_CROSSOVER,
    ATTR_GENETICS_MUTATION_RATE,
    ATTR_GENETICS_INITIAL_MUTS,
    ATTR_GENETICS_CREATE_NEW_GENOME,
    ATTR_GENETICS_BITS_PER_NUC};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
Genetics<T,U>::Genetics(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL)  
    : Action<T>(pPop, pCG, ATTR_GENETICS_NAME,sID),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(apWELL),
      m_iGenomeSize(-1),
      m_pBDist(NULL),
      m_iNumCrossOvers(-1),
      m_dMutationRate(-1),
      m_pTempGenome1(NULL),
      m_pTempGenome2(NULL),
      m_iNumBlocks(0),
      m_bCreateNewGenome(0),
      m_iBitsPerNuc(U::BITSINNUC),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_pGenomeCreator(NULL),
      m_iNumParents(2),
      m_bOwnWELL(false),
      m_pSeqIO(NULL) {


    m_iNumThreads = omp_get_max_threads();

    // GenomeCreator must exist before init() is called
    m_pGenomeCreator = new GenomeCreator<U>(m_iNumParents);
    
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(std::string));
}


//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
Genetics<T,U>::Genetics(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed)  
    : Action<T>(pPop, pCG, ATTR_GENETICS_NAME, sID),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(NULL),
      m_iGenomeSize(-1),
      m_pBDist(NULL),
      m_iNumCrossOvers(-1),
      m_dMutationRate(-1),
      m_pTempGenome1(NULL),
      m_pTempGenome2(NULL),
      m_iNumBlocks(0),
      m_bCreateNewGenome(0),
      m_iBitsPerNuc(U::BITSINNUC),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_pGenomeCreator(NULL),
      m_iNumParents(2),
      m_bOwnWELL(true),
      m_pSeqIO(NULL) {


    m_iNumThreads = omp_get_max_threads();

    // GenomeCreator must exist before init() is called
    m_pGenomeCreator = new GenomeCreator<U>(m_iNumParents);

    // we need to build our own WELLs    
    xha_printf("[Genetics::Genetics] using %u as seed for WELLs\n", iSeed);
    m_apWELL = WELLUtils::buildWELLs(m_iNumThreads, iSeed);
   
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T, class U>
Genetics<T,U>::~Genetics() {
    deleteAllocated();
    if (m_pGenomeCreator != NULL) {
        delete m_pGenomeCreator;
    }

    if (m_bOwnWELL && (m_apWELL != NULL)) {
        WELLUtils::destroyWELLs(m_apWELL, m_iNumThreads);
    }
}


//-----------------------------------------------------------------------------
// deleteAllocated
//
template<typename T, class U>
void Genetics<T,U>::deleteAllocated() {

    if (m_pTempGenome1 != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pTempGenome1[iT] != NULL) {
                delete[] m_pTempGenome1[iT];
            }
        }
        delete[] m_pTempGenome1;
    }

    if (m_pTempGenome2 != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pTempGenome2[iT] != NULL) {
                delete[] m_pTempGenome2[iT];
            }
        }
        delete[] m_pTempGenome2;
    }

    if (m_pBDist != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pBDist[iT] != NULL) {
                delete m_pBDist[iT];
            }
        }
        delete[] m_pBDist;
    }

    if (m_pWriteCopyController != NULL) {
        delete m_pWriteCopyController;
    }

    if (m_bBufferAdded) {
        m_pAgentController->removeBuffer(static_cast<LBBase *>(&m_aGenome));
        m_bBufferAdded = false;
    }

    if (m_pSeqIO != NULL) {
        delete m_pSeqIO;
    }
}


//-----------------------------------------------------------------------------
// init
//   must be called *after* params have been read, 
//   but *before* reading the genome
//
template<typename T, class U>
int Genetics<T,U>::init() {
    std::string yes("yes");
    int iResult = -1;
    xha_printf("init called %s\n",yes);
    deleteAllocated();
    if ((m_iGenomeSize > 0) && 
        /*(m_dMutationRate >= 0) && */(m_dMutationRate <= 1)) {
        
        if (m_iBitsPerNuc == U::BITSINNUC) {
            // number of longs needed to hold genome
            m_iNumBlocks  = U::numNucs2Blocks(m_iGenomeSize);

            // initialize the buffer ...
            xha_printf("initializing m_aGenome with (%d, %d)\n", m_pAgentController->getLayerSize(), m_iNumParents*m_iGenomeSize);
            m_aGenome.init(m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
        
            // ... and add it to the AgentController
            iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aGenome));


            // layeredBuffer and controller for writing a copy of the actual agent array
            if (iResult == 0) {

                m_pWriteCopyController = new LBController;

                m_aWriteCopy.init( m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
                m_pWriteCopyController->init( m_pAgentController->getLayerSize());
                iResult = m_pWriteCopyController->addBuffer(static_cast<LBBase *>(&m_aWriteCopy));
                m_pWriteCopyController->addLayer();
            }

            if (iResult == 0) {
            
                m_bBufferAdded = true;
            
            
                m_pBDist = new BinomialDist*[m_iNumThreads];
                // for each thread we need two temporary arrays for the crossing-over 
                m_pTempGenome1 = new ulong*[m_iNumThreads];
                m_pTempGenome2 = new ulong*[m_iNumThreads];
        
#pragma omp parallel
                {
                    int iT = omp_get_thread_num();
                    m_pBDist[iT] = BinomialDist::create(m_dMutationRate, m_iNumParents*m_iGenomeSize, EPS);
                    if (m_pBDist[iT] != NULL) {

                        // two arrays for crossing over calculations
                        m_pTempGenome1[iT] = new ulong[m_iNumParents*m_iGenomeSize];
                        m_pTempGenome2[iT] = new ulong[m_iNumParents*m_iGenomeSize];
                    
                        iResult = 0;
                    } else {
                        xha_printf("Couldn't create BinomialDistribution\n");
                    }
                }

                m_pSeqIO = SequenceIOUtils<ulong>::createInstance(GENOME_DATASET_NAME.c_str(), H5T_NATIVE_ULONG, &m_aGenome, m_pAgentController, m_pvDeadList, m_iNumParents*m_iNumBlocks);

            } else {
                xha_printf("Couldn't add buffer to controller\n");
            }
        } else {
            xha_printf("[Genetics] This module expects %d bit nucleotides, but the attruibute specifies %d bit nucleotides\n", U::BITSINNUC, m_iBitsPerNuc);
        }

    } else {
        xha_printf("Bad values for genome size or num crossovers or mutation rate\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// initialize
//
template<typename T, class U>
int Genetics<T,U>::initialize(float fTime) { 
    return 0; 
}


//----------------------------------------------------------------------------
// makeOffspring
//   actual creation of babies
//   (called from SPopulation::performBirths()
//
template<typename T, class U>
int  Genetics<T,U>::makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex) {
    int iResult = 0;

    // we have to manipulate inside of a layer buf array
    // better to treat them as normal arrays
    // this is safe because a genome is alway inside  single layer
    ulong *pBabyGenome  = &(m_aGenome[iBabyIndex]);
    ulong *pGenome1     = &(m_aGenome[iMotherIndex]);
    ulong *pGenome2     = &(m_aGenome[iFatherIndex]);

    // clear baby genome
    memset(pBabyGenome, 0, m_iNumParents*m_iNumBlocks*sizeof(ulong));

    int iT = omp_get_thread_num();

    // which strand to chose from genome 1 for baby genome
    int i1 = (int)(m_iNumParents*1.0*m_apWELL[iT]->wrandd());
    // which strand to chose from genome 2 for baby genome
    int i2 = (int)(m_iNumParents*1.0*m_apWELL[iT]->wrandd());


    if (m_iNumCrossOvers > 0) {
        // crossover wants the genome size, not the number of blocks
        U::crossOver(m_pTempGenome1[iT], pGenome1, m_iGenomeSize, m_iNumCrossOvers, m_apWELL[iT]);
        U::crossOver(m_pTempGenome2[iT], pGenome2, m_iGenomeSize, m_iNumCrossOvers, m_apWELL[iT]);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempGenome1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempGenome2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == -1) {
        // free recombination
        U::freeReco(m_pTempGenome1[iT], pGenome1, m_iNumBlocks, m_apWELL[iT]);
        U::freeReco(m_pTempGenome2[iT], pGenome2, m_iNumBlocks, m_apWELL[iT]);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempGenome1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempGenome2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == 0) {
        // no cross over: copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              pGenome1+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, pGenome2+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
    }

    // perhaps some mutations...
    if (m_dMutationRate > 0) {
        int iNumMutations = m_pBDist[iT]->getN(m_apWELL[iT]->wrandd());
        if (iNumMutations > 0) {
            U::mutateNucs(pBabyGenome, m_iNumParents*m_iGenomeSize, iNumMutations, m_apWELL[iT]);
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDFSafe
//  write additional data to the group
//  Here: create a new Dataset for the genome.
//  Write genomes sequentially from all cells.
//  This method does not change the agent- or genome array
//  Go through all cells, and use the BufLayers of m_aGenome as slabs 
//
template<typename T, class U>
int Genetics<T,U>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    xha_printf("[Genetics<T,U>::writeAdditionalDataQDF] Writing Genomes\n");
    iResult = m_pSeqIO->writeSequenceDataQDF(hSpeciesGroup,this->m_pPop->getNumAgentsEffective());
    return iResult;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//  read additional data from the group
//  Read genomes sequentially from all cells which contain agents (1 per agent)
//
// Ass: agents have been read
//      There is exactly one genome per agent
//      Genomes ordered as agents
//  
// The difficulty is that a buffer containing a slab of data from the file
// may contain genome for one or more cells, od the genome for a cell may be 
// spread over several slabs.
// 
template<typename T, class U>
int Genetics<T,U>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    iResult = m_pSeqIO->readSequenceDataQDF(hSpeciesGroup,BUFSIZE_READ_GENOMES);
    return iResult;
}


//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_GENETICS_GENOME_SIZE, 
//    ATTR_GENETICS_NUM_CROSSOVER
//    ATTR_GENETICS_MUTATION_RATE
//    ATTR_GENETICS_INITIAL_MUTS
//    ATTR_GENETICS_CREATE_NEW_GENOME
//    ATTR_GENETICS_BITS_PER_NUC 
//
template<typename T, class U>
int Genetics<T,U>::writeAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_MUTATION_RATE,  1, &m_dMutationRate);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
    }
    if (iResult == 0) {
        iResult = qdf_insertSAttribute(hSpeciesGroup, ATTR_GENETICS_INITIAL_MUTS, m_pGenomeCreator->getInitString());
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_CREATE_NEW_GENOME, 1, &m_bCreateNewGenome);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_BITS_PER_NUC, 1, &m_iBitsPerNuc);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_GENETICS_GENOME_SIZE, 
//    ATTR_GENETICS_NUM_CROSSOVER
//    ATTR_GENETICS_MUTATION_RATE
//    ATTR_GENETICS_INITIAL_MUTS
//    ATTR_GENETICS_CREATE_NEW_GENOME
//    ATTR_GENETICS_BITS_PER_NUC 
//
template<typename T, class U>
int Genetics<T,U>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_GENOME_SIZE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_MUTATION_RATE,  1, &m_dMutationRate);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_MUTATION_RATE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_NUM_CROSSOVER);
        }
    }
    
    if (iResult == 0) {
        std::string sTemp = "";
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_GENETICS_INITIAL_MUTS, sTemp);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_INITIAL_MUTS);
        } else {
            iResult = m_pGenomeCreator->determineInitData(sTemp);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_CREATE_NEW_GENOME, 1, &m_bCreateNewGenome);
        if (iResult != 0) {
            LOG_WARNING("[Genetics] couldn't read attribute [%s]; setting value to false", ATTR_GENETICS_CREATE_NEW_GENOME);
            m_bCreateNewGenome = 0;
            iResult = 0;
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_BITS_PER_NUC, 1, &m_iBitsPerNuc);
        if (iResult != 0) {
            LOG_WARNING("[Genetics] couldn't read attribute [%s]; setting value to %d", ATTR_GENETICS_BITS_PER_NUC, U::BITSINNUC);
            m_iBitsPerNuc = U::BITSINNUC;
            iResult = 0;
        } else {
            if (m_iBitsPerNuc != U::BITSINNUC) {
                iResult = -1;
                LOG_ERROR("[Genetics] value of [%s] does not match current GeneUtils::BITSINNUC", ATTR_GENETICS_BITS_PER_NUC);
            }
        }
    }
    
    xha_printf("[Genetics] ExtractParamsQDF:res %d\n", iResult);
    
    if (iResult == 0) {
        // we must call init() before callnig readAdditionalDataQDF()
        iResult = init();
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// dumpAdditionalDataQDF
//
template<typename T, class U>
int Genetics<T,U>::dumpAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    iResult = m_pSeqIO->dumpSequenceDataQDF(hSpeciesGroup);
    return iResult;
}


//-----------------------------------------------------------------------------
// restoreAdditionalDataQDF
//
template<typename T, class U>
int Genetics<T,U>::restoreAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    iResult = m_pSeqIO->restoreSequenceDataQDF(hSpeciesGroup);
    return iResult;
 }


//-----------------------------------------------------------------------------
// dumpStateQDF
//
template<typename T, class U>
int Genetics<T,U>::dumpStateQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    xha_printf("Genetics WELLState before dump\n");
    showWELLStates();

    if (m_bOwnWELL && (iResult == 0)) {
        iResult = dumpWELL(m_apWELL, m_iNumThreads, ATTR_GENETICS_NAME, hSpeciesGroup);
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// restoreStateQDF
//
template<typename T, class U>
int Genetics<T,U>::restoreStateQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    if (m_bOwnWELL) {
        iResult = restoreWELL(m_apWELL, m_iNumThreads, ATTR_GENETICS_NAME, hSpeciesGroup);
    }
    xha_printf("Genetics WELLState after restore\n");
    showWELLStates();

    return iResult;
        
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T, class U>
int Genetics<T,U>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;
    std::string sTemp; 

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_GENETICS_GENOME_SIZE,       &m_iGenomeSize);
        iResult += getAttributeVal(mParams, ATTR_GENETICS_NUM_CROSSOVER,     &m_iNumCrossOvers);
        iResult += getAttributeVal(mParams, ATTR_GENETICS_MUTATION_RATE,     &m_dMutationRate);
        iResult += getAttributeVal(mParams, ATTR_GENETICS_CREATE_NEW_GENOME, &m_bCreateNewGenome);
        iResult += getAttributeVal(mParams, ATTR_GENETICS_BITS_PER_NUC,      &m_iBitsPerNuc);
    
        int iResult2 = getAttributeStr(mParams, ATTR_GENETICS_INITIAL_MUTS,  sTemp);
        if (iResult2 == 0) {
            iResult2 = m_pGenomeCreator->determineInitData(sTemp);
            if (iResult2 != 0) {
                xha_printf("value for [%s] is malformed or unknown: [%s]\n", ATTR_GENETICS_INITIAL_MUTS, sTemp);
                iResult = -1;
            } 
        }
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// createInitialGenomes
//   only do this if the buffer has been already added to the controller
//
template<typename T, class U>
int Genetics<T,U>::createInitialGenomes(int iNumGenomes) {
    int iResult = -1;
    if (m_bBufferAdded) {
        if (m_bCreateNewGenome != 0) {
            iResult = 0;
            m_bCreateNewGenome = false;
            iResult = m_pGenomeCreator->createInitialGenomes(m_iGenomeSize, iNumGenomes, m_aGenome, m_apWELL);
        } else {
            xha_printf("[Genetics::createInitialGenomes] No new Genome created\n");
            iResult = 0;
        }
    } else {
        xha_printf("Genetics has not been initialized\n");
    }   
    return iResult;
}


//-----------------------------------------------------------------------------
// showWELLStates
//
template<typename T, class U>
void Genetics<T,U>::showWELLStates() {
    for (int i = 0; i < m_iNumThreads; i++) {
        xha_printf("[%08x] ", m_apWELL[i]->getIndex());
        const uint32_t *p = m_apWELL[i]->getState();
        for (uint j = 0; j < STATE_SIZE;j++) {
            xha_printf("%08x ", p[j]);
        }
        xha_printf("\n");
    }
}


//-----------------------------------------------------------------------------
// isEqual
//
template<typename T, class U>
bool Genetics<T,U>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Genetics<T,U>* pA = static_cast<Genetics<T,U>*>(pAction);
    if ((m_iGenomeSize    == pA->m_iGenomeSize) &&
        (m_iNumCrossOvers == pA->m_iNumCrossOvers) &&
        (m_dMutationRate  == pA->m_dMutationRate) &&
        ((!bStrict) || (m_bCreateNewGenome == pA->m_bCreateNewGenome)) &&
        ((!bStrict) || (m_pGenomeCreator->getInitString() == pA->m_pGenomeCreator->getInitString())) &&
        (m_iBitsPerNuc    == pA->m_iBitsPerNuc)) {
        bEqual = true;
    } 
    return bEqual;
}

