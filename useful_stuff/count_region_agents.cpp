#include <cstdio>
#include <cstring>


#include "strutils.h"
#include "LineReader.h"
#include "RegionTester.h"
#include "AgentCounter.h"

typedef std::map<std::string, std::pair<int, double **>> vertlist;

double aaAmericas[][2] = {     
    {-168.4, 87.877},
    {-268.4,-60.923},
    {-29.2, -60.923},
    {-29.2,  48.677},
    {-90.8,  86.01},
};

double aaAfrica[][2] = {
    {-20.8,35.477},
    {-20.8,2.677},
    {20.667,-39.99},
    {60,-38.39},
    {56.533,14.677},
    {44,11.877},
    {32.533, 37.743},
    {12.667,34.810},
    {11.2, 38.943},
    {9.733,37.743},
    {-5.733,35.743},
};


double aaAustralia[][2] = {
    {156.667,-14.523},
    {133.467,-8.390},
    {109.733,-20.79},
    {114.4,-36.123},
    {148.0,-45.857},
    {157.733,-23.323},
};


double aaAustralia2[][2] = {
    {131.733,8.319},
    {108.167,-16.832},
    {111.433,-48.103},
    {156.700,-48.803},
    {156.233,12.097},
};


double aaEurasia[][2] = { 
    {-21.0,  34.143},
    {6.267,  38.143},
    {15.60,  36.010},
    {32.80,  32.81},
    {32.67,  29.077},
    {43.733, 12.543},
    {57.867, 14.277},
    {121.467, -13.457},
    {136.933, 30.277},
    {191.067, 65.477},
    {180.533, 81.477},
    {48.0, 89.610},
    {0.0,81.543},
    {-21.33, 53.343},
};


//----------------------------------------------------------------------------
// copyArr
//
double **copyArr(double aaData[][2], uint iSize) {
    double **ppData = new double *[iSize];
    for (uint i = 0; i < iSize; i++) {
        ppData[i] = new double[2];
        ppData[i][0] = aaData[i][0];
        ppData[i][1] = aaData[i][1];
    }
    return ppData;
}


//----------------------------------------------------------------------------
// init_predefs
//
void init_predefs(vertlist &mVerts) {
    uint iSize =  sizeof(aaAmericas)/sizeof(double *);
    mVerts["americas"]    = std::pair<int, double **>(iSize,   copyArr(aaAmericas, iSize));
    iSize =  sizeof(aaAfrica)/sizeof(double *);
    mVerts["africa"]      = std::pair<int, double **>(iSize,  copyArr(aaAfrica, iSize));
    iSize =  sizeof(aaAustralia)/sizeof(double *);
    mVerts["australia"]   = std::pair<int, double **>(iSize,  copyArr(aaAustralia, iSize));
    iSize =  sizeof(aaAustralia2)/sizeof(double *);
    mVerts["australia2"]   = std::pair<int, double **>(iSize,  copyArr(aaAustralia2, iSize));
    iSize =  sizeof(aaEurasia)/sizeof(double *);
    mVerts["eurasia"]   = std::pair<int, double **>(iSize,  copyArr(aaEurasia, iSize));

}


//----------------------------------------------------------------------------
// createRegionTester
//
RegionTester *createRegionTester(const char *pVerts, vertlist &mVerts) {
    RegionTester *pRT = NULL;
    vertlist::const_iterator it = mVerts.find(pVerts);
    if (it != mVerts.end()) {
        pRT = new RegionTester(it->second.first, it->second.second);
    } else {
        int iResult = 0;
        std::vector<std::pair<double, double>> vVerts;
        LineReader *pLR = LineReader_std::createInstance(pVerts, "rt");
        if (pLR != NULL) {
            while ((iResult == 0) && (!pLR->isEoF())) {
                char *p = pLR->getNextLine();
                if (p != NULL) {
                    char *pX = strtok(p, ",; \t\n");
                    if (pX != NULL) {
                        char *pY = strtok(NULL, ",; \t\n");
                        if (pY != NULL) {
                            double dX;
                            double dY;
                            if (strToNum(pX, &dX) && strToNum(pY, &dY)) {
                                vVerts.push_back(std::pair<double, double>(dX, dY));
                            } else {
                                printf("expected number but got [%s] [%s]\n", pX, pY);
                                iResult = -1;
                            }
                        } else {
                            printf("expected more than one item\n");
                            iResult = -1;
                        }
                    } else {
                        printf("expected more than one item\n");
                        iResult = -1;
                    }
                }
            }    
            delete pLR;
            if ((iResult == 0) && (vVerts.size() > 0)) {
                double **apCoords = new double*[vVerts.size()];
                for (uint i = 0; i < vVerts.size(); i++) {
                    apCoords[i] = new double[2];
                    apCoords[i][0] = vVerts[i].first;
                    apCoords[i][1] = vVerts[i].second;
                }

                pRT = new RegionTester(vVerts.size(), apCoords);
            }
        } else {
            printf("No predefined region or region file with name [%s]\n", pVerts);
        }
    }
    return pRT;
}


