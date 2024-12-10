#include <cstring>
#include <hdf5.h>
#include <omp.h>


#include "types.h"
#include "strutils.h"
#include "geomutils.h"
#include "QDFUtils.h"
#include "WELL512.h"

#include "SCell.h"
#include "SCellGrid.h"
#include "Permutator.h"
#include "EQsahedron.h"
#include "GridGroupReader.h"
#include "GeoGroupReader.h"
#include "GridWriter.h"
#include "GeoWriter.h"
#include "LinkTools.h"



Quat *calcIcoInv() {
    Quat *q1 = Quat::createRotation(Q_PI/5, 0, 0, 1);
    Quat *q2 = Quat::createRotation(Q_PI,   1, 0, 0);
    
    q2->mult(q1);
    return q2;
}


//----------------------------------------------------------------------------
// createInstance
//

LinkTools *LinkTools::createInstance(const char *pQDFGeo) {
    LinkTools *pLS = new LinkTools();
    int iResult = pLS->init(pQDFGeo);
    if (iResult != 0) {
        delete pLS;
        pLS = NULL;
    }
    return pLS;
}


//----------------------------------------------------------------------------
// constructor
//
LinkTools::LinkTools() 
    : m_pCG(NULL),
      m_pAntipodes(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
LinkTools::~LinkTools() {
    if (m_pCG != NULL) {
        delete m_pCG;
    }


    if (m_pAntipodes != NULL) {
        delete[] m_pAntipodes;
    }

    if (m_pEQ != NULL) {
        delete m_pEQ;
    }
}



//----------------------------------------------------------------------------
// init
//
int LinkTools::init(const char *pQDFGeo) {
    int iResult = -1;
    
    m_hFile = qdf_openFile(pQDFGeo, true);
    m_pCG = readGrid();
    if (m_pCG != NULL) {
        m_iNumCells = m_pCG->m_iNumCells;
        stringmap &smSurf = m_pCG->m_smSurfaceData;
        const char *pSubDivs = smSurf[SURF_IEQ_SUBDIVS].c_str();
        if (strToNum(pSubDivs, &m_iSubDivs)) {
            printf("Have subdivs %d\n", m_iSubDivs);
            m_pEQ = EQsahedron::createInstance(m_iSubDivs, true);
            if (m_pEQ != NULL) {
                iResult = createAntipodes();
                if (iResult == 0) {
                    printf("Antipode array created\n");
                } else {
                    printf("Couldn't create Antipodes\n");
                }

            } else {
                printf("Couldn't create EQsahedron\n");
            }
        } else {
            printf("Couldn't read subdivs\n");
        }
    } 
    

    return iResult;
}




//----------------------------------------------------------------------------
// readGeo
//
int LinkTools::readGeo() {
    int iResult = -1;
    
     GeoGroupReader *pGR = GeoGroupReader::createGeoGroupReader(m_hFile);
     if (pGR != NULL) {
         GeoAttributes geoatt;
         iResult = pGR->readAttributes(&geoatt);
         if (iResult == 0) {
             if (geoatt.m_iMaxNeighbors == (uint)m_pCG->m_iConnectivity) {
                 if (geoatt.m_iNumCells == (uint)m_pCG->m_iNumCells) {
                     Geography *pGeo = new Geography(m_pCG, geoatt.m_iNumCells, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
                     iResult = pGR->readData(pGeo);
                     if (iResult == 0) {
                         m_pCG->setGeography(pGeo);
                         printf("[setGeo] GeoReader readData succeeded - Geo: %p, CG: %p!\n", pGeo, m_pCG);
                     } else {
                         printf("[setGeo] Couldn't read data\n");
                     }
                 } else {
                     iResult = -2;
                     printf("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, geoatt.m_iNumCells);
                 }
             } else {
                 iResult = -3;
                 printf("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
             }
             
         } else {
             printf("[setGeo] Couldn't read attributes\n");
         }
         
         delete pGR;
     } else {
         printf("[setGeo] Couldn't create GeoGroupReader: did not find group [%s]\n", GEOGROUP_NAME.c_str());
     }

    return iResult;
}


//----------------------------------------------------------------------------
// readGrid
//  from QDF file
//
SCellGrid *LinkTools::readGrid() {
    int iResult = -1; 
    
    GridGroupReader *pGR = GridGroupReader::createGridGroupReader(m_hFile);
    if (pGR != NULL) {
        GridAttributes gridatt;
        // get the timestamp of the initial qdf file (grid)
        iResult = pGR->readAttributes(&gridatt);
        if (iResult == 0) {
            //int iNumCells = gridatt.m_iNumCells;
            m_pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
            m_pCG->m_aCells = new SCell[gridatt.m_iNumCells];
            iResult = pGR->readData(m_pCG);
            if (iResult == 0) {
                printf("[setGrid] Grid read successfully: %p\n", m_pCG);
                iResult = readGeo();
                if (iResult == 0) {
                    printf("[setGrid] GridReader readData succeeded - Geo: %p, CG: %p!\n", m_pCG->m_pGeography, m_pCG);
                } else {
                    printf("[setGrid] No Geography found in QDF\n");
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
    if (iResult != 0) {
        delete m_pCG;
        m_pCG = NULL;
    }
    return m_pCG;
}


//----------------------------------------------------------------------------
// createAntipodes
//
int LinkTools::createAntipodes() {
    int iResult = 0;
    
    m_pAntipodes = new int[m_iNumCells];
    for (int iCell = 0; iCell < m_iNumCells; iCell++) {
        double dLon = m_pCG->m_pGeography->m_adLongitude[iCell];
        double dLat = m_pCG->m_pGeography->m_adLatitude[iCell];
        
        double dLonA = (dLon<0)?dLon +180:dLon-180;
        double dLatA = -dLat;

        int iAntiCell = m_pEQ->findNode(dLonA, dLatA);
        m_pAntipodes[iCell] = iAntiCell;

    }
    return iResult;
}


//----------------------------------------------------------------------------
// symmetrize
//
int LinkTools::symmetrize() {
    int iResult = 0;
    
    bool *abProcessed = new bool[m_iNumCells];
    memset(abProcessed,0, m_iNumCells*sizeof(bool));
    
    for (int iCell = 0; iCell < m_iNumCells; iCell++) {
        double dLat = m_pCG->m_pGeography->m_adLatitude[iCell];
        if (dLat >= -1e-10) {
            if (!abProcessed[iCell]) {
                int iACell = m_pAntipodes[iCell];
                SCell &sc = m_pCG->m_aCells[iCell];
                SCell &asc = m_pCG->m_aCells[iACell];
                for (int j = 0; j < sc.m_iNumNeighbors; j++) {
                    asc.m_aNeighbors[j] = m_pAntipodes[sc.m_aNeighbors[j]];
                }
                abProcessed[iCell] = true;
                abProcessed[iACell] = true;
            }
        }
    }

    for (int iCell = 0; iCell < m_iNumCells; iCell++) {
        if (!abProcessed[iCell]) {
            printf("Unprocessed: cell %d (%e,%e), antipode %d\n", iCell,  m_pCG->m_pGeography->m_adLongitude[iCell], m_pCG->m_pGeography->m_adLatitude[iCell], m_pAntipodes[iCell]);
        }
    }

    delete[] abProcessed;

    return iResult;
}

//----------------------------------------------------------------------------
// checkAntipodeSymmetry
//
int LinkTools::checkAntipodeSymmetry() {
    int iResult = 0;
    int iNum = 0;
    for (int iCell = 0; (iCell < m_iNumCells); iCell++) {
        SCell &scA0 = m_pCG->m_aCells[iCell];
        SCell &scA1 = m_pCG->m_aCells[m_pAntipodes[iCell]];

        for (int j = 0; (j < scA0.m_iNumNeighbors); j++) {

            int iAA0 = m_pAntipodes[scA0.m_aNeighbors[j]];

            int iAA1 = scA1.m_aNeighbors[j];
            
            if (iAA0 != iAA1) {
                printf("mismatch for cell A %d: p(A) %d: link_%d(A) %d, p(link_%d(A)) %d,  link_%d(p(A)) %d\n", 
                       iCell, m_pAntipodes[iCell], j, scA0.m_aNeighbors[j], j, iAA0, j, iAA1);
                iResult = -1;
                iNum++;
            }
        }
    }
    printf("Found %d mismatch%s\n", iNum, (iNum==1)?"":"es");
    return iResult;
}

//----------------------------------------------------------------------------
// scramble
//
int LinkTools::scramble(WELL512 **pWELL) {
    int iResult = 0;
    
    Permutator *pP = Permutator::createInstance(6);
    int temp[6];

#pragma omp parallel for
    for (int iCell = 0; iCell < m_iNumCells; iCell++) {
        SCell &sc = m_pCG->m_aCells[iCell];
        const uint *a = pP->permute(sc.m_iNumNeighbors,  sc.m_iNumNeighbors, pWELL[omp_get_thread_num()]);
        /*
        for (int j = 0; j < sc.m_iNumNeighbors; j++) {
            printf("a[%d]: %d ", j, a[j]);
        }
        printf("\n");

        for (int j = 0; j < sc.m_iNumNeighbors; j++) {
            printf("ne[%d]: %d ", j, sc.m_aNeighbors[j]);
        }
        printf("\n");

        for (int j = 0; j < sc.m_iNumNeighbors; j++) {
            printf("sne[%d]: %d ", j, sc.m_aNeighbors[a[j]]);
        }
        printf("\n");
        */
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
// findNeighborCoords
//
int LinkTools::findNeighborCoords(int iCell) {
    int iResult = 0;

    SCell &sc = m_pCG->m_aCells[iCell];
    printf("Cell %d %d neighs\n", iCell,  sc.m_iNumNeighbors);
    printf("Cell NeighborIndex NeighborID lon lat\n");
    for (int j = 0; j < sc.m_iNumNeighbors; j++) {
        int iCell2 = sc.m_aNeighbors[j];
        double dLon = m_pCG->m_pGeography->m_adLongitude[iCell2];
        double dLat = m_pCG->m_pGeography->m_adLatitude[iCell2];
        printf("%d %d %d %lf %lf\n", iCell, j, iCell2, dLon, dLat);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// save
//
int LinkTools::save() {
    int iResult = 0;
    
    H5Gunlink(m_hFile, GRIDGROUP_NAME.c_str());
    H5Gunlink(m_hFile, GEOGROUP_NAME.c_str());
    GridWriter *pGridW  = new GridWriter(m_pCG, &m_pCG->m_smSurfaceData);
    GeoWriter  *pGeoW   = new GeoWriter(m_pCG->m_pGeography);
    iResult = pGridW->write(m_hFile);
    if (iResult == 0) {
        iResult = pGeoW->write(m_hFile);
        if (iResult == 0) {
            printf("OK\n");
        } else {
            printf("COuldn't write geo\n");
        }
        
    } else {
        printf("COuldn't write grid\n");
    }
    qdf_closeFile(m_hFile);
    return iResult;
}
/*
 herr_t H5Adelete (hid_t loc_id, const char *name)
A Dataset can be added to a Group with the H5Glink call, and deleted from a group with H5Gunlink.
*/
