#include <cstdio>
#include <cmath>
#include <vector>
#include <map>

#include "strutils.h"
#include "stdstrutilsT.h"

#include "geomutils.h"
#include "Sampling.h"
#include "GridRangeSampling.h"


//----------------------------------------------------------------------------
//  constructor
//
GridRangeSampling::GridRangeSampling(int iNumCells, double **apCoords, double dScale, bool bSpher)  :
    CellRangeSampling(iNumCells, apCoords, dScale, bSpher) {
    


}

//----------------------------------------------------------------------------
//  destructor
//
GridRangeSampling::~GridRangeSampling() {};

//----------------------------------------------------------------------------
//  setRangeDescription
//
int GridRangeSampling::setRangeDescription(void *pDescr) {
    int iResult = 0;
    griddef vGridDefs = *((griddef*)(pDescr));
   
    cellrad vCDisks;
   
    for (unsigned int j = 0; j < vGridDefs.size(); ++j) {
        double dXMin = vGridDefs[j].m_dXMin;
        double dXMax = vGridDefs[j].m_dXMax;
        double dStepX = vGridDefs[j].m_dXStep;
        double dYMin = vGridDefs[j].m_dYMin;
        double dYMax = vGridDefs[j].m_dYMax;
        double dStepY = vGridDefs[j].m_dYStep;
        double dRange = vGridDefs[j].m_dRange;

        // calculate coordinates of grid points
        std::vector<std::pair<double,double>> vGridPoints;
        double dY = dYMin;
        while (dY < dYMax) {
            double dX = dXMin;
            while (dX < dXMax) {
                
                vGridPoints.push_back(std::pair<double,double>(dX, dY));
                dX += dStepX;
            }
            dY += dStepY;
        }

        // convert coordinates to cell id

        for (unsigned int k = 0; k < vGridPoints.size(); k++) {
            // for hex
            double dX0 = vGridPoints[k].first+(int(vGridPoints[k].second)%2)*0.5;
            double dY0 = vGridPoints[k].second*sqrt(3)/2;
            // for 4, or ieq
            // double dX0 = vGridPoints[k].first;
            // double dY0 = vGridPoints[k].second;
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
            //printf("Have %f, %f -> %d (%f, %f)\n", dX0, dY0, iRefCell, m_apCoords[0][iRefCell],  m_apCoords[1][iRefCell]);
            vCDisks.push_back(std::pair<int, double>(iRefCell, dRange));
            
        }
    }
    
    CellRangeSampling::setRangeDescription(&vCDisks);
    //makeRefs();

    return iResult;
}
