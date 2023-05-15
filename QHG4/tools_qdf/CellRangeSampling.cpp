#include <cmath>
#include <cstdio>
#include <vector>
#include <map>

#include "strutils.h"
#include "stdstrutilsT.h"

#include "geomutils.h"
#include "Sampling.h"
#include "CellRangeSampling.h"


//----------------------------------------------------------------------------
//  constructor
//
CellRangeSampling::CellRangeSampling(int iNumCells, double **apCoords, double dScale, bool bSpher) :
    m_iNumCells(iNumCells),
    m_apCoords(apCoords),
    m_distFunc(bSpher?(&spherdistDeg):(&cartdist)),
    m_dScale(dScale),
    m_bSpher(bSpher) {

}


//----------------------------------------------------------------------------
//  destructor
//
CellRangeSampling::~CellRangeSampling() {};


//----------------------------------------------------------------------------
//  setRangeDescription
//
int CellRangeSampling::setRangeDescription(void *pDescr) {
    int iResult = 0;
    cellrad vDisks = *((cellrad *)(pDescr));

    for (unsigned int j = 0; j < vDisks.size(); ++j) {
        int iRefCell = vDisks[j].first;
        double dX0 = m_apCoords[0][iRefCell];
        double dY0 = m_apCoords[1][iRefCell];
        double dRange = vDisks[j].second;

        if (dRange == 0) {
            m_mGroups[iRefCell].push_back(iRefCell);
        } else {
            for (int i = 0; i < m_iNumCells; ++i) {
                double dX1 = m_apCoords[0][i];
                double dY1 = m_apCoords[1][i];
                
                if ((*m_distFunc)(dX0, dY0, dX1, dY1, m_dScale) < vDisks[j].second) { 
                    m_mGroups[iRefCell].push_back(i);
                }
            }
        }
        //printf("%zd elements in group for %d\n", m_mGroups[iRefCell].size(), iRefCell);
    }
    if (m_bVerbose) stdprintf("before make refs %zd groups\n", m_mGroups.size());
    makeRefs();
    if (m_bVerbose) stdprintf("after make refs %zd groups\n", m_mGroups.size());
    
    return iResult;
};



