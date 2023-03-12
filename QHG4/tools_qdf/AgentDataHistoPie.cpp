#include "strutils.h"
#include "stdstrutilsT.h"

#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"

#include "AgentDataHistoPie.h"

#include "Agent2DataExtractor.h"
#include "LineReader.h"
#include "Sampling.h"
#include "SamplingT.cpp"
#include "CellSampling.h"
#include "FullSampling.h"
#include "RangeSampling.h"
#include "HistoMaker.h"


//----------------------------------------------------------------------------
// createInstance
//
AgentDataHistoPie *AgentDataHistoPie::createInstance(std::string sQDFInputFile, std::string sPopName, std::string sDataItemName) {
    AgentDataHistoPie *pADHP = new AgentDataHistoPie(sQDFInputFile, sPopName, sDataItemName);
    
    int iResult = pADHP->init();
    if (iResult != 0) {
        delete pADHP;
        pADHP = NULL;
    }
    return pADHP;
}
    


//----------------------------------------------------------------------------
// destructor
//
AgentDataHistoPie::~AgentDataHistoPie() {
    if (m_apCoords != NULL) {
        if (m_apCoords[0] != NULL) {
            delete[] m_apCoords[0];
        }
        if (m_apCoords[1] != NULL) {
            delete[] m_apCoords[1];
        }
        delete[] m_apCoords;
    }

    maphistos::const_iterator it;
    for (it = m_mHistos.begin(); it != m_mHistos.end(); ++it) {
        delete[] it->second;
    }
}

//----------------------------------------------------------------------------
// constructor
//
AgentDataHistoPie::AgentDataHistoPie(std::string sQDFInputFile, std::string sPopName, std::string sDataItemName) 
    : m_sQDFInputFile(sQDFInputFile),
      m_sPopName(sPopName),
      m_sDataItemName(sDataItemName),
      m_iNumBins(0),
      m_iNumCells(0),
      m_bSpherical(false),
      m_apCoords(NULL),
      m_bVerbose(false) {

}


//----------------------------------------------------------------------------
// init
//
int AgentDataHistoPie::init() {
    int iResult = 0;

    iResult = extractData();
    if (iResult == 0) {
        //stdprintf("getting coords\n");
        m_apCoords = fillCoordMap();
        if (m_apCoords != NULL) {
            if (m_bVerbose) stdprintf("Got %u cells with coords\n", m_iNumCells);
        }  else {
            if (m_bVerbose) stdprintf("Couldn't extract coordinates from [%s]\n", m_sQDFInputFile);
            iResult = -1;
        }    
    }
    return iResult;
}


