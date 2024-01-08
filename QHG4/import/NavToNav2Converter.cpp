
#include <cstring>
#include <string>

#include "hdf5.h"

#include "types.h"
#include "GroupReader.h"

#include "SCellGrid.h"
#include "strutils.h"
#include "QDFUtils.h"

#include "Navigation.h"
#include "NavGroupReader.h"

#include "Navigation2.h"
#include "NavWriter2.h"

#include "NavToNav2Converter.h"


//----------------------------------------------------------------------------
// createInstance
//
NavToNav2Converter *NavToNav2Converter::createInstance(std::string sInputFile, int iNumCells) {
    NavToNav2Converter *pNNC = new NavToNav2Converter();
    int iResult = pNNC->init(sInputFile, iNumCells);
    if (iResult != 0) {
        delete pNNC;
        pNNC = NULL;
    }
    return pNNC;
}

//----------------------------------------------------------------------------
// constructor
//
NavToNav2Converter::NavToNav2Converter()
    : m_pCG(NULL),
      m_sInputFile(""),
      m_iNumCells(0),
      m_hInputFile(H5P_DEFAULT),
      m_pNav(NULL) {
}

//----------------------------------------------------------------------------
// destructor
//
NavToNav2Converter::~NavToNav2Converter() {
    if (m_pCG != NULL) {
        delete m_pCG;
    }
    if (m_pNav != NULL) {
        delete m_pNav;
    }
}


//----------------------------------------------------------------------------
// init
//
int NavToNav2Converter::init(std::string sInputFile, uint iNumCells) {
    int iResult = 0;

    m_sInputFile = sInputFile;
    m_hInputFile = qdf_openFile(sInputFile, true);
    if (m_hInputFile != H5P_DEFAULT) {
        uint iNC = getNumCells();
        if (iNC > 0) {
            if (iNC != iNumCells) {
                printf("Provided number of cells [%d] is not eqqual to number of files in file [%d]\n", iNumCells, iNC);
                iResult = -1;
            } else {
                m_iNumCells = iNC;
                printf("NumCells from file: %d\n", m_iNumCells);
                iResult = 0;
            }
        } else {
            if (iNumCells > 0) {
                m_iNumCells = iNumCells;
                printf("NumCells from input: %d\n", m_iNumCells);
                iResult = 0;
            } else {
                printf("No NumCells prrovided and no NumCells found in file\n");
                iResult = -1;
            }
        }

        stringmap sm;
        sm["DUMMY"] = "dummy";        
        m_pCG = new SCellGrid(0, m_iNumCells, sm);

        if (iResult == 0) {
            m_pNav = readNavigation();
            
            if (m_pNav != NULL) {
            } else {
                iResult = -1;
            }
        }

        // we dont need the file anymopre
        qdf_closeFile(m_hInputFile);

    } else {
        printf("Couldn't open inout file [%s]\n", m_sInputFile.c_str());
        iResult = -1;
    }

    return iResult;
            
}


//----------------------------------------------------------------------------
// readNavigation
//
Navigation *NavToNav2Converter::readNavigation() {
    int iResult = 0;

    NavGroupReader *pNR = NavGroupReader::createNavGroupReader(m_hInputFile);
    m_pNav = new Navigation(m_pCG);

    NavAttributes navatt;
    memset(&navatt, 0, sizeof(NavAttributes));
    iResult = pNR->readAttributes(&navatt);
    if (iResult == 0) {

        iResult = pNR->readData(m_pNav);
        if (iResult == 0) {
            if (navatt.m_iNumBridges > 0) {
                iResult = pNR->readBridges(m_pNav);
            }
            if (iResult == 0) {
                m_pCG->setNavigation(m_pNav);
            } else {
                printf("[setNav] Couldn't read bridges\n");
                iResult = -1;
            }
        } else {
            printf("[setNav] Couldn't read data\n");
            iResult = -1;
        } 
    } else {
        printf("[setNav] Couldn't read attributes\n");
        iResult = -1;
    }

    if (iResult != 0) {
        delete m_pNav;
        m_pNav = NULL;
    }

    return m_pNav;
}

//----------------------------------------------------------------------------
// countConnections
//
uint NavToNav2Converter::countConnections() {
    uint waycount = 0;

    distancemap &dimi = m_pNav->m_mDestinations;
    
    distancemap::const_iterator it1;
    for (it1 = dimi.begin(); it1 != dimi.end(); ++it1) {
        waycount += it1->second.size();
    }
    printf("found %u connections in input file\n", waycount);
    return waycount;
}


//----------------------------------------------------------------------------
// display
//
void NavToNav2Converter::display() {
    distancemap &dimi = m_pNav->m_mDestinations;

    distancemap::const_iterator it1;
    for (it1 = dimi.begin(); it1 != dimi.end(); ++it1) {
        printf("%d\n", it1->first);
        distlist::const_iterator it2;
        for (it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
            printf("  %d %f\n", it2->first, it2->second);
        }
    }
}


//----------------------------------------------------------------------------
// createWriteNav2
//
int NavToNav2Converter::createWriteNav2(std::string sOutputFile) {
    int iResult = 0;

    uint x = countConnections();
    Navigation2 *pNav2 = new Navigation2(m_pCG);
    pNav2->m_iNumWaterWays = x; //countConnections();
    pNav2->m_dSampleDist   = m_pNav->m_dSampleDist;
    pNav2->m_iNumBridges   = m_pNav->m_iNumBridges;

    pNav2->m_mWaterWays  = m_pNav->m_mDestinations;

    if (m_pNav->m_iNumBridges > 0) {
        pNav2->m_vBridges = m_pNav->m_vBridges;
    }

    hid_t hOutputFile = qdf_opencreateFile(sOutputFile, 0, 0, "NavCopy", true);
    if (hOutputFile != H5P_DEFAULT) {
        NavWriter2 *pNW = new NavWriter2(pNav2);

        iResult = pNW->write(hOutputFile);
        if (iResult == 0) {
            // nothing to be done
        } else {
            printf("couldn't write to [%s]\n", sOutputFile.c_str());
            iResult = -1;
        }
        delete pNW;
        qdf_closeFile(hOutputFile);
    } else {
        printf("Couldn't open file [%s]\n", sOutputFile.c_str());
        iResult = -1;
    }

    delete pNav2;

    return iResult;
}


//----------------------------------------------------------------------------
// getNumCells
//
int NavToNav2Converter::getNumCells() {
    int iResult = 0;

    std::string sPath = "/"+GRIDGROUP_NAME+"/"+GRID_ATTR_NUM_CELLS;
    hid_t hGridGroup = qdf_openGroup(m_hInputFile, GRIDGROUP_NAME);
    if (hGridGroup != H5P_DEFAULT) {
        iResult = qdf_extractAttribute(hGridGroup, GRID_ATTR_NUM_CELLS, 1, &m_iNumCells);
        if (iResult != 0) {
            printf("Couldn'tread attribute [%s]\n", GRID_ATTR_NUM_CELLS.c_str());
            m_iNumCells = 0;
        }
    } else {
        printf("Couldn't open group [%s]\n", GRIDGROUP_NAME.c_str());
    }
    return m_iNumCells;
}


//----------------------------------------------------------------------------
// convert
//
int NavToNav2Converter::convert(std::string sOutputFile) {
    int iResult = -1;

    if (m_pNav != NULL) {
        //display();

        iResult = createWriteNav2(sOutputFile);
    } else {
        printf("It seems as if init() has not been called\n");
        iResult = -1;
    }

    return iResult;
}
