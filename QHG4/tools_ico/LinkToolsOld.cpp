
#include <cstring>
#include <hdf5.h>
#include <omp.h>


#include "Vec3D.h"
#include "Quat.h"
#include "types.h"
#include "strutils.h"
#include "geomutils.h"
#include "QDFUtils.h"
#include "WELL512.h"

#include "SCell.h"
#include "SCellGrid.h"
#include "Permutator.h"
#include "GridGroupReader.h"
#include "GeoGroupReader.h"
#include "GridWriter.h"
#include "GeoGroupReader.h"
#include "LinkTools.h"



//----------------------------------------------------------------------------
// createInstance
//
LinkTools *LinkTools::createInstance(const char *pQDFGrid) {
    LinkTools *pLS = new LinkTools();
    int iResult = pLS->init(pQDFGrid);
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
    : m_pCG(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
LinkTools::~LinkTools() {
    if (m_pCG != NULL) {
        delete m_pCG;
    }
}



//----------------------------------------------------------------------------
// init
//
int LinkTools::init(const char *pQDFGrid) {
    int iResult = -1; 
    
    m_hFile = qdf_openFile(pQDFGrid, true);

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
                m_iNumCells = m_pCG->m_iNumCells;
                iResult = readGeo();
                if (iResult == 0) {
                    printf("[setGrid] Grid read %d nodes successfully: %p\n", m_iNumCells, m_pCG);
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
                     Geography *pGeo = new Geography(geoatt.m_iNumCells, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
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
         printf("[setGeo] Couldn't create GeoGroupReader: did not find group [%s]\n", GEOGROUP_NAME);
     }

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
        uint *a = pP->permute(sc.m_iNumNeighbors,  sc.m_iNumNeighbors, pWELL[omp_get_thread_num()]);
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
        printf("NN %d %d %d %lf %lf\n", iCell, j, iCell2, dLon, dLat);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// save
//
int LinkTools::save() {
    int iResult = 0;
    
    H5Gunlink(m_hFile, GRIDGROUP_NAME);
    GridWriter *pGridW  = new GridWriter(m_pCG, &m_pCG->getSurfaceData());
    iResult = pGridW->write(m_hFile);
    if (iResult == 0) {
        printf("OK\n");
    } else {
        printf("Couldn't write grid\n");
    }
    qdf_closeFile(m_hFile);
    return iResult;
}
/*
 herr_t H5Adelete (hid_t loc_id, const char *name)
A Dataset can be added to a Group with the H5Glink call, and deleted from a group with H5Gunlink.
*/
