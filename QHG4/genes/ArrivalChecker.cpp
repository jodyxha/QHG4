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
#include "LineReader.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"
#include "QDFArray.cpp"
#include "AnalysisUtils.h"
#include "ArrivalChecker.h"

#define RADIUS 6371.3

#define SPOP_DT_CELL_ID    "CellID"
#define SPOP_DT_AGENT_ID   "AgentID"

//----------------------------------------------------------------------------
// constructor
//
ArrivalChecker::ArrivalChecker(FILE *fOut) 
    : m_iNumCells(0),
      m_pCellIDs(NULL),
      m_pTravelTimes(NULL),
      m_pTravelDists(NULL), 
      m_iNumAgents(0),
      m_iTotal(0),
      m_pInfos(NULL),
      m_pCounts(NULL),
      m_bEmptyOutput(false),
      m_fOut(fOut) {
}


//----------------------------------------------------------------------------
// destructor
//
ArrivalChecker::~ArrivalChecker() {

    deleteArrays();

}


//----------------------------------------------------------------------------
// createInstance
//
ArrivalChecker *ArrivalChecker::createInstance(const std::string sQDFGrid, const std::string sQDFStats, const std::string sLocFile, double dDistance, FILE *fOut) {
    ArrivalChecker *pAC = new ArrivalChecker(fOut);
    int iResult = pAC->init(sQDFGrid, sQDFStats, sLocFile, dDistance);
    if (iResult != 0) {
        delete pAC;
        pAC = NULL;
    }
    return pAC;
}


//----------------------------------------------------------------------------
// deleteArrays
//
void ArrivalChecker::deleteArrays() {

    if (m_pCellIDs != NULL) {
        delete[]  m_pCellIDs;
    }
    m_pCellIDs = NULL;

    if (m_pTravelTimes != NULL) {
        delete[]  m_pTravelTimes;
    }
    m_pTravelTimes = NULL;

    if (m_pTravelDists != NULL) {
        delete[]  m_pTravelDists;
    }
    m_pTravelDists = NULL;

    if (m_pmCandidates != NULL) {
        delete[] m_pmCandidates;
    }
    m_pmCandidates = NULL;

    if (m_pInfos != NULL) {
        delete[] m_pInfos;
    }
    m_pInfos = NULL;

    if (m_pCounts != NULL) {
        delete[] m_pCounts;
    }
    m_pCounts = NULL;

    loccelldist::iterator it;
    for (it = m_mFinalCellDist.begin(); it != m_mFinalCellDist.end(); ++it) {
        delete it->second;
    }
}