//----------------------------------------------------------------------------
// extractData
//
int AgentDataHistoPie::extractData() {
    int iResult = -1;

    std::string sDSPath =  "/Populations/" + m_sPopName + "/AgentDataSet";
    Agent2DataExtractor *pADE = Agent2DataExtractor::createInstance(m_sQDFInputFile, sDSPath);
    if (pADE != NULL) {
       
        if (m_bVerbose) stdprintf("Extracting [%s]\n", m_sDataItemName);
        stringvec vItems;
        vItems.push_back("CellIdx");
        vItems.push_back(m_sDataItemName);
        /*
        // displaying items to be extracted
        for (uint i = 0; i < vItems.size(); i++) {
            stdprintf(" [%s]", vItems[i].c_str());
        }
        stdprintf("\n");
        */
        struct_manager *pSM = pADE->extractVarV(vItems);
        if (pSM != NULL) {
            int iNumItems = pADE->getNumItems();
            pSM->makeIndexedVals(iNumItems, m_vIndexedVals);
           
            if (m_bVerbose) stdprintf("Got %zd values.\n", m_vIndexedVals.size());
            delete pSM;
        } else {
            stdprintf("Couldn't create struct manager\n");
        } 
       
        iResult = 0;
        delete pADE;
    } else {
        stdprintf("Couldn't create AgentDataExtractor for [%s]\n", m_sQDFInputFile);
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//  get longitude and latitude arrays form the geogrid
//
double **AgentDataHistoPie::fillCoordMap() {
    double **apCoords = NULL;
    int iResult = -1;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
     
    std::string sType;
    iResult = qdf_getSurfType(m_sQDFInputFile, sType);
    if (iResult == 0) {
        m_bSpherical = (sType == GRID_STYPE_ICO) || (sType == GRID_STYPE_IEQ);
    }
    QDFArray *pQA = QDFArray::create(m_sQDFInputFile);
    if (pQA != NULL) {

        // get the cell IDs
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            m_iNumCells = pQA->getSize();
            pCellIDs = new int[m_iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, m_iNumCells, GRID_DS_CELL_ID);
            if (iCount == m_iNumCells) {
                //                fprintf(stderr, "Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                stdfprintf(stderr, "Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", m_sQDFInputFile, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, m_iNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            stdfprintf(stderr, "Couldn't open QDF array for [%s:%s/%s]\n", m_sQDFInputFile, GRIDGROUP_NAME, CELL_DATASET_NAME);
        }

        // get the cell Longitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == m_iNumCells) {
                    pdLon = new double[m_iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, m_iNumCells);
                    if (iCount == m_iNumCells) {
                        //stdfprintf(stderr, "Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, m_iNumCells);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, m_iNumCells, m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LONGITUDE);
                }
                pQA->closeArray();
            }
        }

        // get the cell Latitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LATITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == m_iNumCells) {
                    pdLat = new double[m_iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, m_iNumCells);
                    if (iCount == m_iNumCells) {
                        //stdfprintf(stderr, "Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "Couldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL, m_iNumCells);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, m_iNumCells, m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        stdfprintf(stderr, "Couldn't create QDFArray\n");
    }
     
    // put everything into a map CellID => (lon, lat)
    if (iResult == 0) {
        // save coordinate values
        apCoords = new double*[2];
        apCoords[0] = new double[m_iNumCells];
        apCoords[1] = new double[m_iNumCells];
        for (uint i = 0; i < m_iNumCells; i++) {
            apCoords[0][i] = pdLon[i];
            apCoords[1][i] = pdLat[i];
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

    return apCoords;
}


//----------------------------------------------------------------------------
// createSampling
//
int AgentDataHistoPie::createSampling(std::string sSamplingInfo)  {
    
    int iResult = 0;

    Sampling *pSamp = NULL;

    LineReader *pLR = LineReader_std::createInstance(sSamplingInfo, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        
        if (pLine != NULL) {
            std::string sType = trim(pLine);
            if (sType == "RangeSampling") {
                pointrad pr;
                pLine = pLR->getNextLine(GNL_IGNORE_ALL);
                while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
                    iResult = -1; 
                    stringvec vParts;
                    uint iNumParts = splitString(pLine, vParts, " ", false);
                    if (iNumParts == 2) {
                        iResult = 0;
                        int i0 = 0;
                        if (strToNum(vParts[0], &i0)) {
                            double d = 0.0;
                            if (strToNum(vParts[1], &d)) {
                                pr.push_back(std::pair<int, double>(i0, d));
                            } else {
                                stdprintf("Expected double but got [%s}\n", vParts[1]);
                                iResult = -1;
                            }
                            
                        } else {
                            stdprintf("Expected int but got [%s}\n", vParts[0]);
                            iResult = -1;
                        }
                    } else {
                        stdprintf("Need at least 2 numbers for a range sampling entry\n");
                    }
                    
                    pLine = pLR->getNextLine(GNL_IGNORE_ALL);
                }
                if (iResult ==0){
                    pSamp = new RangeSampling(m_iNumCells, pr, m_apCoords[0], m_apCoords[1], 1.0, m_bSpherical);
                }
            }  else if  (sType == "CellSampling") {
                pSamp = new CellSampling(m_iNumCells);
            }  else if  (sType == "FullSampling") {
                pSamp = new FullSampling(m_iNumCells);
            } else {
                stdprintf("Unknown sampling type [%s]\n", sType);
            }
        }
        
        if (pSamp != NULL) {
            pSamp->setVerbosity(m_bVerbose);
            iResult = pSamp->groupValues(m_vIndexedVals, m_vGroupedVals);
            delete pSamp;
        }

        delete pLR;
    }
    
    return iResult;       
}

//----------------------------------------------------------------------------
// splitBinInfo
//
int splitBinInfo(const std::string sBinInfo, double *pdMinVal, double *pdMaxVal, uint *piNumBins, bool *pbStrict) {
    int iResult = -1;

    stringvec vParts;
    uint iNumParts = splitString(sBinInfo, vParts, ":", true);
    if (iNumParts >= 3) {
        if(strToNum(vParts[0], pdMinVal)){
            if(strToNum(vParts[1], pdMaxVal)){
                if(strToNum(vParts[2], piNumBins)){
                    if (iNumParts == 4) {
                        if (vParts[3] == "!") {
                            *pbStrict = true;(vParts[3] == "!");
                            iResult = 0;
                        } else {
                            stdprintf("Expected '!' as last element [%s]\n", sBinInfo);
                            iResult = -1;
                        }
                    } else {
                        iResult = 0;
                    }
                } else {
                    stdprintf("Invalid value for numBins [%s}\n", vParts[2]);
                }
            } else {
                stdprintf("Invalid value for maxVal [%s}\n", vParts[1]);
            }
        } else {
            stdprintf("Invalid value for minVal [%s}\n", vParts[0]);
        }
    } else {
        stdprintf("Expected <minVal>:<maxVal>;<numBins> but got %d parts\n", iNumParts);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createHisto
//
int AgentDataHistoPie::createHisto(std::string sBinInfo){
    int iResult = 0;

    m_dMinVal    = 0;
    m_dMaxVal    = 0;
    m_iNumBins   = 0;
    bool bStrict = false;

    iResult = splitBinInfo(sBinInfo, &m_dMinVal, &m_dMaxVal, &m_iNumBins, &bStrict);

    if (iResult == 0) {
        if ((m_iNumBins > 0) &&(m_dMinVal < m_dMaxVal)){
        
            HistoMaker *pHM = new HistoMaker(m_vGroupedVals);
            pHM->setVerbosity(m_bVerbose);

            int **pHistos = pHM->createHistos(m_dMinVal, m_dMaxVal, m_iNumBins, bStrict);
                            
                                                       
            int i = 0;
            typename groupedvals<double>::const_iterator it;
            for (it = m_vGroupedVals.begin(); it != m_vGroupedVals.end(); ++it) {
                m_mHistos[it->first] = pHistos[i];
                i++;
            }; 
    
            if (m_bVerbose) {
                printf("Histos:\n"); fflush(stdout);
                pHM->showHistos(m_mHistos);
            }
            delete pHM; // this will not delete pHistos

        } else {
            stdprintf("minimum (%f) must be less than maximum (%f) and num bins (%u) mut be greater than 0\n", m_dMinVal, m_dMaxVal, m_iNumBins); 
            iResult = -1;
        }    
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writePie
//
int AgentDataHistoPie::writePie(const std::string sQDFOutputFile) {
    int iResult = -1;

    if (m_bVerbose) stdprintf("writing pie output");

    PieWriter *pPW = PieWriter::createInstance(m_sDataItemName, m_mHistos, m_iNumBins);
    if (pPW != NULL) {
        pPW->setVerbosity(m_bVerbose);
        iResult = pPW->prepareData(m_iNumCells, m_apCoords[0], m_apCoords[1], true, 6371);
        if (iResult == 0) {
            iResult = pPW->writeToQDF(sQDFOutputFile);
            if (iResult == 0) {
                if (m_bVerbose) stdprintf("+++ successfully written pie to [%s]\n", sQDFOutputFile);
                
            } else {
                stdprintf("couldn't write data\n");
            }
        } else {
            stdprintf("couldn't prepare data\n");
        }
        delete pPW;
    } else {
        stdprintf("couldn't create pie writer\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeText
//  simple text output: id [value]*
//
int AgentDataHistoPie::writeText(const std::string sTextOutputFile, bool bstd) {
    int iResult = 0;

    // determine number of digits of cell iDs and of values
    int iNumDigits = 1;
    int iNumDigitsVals = 1;

    maphistos::const_iterator it;
    for (it = m_mHistos.begin(); (iResult == 0) && (it != m_mHistos.end()); ++it) {
        int iN = 0;
        if (it->first > 0) {
            iN = 1+(int)(log(it->first)/log(10));
        } else if (it->first < 0) {
            iN = 2+(int)(log(fabs(it->first))/log(10));
        } 
        if (iN > iNumDigits) {
            iNumDigits = iN;
        }

        int iNV = 0;
        for (uint i = 0; i < m_iNumBins; i++) {
            int iVal = it->second[i];
            
            if (iVal > 0) {
                iNV = 1+(int)(log(iVal)/log(10));
            } else if (iVal < 0) {
                iNV = 2+(int)(log(fabs(iVal))/log(10));
            } 
            if (iNV > iNumDigitsVals) {
                iNumDigitsVals = iNV;
            }
        }
    }

    FILE *fOut = fopen(sTextOutputFile.c_str(), "wt");
    if (fOut != NULL) {

        for (it = m_mHistos.begin(); (iResult == 0) && (it != m_mHistos.end()); ++it) {
            std::string s0 = std::to_string(it->first);
            std::string s1(iNumDigits - s0.size(), '0');

            std::string sLine = s1 + s0;
        
            for (uint i = 0; i < m_iNumBins; i++) {
                
                std::string s0v = std::to_string(it->second[i]);
                std::string s1v(iNumDigitsVals - s0v.size(), ' ');

                sLine += " " + s1v + s0v;
            }
            stdfprintf(fOut, "%s\n", sLine);
            if (bstd) {
                stdprintf("%s\n", sLine);
            }

        }

        fclose(fOut);
        if (m_bVerbose) stdprintf("+++ successfully written histogram to [%s]\n", sTextOutputFile);
    } else {
        stdsprintf("Couldn't open [%s] for writing\n", sTextOutputFile);
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeCSV
//   csv file
//     ID;longitude(x);latitude(y):minval;maxval;numbins;item_0;....;itemN
//
int AgentDataHistoPie::writeCSV(const std::string sCSVOutputFile, bool bstd) {
    int iResult = 0;

    FILE *fOut = fopen(sCSVOutputFile.c_str(), "wt");
    if (fOut != NULL) {
        
        std::string sHeader = "ID";
        if (m_bSpherical) {
            sHeader += ";Longitude;Latitude";
        } else {
            sHeader += ";x;y";
        }
        sHeader += ";minval;maxval;numbins";
        int iNumDigits = 1+(int)(log(m_iNumBins)/log(10));

        for (uint i = 0; i < m_iNumBins; i++) {

            std::string s0 = std::to_string(i);
            std::string s1(iNumDigits - s0.size(), '0');
            sHeader += ";item_"+s1+s0;
        }
        stdfprintf(fOut, "%s\n", sHeader);
        if (bstd) {
            stdprintf("%s\n", sHeader);
        }

        maphistos::const_iterator it;
        for (it = m_mHistos.begin(); (iResult == 0) && (it != m_mHistos.end()); ++it) {
            int iCellID = it->first;
            if ((iCellID >= 0) && (iCellID < (int)m_iNumCells)) {
                double dLon = m_apCoords[0][iCellID];
                double dLat = m_apCoords[1][iCellID];
           
                std::string sLine = std::to_string(iCellID);
                sLine += ";" + std::to_string(dLon);
                sLine += ";" + std::to_string(dLat);
                sLine += ";" + std::to_string(m_dMinVal);
                sLine += ";" + std::to_string(m_dMaxVal);
                sLine += ";" + std::to_string(m_iNumBins);
                for (uint i = 0; i < m_iNumBins; i++) {
                    sLine += ";" + std::to_string(it->second[i]);
                }
                stdfprintf(fOut, "%s\n", sLine);
                if (bstd) {
                    stdprintf("%s\n", sLine);
                }

            } else {
                stdprintf("Invalid cellID [%d] should be in [0, %d]\n", iCellID, 0,  m_iNumCells-1);
                iResult = -1;
            }
        }

        fclose(fOut);
        stdprintf("+++ successfully written histogram to [%s]\n", sCSVOutputFile);
    } else {
        stdsprintf("Couldn't open [%s] for writing\n", sCSVOutputFile);
        iResult = -1;
    }

    return iResult;
}
