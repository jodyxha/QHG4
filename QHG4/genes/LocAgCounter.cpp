#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <omp.h>
#include "types.h"
#include "qhg_consts.h"
#include "xha_strutilsT.h"
#include "colors.h"
#include "geomutils.h"
#include "WELL512.h"
#include "QDFUtils.h" 
#include "QDFArray.h"
#include "QDFArrayT.h" 
#include "AnalysisUtils.h"
#include "LocAgCounter.h"

#define RADIUS 6371.3

//----------------------------------------------------------------------------
// destructor
//
LocAgCounter::~LocAgCounter() {
    deleteArrays();

}

//----------------------------------------------------------------------------
// createInstance
//
LocAgCounter *LocAgCounter::createInstance(const std::string sQDFGrid, const std::string sQDFTime, const std::string sLocFile, bool bCartesian) {
    LocAgCounter *pLAC = new LocAgCounter();
    int iResult = pLAC->init(sQDFGrid, sQDFTime, sLocFile, bCartesian);
    if (iResult != 0) {
        delete pLAC;
        pLAC = NULL;
    }
    return pLAC;
}


//----------------------------------------------------------------------------
// doCounts
//
int LocAgCounter::doCounts() {
    int iResult = -1;
    iResult = getCandidatesNew();
    if (iResult == 0) {
        
    
    }  
    return iResult;
}


//----------------------------------------------------------------------------
// constructor
//
LocAgCounter::LocAgCounter()
    : m_iNumAgents(0),
      m_pIDs(NULL),
      m_pCellIDs(NULL) {
}


//----------------------------------------------------------------------------
// deleteArrays
//
void LocAgCounter::deleteArrays() {
    if (m_pIDs != NULL) {
        delete[]  m_pIDs;
    }
    m_pIDs = NULL;

    if (m_pCellIDs != NULL) {
        delete[]  m_pCellIDs;
    }
    m_pCellIDs = NULL;
}