//----------------------------------------------------------------------------
// init
//
int ArrivalChecker::init(const std::string sQDFGrid, const std::string sQDFStats, const std::string sLocFile, double dDistance) {
    int iResult = 0;
    
    if (!sQDFGrid.empty()) {

        stringvec vParts;
        std::string sSpecies1("");
        std::string sSpecies2("");
        std::string sQDFGridReal("");
        std::string sQDFStatsReal("");

        if (iResult == 0) {
            uint iNum1 = splitString(sQDFGrid, vParts, ":");
            if (iNum1 > 0) {
                sQDFGridReal = vParts[0];
                if (iNum1 == 2) {
                    sSpecies1 = vParts[1];
                    xha_fprintf(stderr, "found species [%s] with file [%s]\n", sSpecies1, sQDFGridReal);
                }
            } else {
                iResult = -1;
            }
        }

        if (iResult == 0) {
            uint iNum2 = splitString(sQDFStats, vParts, ":");
            if (iNum2 > 0) {
                sQDFStatsReal = vParts[0];
                if (iNum2 == 2) {
                    sSpecies2 = vParts[1];
                    xha_fprintf(stderr, "found species [%s] with file [%s]\n", sSpecies2, sQDFStatsReal);
                }
            } else {
                iResult = -1;
            }
        }

        if (iResult == 0) {
            if (sSpecies1.empty()) {
                if (sSpecies2.empty()) {
                    iResult = -1;
                    xha_fprintf(stderr, "[init] No species names provided\n");
                } else {
                    sSpecies1 = sSpecies2;
                    iResult = 0;
                }
            } else {
                if (!sSpecies2.empty()) {
                    if (sSpecies1 != sSpecies2) {
                        xha_fprintf(stderr, "[init] Two different species names provided: [%s], [%s]\n", sSpecies1, sSpecies2);
                        iResult = -1;
                    } else {
                        iResult = 0;
                    }
                } else {
                    iResult = 0;
                }
            }
        }
        xha_fprintf(stderr, "[init] using speciea [%s]\n", sSpecies1);
        
        if (iResult == 0) {
            iResult = fillCoordMap(sQDFGridReal);
        }

        if (iResult == 0) {
            iResult = readStats(sQDFStatsReal, sSpecies1);
            xha_fprintf(stderr, "[init] readStats(%s,%s): res %d\n", sQDFStatsReal, sSpecies1, iResult);
            if (iResult == 0) {
                locspec locSpec(sLocFile, dDistance, 0);
                m_vNames.clear();
 
                iResult = fillLocData(&locSpec, m_mLocData, &m_vNames);
                xha_fprintf(stderr, "[init] fillLocData(...): res %d\n", iResult);
                if (iResult == 0) { 
                    xha_fprintf(stderr, "[init] loading agents cell (%s,%s)\n", sQDFStatsReal, sSpecies1);
                    iResult = loadAgentsCell(sQDFStatsReal, sSpecies1);
                    xha_fprintf(stderr, "[init] loadAgentsCell(%s,%s): reds %d\n", sQDFStatsReal, sSpecies1, iResult);
                    if (iResult == 0) {
                        xha_fprintf(stderr, "[init] getting cell agent counts\n");
                        iResult = getCellAgentCounts();
                        if (iResult == 0)  {
                            m_pmCandidates = new loccelldists[omp_get_max_threads()];
                        } else {
                            xha_fprintf(stderr, "[init] Couldn't create counts\n");
                        }
                    } else {
                        xha_fprintf(stderr, "[init] Couldn't read agent data\n");
                    }
                }
            }
        }
    } else {
        m_bEmptyOutput = true;
        locspec locSpec(sLocFile, dDistance, 0);
        m_vNames.clear();
        iResult = fillLocData(&locSpec, m_mLocData, &m_vNames);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// findClosestCandidates
//
int ArrivalChecker::findClosestCandidates(bool bSpherical) {
    int iResult = 0;

    if (bSpherical) {
        iResult = calcSphericalDistances();
    } else {
        iResult = calcCartesianDistances();
    }

    // accumulate
    for (int iT = 1; iT < omp_get_max_threads(); ++iT) {
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            m_pmCandidates[0][it->first].insert(m_pmCandidates[0][it->first].end(),
                                                m_pmCandidates[iT][it->first].begin(),
                                                m_pmCandidates[iT][it->first].end());
        }
    }

    // find closest: replace vector of candidates with a one-element vector of closest
    m_mFinalCellDist.clear(); 
    loc_data::const_iterator it;
    for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
        double dMin = 1e9;
        int iCellMin = -1;
        std::vector<celldist> &v = m_pmCandidates[0][it->first];
        for (uint i = 0; i < v.size(); ++i) {
            if (m_pTravelTimes[v[i].first] >= 0) {
                if (v[i].second < dMin) {
                    dMin = v[i].second;
                    iCellMin = v[i].first;
                }
            }
        }
        int iNum = 0;
	iNum = getCountsForCells(v);
	m_mFinalCellDist[it->first] = new celldistnum(iCellMin, (dMin<1e9)?dMin:-1, iNum); 
    }
    return iResult;
}


