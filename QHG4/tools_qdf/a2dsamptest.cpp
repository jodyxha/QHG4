#include <cstdio>
#include <cmath>

#include "strutils.h"
#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"

#include "Sampling.h"
#include "SamplingT.cpp"
#include "CellSampling.h"
#include "FullSampling.h"
#include "RangeSampling.h"
#include "Agent2DataExtractor.h"
#include "HistoMaker.h"
#include "PieWriter.h"


//typedef std::map<int, std::pair<double, double> >    arrpos_coords;
typedef std::map<int, int *>                           maphistos;


//----------------------------------------------------------------------------
// show groups
//  get longitude and latitude arrays form the geogrid
//
void showGroups(groupedvals<double> &vG) {
    int i = 0;
    int n = 1+floor(log(vG.size())/log(10));
    groupedvals<double>::const_iterator it;
    for (it = vG.begin(); it != vG.end(); ++it) {
        printf("(%0*d) %03d: ", n, i++, it->first);
	for (unsigned int k = 0; k < it->second.size(); ++k) {
             xha_printf(" %f", it->second[k]);
	}
	xha_printf("\n");
    }
}


//----------------------------------------------------------------------------
// showHistos
//
void showHistos(maphistos &mH, int iNumBin) {
    maphistos::const_iterator it;
    for (it = mH.begin(); it != mH.end(); ++it) {
        xha_printf("(%d): ", it->first);
	for (int k = 0; k < iNumBin; ++k) {
             xha_printf(" %d", it->second[k]);
	}
	xha_printf("\n");
    }
}


