#ifndef __SELPAIR_H__
#define __SELPAIR_H__

#include <omp.h>
#include <vector>

#include "Action.h"
#include "ParamProvider2.h"
#include "PolyLine.h"
#include "LayerArrBuf.h"
#include "WELL512.h"

const static std::string ATTR_SELPAIR_NAME        = "SelPair";
const static std::string ATTR_SELPAIR_PROB_NAME   = "SelPairProb";

#define MAX_SPL_NAME  256

class WELL512;
class PolyLine;

template<typename T>
class SelPair : public Action<T> {
    
 public:
    SelPair(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~SelPair();
    int initialize(float fT);
    int finalize(float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  

    void setGenome(LayerArrBuf<ulong> *pGenome, int iNumBlocks) { m_pGenome = pGenome; m_iNumBlocks = iNumBlocks;};

    bool isEqual(Action<T> *pAction, bool bStrict);
    
    //    void showAttributes();
 protected:
    int findMates();
    int findCompatiblePartner(int iCur, std::vector<int> &vAvailable);
    WELL512 **m_apWELL;
    std::vector<int> *m_vLocFemalesID;
    std::vector<int> *m_vLocMalesID;

    PolyLine *m_pPLDistValues;           // how to go from genetic distances to weights
    LayerArrBuf<ulong> *m_pGenome;
    int m_iNumBlocks;

    omp_lock_t* m_aFLocks;
    omp_lock_t* m_aMLocks;

public:
    static const std::string asNames[];
};

#endif
