
#include <vector>
#include <map>
#include "Sampling.h"
#include "CoordRangeSampling.h"

//----------------------------------------------------------------------------
//  constructor
//
CoordRangeSampling::CoordRangeSampling(int iNumCells, double **apCoords, double dScale, bool bSpher)  :
    CellRangeSampling(iNumCells, apCoords, dScale, bSpher) {
    
 
}

//----------------------------------------------------------------------------
//  destructor
//
CoordRangeSampling::~CoordRangeSampling() {};

//----------------------------------------------------------------------------
//  setRangeDescription
//
int CoordRangeSampling::setRangeDescription(void *pDescr) {
    int iResult = 0;
    coordrad vDisks = *((coordrad*)(pDescr));
   
    cellrad vCDisks;
    if (m_bVerbose) xha_printf("Doing %zd discs\n", vDisks.size());

    // here: convert x,y to cell ID and hand over to CellRangeSampling

    for (unsigned int j = 0; j < vDisks.size(); ++j) {
        double dX0 = vDisks[j].m_dX;
        double dY0 = vDisks[j].m_dY;
        double dRange =  vDisks[j].m_dRange;
        
        intvec vCandidates;
        double dMin = 1e12;
        int iRefCell = -1;
        for (int i = 0; i < m_iNumCells; ++i) {
            double dX1 = m_apCoords[0][i];
            double dY1 = m_apCoords[1][i];
            double dDist = (*m_distFunc)(dX0, dY0, dX1, dY1, m_dScale);
            if (dDist < dMin) {
                dMin = dDist;
                iRefCell = i;
            }
        }
        printf("adding disk for %d\n", iRefCell);
        vCDisks.push_back(std::pair<int, double>(iRefCell, dRange));
    }
    CellRangeSampling::setRangeDescription(&vCDisks);

    makeRefs();
    return iResult;
}
