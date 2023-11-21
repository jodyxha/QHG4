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
#include "stdstrutilsT.h"

#include "BinomialDist.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "WELLUtils.h"
#include "WELLDumpRestore.h"

#include "GeneUtils.h"
#include "SequenceIOUtils.h"


#include "clsutils.cpp"
#include "ParamProvider2.h"
#include "Hybrids.h"

// needed for BinomialDistribution
#define EPS 1e-6

static const uint  BUFSIZE_READ_GENOMES2=1000;

#define HYBRIDIZATION_NAME "Hybridization"

#define MAX_INIT_NAME 32
// this number must changed if the parameters change
template<typename T, class U>
int Hybrids<T,U>::NUM_HYBRIDS_PARAMS = 6;

template<typename T, class U>
const char *Hybrids<T,U>::asNames[] = {
    ATTR_HYBRIDS_GENOME_SIZE,
    ATTR_HYBRIDS_NUM_CROSSOVER,
    ATTR_HYBRIDS_MUTATION_RATE,
    ATTR_HYBRIDS_INITIAL_MUTS,
    ATTR_HYBRIDS_INITIAL_ORIS,
    ATTR_HYBRIDS_CREATE_NEW_GENOME,
    ATTR_HYBRIDS_BITS_PER_NUC};

//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
Hybrids<T,U>::Hybrids(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL)  
    : Action<T>(pPop, pCG, ATTR_HYBRIDS_NAME,sID),
      m_pAgentController(pAgentController),
      m_pWriteCopyControllerGenome(NULL),
      m_pWriteCopyControllerOrigome(NULL),
      m_afHybridization(NULL),
      m_apWELL(apWELL),
      m_iGenomeSize(-1),
      m_pBDist(NULL),
      m_iNumCrossOvers(-1),
      m_dMutationRate(-1),
      m_pTempSeq1(NULL),
      m_pTempSeq2(NULL),
      m_pMasks(NULL),
      m_iNumBlocks(0),
      m_bCreateNewGenome(0),
      m_iBitsPerNuc(U::BITSINNUC),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_pGenomeCreator(NULL),
      m_pOrigomeCreator(NULL),
      m_iNumParents(2),
      m_bOwnWELL(false),
      m_pSeqIOGenome(NULL),
    m_pSeqIOOrigome(NULL) {


    m_iNumThreads = omp_get_max_threads();

    // GenomeCreator must exist before init() is called
    m_pGenomeCreator  = new GenomeCreator<U>(m_iNumParents);
    m_pOrigomeCreator = new OrigomeCreator<U>(m_iNumParents);

    m_afHybridization = new float[this->m_pCG->m_iNumCells];
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));

}


