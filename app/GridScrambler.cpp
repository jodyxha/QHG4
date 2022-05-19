#include <cstdio>
#include <omp.h>


#include "WELL512.h"
#include "WELLUtils.h"
#include "Permutator.h"
#include "SCellGrid.h"
#include "GridScrambler.h"


//----------------------------------------------------------------------------
// createInstance
//
GridScrambler *GridScrambler::createInstance(SCellGrid *pCG, int iSeed) {
    GridScrambler *pGS = new GridScrambler(pCG);
    int iResult = pGS->init(iSeed);
    if (iResult != 0) {
        delete pGS;
        pGS = NULL;
    }
    return pGS;
}


//----------------------------------------------------------------------------
// destructor
//
GridScrambler::~GridScrambler() {
    if (m_pPerm != NULL) {
        delete m_pPerm;
    }

    if (m_apWELLs != NULL) {
        WELLUtils::destroyWELLs(m_apWELLs, omp_get_max_threads());
    }
}


//----------------------------------------------------------------------------
// scrambleConnections
//
int GridScrambler::scrambleConnections() {
    int iResult = 0;
    printf("scrambling connections\n");

    //#pragma omp parallel for
    for (uint iCell = 0; iCell < m_pCG->m_iNumCells; iCell++) {
        int temp[6];
        SCell &sc = m_pCG->m_aCells[iCell];
        const uint *a = m_pPerm->permute(sc.m_iNumNeighbors,  sc.m_iNumNeighbors, m_apWELLs[omp_get_thread_num()]);
    

        for (int j = 0; j < sc.m_iNumNeighbors; j++) {
            temp[j] = sc.m_aNeighbors[a[j]];
        }
        for (int j = 0; j < sc.m_iNumNeighbors; j++) {
            sc.m_aNeighbors[j] = temp[j];
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// constructor
//
GridScrambler::GridScrambler(SCellGrid *pCG) 
    : m_pCG(pCG),
      m_apWELLs(NULL),
      m_pPerm(NULL) {
    
}


//----------------------------------------------------------------------------
// init
//
int GridScrambler::init(int iSeed) {
     int iResult = 0;

     m_apWELLs = WELLUtils::buildWELLs(omp_get_max_threads(), iSeed);
     m_pPerm = Permutator::createInstance(m_pCG->m_iConnectivity);

     return iResult = 0;

 }

