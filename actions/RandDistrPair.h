#ifndef __RANDDISTRPAIR_H__
#define __RANDDISTRPAIR_H__

#include <omp.h>
#include <vector>
#include "Permutator.h"
#include "Genetics.h"
#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_RANDDISTRPAIR_NAME  "RandDistrPair"
#define ATTR_RANDDISTRPAIR_DCRIT "RandDistrPair_dcrit"
#define ATTR_RANDDISTRPAIR_VCRIT "RandDistrPair_vcrit"


#define INIT_PERM_SIZE 100

class WELL512;

template<typename T, class U>
class RandDistrPair : public Action<T> {
    
 public:
    RandDistrPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, Genetics<T, U> *pGenetics, WELL512 **apWELL);
    virtual ~RandDistrPair();

    int preLoop();
    int initialize(float fT);
    int finalize(float fT);

    virtual int extractParamsQDF(hid_t hSpeciesGroup);
    virtual int writeParamsQDF(hid_t hSpeciesGroup);

    virtual int tryGetParams(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);
    
 
 protected:
    int findMates();
    WELL512 **m_apWELL;
    Genetics<T, U> *m_pGenetics;

    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;
    Permutator      **m_ppPermutators;

    int m_iNumCells;
    int m_iGenomeSize;
    int m_iNumBlocks;

    float m_fDCrit;    
    float m_fVCrit;
    float m_fA;

    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;

    static const char *asNames[];
};

#endif
