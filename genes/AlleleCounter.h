#ifndef __ALLELECOUNTER_h__
#define __ALLELECOUNTER_h__

#include <string>
#include <vector>
#include <algorithm>

#include "types.h"
#include "QDFUtils.h"

const int DEF_BITS_PER_NUC = 2;

const std::string POP_DS_GENOME          = "Genome";
const std::string POP_DS_AGENTS          = "AgentDataSet";
const std::string POP_ATTR_GENOME_SIZE   = "Genetics_genome_size";
const std::string POP_ATTR_BITS_PER_NUC  = "Genetics_bits_per_nuc";
const std::string POP_ATTR_MUT_RATE      = "Genetics_mutation_rate";

const int SEL_ALL      = 0;
const int SEL_GENDER_F = 1;
const int SEL_GENDER_M = 2;


class AlleleCounter {
    
public:
    static AlleleCounter *createInstance(const std::string sQDFPopFile, const std::string sSpeciesName);

    int countAlleles(int iSelectionType);

    uint **getCounts()   { return m_ppCounts;};
    int getGenomeSize()  { return m_iGenomeSize;};
    int getNumNucs()     { return m_iNumNucs;};
    int getNumGenomes()  { return m_iNumSelected;};

    virtual ~AlleleCounter();

protected:
    
    AlleleCounter();
    int init(const std::string sQDFFile, const std::string sSpeciesName);

    int init_qdf(const std::string sQDFPopFile, 
                 const std::string sSpeciesName);

    
    int prepareArrays();

    
    int selectIndexes(int iSelectionType);
    int loadGenomes(int iNumPerBuf);
    
    int m_iGenomeSize;
    int m_iNumBlocks;
    uint m_iBitsPerNuc;
    int m_iNumNucs;
    int m_iNucsInBlock;
    int m_iNumGenomes;
    int m_iNumSelected;
    
    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    ulong **m_ppGenomes;
    uint  **m_ppCounts;
    std::vector<int> m_vSelectedIndexes;
    bool m_bVerbose;
    std::string m_sPopName;
   
};

#endif
