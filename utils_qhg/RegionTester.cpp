#include <cstdio>

#include "RegionTester.h"

RegionTester::RegionTester(int iNumVerts, double **apCoordinates)
    : m_iNumVerts(iNumVerts),
      m_apCoords(apCoordinates) {

}

bool RegionTester::pointInPoly(double fX, double fY) {
    bool bInside = false;

    for (int i = 0; i < m_iNumVerts; i++) {
        int j = (i > 0)? i-1 : m_iNumVerts-1;
    
        if (((m_apCoords[i][1] > fY) != (m_apCoords[j][1] > fY)) &&
            (fX > (m_apCoords[j][0] - m_apCoords[i][0])*(fY - m_apCoords[i][1])/(m_apCoords[j][1]-m_apCoords[i][1]) + m_apCoords[i][0])) {
            bInside = !bInside;
        }
    }
    
    return bInside;
}