//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
Hybrids<T,U>::Hybrids(SPopulation<T> *pPop,  SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed)  
    : Action<T>(pPop, pCG, ATTR_HYBRIDS_NAME,sID),
      m_pAgentController(pAgentController),
      m_pWriteCopyControllerGenome(NULL),
      m_pWriteCopyControllerOrigome(NULL),
      m_afHybridization(NULL),
      m_apWELL(NULL),
      m_iGenomeSize(-1),
      m_pBDist(NULL),
      m_iNumCrossOvers(-1),
      m_dMutationRate(-1),
      m_pTempSeq1(NULL),
      m_pTempSeq2(NULL),
      m_pMasks(NULL),
      m_iNumBlocks(0),
      m_bCreateNewGenome(0),
      m_iBitsPerNuc(U::BITSINNUC),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_pGenomeCreator(NULL),
      m_pOrigomeCreator(NULL),
      m_iNumParents(2),
      m_bOwnWELL(true),
      m_pSeqIOGenome(NULL),
      m_pSeqIOOrigome(NULL) {

    m_iNumThreads = omp_get_max_threads();

    // GenomeCreator must exist before init() is called
    m_pGenomeCreator  = new GenomeCreator<U>(m_iNumParents);
    m_pOrigomeCreator = new OrigomeCreator<U>(m_iNumParents);
   
    this->m_vNames.insert(this->m_vNames.end(), asNames, asNames+sizeof(asNames)/sizeof(char*));
    m_afHybridization = new float[this->m_pCG->m_iNumCells];

    // we need to build our own WELLs    
    stdprintf("[Hybrids::Hybrids] using %u as seed for %d WELLs\n", iSeed, m_iNumThreads);
    m_apWELL = WELLUtils::buildWELLs(m_iNumThreads, iSeed);
 
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T, class U>
Hybrids<T,U>::~Hybrids() {
    deleteAllocated();
    if (m_pGenomeCreator != NULL) {
        delete m_pGenomeCreator;
    }
    if (m_pOrigomeCreator != NULL) {
        delete m_pOrigomeCreator;
    }
    if (m_afHybridization != NULL) {
        delete[] m_afHybridization;
    }

    if (m_bOwnWELL && (m_apWELL != NULL)) {
        WELLUtils::destroyWELLs(m_apWELL, m_iNumThreads);
    }
}


//-----------------------------------------------------------------------------
// deleteAllocated
//
template<typename T, class U>
void Hybrids<T,U>::deleteAllocated() {

    if (m_pTempSeq1 != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pTempSeq1[iT] != NULL) {
                delete[] m_pTempSeq1[iT];
            }
        }
        delete[] m_pTempSeq1;
    }

    if (m_pTempSeq2 != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pTempSeq2[iT] != NULL) {
                delete[] m_pTempSeq2[iT];
            }
        }
        delete[] m_pTempSeq2;
    }

    if (m_pMasks != NULL) {
        delete[] m_pMasks;
    }

    if (m_pBDist != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pBDist[iT] != NULL) {
                delete m_pBDist[iT];
            }
        }
        delete[] m_pBDist;
    }

    if (m_pWriteCopyControllerGenome != NULL) {
        delete m_pWriteCopyControllerGenome;
    }

    if (m_pWriteCopyControllerOrigome != NULL) {
        delete m_pWriteCopyControllerOrigome;
    }

    if (m_bBufferAdded) {
        m_pAgentController->removeBuffer(static_cast<LBBase *>(&m_aGenome));
        m_pAgentController->removeBuffer(static_cast<LBBase *>(&m_aOrigome));
        m_bBufferAdded = false;
    }

    if (m_pSeqIOGenome != NULL) {
        delete m_pSeqIOGenome;
    }
    if (m_pSeqIOOrigome != NULL) {
        delete m_pSeqIOOrigome;
    }
}


