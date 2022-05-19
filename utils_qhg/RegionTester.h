#ifndef __REGIONTESTER_H__
#define __REGIONTESTER_H__

class RegionTester {
public:
    RegionTester(int iNumVerts, double **apCoordinates);

    bool pointInPoly(double fX, double fY);
private:
    int m_iNumVerts;
    double **m_apCoords;
};



#endif