//----------------------------------------------------------------------------
// showTable
//
void ArrivalChecker::showTable(bool bNice, bool bSort) {
    

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
            locitem &lItem = m_mLocData[*itn];
            int    iCellMin   = m_mFinalCellDist[*itn]->m_iCellID;
            double dMin       = m_mFinalCellDist[*itn]->m_dDist;
	    int    iNumAgents = m_mFinalCellDist[*itn]->m_iNumAgents;
            std::string sName(*itn);
            if (sName.length() < iMaxL) {
                sName.append(iMaxL - sName.length(), ' ');
            }
            xha_fprintf(m_fOut, "%s  (%+7.2f,%+6.2f;%6.1f):\tT %7.1f D %7.1f d%6.1f N %d\n", 
                       *itn, 
                       lItem.dLon, 
                       lItem.dLat, 
                       lItem.dDist,
                       (dMin < 0)?-1:m_pTravelTimes[iCellMin], 
                       (dMin < 0)?-1:m_pTravelDists[iCellMin], 
                       ((iCellMin >= 0) && (m_pTravelTimes[iCellMin] >= 0))?dMin:-1.0,
                       iNumAgents);
          
        }

    } else {

        xha_fprintf(m_fOut, "#Location\tLon\tLat\tSampDist\tTravelTime\tTravelDist\tmindist\tnumagents\n");
        stringvec::const_iterator itn;
        for (itn = m_vNames.begin(); itn != m_vNames.end(); ++itn) {
            locitem &lItem = m_mLocData[*itn];
            int    iCellMin   = m_mFinalCellDist[*itn]->m_iCellID;
            double dMin       = m_mFinalCellDist[*itn]->m_dDist;
	    int    iNumAgents = m_mFinalCellDist[*itn]->m_iNumAgents;
            xha_fprintf(m_fOut, "%s\t%6.2f\t%5.2f\t%6.1f\t%7.1f\t%7.1f\t%6.1f\t%d\n", 
                   *itn, 
                   lItem.dLon, 
                   lItem.dLat, 
                   lItem.dDist,
                   (dMin < 0)?-1:m_pTravelTimes[iCellMin], 
                   (dMin < 0)?-1:m_pTravelDists[iCellMin], 
                   ((iCellMin >= 0) && (m_pTravelTimes[iCellMin] >= 0))?dMin:-1.0,
		   iNumAgents);
        }
    }
}


//----------------------------------------------------------------------------
// show
//
void ArrivalChecker::showCSV(bool bHead, int iWhat) {
    
    std::string sSep="";
    stringvec::const_iterator itn;
    if (bHead) {
        for (itn = m_vNames.begin(); itn != m_vNames.end(); ++itn) {
            if  ((iWhat & ARR_NAME) != 0) {
                xha_fprintf(m_fOut, "%s%s_name", sSep, *itn);
                sSep = ";";
            }
            if  ((iWhat & ARR_LON) != 0) {
                xha_fprintf(m_fOut, "%s%s_lon", sSep, *itn);
                sSep = ";";
            }
            if  ((iWhat & ARR_LAT) != 0) {
                xha_fprintf(m_fOut, "%s%s_lat", sSep, *itn);
                sSep = ";";
            }
            if  ((iWhat & ARR_TIME) != 0) {
                xha_fprintf(m_fOut, "%s%s_arrival", sSep, *itn);
                sSep = ";";
            }
            if  ((iWhat & ARR_COUNT) != 0) {
                xha_fprintf(m_fOut, "%s%s_count", sSep, *itn);
                sSep = ";";
            }
        }
        xha_fprintf(m_fOut, "\n");
        xha_fprintf(stderr, "[showCSV] one line written\n");
    }
    
    sSep = "";
    for (itn = m_vNames.begin(); itn != m_vNames.end(); ++itn) {
        locitem &lItem = m_mLocData[*itn];
        if ((iWhat & ARR_NAME) != 0) {
            xha_fprintf(m_fOut, "%s%s", sSep,  *itn);
            sSep = ";";
        }
        if ((iWhat & ARR_LON) != 0) {
            xha_fprintf(m_fOut, "%s%f", sSep,  lItem.dLon);
            sSep = ";";
        }
        if  ((iWhat & ARR_LAT) != 0) {
            xha_fprintf(m_fOut, "%s%f", sSep,  lItem.dLat);
            sSep = ";";
        }
        if (m_bEmptyOutput) {
            if  ((iWhat & ARR_TIME) != 0) {
                xha_fprintf(m_fOut, "%s-1", sSep);
                sSep = ";";
            }
            if  ((iWhat & ARR_COUNT) != 0) {
                xha_fprintf(m_fOut, "%s-1", sSep);
                sSep = ";";
            }
        } else {
            double dMin       = m_mFinalCellDist[*itn]->m_dDist;
            int    iCellMin   = m_mFinalCellDist[*itn]->m_iCellID;
            if  ((iWhat & ARR_TIME) != 0) {
                xha_fprintf(m_fOut, "%s%f", sSep,   (dMin < 0)?-1:m_pTravelTimes[iCellMin]);
                sSep = ";";
            }
            if  ((iWhat & ARR_COUNT) != 0) {
                xha_fprintf(m_fOut, "%s%d", sSep,  m_mFinalCellDist[*itn]->m_iNumAgents);
                sSep = ";";
            }
        }
    }
    xha_fprintf(m_fOut, "\n");
    
}
 


