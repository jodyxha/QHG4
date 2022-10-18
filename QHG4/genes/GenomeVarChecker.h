#ifndef __GENOMEVARCHECKER_H__
#define __GENOMEVARCHECKER_H__

#include <string>
#include <hdf5.h>

#include "types.h"

class GenomeVarChecker {
public:
    static GenomeVarChecker *createInstance(const std::string sQDFPopFile, 
                                            const std::string sSpeciesName, 
                                            const std::string sAttrGenomeSize,
                                            const std::string sAttrBitsPerNuc,
                                            const std::string sDataSetGenome,
                                            int iBufSizeGenomes=-1);

    static GenomeVarChecker *createInstance(const std::string sBinGeneFile,
                                            int iBufSizeGenomes=-1);
    
    virtual ~GenomeVarChecker();
    int init_qdf(const std::string sQDFPopFile, 
                 const std::string sSpeciesName, 
                 const std::string sAttrGenomeSize,
                 const std::string sAttrBitsPerNuc,
                 const std::string sDataSetGenome);

    int init_bin(const std::string sBinGeneFile);
    
    //    ulong *getHits() { return m_ulHits;};
    //    uchar *getCounts() { return m_acCounts;};
    uchar *getCounts2(){ return m_acCounts2;};
    int    getGenomeSize() { return m_iGenomeSize;};
    int    getNumBlocks() { return m_iNumBlocks;};
    int    getNumGenomes() { return m_iNumGenomes;};
    uint   getBitsPerNuc() { return m_iBitsPerNuc;};
    float *getOrderedFreqs() { return m_afOrderedFreqs;};
    int   *getNonZeroCount() { return m_aNonZeroCount;};
protected:
    GenomeVarChecker(int iBufSizeGenomes);
    
    int processGenomesPar(const std::string sDataSetGenome);
    int processGenome(ulong *pGenome);
    int processGenomePar(ulong *pGenome);
    void prepareArrays();
    int explicitCounts(ulong *pGenome);
    int makeCounts();
    int accumulateThreadData();
    int calcOderedFreqs();

    int   ***m_aNucCountTemp; //explicit nucleotide counts per site and thread
    int   **m_aNucCount; //explicit nucleotide counts per site (thread-accumulated)
    uchar  *m_acCounts2; // number of variants for each site

    int **m_aNonZeroCountTemp; // explicit count of non-zero nucleotides per site and thread
    int *m_aNonZeroCount;      // explicit count of non-zero nucleotides per site (thread-accumulated)

    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    int m_iGenomeSize;
    int m_iNumBlocks;
    int m_iNumGenomes;
    uint m_iBitsPerNuc;
    uint m_iNumNucs;

    int m_iBufSizeGenomes;

    std::string m_sDataSetGenome;
    float m_afOrderedFreqs[4];
    
};

#endif