//----------------------------------------------------------------------------
// countAgentsInRegion
//
int countAgentsInRegion(AgentCounter *pAC,  RegionTester *pRT) {
    int iTotal = 0;

    int iNumCells = pAC->getNumCells();
    const double *pdLon = pAC->getLongitude();
    const double *pdLat = pAC->getLatitude();
    int          *piCnt = pAC->getPopCounts(NULL);

#pragma omp parallel for
    for (int iCell = 0; iCell < iNumCells; iCell++) {
        double dLon = pdLon[iCell];
        double dLat = pdLat[iCell];

        if (!pRT->pointInPoly(dLon, dLat)) {
            piCnt[iCell] = 0;
        }
    }

#pragma omp parallel for reduction(+:iTotal)
    for (int j = 0; j < iNumCells; j++) {
        iTotal += piCnt[j];
    }
    return iTotal;
}


//----------------------------------------------------------------------------
// writeCounts
//
int writeCounts(AgentCounter *pAC, const char *pOut) {
    int iResult = -1;

    int iNumCells = pAC->getNumCells();
    const double *pdLon = pAC->getLongitude();
    const double *pdLat = pAC->getLatitude();
    const double *pdAlt = pAC->getAltitude();
    const double *pdIce = pAC->getIceCover();
    int          *piCnt = pAC->getPopCounts(NULL);
    double       *pCC   = pAC->getCC(NULL);

    FILE *fOut = fopen(pOut, "wt");
    if (fOut != NULL) {
        iResult = 0;
        for (int i = 0; i < iNumCells; i++) {
            if ((pdAlt[i] > 0) && (pdIce[i] == 0)) {
                fprintf(fOut, "%d  %f %f %d  %f\n", i, pdLon[i], pdLat[i], piCnt[i], pCC[i]);
            }
        }
        fclose(fOut);
    } else {
        printf("Failed to open [%s]\n", pOut);
        iResult = -1;
    }


    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    RegionTester *pRT = NULL;
    vertlist m_mVerts;

    if (iArgC > 3) {

        const char *pPop = apArgV[1];
        const char *pGeo = apArgV[2];
        const char *pVeg = pGeo;
        const char *pVerts = NULL;
        const char *pOut   = NULL;
        if (iArgC > 4) {
            pVerts = apArgV[3];
            pOut   = apArgV[4];
        } else {
            pOut   = apArgV[3];
        }
        
        if (pVerts != NULL) {
            init_predefs(m_mVerts);
            pRT = createRegionTester(pVerts, m_mVerts);
        }

        AgentCounter *pAC = AgentCounter::createInstance(pPop, pGeo, pVeg);
        if (pAC != NULL) {
            int iRegionCount = 0;
            int iTotalCount  = pAC->countAgentsInCells();
            printf("TotalCount: %d\n", iTotalCount);
            if (pRT != NULL) {
                iRegionCount = countAgentsInRegion(pAC, pRT);
                printf("RegionCount: %d\n", iRegionCount);
                iResult = writeCounts(pAC, pOut);
            }                
                
            delete pAC;
        }
            
    } else  {
        printf("Usage:\n");
        printf("  %s <pop-qdf> <geo-qdf> [<verts>] <output>\n", apArgV[0]);
    }
    return iResult;
}
