#ifndef __PHENETICS_H__
#define __PHENETICS_H__

#include <hdf5.h>

#include <vector>

#include "Action.h"
#include "ParamProvider2.h"
#include "LayerArrBuf.h"
#include "WELL512.h"
#include "BinomialDist.h"
#include "Permutator.h"
#include "GenomeCreator.h"

class LBController;
class BinomialDist;

typedef float phentype;
const   hid_t hdf_phentype = H5T_NATIVE_FLOAT;
 
const static std::string ATTR_PHENETICS_NAME                 = "Phenetics";
const static std::string ATTR_PHENETICS_PHENOME_SIZE         = "Phenetics_phenome_size";
const static std::string ATTR_PHENETICS_MUTATION_RATE        = "Phenetics_mutation_rate";
const static std::string ATTR_PHENETICS_MUTATION_SIGMA       = "Phenetics_mutation_sigma";
const static std::string ATTR_PHENETICS_CREATE_NEW_PHENOME   = "Phenetics_create_new_phenome";
const static std::string ATTR_PHENETICS_INITIAL_SIGMA        = "Phenetics_initial_sigma";
const static std::string ATTR_PHENETICS_MIX_AVG              = "Phenetics_mix_avg";
const static std::string ATTR_PHENETICS_MUTATION_TYPE        = "Phenetics_mutation_type";

//const static std::string PHENOME_DATASET_NAME      = "Phenome";


#define MUT_TYPE_ALL 0
#define MUT_TYPE_SEL 1
#define MUT_TYPE_POS 2
#define MUT_TYPE_INC 3

// it is not very nice to derive Genetics from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T>
class Phenetics : public Action<T> {
public:

    typedef void (Phenetics::*mixer)(phentype *pMixedPhenome, phentype *pPhenomeM, phentype *pPhenomeP, WELL512 *pWELL);
    typedef void (Phenetics::*mutator)(phentype *pPhenome, double dSigma, BinomialDist *pBDist, WELL512 *pWELL, Permutator *pPerm);

    Phenetics(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL);
    Phenetics(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed);
    virtual ~Phenetics();

    virtual int makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex);

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int dumpAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int restoreAdditionalDataQDF(hid_t hSpeciesGroup);

    virtual int dumpStateQDF(hid_t hSpeciesGroup);
    virtual int restoreStateQDF(hid_t hSpeciesGroup);


    void showWELLStates();

    int createInitialPhenomes(int iNumPhenomes);
    bool isReady() { return m_bBufferAdded;};

    // from Action
    virtual int initialize(float fTime);

    // execute does nothing
    virtual int execute(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    int getPhenomeSize() { return m_iPhenomeSize;};
    LayerArrBuf<phentype> *getPhenome() { return &m_aPhenome;};
    phentype *getPhenome(uint i) { return &m_aPhenome[i];};

    bool isEqual(Action<T> *pAction, bool bStrict);

 

protected:
    static int NUM_PHENETIC_PARAMS;
protected:
    int init();
    void deleteAllocated();

    // "genetic" utilities
    void   mix_avg(phentype *pMixedPhenome, phentype *pPhenomeM, phentype *pPhenomeP, WELL512 *pWELL);
    void   mix_sel(phentype *pMixedPhenome, phentype *pPhenomeM, phentype *pPhenomeP, WELL512 *pWELL);
    
    void   mutateAll(phentype *pPhenome, double dSigma, BinomialDist *pBDist, WELL512 *pWELL, Permutator *pPerm);
    void   mutateSel(phentype *pPhenome, double dSigma, BinomialDist *pBDist, WELL512 *pWELL, Permutator *pPerm);
    void   mutatePos(phentype *pPhenome, double dSigma, BinomialDist *pBDist, WELL512 *pWELL, Permutator *pPerm);
    void   mutateInc(phentype *pPhenome, double dSigma, BinomialDist *pBDist, WELL512 *pWELL, Permutator *pPerm);

    LBController *m_pAgentController;
    LBController *m_pWriteCopyController;

    WELL512** m_apWELL;
    int           m_iPhenomeSize;
    // the genomes must be controlled by ther agent controller m_AgentController
    LayerArrBuf<phentype> m_aPhenome; // 1 per cell; the genomes of the agents follow each other; first allele 0 then allele 1

    // auxiliary layerbuf for writing
    LayerArrBuf<phentype> m_aWriteCopy; 

    BinomialDist  **m_pBDist;
    Permutator    **m_apPerm;

    double  m_dMutationRate;
    double  m_dMutationSigma;
    double  m_dInitialSigma;
    char    m_bCreateNewPhenome;
    char    m_bMixAvg;
    int     m_iMutType;
    int     m_iNumSetParams;

    int     m_iNumThreads;
    bool    m_bBufferAdded;
    std::vector<int> *m_pvDeadList;

    bool m_bOwnWELL;

    mixer  m_fMixing;
    //    void (Phenetics::*m_fMixing)(phentype *pMixedPhenome, phentype *pPhenome, WELL512 *pWELL);
    mutator  m_fMutate;

    float m_fCurTime;
    SequenceIOUtils<float> *m_pSeqIO;

    int    *m_aiLocs;
    double *m_adMuts;

public:
    static const std::string asNames[];
};


#endif
