#include <cstdio>
#include <hdf5.h>

#include "qhg_consts.h"
#include "strutils.h"
#include "Geography.h"
#include "SCellGrid.h"
#include "EQsahedron.h"
#include "QDFUtils.h"
#include "GridGroupReader.h"
#include "GeoGroupReader.h"
#include "CellRegions.h"

void printset(intset s, const char *pCaption, int iLim) {
    printf("%s\n", pCaption);
        int i = 0;
        intset::const_iterator it;
        for (it = s.begin(); it != s.end(); ++it) {
            printf("%d ", *it);
            i++;
            if (i>iLim) {
                printf("...");
                break;
            }
        }
        printf("\n");
    
}


//----------------------------------------------------------------------------
// setGeo
//  from QDF file
//
Geography *createGeo(hid_t hFile, SCellGrid *pCG) {
    int iResult = -1;
    Geography *pGeo = NULL;
    GeoGroupReader *pGR = GeoGroupReader::createGeoGroupReader(hFile);
    if (pGR != NULL) {
        GeoAttributes geoatt;
        iResult = pGR->readAttributes(&geoatt);
        if (iResult == 0) {
            if (geoatt.m_iMaxNeighbors == (uint)pCG->m_iConnectivity) {
                if (geoatt.m_iNumCells == (uint)pCG->getNumCells()) {
                    pGeo = new Geography(geoatt.m_iNumCells, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
                    iResult = pGR->readData(pGeo);
                    if (iResult == 0) {
                        printf("[setGeo] GeoReader readData succeeded - Geo: %p, CG: %p!\n", pGeo, pCG);
                    } else {
                        printf("[setGeo] Couldn't read data\n");
                        
                    }
                } else {
                    iResult = -2;
                    printf("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", pCG->getNumCells(), geoatt.m_iNumCells);
                }
            } else {
                iResult = -3;
                printf("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
            }
             
        } else {
            printf("[setGeo] Couldn't read attributes\n");
        }
         
        delete pGR;
    } else {
        printf("[setGeo] Couldn't create GeoGroupReader: did not find group [%s]\n", GEOGROUP_NAME);
    }

    if (iResult < 0) {
        delete pGeo;
        pGeo = NULL;
    }

    return pGeo;
}


//----------------------------------------------------------------------------
// setGrid
//  from QDF file
//
SCellGrid *createGrid(char *pFile) {
    int iResult = -1;
    SCellGrid *pCG = NULL;
    hid_t hFile = qdf_openFile(pFile, false);
    if (hFile != H5P_DEFAULT) {
        GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
        if (pGR != NULL) {
            GridAttributes gridatt;
            char sTime[32];
            // get the timestamp of the initial qdf file (grid)
            iResult = qdf_extractSAttribute(hFile,  ROOT_STEP_NAME, 31, sTime);
            if (iResult != 0) {
                printf("Couldn't read time attribute from grid file\n");
                iResult = 0;
            } else {
                int iStartStep = 0;
                if (strToNum(sTime, &iStartStep)) {
                    iResult = 0;
                    printf("Have timestamp %d\n", iStartStep);
                } else {
                    printf("Timestamp not valid [%s]\n", sTime);
                    iResult = -1;
                }
            }
            iResult = pGR->readAttributes(&gridatt);

            if (iResult == 0) {
                pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
                pCG->m_aCells = new SCell[gridatt.m_iNumCells];
                iResult = pGR->readData(pCG);
                if (iResult == 0) {
                    // ok
                    Geography *pGeo = createGeo(hFile, pCG);
                    if (pGeo != NULL) {
                        pCG->setGeography(pGeo);
                    } else {
                        printf("[setGrid] GridReader couldn't read geo\n");
                    }
 
                } else {
                    printf("[setGrid] GridReader couldn't read data\n");
                 }
            } else {
                printf("[setGrid] GridReader couldn't read attributes\n");
             }
            delete pGR;
        } else {
            printf("[setGrid] Couldn't create GridReader\n");
         }
    } else {
        printf("[setGrid] Couldn't open %s as QDF\n", pFile);
    }
    if (iResult < 0) {
        delete pCG;
        pCG = NULL;
    }
    return pCG;
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    if (iArgC > 3) {
        char *pGridQDF = apArgV[1];
        int    iCellID = atoi(apArgV[2]);
        double dDist   = atof(apArgV[3]);
        /*
        double dLon    = atof(apArgV[2]);
        double dLat    = atof(apArgV[3]);
        double dDist   = atof(apArgV[4]);
        */

        SCellGrid *pCG = createGrid(pGridQDF);
        if (pCG != NULL) {
            double dLon = pCG->getGeography()->getLongitude()[iCellID];
            double dLat = pCG->getGeography()->getLatitude()[iCellID];

            CellRegions *pCR = CellRegions::createInstance(pCG, RADIUS_EARTH_KM);
            if (pCR != NULL) {
                intset sCells;
                
                pCR->findInside(dLon, dLat, dDist, sCells);
                char sCaption[256];
                sprintf(sCaption, "found %zd cells\n", sCells.size());
                printset(sCells, sCaption, 99);
                delete pCR;
            }
            
            delete pCG;
        }
    } else {
        printf("Usage:\n");
        //        printf("  %s <qdf-grid> <lon> <lat> <dist>\n", apArgV[0]);
        printf("  %s <qdf-grid> <cell-id> <dist>\n", apArgV[0]);
    }
    return iResult;
}
