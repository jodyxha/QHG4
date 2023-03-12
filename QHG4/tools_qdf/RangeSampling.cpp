#include <cmath>
#include <cstdio>
#include <vector>
#include <map>

#include "strutils.h"
#include "stdstrutilsT.h"

#include "Sampling.h"
#include "RangeSampling.h"

//----------------------------------------------------------------------------
//  spherdist
//
double spherdist(double dLon1, double  dLat1, double dLon2, double dLat2, double dR) {
    double dX1 = cos(dLon1)*cos(dLat1);    
    double dY1 = sin(dLon1)*cos(dLat1);    
    double dZ1 = sin(dLat1);    
    double dX2 = cos(dLon2)*cos(dLat2);    
    double dY2 = sin(dLon2)*cos(dLat2);    
    double dZ2 = sin(dLat2);    
    double dProd = dX1*dX2+dY1*dY2+dZ1*dZ2;
    if (dProd > 1) {
        dProd = 1;
    } else if (dProd < -1) {
        dProd = -1;
    }
    return dR * acos(dProd);
}


//----------------------------------------------------------------------------
//  spherdistDeg
//    if we have an IEQ (or ICO), longitude and latitude are given in degrees
//
double spherdistDeg(double dLon1, double  dLat1, double dLon2, double dLat2, double dR) {
    return spherdist(dLon1*M_PI/180, dLat1*M_PI/180, dLon2*M_PI/180, dLat2*M_PI/180, dR);
}


//----------------------------------------------------------------------------
//  cartdist
//
double cartdist(double dX1, double  dY1, double dX2, double dY2, double dScale) {
    double d = (dX2-dX1)*(dX2-dX1)+(dY2-dY1)*(dY2-dY1);
    return dScale*sqrt(d);
}


//----------------------------------------------------------------------------
//  constructor
//
RangeSampling::RangeSampling(int iNumCells, pointrad &vDisks, double *dLon, double *dLat, double dScale, bool bSpher) :
    m_iNumCells(iNumCells),
    m_dLon(dLon),
    m_dLat(dLat),
    m_distFunc(bSpher?(&spherdistDeg):(&cartdist)) {
    if (m_bVerbose) stdprintf("Doing %zd discs\n", vDisks.size());
    for (unsigned int j = 0; j < vDisks.size(); ++j) {
        int iRefCell = vDisks[j].first;
        double dX2 = dLon[iRefCell];
        double dY2 = dLat[iRefCell];

        for (int i = 0; i < m_iNumCells; ++i) {
            double dX1 = dLon[i];
            double dY1 = dLat[i];

	    if ((*m_distFunc)(dX1, dY1, dX2, dY2, dScale) < vDisks[j].second) { 
                 m_mGroups[iRefCell].push_back(i);
            }
        }
        //printf("%zd elements in group for %d\n", m_mGroups[iRefCell].size(), iRefCell);
    }
    if (m_bVerbose) stdprintf("before make refs %zd groups\n", m_mGroups.size());
    makeRefs();
    if (m_bVerbose) stdprintf("after make refs %zd groups\n", m_mGroups.size());
}


//----------------------------------------------------------------------------
//  destructor
//
RangeSampling::~RangeSampling() {};




