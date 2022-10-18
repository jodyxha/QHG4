#ifndef __HYBRIDS_H__
#define __HYBRIDS_H__

#include <hdf5.h>

#include <vector>

#include "Action.h"
#include "ParamProvider2.h"
#include "LayerArrBuf.h"
#include "WELL512.h"
#include "SequenceIOUtils.h"
#include "GenomeCreator.h"
#include "OrigomeCreator.h"

class LBController;
class BinomialDist;

#define ATTR_HYBRIDS_NAME              "Hybrids"
#define ATTR_HYBRIDS_GENOME_SIZE       "Hybrids_genome_size"
#define ATTR_HYBRIDS_NUM_CROSSOVER     "Hybrids_num_crossover"
#define ATTR_HYBRIDS_MUTATION_RATE     "Hybrids_mutation_rate"
#define ATTR_HYBRIDS_INITIAL_MUTS      "Hybrids_initial_muts"
#define ATTR_HYBRIDS_INITIAL_ORIS      "Hybrids_initial_oris"
#define ATTR_HYBRIDS_CREATE_NEW_GENOME "Hybrids_create_new_genome"
#define ATTR_HYBRIDS_BITS_PER_NUC      "Hybrids_bits_per_nuc"

#define GENOME_DATASET_NAME    "Genome"
#define ORIGOME_DATASET_NAME   "Origome"
        

// it is not very nice to derive Genetics from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T, class U>
class Hybrids : public Action<T> {
public:

    Hybrids(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL);
    Hybrids(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed);
    virtual ~Hybrids();

    virtual int makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex);
    int determineHybridization();
    int calcAgentHybridizations();

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int dumpAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int restoreAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int dumpStateQDF(hid_t hSpeciesGroup);
    virtual int restoreStateQDF(hid_t hSpeciesGroup);

    //    void showWELLStates();

    int createInitialGenomes(int iNumGenomes);
    bool isReady() { return m_bBufferAdded;};

    // from Action
    virtual int initialize(float fTime);
    virtual int preWrite(float fTime);

    // execute does nothing
    virtual int execute(int iA, float fT) { return 0; };
    virtual int finalize(float fTime);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    int getGenomeSize() { return m_iGenomeSize;};
    LayerArrBuf<ulong> *getGenome() { return &m_aGenome;};
    ulong *getGenome(uint i) { return &m_aGenome[i];};
    LayerArrBuf<ulong> *getOrigome() { return &m_aOrigome;};
    ulong *getOrigome(uint i) { return &m_aOrigome[i];};
    int getNumBlocks() { return m_iNumBlocks; };

    bool isEqual(Action<T> *pAction, bool bStrict);
    

protected:
    static int NUM_HYBRIDS_PARAMS;
protected:
    int init();
    void deleteAllocated();

    LBController *m_pAgentController;
    LBController *m_pWriteCopyControllerGenome;
    LBController *m_pWriteCopyControllerOrigome;

    float *m_afHybridization;
    WELL512** m_apWELL;
    int           m_iGenomeSize;
    // the genomes must be controlled by ther agent controller m_AgentController
    LayerArrBuf<ulong> m_aGenome; // 1 per cell; the genomes of the agents follow each other; first allele 0 then allele 1
    LayerArrBuf<ulong> m_aOrigome; // 1 per cell; the genomes of the agents follow each other; first allele 0 then allele 1

    // auxiliary layerbuf for writing
    LayerArrBuf<ulong> m_aWriteCopyGenome; 
    LayerArrBuf<ulong> m_aWriteCopyOrigome; 

    BinomialDist **m_pBDist;

    int     m_iNumCrossOvers;
    double  m_dMutationRate;
    ulong **m_pTempSeq1;
    ulong **m_pTempSeq2;
    ulong  *m_pMasks;
    int     m_iNumBlocks;

    char    m_bCreateNewGenome;
    uint    m_iBitsPerNuc;

    int     m_iNumSetParams;
    int     m_iNumThreads;
    bool    m_bBufferAdded;
    std::vector<int> *m_pvDeadList;

    GenomeCreator<U> *m_pGenomeCreator;
    OrigomeCreator<U> *m_pOrigomeCreator;

    uint m_iNumParents;
    bool m_bOwnWELL;

    SequenceIOUtils<ulong> *m_pSeqIOGenome;
    SequenceIOUtils<ulong> *m_pSeqIOOrigome;    

    static const char *asNames[];
};


#endif