//----------------------------------------------------------------------------
// init
//
int LocAgCounter::init(const std::string sQDFGrid, const std::string sQDFTime, const std::string sLocFile, bool bCartesian) {
    int iResult = -1;
    
    std::string sPopName = "";
    size_t iPos = sQDFTime.find(":");
    if (iPos != std::string::npos) {
        sPopName = sQDFTime.substr(iPos+1);
    } else {
        sPopName = qdf_getFirstPopulation(sQDFTime);
    }
    std::string sQDFTimeReal = sQDFTime.substr(0, iPos);
    
    xha_printf("Popfile [%s], Population [%s]\n", sQDFTime, sPopName);

    iResult = fillCoordMap(sQDFGrid);
    if (iResult == 0) {
        locspec ls(sLocFile);
        m_mLocData.clear();
        
        iResult = readArrays(sQDFTimeReal, sPopName);
        if (iResult == 0) {
            iResult = fillLocData(&ls, m_mLocData, &m_vNames);
        }

    }  

    if (bCartesian) {
        m_fCalcDist = &cartdist;
    } else { 
        m_fCalcDist = &spherdistDeg;
        m_dScale = RADIUS;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// getCandidatesNew
//
int LocAgCounter::getCandidatesNew() {
    int iResult = 0;

    xha_printf("new version 1: loc-parag\n");
    // each agent should only belong to one region
    loc_data::const_iterator it;
    loccounts *amvCandidatesPar = new loccounts[omp_get_max_threads()];

    for (it = m_mLocData.begin(); (iResult == 0)  && (it != m_mLocData.end()); ++it) {
        m_mCounts[it->first] = 0;
        for (int iT = 0; iT < omp_get_max_threads(); iT++) {
            amvCandidatesPar[iT][it->first] = 0;
        }

        double dLon0 = it->second.dLon;
        double dLat0 = it->second.dLat;
        
#pragma omp parallel for
        for (int i = 0; i < m_iNumAgents; i++) {
            double dLon1 = m_mCoords[m_pCellIDs[i]].first;
            double dLat1 = m_mCoords[m_pCellIDs[i]].second;
        
            
            //            double d = spherdist(dLon0*Q_PI/180, dLat0*Q_PI/180, dLon1*Q_PI/180, dLat1*Q_PI/180, RADIUS);
            double d = m_fCalcDist(dLon0, dLat0, dLon1, dLat1, m_dScale);
            

            if (d < it->second.dDist) {
                amvCandidatesPar[omp_get_thread_num()][it->first]++;
            }
        }
    }
    
    for (int i = 0; i < omp_get_max_threads(); i++) {
        loccounts::const_iterator it2;
        for (it2 = amvCandidatesPar[i].begin(); it2 != amvCandidatesPar[i].end(); ++it2) {
            m_mCounts[it2->first] += it2->second;
        }            
    }
    // printf("iTotPar: %d (mvCandidates:%zd)\n", iTotPar, mvCandidates.size());
    delete[] amvCandidatesPar;
    return iResult;
}




//----------------------------------------------------------------------------
// readArrays
//
int LocAgCounter::readArrays(const std::string sQDFTime, const std::string sPopName) {
    int iResult = -1;

    deleteArrays();

    m_pIDs        = NULL;
    m_pCellIDs    = NULL;

    m_iNumAgents  = 0;

    QDFArray *pQA = QDFArray::create(sQDFTime);
    if (pQA != NULL) {

        iResult = pQA->openArray(POPGROUP_NAME, sPopName, AGENT_DATASET_NAME);
        if (iResult == 0) {
            m_iNumAgents = pQA->getSize();

            m_pIDs      = new idtype[m_iNumAgents];
            m_pCellIDs  = new int[m_iNumAgents];
            
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pIDs, m_iNumAgents, "AgentID");
                if (iCount != m_iNumAgents) {
                    xha_fprintf(stderr, "%sGot %d agent IDs instead of %d%s\n", colors::RED, iCount, m_iNumAgents, colors::OFF);
                    iResult = -1;
                }
            }
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pCellIDs, m_iNumAgents, "CellID");
                if (iCount != m_iNumAgents) {
                    xha_fprintf(stderr, "%sGot %d cell IDs instead of %d%s\n", colors::RED, iCount, m_iNumAgents, colors::OFF);
                    iResult = -1;
                }
            }
                
            if (iResult == 0) {
                printf("  read agent data: %d items\n", m_iNumAgents);
            }
        } else {
            xha_fprintf(stderr, "%sCouldn't open dataset [%s/%s%s]%s\n", colors::RED, POPGROUP_NAME, sPopName, AGENT_DATASET_NAME, colors::OFF);
        }
        pQA->closeArray();

        delete pQA;
    } else {
        iResult = -1;
        xha_fprintf(stderr, "%sCouldn't open file [%s]%s\n", colors::RED, sQDFTime, colors::OFF);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//
int LocAgCounter::fillCoordMap(const std::string sQDFGeoGrid) {
    int iResult = -1;
    uint iNumCells = 0;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
    
    QDFArray *pQA = QDFArray::create(sQDFGeoGrid);
    if (pQA != NULL) {
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            iNumCells = pQA->getSize();
            pCellIDs = new int[iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, iNumCells, GRID_DS_CELL_ID);
            if (iCount == iNumCells) {
                //                printf("Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                xha_fprintf(stderr, "%sRead bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)%s\n", colors::RED, sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells, colors::OFF);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            xha_fprintf(stderr, "%sCouldn't open QDF array for [%s:%s/%s]%s\n", colors::RED, sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME, colors::OFF);
        }

        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLon = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, iNumCells);
                    if (iCount == iNumCells) {
                        xha_printf("Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "%sRead bad number of read longitudes from [%s:%s/%s]: %d instead of %d%s\n", colors::RED, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells, colors::OFF);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "%sNumber of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", colors::RED, iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, colors::OFF);
                }
                pQA->closeArray();
            }
        }

        if (iResult == 0) {
            std::string sPath = xha_sprintf("%s/%s", GEOGROUP_NAME, GEO_DS_LATITUDE);
            iResult = pQA->openArray(sPath);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLat = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, iNumCells);
                    if (iCount == iNumCells) {
                        xha_printf("Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "%sCouldn't read latitudes from [%s:%s/%s]: %d instead of %d%s\n", colors::RED, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL,iNumCells, colors::OFF);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "%sNumber of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", colors::RED, iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, colors::OFF);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        xha_fprintf(stderr, "%sCouldn't create QDFArray%s\n", colors::RED, colors::OFF);
    }
     
    if (iResult == 0) {
        // save coordinate values
        for (uint i = 0; i < iNumCells; i++) {
            m_mCoords[pCellIDs[i]] = std::pair<double, double>(pdLon[i], pdLat[i]);
        }
    }

    if (pCellIDs != NULL) {
        delete[] pCellIDs;
    }
    if (pdLon != NULL) {
        delete[] pdLon;
    }
    if (pdLat != NULL) {
        delete[] pdLat;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// show
//
void LocAgCounter::show(bool bNice, bool bSort) {
    

    if (bSort) {
        std::sort(m_vNames.begin(), m_vNames.end());
    }


    if (bNice) {

        uint iMaxL = 0;
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            uint iL = it->first.length();
            if (iL > iMaxL) {
                iMaxL = iL;
            }
        }
        
        stringvec::const_iterator itn;
        for (itn = m_vNames.begin(); itn != m_vNames.end(); ++itn) {
            std::string sName = *itn;
            if (sName.length() < iMaxL) {
                sName.append(iMaxL - sName.length(), ' ');
            }
            locitem &lItem = m_mLocData[*itn];
            xha_printf("%s  (%+7.2f,%+6.2f;%6.1f):\tNum %d\n", 
                   sName, 
                   lItem.dLon, 
                   lItem.dLat, 
                   lItem.dDist,
                   m_mCounts[sName]);
          
        }

    } else {

        printf("#Location\tLon\tLat\tSampDist\tNumAgennts\n");
        stringvec::const_iterator itn;
        for (itn = m_vNames.begin(); itn != m_vNames.end(); ++itn) {
            locitem &lItem = m_mLocData[*itn];
            xha_printf("%s\t%6.2f\t%5.2f\t%6.1f\t%d\n", 
                   *itn, 
                   lItem.dLon, 
                   lItem.dLat, 
                   lItem.dDist,
                   m_mCounts[*itn]);
        }
    }
}