//----------------------------------------------------------------------------
// readStats
//
int ArrivalChecker::readStats(const std::string sQDFStats, const std::string sSpecies) {
    int iResult = -1;
    
    QDFArray *pQA2 = QDFArray::create(sQDFStats);
    if (pQA2 != NULL) {
        std::string sGroupSpec = "";
        std::string sCurSpecies = "";
        if (sSpecies.empty())  {
            std::string sFirstPop = qdf_getFirstPopulation(sQDFStats);
            if (!sFirstPop.empty()) {
                sCurSpecies = sFirstPop;
                xha_fprintf(stderr, "[readStats] no species given - using first species [%s]\n", sCurSpecies);
            }
        } else {
            sCurSpecies = sSpecies;
            xha_fprintf(stderr, "[readStats] using species [%s]\n", sCurSpecies);
        }

        // get arrival times
        if (!sCurSpecies.empty()) {
            sGroupSpec = xha_sprintf("Populations/%s/MoveStats", sCurSpecies); 
            iResult = pQA2->openArray(sGroupSpec, SPOP_DS_TIME);
        }

        if (iResult == 0) {
            m_iNumCells = pQA2->getSize();
            m_pTravelTimes = new double[m_iNumCells];
            uint iCount = pQA2->getFirstSlab(m_pTravelTimes, m_iNumCells);
            if (iCount == m_iNumCells) {
                //                printf("Read %d CellIDs\n", iCount);
                xha_fprintf(stderr,"[readStats] Read TravelTimes [%s; %s/%s]", sQDFStats, sGroupSpec, SPOP_DS_TIME);
                iResult = 0;
            } else {
                xha_fprintf(stderr, "%s[readStats] Read bad number of grid IDs from [%s:%s/%s]: %d (instead of %d)%s\n", 
                           colors::RED, sQDFStats, sGroupSpec, SPOP_DS_TIME, iCount, m_iNumCells, colors::OFF);
                iResult = -1;
            }
            pQA2->closeArray();
        } else {
            iResult = -1;
            xha_fprintf(stderr, "%s[readStats] Couldn't open QDF array for [%s:%s/%s]%s\n", 
                    colors::RED, sQDFStats, sGroupSpec, SPOP_DS_TIME, colors::OFF);
        }

        if (iResult == 0) {
            // get travelled distance
            sGroupSpec = xha_sprintf("Populations/%s/MoveStats", sCurSpecies); 
            iResult = pQA2->openArray(sGroupSpec, SPOP_DS_DIST);
            

            if (iResult == 0) {
                //iNumCells = pQA2->getSize();
                m_pTravelDists = new double[m_iNumCells];
                uint iCount = pQA2->getFirstSlab(m_pTravelDists, m_iNumCells);
                if (iCount == m_iNumCells) {
                    //                printf("Read %d CellIDs\n", iCount);
                    xha_fprintf(stderr,"[readStats] Read Distances [%s; %s/%s]", sQDFStats, sGroupSpec, SPOP_DS_DIST);
                    iResult = 0;
                } else {
                    xha_fprintf(stderr, "%s[readStats] Read bad number of grid IDs from [%s:%s/%s]: %d (instead of %d)%s\n", 
                            colors::RED, sQDFStats, sGroupSpec, SPOP_DS_DIST, iCount, m_iNumCells, colors::OFF);
                    iResult = -1;
                }
                pQA2->closeArray();
            } else {
                iResult = -1;
                xha_fprintf(stderr, "%s[readStats] Couldn't open QDF array for [%s:%s/%s]%s\n", 
                        colors::RED, sQDFStats, sGroupSpec, SPOP_DS_DIST, colors::OFF);
            }
            
        }
        delete pQA2;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//
int ArrivalChecker::fillCoordMap(const std::string sQDFGeoGrid) {
    int iResult = -1;
  
    double *pdLon = NULL;
    double *pdLat = NULL;
    
    QDFArray *pQA = QDFArray::create(sQDFGeoGrid);
    if (pQA != NULL) {
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            m_iNumCells = pQA->getSize();
            m_pCellIDs = new int[m_iNumCells];
            uint iCount = pQA->getFirstSlab(m_pCellIDs, m_iNumCells, GRID_DS_CELL_ID);
            if (iCount == m_iNumCells) {
                //                printf("Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                xha_fprintf(stderr, "%s[fillCoordMap] Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)%s\n", 
                        colors::RED, sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME, GRID_DS_CELL_ID, iCount, m_iNumCells, colors::OFF);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            xha_fprintf(stderr, "%s[fillCoordMap] Couldn't open QDF array for [%s:%s/%s]%s\n", 
                    colors::RED, sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME, colors::OFF);
        }

        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == m_iNumCells) {
                    pdLon = new double[m_iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, m_iNumCells);
                    if (iCount == m_iNumCells) {
                        //                    printf("Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "%s[fillCoordMap] Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d%s\n", 
                                colors::RED, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, m_iNumCells, colors::OFF);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "%s[fillCoordMap] Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", 
                            colors::RED, iNumCellsL, m_iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, colors::OFF);
                }
                pQA->closeArray();
            }
        }

        if (iResult == 0) {
            iResult = pQA->openArray( GEOGROUP_NAME, GEO_DS_LATITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == m_iNumCells) {
                    pdLat = new double[m_iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, m_iNumCells);
                    if (iCount == m_iNumCells) {
                        //                        printf("Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "%s[fillCoordMap] Couldn't read latitudes from [%s:%s/%s]: %d instead of %d%s\n", 
                                colors::RED, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iNumCellsL,m_iNumCells, colors::OFF);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "%s[fillCoordMap] Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", 
                            colors::RED, iNumCellsL, m_iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, colors::OFF);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        xha_fprintf(stderr, "%s[fillCoordMap] Couldn't create QDFArray%s\n", colors::RED, colors::OFF);
    }

 
     
    if (iResult == 0) {
        // save coordinate values
        for (uint i = 0; i < m_iNumCells; i++) {
            m_mCoords[m_pCellIDs[i]] = std::pair<double, double>(pdLon[i], pdLat[i]);
        }
 
        xha_fprintf(stderr, "[fillCoordMap]   read cell coordinates: %zd items\n", m_mCoords.size());
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
// SphericalDistance
//
int ArrivalChecker::calcSphericalDistances() {
    int iResult = 0;
#pragma omp parallel for
    for (uint iCell = 0; iCell < m_iNumCells; ++iCell) {
        std::pair<double,double> &coords1 = m_mCoords[iCell];
        double dLon0 = coords1.first*Q_PI/180;
        double dLat0 = coords1.second*Q_PI/180;
        double dX0 = cos(dLon0)*cos(dLat0);    
        double dY0 = sin(dLon0)*cos(dLat0);    
        double dZ0 = sin(dLat0);    
 
        int iT = omp_get_thread_num();
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            double dLon1 = it->second.dLon*Q_PI/180;
            double dLat1 = it->second.dLat*Q_PI/180;
            double dX1   = cos(dLon1)*cos(dLat1);    
            double dY1   = sin(dLon1)*cos(dLat1);    
            double dZ1   = sin(dLat1);    
            double dProd = dX0*dX1+dY0*dY1+dZ0*dZ1;
            if (dProd > 1) {
                dProd = 1;
            } else if (dProd < -1) {
                dProd = -1;
            }
            double dDist = RADIUS * acos(dProd);
            
            if (dDist < it->second.dDist) {
                m_pmCandidates[iT][it->first].push_back(celldist(iCell, dDist));
            }
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// CartesianDistance
//
int ArrivalChecker::calcCartesianDistances() {
    int iResult = 0;
#pragma omp parallel for
    for (uint iCell = 0; iCell < m_iNumCells; ++iCell) {
        std::pair<double,double> &coords1 = m_mCoords[iCell];
        double dX0 = coords1.first;
        double dY0 = coords1.second;
 
        int iT = omp_get_thread_num();
        loc_data::const_iterator it;
        for (it = m_mLocData.begin(); it !=  m_mLocData.end(); ++it) {
            double dX1 = it->second.dLon;
            double dY1 = it->second.dLat;
            double dDist = sqrt((dX0-dX1)*(dX0-dX1) + (dY0-dY1)*(dY0-dY1));
            
            if (dDist < it->second.dDist) {
                m_pmCandidates[iT][it->first].push_back(celldist(iCell, dDist));
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// loadAgentsCell
//
int ArrivalChecker::loadAgentsCell(const std::string sPop, const std::string sPopName) {
    hid_t hFilePop     = qdf_openFile(sPop);
    hid_t hPopulation  = qdf_openGroup(hFilePop, POPGROUP_NAME);
    xha_fprintf(stderr, "[loadAgentsCell] pop %s,popname %s\n", sPop, sPopName);
    // at this point m_iNumCells should be known (loadNPP() loadAltIce() already called)
    m_pCounts = new int[m_iNumCells];

    int iResult = 0;

    //std::vector<std::string>::iterator it;
    //for (it = m_vNames.begin(); (iResult == 0) && (it != m_vNames.end()); ++it) {

        hid_t hSpecies     = qdf_openGroup(hPopulation, sPopName);
        hid_t hDataSet     = qdf_openDataSet(hSpecies, AGENT_DATASET_NAME, H5P_DEFAULT);
        hid_t hDataSpace   = H5Dget_space(hDataSet);

        hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof (aginfo));

        H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     HOFFSET(aginfo, m_ulCellID),   H5T_NATIVE_INT);
        H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    HOFFSET(aginfo, m_ulID),       H5T_NATIVE_LONG);
        
        hsize_t dims;
        H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        m_iNumAgents = dims;
        m_pInfos = new aginfo[dims];

        hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
        herr_t status = H5Dread(hDataSet, hAgentDataType, hMemSpace, hDataSpace, H5P_DEFAULT, m_pInfos);
        if (status >= 0) {
            xha_fprintf(stderr, "[loadAgentsCell] pop %s: %llu\n", sPopName, dims);
        } else {
            xha_fprintf(stderr, "[loadAgentsCell] bad status for pop %s\n", sPopName);

            delete[] m_pInfos;
	    m_pInfos = NULL;
            iResult = -1;
        }
        
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSet(hDataSet);
        qdf_closeGroup(hSpecies);
    //}

    qdf_closeGroup(hPopulation);
    qdf_closeFile(hFilePop);

    return iResult;
}


//----------------------------------------------------------------------------
// getCellAgentCounts
//
int ArrivalChecker::getCellAgentCounts() {
    int iResult = 0;
    int iNumThreads = omp_get_max_threads();

    // create arrays for threads
    int **pCountsT = new int*[iNumThreads];
    for (int i = 0; i < iNumThreads; i++) {
        pCountsT[i] = new int[m_iNumCells];
        memset(pCountsT[i], 0, m_iNumCells*sizeof(int));
    }
    m_iTotal = 0;
    int iTotal = 0;
    popcounts::iterator it;
        
    // create arrays for threads
    for (int i = 0; i < iNumThreads; i++) {
        memset(pCountsT[i], 0, m_iNumCells*sizeof(int));
    }

#pragma omp parallel for
    for (int i = 0; i < m_iNumAgents; i++) {
        int iCell = m_pInfos[i].m_ulCellID;
        pCountsT[omp_get_thread_num()][iCell]++;
    }
    
    memset(m_pCounts, 0, m_iNumCells*sizeof(int));

    // collate counts
#pragma omp parallel for
    for (uint j = 0; j < m_iNumCells; j++) {
        for (int i = 0; i < iNumThreads; i++) {
            m_pCounts[j] += pCountsT[i][j];
        }
    }

#pragma omp parallel for reduction(+:iTotal)
    for (uint j = 0; j < m_iNumCells; j++) {
        iTotal += m_pCounts[j];
    }

    m_iTotal = iTotal;
    // delete thread arrays
    for (int i = 0; i < iNumThreads; i++) {
        delete[] pCountsT[i];
    }
    delete[] pCountsT;

    return iResult;
}


//----------------------------------------------------------------------------
// getCountForCells
//
int ArrivalChecker::getCountsForCells(std::vector<celldist> &v) {
    int iNum = 0;
    for (uint i = 0; i < v.size(); i++) {
    	int iCellID = v[i].first;
	iNum += m_pCounts[iCellID];
    }
    return iNum;
}