//-----------------------------------------------------------------------------
// init
//   must be called *after* params have been read, 
//   but *before* reading the genome
//   
template<typename T, class U>
int Hybrids<T,U>::init() {
    int iResult = -1;
    stdprintf("init called\n");
    deleteAllocated();

    
    if ((m_iGenomeSize > 0) && 
        /*(m_dMutationRate >= 0) && */(m_dMutationRate <= 1)) {
        
        if (m_iBitsPerNuc == U::BITSINNUC) {
            // number of longs needed to hold genome
            m_iNumBlocks  = U::numNucs2Blocks(m_iGenomeSize);

            // initialize the buffer ...
            stdprintf("initializing m_aGenome with (%d, %d)\n", m_pAgentController->getLayerSize(), m_iNumParents*m_iGenomeSize);
            m_aGenome.init(m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
        
            // ... and add it to the AgentController
            iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aGenome));


            // layeredBuffer and controller for writing a copy of the actual agent array
            if (iResult == 0) {

                m_pWriteCopyControllerGenome = new LBController;

                m_aWriteCopyGenome.init( m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
                m_pWriteCopyControllerGenome->init( m_pAgentController->getLayerSize());
                iResult = m_pWriteCopyControllerGenome->addBuffer(static_cast<LBBase *>(&m_aWriteCopyGenome));
                m_pWriteCopyControllerGenome->addLayer();
            }


            if (iResult == 0) {
                m_aOrigome.init(m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
                iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aOrigome));
                // do we need a separate WriteCopyController
                if (iResult == 0) {

                    m_pWriteCopyControllerOrigome = new LBController;

                    m_aWriteCopyOrigome.init( m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
                    m_pWriteCopyControllerOrigome->init( m_pAgentController->getLayerSize());
                    iResult = m_pWriteCopyControllerOrigome->addBuffer(static_cast<LBBase *>(&m_aWriteCopyOrigome));
                    m_pWriteCopyControllerOrigome->addLayer();
                }
            }


            
            if (iResult == 0) {
            
                m_bBufferAdded = true;
            
            
                m_pBDist = new BinomialDist*[m_iNumThreads];
                // for each thread we need two temporary arrays for the crossing-over 
                m_pTempSeq1 = new ulong*[m_iNumThreads];
                m_pTempSeq2 = new ulong*[m_iNumThreads];
                m_pMasks = new ulong[m_iNumBlocks];
        
#pragma omp parallel
                {
                    int iT = omp_get_thread_num();
                    m_pBDist[iT] = BinomialDist::create(m_dMutationRate, m_iNumParents*m_iGenomeSize, EPS);
                    if (m_pBDist[iT] != NULL) {

                        // two arrays for crossing over calculations
                        m_pTempSeq1[iT] = new ulong[m_iNumParents*m_iNumBlocks];
                        m_pTempSeq2[iT] = new ulong[m_iNumParents*m_iNumBlocks];
                    
                        iResult = 0;
                    } else {
                        stdprintf("Couldn't create BinomialDistribution\n");
                    }
                }

                m_pSeqIOGenome  = SequenceIOUtils<ulong>::createInstance(GENOME_DATASET_NAME, H5T_NATIVE_ULONG, &m_aGenome, m_pAgentController, m_pvDeadList, m_iNumParents*m_iNumBlocks);
                m_pSeqIOOrigome = SequenceIOUtils<ulong>::createInstance(ORIGOME_DATASET_NAME, H5T_NATIVE_ULONG, &m_aOrigome, m_pAgentController, m_pvDeadList, m_iNumParents*m_iNumBlocks);

            } else {
                stdprintf("Couldn't add buffer to controller\n");
            }
        } else {
            stdprintf("[Hybrids] This module expects %d bit nucleotides, but the attruibute specifies %d bit nucleotides\n", U::BITSINNUC, m_iBitsPerNuc);
        }

    } else {
        stdprintf("Bad values for genome size or num crossovers or mutation rate\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// initialize
//
template<typename T, class U>
int Hybrids<T,U>::initialize(float fTime) { 
    return 0; 
}


//----------------------------------------------------------------------------
// finalize
//
template<typename T, class U>
int Hybrids<T,U>::finalize(float fTime) { 
    int iResult = calcAgentHybridizations();
    return iResult; 
}
 

//----------------------------------------------------------------------------
// preWrite
//
template<typename T, class U>
int Hybrids<T,U>::preWrite(float fTime) { 
    return determineHybridization();
}




//----------------------------------------------------------------------------
// makeOffspring
//   actual creation of babies
//   (called from SPopulation::performBirths()
//
template<typename T, class U>
int  Hybrids<T,U>::makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex) {
    int iResult = 0;

    // we have to manipulate inside of a layer buf array
    // better to treat them as normal arrays
    // this is safe because a genome is alway inside  single layer
    ulong *pBabyGenome  = &(m_aGenome[iBabyIndex]);
    ulong *pGenome1     = &(m_aGenome[iMotherIndex]);
    ulong *pGenome2     = &(m_aGenome[iFatherIndex]);

    ulong *pBabyOrigome  = &(m_aOrigome[iBabyIndex]);
    ulong *pOrigome1     = &(m_aOrigome[iMotherIndex]);
    ulong *pOrigome2     = &(m_aOrigome[iFatherIndex]);

    // clear baby genome
    memset(pBabyGenome, 0, m_iNumParents*m_iNumBlocks*sizeof(ulong));

    int iT = omp_get_thread_num();

    // which strand to chose from genome 1 for baby genome
    int i1 = (int)(m_iNumParents*1.0*m_apWELL[iT]->wrandd());
    // which strand to chose from genome 2 for baby genome
    int i2 = (int)(m_iNumParents*1.0*m_apWELL[iT]->wrandd());

    if (m_iNumCrossOvers > 0) {
        blockbreak_t vBlockBreaks;
        U::makeBlockBreaks(vBlockBreaks, m_iNumBlocks, m_iNumCrossOvers, m_apWELL[iT]);
        // crossover wants the genome size, not the number of blocks
        U::crossOver2(m_pTempSeq1[iT], pGenome1, m_iNumBlocks, vBlockBreaks);
        U::crossOver2(m_pTempSeq2[iT], pGenome2, m_iNumBlocks, vBlockBreaks);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempSeq1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempSeq2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

        // crossover wants the genome size, not the number of blocks
        U::crossOver2(m_pTempSeq1[iT], pOrigome1, m_iNumBlocks, vBlockBreaks);
        U::crossOver2(m_pTempSeq2[iT], pOrigome2, m_iNumBlocks, vBlockBreaks);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyOrigome,              m_pTempSeq1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyOrigome+m_iNumBlocks, m_pTempSeq2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == -1) {
        ulong *pMasks = U::makeFreeMasks(m_pMasks, m_iNumBlocks, m_apWELL[iT]);
        // full recombination
        U::freeReco2(m_pTempSeq1[iT], pGenome1, pMasks, m_iNumBlocks);
        U::freeReco2(m_pTempSeq2[iT], pGenome2, pMasks, m_iNumBlocks);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempSeq1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempSeq2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

        U::freeReco2(m_pTempSeq1[iT], pOrigome1, pMasks, m_iNumBlocks);
        U::freeReco2(m_pTempSeq2[iT], pOrigome2, pMasks, m_iNumBlocks);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyOrigome,              m_pTempSeq1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyOrigome+m_iNumBlocks, m_pTempSeq2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == 0) {
        // no cross over: copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              pGenome1+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, pGenome2+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyOrigome,              pOrigome1+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyOrigome+m_iNumBlocks, pOrigome2+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
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
// calcAgentHybridizations
//
template<typename T, class U>
int Hybrids<T,U>::calcAgentHybridizations() {
    // loop through all agents
    int iFirst =  this->m_pPop->getFirstAgentIndex();
    int iLast  =  this->m_pPop->getLastAgentIndex();

#pragma omp parallel for 
    for (int iA = iFirst; iA <= iLast; iA++) {
        T *pa = &(this->m_pPop->m_aAgents[iA]);
        if (pa->m_iLifeState != 0) {
            ulong *pOrigome  = &(m_aOrigome[iA]);
            int iSetCount = 0;
            for (int j = 0; j < m_iNumBlocks; j++) {
                iSetCount += U::bitcount(pOrigome[j]);
            }
            iSetCount /= U::BITSINNUC;
            float fH = (1.0*iSetCount)/m_iGenomeSize; 
            pa ->m_fHybridization = fH;
        }
    }
    return 0;
}



//----------------------------------------------------------------------------
// determineHybridization
//
template<typename T, class U>
int Hybrids<T,U>::determineHybridization() {
    int iResult = 0;

#pragma omp parallel for
    for (uint i = 0; i < this->m_pCG->m_iNumCells; i++) {
        m_afHybridization[i] = fNaN;
    }
    // loop through all agents
    int iFirst =  this->m_pPop->getFirstAgentIndex();
    int iLast  =  this->m_pPop->getLastAgentIndex();
 
    for (int iA = iFirst; iA <= iLast; iA++) {
        T *pa = &(this->m_pPop->m_aAgents[iA]);
        if (pa->m_iLifeState != 0) {
            int iCellIndex = pa->m_iCellIndex;
            
            if (std::isnan(m_afHybridization[iCellIndex])) {
                m_afHybridization[iCellIndex] = 0;
            }
            // count bits and divide by bitspernuc
            ulong *pOrigome  = &(m_aOrigome[iA]);
            int iSetCount = 0;
            for (int j = 0; j < m_iNumBlocks; j++) {
                iSetCount += U::bitcount(pOrigome[j]);
            }
            iSetCount /= U::BITSINNUC;
            float fH = (1.0*iSetCount)/m_iGenomeSize; 
            pa->m_fHybridization = fH;

            m_afHybridization[iCellIndex] += fH/this->m_pPop->getNumAgents(iCellIndex);
            if (std::isnan( m_afHybridization[iCellIndex])) {
                stdprintf("nan in cell %d ag %lu, fH %f, numA %lu\n", iCellIndex, pa->m_ulID, fH, this->m_pPop->getNumAgents(iCellIndex));
            }
        }
    } 

    //   count  origome fields with value 1 or 3
    //    *(float*)((char*)pa + m_iHybridizationOffset) = iSetCount/m_iGenome 

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
int Hybrids<T,U>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    stdprintf("[Hybrids<T,U>::writeAdditionalDataQDF] Writing Genomes\n");

    iResult = m_pSeqIOGenome->writeSequenceDataQDF(hSpeciesGroup,this->m_pPop->getNumAgentsEffective());
    if (iResult == 0) {
        iResult = m_pSeqIOOrigome->writeSequenceDataQDF(hSpeciesGroup,this->m_pPop->getNumAgentsEffective());
        if (iResult == 0) {
            iResult = qdf_writeArray(hSpeciesGroup, HYBRIDIZATION_NAME, this->m_pCG->m_iNumCells, m_afHybridization);
        }
    }
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
int Hybrids<T,U>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    iResult = m_pSeqIOGenome->readSequenceDataQDF(hSpeciesGroup,BUFSIZE_READ_GENOMES2);
    if (iResult == 0) {
        iResult = m_pSeqIOOrigome->readSequenceDataQDF(hSpeciesGroup,BUFSIZE_READ_GENOMES2);
        if (iResult == 0) {
            iResult = qdf_readArray(hSpeciesGroup, HYBRIDIZATION_NAME, this->m_pCG->m_iNumCells, m_afHybridization);
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_HYBRIDS_GENOME_SIZE, 
//    ATTR_HYBRIDS_NUM_CROSSOVER
//    ATTR_HYBRIDS_MUTATION_RATE
//    ATTR_HYBRIDS_INITIAL_MUTS
//    ATTR_HYBRIDS_INITIAL_ORIS
//    ATTR_HYBRIDS_CREATE_NEW_GENOME
//    ATTR_HYBRIDS_BITS_PER_NUC 
//
template<typename T, class U>
int Hybrids<T,U>::writeAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_HYBRIDS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_HYBRIDS_MUTATION_RATE,  1, &m_dMutationRate);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_HYBRIDS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
    }
    if (iResult == 0) {
        iResult = qdf_insertSAttribute(hSpeciesGroup, ATTR_HYBRIDS_INITIAL_MUTS, m_pGenomeCreator->getInitString());
    }
    if (iResult == 0) {
        iResult = qdf_insertSAttribute(hSpeciesGroup, ATTR_HYBRIDS_INITIAL_ORIS, m_pOrigomeCreator->getInitString());
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_HYBRIDS_CREATE_NEW_GENOME, 1, &m_bCreateNewGenome);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_HYBRIDS_BITS_PER_NUC, 1, &m_iBitsPerNuc);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_HYBRIDS_GENOME_SIZE, 
//    ATTR_HYBRIDS_NUM_CROSSOVER
//    ATTR_HYBRIDS_MUTATION_RATE
//    ATTR_HYBRIDS_INITIAL_MUTS
//    ATTR_HYBRIDS_INITIAL_ORIS
//    ATTR_HYBRIDS_CREATE_NEW_GENOME
//    ATTR_HYBRIDS_BITS_PER_NUC 
//
template<typename T, class U>
int Hybrids<T,U>::extractAttributesQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBRIDS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
        if (iResult != 0) {
            LOG_ERROR("[Hybrids] couldn't read attribute [%s]", ATTR_HYBRIDS_GENOME_SIZE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBRIDS_MUTATION_RATE,  1, &m_dMutationRate);
        if (iResult != 0) {
            LOG_ERROR("[Hybrids] couldn't read attribute [%s]", ATTR_HYBRIDS_MUTATION_RATE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBRIDS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
        if (iResult != 0) {
            LOG_ERROR("[Hybrids] couldn't read attribute [%s]", ATTR_HYBRIDS_NUM_CROSSOVER);
        }
    }
    
    if (iResult == 0) {
        std::string sTemp = "";
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_HYBRIDS_INITIAL_MUTS, sTemp);
        if (iResult != 0) {
            LOG_ERROR("[Hybrids] couldn't read attribute [%s]", ATTR_HYBRIDS_INITIAL_MUTS);
        } else {
            iResult = m_pGenomeCreator->determineInitData(sTemp);
        }
    }
    
    if (iResult == 0) {
        std::string sTemp = "";
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_HYBRIDS_INITIAL_ORIS, sTemp);
        if (iResult != 0) {
            LOG_ERROR("[Hybrids] couldn't read attribute [%s]", ATTR_HYBRIDS_INITIAL_ORIS);
        } else {
            iResult = m_pOrigomeCreator->determineInitData(sTemp);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBRIDS_CREATE_NEW_GENOME, 1, &m_bCreateNewGenome);
        if (iResult != 0) {
            LOG_WARNING("[Hybrids] couldn't read attribute [%s]; setting value to false", ATTR_HYBRIDS_CREATE_NEW_GENOME);
            m_bCreateNewGenome = 0;
            iResult = 0;
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_HYBRIDS_BITS_PER_NUC, 1, &m_iBitsPerNuc);
        if (iResult != 0) {
            LOG_WARNING("[Hybrids] couldn't read attribute [%s]; setting value to %d", ATTR_HYBRIDS_BITS_PER_NUC, U::BITSINNUC);
            m_iBitsPerNuc = U::BITSINNUC;
            iResult = 0;
        } else {
            if (m_iBitsPerNuc != U::BITSINNUC) {
                iResult = -1;
                LOG_ERROR("[Hybrids] value of [%s] does not match current GeneUtils::BITSINNUC", ATTR_HYBRIDS_BITS_PER_NUC);
            }
        }
    }
    
    stdprintf("[Hybrids] ExtractParamsQDF:res %d\n", iResult);
    
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
int Hybrids<T,U>::dumpAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    iResult = m_pSeqIOGenome->dumpSequenceDataQDF(hSpeciesGroup);
    if (iResult == 0) {
        iResult = m_pSeqIOOrigome->dumpSequenceDataQDF(hSpeciesGroup);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// restoreAdditionalDataQDF
//
template<typename T, class U>
int Hybrids<T,U>::restoreAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    iResult = m_pSeqIOGenome->restoreSequenceDataQDF(hSpeciesGroup);
    if (iResult == 0) {
        iResult = m_pSeqIOOrigome->restoreSequenceDataQDF(hSpeciesGroup);
    }
    return iResult;
 }


//-----------------------------------------------------------------------------
// dumpStateQDF
//
template<typename T, class U>
int Hybrids<T,U>::dumpStateQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    if (m_bOwnWELL && (iResult == 0)) {
        iResult = dumpWELL(m_apWELL, m_iNumThreads, ATTR_HYBRIDS_NAME, hSpeciesGroup);
    }
    stdprintf("Hybrids WELLState after dump\n");
    WELLUtils::showStates(m_apWELL, m_iNumThreads, false);
    return iResult;
}



//-----------------------------------------------------------------------------
// restoreStateQDF
//
template<typename T, class U>
int Hybrids<T,U>::restoreStateQDF(hid_t hSpeciesGroup) {
    int iResult = 0;
    if (m_bOwnWELL) {
        iResult = restoreWELL(m_apWELL, m_iNumThreads, ATTR_HYBRIDS_NAME, hSpeciesGroup);
    }
    return iResult;
        
}


//-----------------------------------------------------------------------------
// tryGetAttributes
//   
//
template<typename T, class U>
int Hybrids<T,U>::tryGetAttributes(const ModuleComplex *pMC) {
    int iResult = -1;


    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        std::string sTemp = ""; 
    
        iResult += getAttributeVal(mParams, ATTR_HYBRIDS_GENOME_SIZE,       &m_iGenomeSize);
        iResult += getAttributeVal(mParams, ATTR_HYBRIDS_NUM_CROSSOVER,     &m_iNumCrossOvers);
        iResult += getAttributeVal(mParams, ATTR_HYBRIDS_MUTATION_RATE,     &m_dMutationRate);
        iResult += getAttributeVal(mParams, ATTR_HYBRIDS_CREATE_NEW_GENOME, &m_bCreateNewGenome);
        iResult += getAttributeVal(mParams, ATTR_HYBRIDS_BITS_PER_NUC,      &m_iBitsPerNuc);
    
        int iResult2 = getAttributeStr(mParams, ATTR_HYBRIDS_INITIAL_MUTS,  sTemp);
        if (iResult2 == 0) {
            iResult2 = m_pGenomeCreator->determineInitData(sTemp);
            if (iResult2 != 0) {
                stdprintf("value for [%s] is malformed or unknown: [%s]\n", ATTR_HYBRIDS_INITIAL_MUTS, sTemp);
                iResult = -1;
            } 
        }

        iResult2 = getAttributeStr(mParams, ATTR_HYBRIDS_INITIAL_ORIS,  sTemp);
        if (iResult2 == 0) {
            iResult2 = m_pOrigomeCreator->determineInitData(sTemp);
            if (iResult2 != 0) {
                stdprintf("value for [%s] is malformed or unknown: [%s]\n", ATTR_HYBRIDS_INITIAL_ORIS, sTemp);
                iResult = -1;
            } 
        }
        if (iResult == 0) {
            iResult = init();
        }

    }
    return iResult;
}




//-----------------------------------------------------------------------------
// createInitialGenomes
//   only do this if the buffer has been already added to the controller
//
template<typename T, class U>
int Hybrids<T,U>::createInitialGenomes(int iNumGenomes) {
    int iResult = -1;
    if (m_bBufferAdded) {
        if (m_bCreateNewGenome != 0) {
            iResult = 0;
            m_bCreateNewGenome = false;
            iResult = m_pGenomeCreator->createInitialGenomes(m_iGenomeSize, iNumGenomes, m_aGenome, m_apWELL);
            if (iResult == 0) {
                iResult = m_pOrigomeCreator->createInitialOrigomes(m_iGenomeSize, iNumGenomes, m_aOrigome);
            }
        } else {
            stdprintf("[Hybrids::createInitialGenomes] No new Genome created\n");
            iResult = 0;
        }
    } else {
        stdprintf("Hybrids has not been initialized\n");
    }   
    return iResult;
}

/*
//-----------------------------------------------------------------------------
// showWELLStates
//
template<typename T, class U>
void Hybrids<T,U>::showWELLStates() {
    for (int i = 0; i < m_iNumThreads; i++) {
        printf("[%08x] ", m_apWELL[i]->getIndex());
        const uint32_t *p = m_apWELL[i]->getState();
        for (uint j = 0; j < STATE_SIZE;j++) {
            printf("%08x ", p[j]);
        }
        printf("\n");
    }
}
*/

//-----------------------------------------------------------------------------
// isEqual
//   
template<typename T, class U>
bool Hybrids<T,U>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    Hybrids<T,U>* pA = static_cast<Hybrids<T,U>*>(pAction);
    if ((m_iGenomeSize    == pA->m_iGenomeSize) &&
        (m_dMutationRate  == pA->m_dMutationRate) &&
        (m_iNumCrossOvers == pA->m_iNumCrossOvers) &&
        ((!bStrict) || (m_bCreateNewGenome == pA->m_bCreateNewGenome)) &&
        ((!bStrict) || (m_pGenomeCreator->getInitString() == pA->m_pGenomeCreator->getInitString())) &&
        (m_iBitsPerNuc    == pA->m_iBitsPerNuc)) {
        bEqual = true;
    } 
    return bEqual;
}

