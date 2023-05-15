#ifndef __HYBCHECKER_H__
#define __HYBCHECKER_H__

#include <string>

#include "hdf5.h"

#include "types.h"
#include "BinFunc.h"
#include "HybAgentLoader.h"

class HybChecker {
public:
    const static int MODE_NONE = 0;
    const static int MODE_TXT  = 1;
    const static int MODE_CSV  = 2;
    const static int MODE_HDR  = 3;

    static HybChecker *createInstance(const std::string sPop, BinFunc *pBF);

    ~HybChecker();

    int analyze();

    void showSimple();

    void showCSVHeader();
    void showCSVLine();

protected:
    HybChecker();
    int init(const std::string sPop, BinFunc *pBF);

    HybAgentLoader *m_pHAL;     
    BinFunc        *m_pBF;     

    int   m_iNumAgents;
    int   m_iZeros;
    int   m_iOnes;
    float m_fMinNonZero;
    float m_fMaxNonOne;

    uint     *m_piBins;
    uint      m_iNumBins;

};
#endif
