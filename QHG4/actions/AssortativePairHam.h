#ifndef __ASSORTATIVEPAIRHAM_H__
#define __ASSORTATIVEPAIRHAM_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "ParamProvider2.h"
#include "Genetics.h"

const static std::string ATTR_ASSPAIRHAM_NAME       = "AssortativePairHamming";
const static std::string ATTR_ASSPAIRHAM_CUTOFF     = "AssortativePairHamming_cutoff";
const static std::string ATTR_ASSPAIRHAM_PERMUTE    = "AssortativePairHamming_permute";

class WELL512;

template<typename T, class U>
class AssortativePairHam : public Action<T> {
    
 public:
    AssortativePairHam(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, Genetics<T, U> *pGenetics,  WELL512 **apWELL);
    virtual ~AssortativePairHam();
    virtual int initialize(float fT);
    virtual int finalize(float fT);
    virtual int preLoop(); 

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    bool isEqual(Action<T> *pAction, bool bStrict);
    
 
 protected:
    int findMates();
    Genetics<T, U> *m_pGenetics;
    WELL512 **m_apWELL;
    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;

    float m_fCutOff;
    bool  m_bPermute;
    int m_iNumCells;
    int m_iGenomeSize;
    int m_iNumBlocks;

    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;

    static const char *asNames[];
};


#endif
