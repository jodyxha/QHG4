#include <cstdio>
#include <hdf5.h>
#include <vector>

#include "stdstrutilsT.h"
#include "SCellGrid.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GridWriter.h"
#include "PopWriter.h"
#include "GeoWriter.h"
#include "ClimateWriter.h"
#include "VegWriter.h"
#include "NavWriter.h"
#include "OccWriter.h"
#include "StatusWriter.h"

//-----------------------------------------------------------------------------
// createInstance
//
StatusWriter *StatusWriter::createInstance(SCellGrid *pCG, std::vector<PopBase *> vPops) {
    StatusWriter *pSW = new StatusWriter();
    int iResult = pSW->init(pCG, vPops);
    if (iResult < 0) {
        delete pSW; 
        pSW = NULL;
    }
    return pSW;
}


//-----------------------------------------------------------------------------
// destructor
//
StatusWriter::~StatusWriter() {
    if (m_pPopW != NULL) {
        delete m_pPopW;
    }
    if (m_pGridW != NULL) {
        delete m_pGridW;
    }

    if (m_pGeoW != NULL) {
        delete m_pGeoW;
    }

    if (m_pCliW != NULL) {
        delete m_pCliW;
    }

    if (m_pVegW != NULL) {
        delete m_pVegW;
    }

    if (m_pNavW != NULL) {
        delete m_pNavW;
    }

    if (m_pOccW != NULL) {
        delete m_pOccW;
    }
}


//-----------------------------------------------------------------------------
// constructor
//
StatusWriter::StatusWriter()    
    : m_hFile(H5P_DEFAULT),
      m_pPopW(NULL),
      m_pGridW(NULL),
      m_pGeoW(NULL),
      m_pCliW(NULL),
      m_pVegW(NULL),
      m_pNavW(NULL),
      m_pOccW(NULL) {
    m_sError = "";
}


//-----------------------------------------------------------------------------
// init
//
int StatusWriter::init(SCellGrid *pCG, std::vector<PopBase *> vPops) {
    int iResult = 0;
    if (vPops.size() != 0) {
        m_pPopW   = new PopWriter(vPops);
    }
    m_pGridW  = new GridWriter(pCG, &pCG->m_smSurfaceData);
    m_pGeoW   = new GeoWriter(pCG->m_pGeography);
    m_pCliW   = new ClimateWriter(pCG->m_pClimate);
    m_pVegW   = new VegWriter(pCG->m_pVegetation);
    m_pNavW   = new NavWriter(pCG->m_pNavigation);
    m_pOccW   = new OccWriter(pCG->m_pOccTracker);

    return iResult;
}


//-----------------------------------------------------------------------------
// write
//   write elements specified by iWhat=ored combination of WR_XXX constants
//
int StatusWriter::write(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, int iWhat, int iDumpMode) {
    std::vector<std::pair<std::string, popwrite_flags>> vEmptySub;
    int iResult = write(sFileName, iStep, fStartTime, sInfoString, iWhat, vEmptySub, iDumpMode);
    return iResult;
}


//-----------------------------------------------------------------------------
// write
//   write elements specified by iWhat=ored combination of WR_XXX constants
//
int StatusWriter::write(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, int iWhat, std::vector<std::pair<std::string, popwrite_flags>> &vSub, int iDumpMode) {
    int iResult = 0;
    int iCur = iWhat;
    int iTemp = 0;
 
    stdprintf("[StatusWriter::write]iCur %d, WR_POP: %d, m_pPopW: %p\n", iCur, WR_POP, m_pPopW);
      
    m_hFile = qdf_createFile(sFileName, iStep, fStartTime, sInfoString);
    if (m_hFile > 0) {
	// there may be several populations - each contributing an additional WR_POP

        //        if (((iResult / output_flags::WR_POP) > 0) && (m_pPopW != NULL)) {
        stdprintf("iCur %d, WR_POP: %d, m_pPopW: %p\n", iCur, WR_POP, m_pPopW);
        if ((iCur /  WR_POP  > 0) && (m_pPopW != NULL)) {
            if (vSub.size() > 0) {
                stdprintf("writing pop\n");
                iTemp = 0;
                for (uint k = 0; (iTemp == 0) && (k < vSub.size()); k++) {
                    stdprintf("[StatusWriter::write] writing %s, iResult = %d\n",  vSub[k].first, iResult);
                    iTemp = m_pPopW->write(m_hFile, vSub[k].first, vSub[k].second, iDumpMode);
                    iCur -= (iTemp == 0)?WR_POP:0;
                }
            } else {
                m_sError += "Error : no pop names provided";
                iResult =-1;
            }
        }
        if ((iCur & WR_GRID) != 0) {
            iTemp = m_pGridW->write(m_hFile);
            iCur -= (iTemp == 0)?WR_GRID:0;
        }
        if ((iCur & WR_GEO) != 0) {
            iTemp = m_pGeoW->write(m_hFile);
            iCur -= (iTemp == 0)?WR_GEO:0;
        }
        if ((iCur & WR_CLI) != 0) {
            iTemp = m_pCliW->write(m_hFile);
            iCur -= (iTemp == 0)?WR_CLI:0;
        }
        if ((iCur & WR_VEG) != 0) {
            iTemp = m_pVegW->write(m_hFile);
            iCur -= (iTemp == 0)?WR_VEG:0;
        }
        if ((iCur & WR_NAV) != 0) {
            iTemp = m_pNavW->write(m_hFile);
            iCur -= (iTemp == 0)?WR_NAV:0;
        }
        if ((iCur & WR_OCC) != 0) {
            iTemp = m_pOccW->write(m_hFile);
            iCur -= (iTemp == 0)?WR_NAV:0;
        }

        // report unwritten stuff
        if (iCur > 0) {
            m_sError += "Warning : Couldn't write: ";
            
            if ((iCur / WR_POP) > 0) {
                m_sError += " pop";
            }
            if ((iCur & WR_GRID) != 0) {
                m_sError += " grid";
            }
            if ((iCur & WR_GEO) != 0) {
                m_sError += " geo";
            }
            if ((iCur & WR_CLI) != 0) {
                m_sError += " climate";
            }
            if ((iCur & WR_VEG) != 0) {
                m_sError += " veg";
            }
            if ((iCur & WR_NAV) != 0) {
                m_sError += " nav";
            }


            m_sError += "\n";
            //            if (iResult == iWhat) {
                iResult = -1;
                //
        }

        qdf_closeFile(m_hFile);  
        m_hFile = H5P_DEFAULT;
    }
    return iResult;
}

