#ifndef __GENETICS_H__
#define __GENETICS_H__

#include <hdf5.h>

#include <vector>


#include "Action.h"
#include "ParamProvider2.h"
#include "LayerArrBuf.h"
#include "WELL512.h"
#include "SequenceIOUtils.h"
#include "GenomeCreator.h"

class LBController;
class BinomialDist;

const static std::string ATTR_GENETICS_NAME                = "Genetics";
const static std::string ATTR_GENETICS_GENOME_SIZE         = "Genetics_genome_size";
const static std::string ATTR_GENETICS_NUM_CROSSOVER       = "Genetics_num_crossover";
const static std::string ATTR_GENETICS_MUTATION_RATE       = "Genetics_mutation_rate";
const static std::string ATTR_GENETICS_INITIAL_MUTS        = "Genetics_initial_muts";
const static std::string ATTR_GENETICS_CREATE_NEW_GENOME   = "Genetics_create_new_genome";
const static std::string ATTR_GENETICS_BITS_PER_NUC        = "Genetics_bits_per_nuc";

//const static std::string GENOME_DATASET_NAME      = "Genome";
        

// it is not very nice to derive Genetics from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T, class U>
class Genetics : public Action<T> {
public:

    Genetics(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL);
    Genetics(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed);
    virtual ~Genetics();

    virtual int makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex);

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int dumpAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int restoreAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int dumpStateQDF(hid_t hSpeciesGroup);
    virtual int restoreStateQDF(hid_t hSpeciesGroup);

    void showWELLStates();

    int createInitialGenomes(int iNumGenomes);
    bool isReady() { return m_bBufferAdded;};

    // from Action
    virtual int initialize(float fTime);

    // execute does nothing
    virtual int execute(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);

    int getGenomeSize() { return m_iGenomeSize;};
    LayerArrBuf<ulong> *getGenome() { return &m_aGenome;};
    ulong *getGenome(uint i) { return &m_aGenome[i];};
    int getNumBlocks() { return m_iNumBlocks; };

    bool isEqual(Action<T> *pAction, bool bStrict);


 
protected:
    static int NUM_GENETIC_PARAMS;
protected:
    int init();
    void deleteAllocated();

    LBController *m_pAgentController;
    LBController *m_pWriteCopyController;

    WELL512** m_apWELL;
    int           m_iGenomeSize;
    // the genomes must be controlled by ther agent controller m_AgentController
    LayerArrBuf<ulong> m_aGenome; // 1 per cell; the genomes of the agents follow each other; first allele 0 then allele 1

    // auxiliary layerbuf for writing
    LayerArrBuf<ulong> m_aWriteCopy; 

    BinomialDist **m_pBDist;

    int     m_iNumCrossOvers;
    double  m_dMutationRate;
    ulong **m_pTempGenome1;
    ulong **m_pTempGenome2;
    int     m_iNumBlocks;

    char    m_bCreateNewGenome;
    uint    m_iBitsPerNuc;

    int     m_iNumSetParams;
    int     m_iNumThreads;
    bool    m_bBufferAdded;
    std::vector<int> *m_pvDeadList;

    GenomeCreator<U> *m_pGenomeCreator;
    uint m_iNumParents;
    bool m_bOwnWELL;

    SequenceIOUtils<ulong> *m_pSeqIO;

public:
    static const std::string asNames[];
};


#endif
