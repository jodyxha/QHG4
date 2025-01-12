#include <cstdio>
#include <cstring>

#include "strutils.h"
#include "xha_strutilsT.h"
#include "Surface.h"
#include "EQsahedron.h"
#include "GridGroupReader.h"
#include "QDFUtils.h"
#include "SCellGrid.h"

#include "SurfaceGrid.h"

//----------------------------------------------------------------------------
// createInstance
//
SurfaceGrid *SurfaceGrid::createInstance(const std::string sQDF) {
    SurfaceGrid *pSG = new SurfaceGrid();
    int iResult = pSG->init(sQDF);
    if (iResult != 0) {
        delete pSG;
        pSG = NULL;
    }
    return pSG;
}


//----------------------------------------------------------------------------
// constructor
//
SurfaceGrid::SurfaceGrid() 
    : m_pCG(NULL),
      m_pSurf(NULL) {
}


//----------------------------------------------------------------------------
// destructor
//
SurfaceGrid::~SurfaceGrid() {

    if (m_pCG != NULL) {
        // delete[]  m_pCG->m_aCells;
        m_pCG->delGeography();
        m_pCG->delClimate();
        m_pCG->delVegetation();
        delete m_pCG;
    }

    if (m_pSurf != NULL) {
        delete m_pSurf;
    }
}


//----------------------------------------------------------------------------
// init
//
int SurfaceGrid::init(const std::string sQDF) {
    int iResult = 0;

    iResult = createCellGrid(sQDF);
    if (iResult == 0) {
        iResult = createSurface();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createCelLGrid
//
int SurfaceGrid::createCellGrid(const std::string sQDF) {
    int iResult = -1;
    hid_t hFile = qdf_openFile(sQDF);

    GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
    if (pGR != NULL) {
        GridAttributes gridatt;
        std::string sTime;
        // get the timestamp of the initial qdf file (grid)
        iResult = qdf_extractSAttribute(hFile,  ROOT_STEP_NAME, sTime);
        if (iResult != 0) {
            xha_printf("[createCellGrid] Couldn't read time attribute from grid file [%s]\n", sQDF);
            iResult = 0;
        } else {
            int iCurStep;
            if (strToNum(sTime, &iCurStep)) {
                iResult = 0;
                //                xha_printf("Have timestamp %f\n", fStartTime);
            } else {
                xha_printf("[createCellGrid] Timestamp not valid [%s]\n", sTime);
                iResult = -1;
            }
        }

        if (iResult == 0) {
            iResult = pGR->readAttributes(&gridatt);
            if (iResult == 0) {
                m_pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
                m_pCG->m_aCells = new SCell[gridatt.m_iNumCells];
                iResult = pGR->readData(m_pCG);

                if (iResult == 0) {
                    //                    xha_printf("[setGrid] Grid read successfully: %p\n", m_pCG);
                }
            } else {
                xha_printf("[createCellGrid] GridReader couldn't read  celldata\n");
            }
            if (iResult != 0) {
                delete m_pCG;
                m_pCG = NULL;
            }
        } else {
            xha_printf("[createCellGrid] GridReader couldn't read attributes\n");
        }
    
        delete pGR;
        
    } else {
        xha_printf("[createCellGrid] Couldn't create GridReader\n");
    }


    qdf_closeFile(hFile);

    return iResult;
}


//-----------------------------------------------------------------------------
// createSurface
//
int SurfaceGrid::createSurface() {
    int iResult = -1;

    if (m_pSurf == NULL) {
        if (m_pCG != NULL) {
            stringmap &smSurf = m_pCG->m_smSurfaceData;
            std::string sSurfType = smSurf["SURF_TYPE"].c_str();


            if (sSurfType == SURF_EQSAHEDRON) {
                int iSubDivs = -1;
                const char *pSubDivs = smSurf[SURF_IEQ_SUBDIVS].c_str();
                if (strToNum(pSubDivs, &iSubDivs)) {
                    if (iSubDivs >= 0) {
                        EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, true);
                        if (pEQ != NULL) {
                            //pEQ->relink();
                            m_pSurf = pEQ;
                            iResult = 0;
                            //printf("[createSurface] Have EQsahedron\n");
                        }
                    } else {
                        xha_printf("[createSurface] subdivs must be positive [%s]\n", pSubDivs);
                    }
                } else {
                    xha_printf("[createSurface] subdivs is not a number [%s]\n", pSubDivs);
                }

            } else {
                xha_printf("[createSurface] unknown surface type [%s] - we only do EQsahedron\n", sSurfType);
            }
            
        } else {
            xha_printf("[createSurface] can't create surface without CellGrid data\n");
        }
    }
    return iResult;
};