//----------------------------------------------------------------------------
// fillCoordMap
//  get longitude and latitude arrays form the geogrid
//
double **fillCoordMap(const std::string sQDFGeoGrid, uint *piNumCells) {
    double **apCoords = NULL;
    int iResult = -1;
    uint iNumCells = 0;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
     
    QDFArray *pQA = QDFArray::create(sQDFGeoGrid);
    if (pQA != NULL) {

        // get the cell IDs
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            iNumCells = pQA->getSize();
            pCellIDs = new int[iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, iNumCells, GRID_DS_CELL_ID);
            if (iCount == iNumCells) {
                //                fprintf(stderr, "Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                xha_fprintf(stderr, "Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            xha_fprintf(stderr, "Couldn't open QDF array for [%s:%s/%s]\n", sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME);
        }

        // get the cell Longitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLon = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, iNumCells);
                    if (iCount == iNumCells) {
                        xha_fprintf(stderr, "Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE);
                }
                pQA->closeArray();
            }
        }

        // get the cell Latitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LATITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLat = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, iNumCells);
                    if (iCount == iNumCells) {
                        xha_fprintf(stderr, "Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "Couldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL,iNumCells);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        xha_fprintf(stderr, "Couldn't create QDFArray\n");
    }
     
    // put everything into a map CellID => (lon, lat)
    if (iResult == 0) {
        // save coordinate values
        apCoords = new double*[2];
        apCoords[0] = new double[iNumCells];
        apCoords[1] = new double[iNumCells];
        for (uint i = 0; i < iNumCells; i++) {
            apCoords[0][i] = pdLon[i];
            apCoords[1][i] = pdLat[i];
        }
        *piNumCells = iNumCells;
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

    return apCoords;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 2) {
        bool bList = true;
        std::string sFileName = apArgV[1];
        std::string sQDFFileName = apArgV[2];
        std::string sDSPath = "/Populations/sapiens/AgentDataSet";
        stringvec vItems;

        if (iArgC > 3) {
            for (int i = 3; i < iArgC;  i++) {
                vItems.push_back(apArgV[i]);
            }
            bList = false;
        }   
        
        Agent2DataExtractor *pADE = Agent2DataExtractor::createInstance(sFileName, sDSPath);
        if (pADE != NULL) {
            if (bList) {
                xha_printf("Members of compund data type\n");
                pADE->listDataType();
            } else {
                xha_printf("Extracting ");
                for (uint i = 0; i < vItems.size(); i++) {
                    xha_printf(" [%s]", vItems[i].c_str());
                }
                xha_printf("\n");

                struct_manager *pSM = pADE->extractVarV(vItems);
                if (pSM != NULL) {
                    int iNumItems = pADE->getNumItems();
                    std::vector<std::pair<int, double>> vIV;
                    pSM->makeIndexedVals(iNumItems, vIV);
                    
                    xha_printf("Got %zd values.\n", vIV.size());
                    
                    /*
                      for (uint i = 0; i < vIV.size(); i++) {
                      xha_printf("%d: %f\n", vIV[i].first, vIV[i].second);
                      }
                    */
                    uint iNumCells;

                    double **apCoords = fillCoordMap(sFileName, &iNumCells);
                    if (apCoords != NULL) {
                        xha_printf("Got %u cells.\n", iNumCells);
                        
                        if (iResult == 0) {
                            /*
                            cellrad pr {{ 0, 0.3}, { 1, 0.3}, { 2, 0.3}, { 3, 0.3}, 
                                         { 4, 0.3}, { 5, 0.3}, { 6, 0.3}, { 7, 0.3},
                                         { 8, 0.3}, { 9, 0.3}, {10, 0.3}, {11, 0.3}};
                            */
                            cellrad pr {{ 1, 0.1}, {30, 0.1}, { 175, 0.1}, { 303, 0.1}};

                            groupedvals<double> vG;
                            /*
                            CellSampling cs(iNumCells);
                            iResult = cs.groupValues(vIV, vG);
                            */
                            RangeSampling rs(iNumCells, pr, apCoords[0], apCoords[1], 1.0, true); 

                            iResult = rs.groupValues(vIV, vG);
                           
                            printf("CellSampling Samples (%zd):\n", vG.size()); 

                            //showGroups(vG);
                            
                            HistoMaker *pHM = new HistoMaker(vG);
                            
                            int iNumBins = 7;
                            int **pHistos = pHM->createHistos(0, 60, iNumBins);
                            
                            maphistos mH;

                            int i = 0;
                            typename groupedvals<double>::const_iterator it;
                            for (it = vG.begin(); it != vG.end(); ++it) {
                                mH[it->first] = pHistos[i];
                                i++;
                            }; 
                            printf("Histos:\n"); fflush(stdout);
                            showHistos(mH, iNumBins);
                            //                            PieWriter *pPW = PieWriter::createInstance(vItems[1], vG);
                            PieWriter *pPW = PieWriter::createInstance(vItems[1], mH, iNumBins);
                            if (pPW != NULL) {
                                int iResult = pPW->prepareData(iNumCells, apCoords[0], apCoords[1], true, 6371);
                                if (iResult == 0) {
                                    iResult = pPW->writeToQDF(sQDFFileName);
                                    if (iResult == 0) {
                                        xha_printf("+++ successfully written\n");
                                        
                                    } else {
                                        xha_printf("couldn't write data\n");
                                    }
                                } else {
                                    xha_printf("couldn't prepare data\n");
                                }
                                delete pPW;
                            } else {
                                xha_printf("couldn't create pie writer\n");
                            }
                            
                            
                            delete pHM; // will also delete pHistos
                            
                        }

                        
                        delete[] apCoords[0];
                        delete[] apCoords[1];
                        delete[] apCoords;
                    } else {
                        xha_printf("Couldn't extract coordinates from [%s]\n", sFileName);
                    }
                    delete pSM;
                } else {
                    xha_printf("Couldn't create struct manager\n");
                }
               
                
            }
            delete pADE;
        } else {
            xha_printf("Couldn't create AgentDataExtractorn");
        }
    } else {
        xha_printf("usage ; %s <qdf-file> [<item-name>]\n", apArgV[0]);
    }
    
}
