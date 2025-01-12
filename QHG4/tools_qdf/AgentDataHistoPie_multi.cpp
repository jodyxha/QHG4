#include <algorithm>
#include <string>

#include "strutils.h"
#include "xha_strutilsT.h"

#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"

#include "AgentDataHistoPie_multi.h"

#include "Agent2DataExtractor.h"
#include "LineReader.h"
#include "Sampling.h"
#include "SamplingT.cpp"
#include "FullSampling.h"
#include "EachSampling.h"
#include "CellRangeSampling.h"
#include "CoordRangeSampling.h"
#include "GridRangeSampling.h"
#include "HistoMaker_multi.h"


//----------------------------------------------------------------------------
// createInstance
//
AgentDataHistoPie_multi *AgentDataHistoPie_multi::createInstance(std::string sQDFInputFile, std::string sPopName, stringvec &vDataItemNames, bool bVerbose) {
    AgentDataHistoPie_multi *pADHP = new AgentDataHistoPie_multi(sQDFInputFile, sPopName, vDataItemNames, bVerbose);
    
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
AgentDataHistoPie_multi::~AgentDataHistoPie_multi() {
    if (m_apCoords != NULL) {
        if (m_apCoords[0] != NULL) {
            delete[] m_apCoords[0];
        }
        if (m_apCoords[1] != NULL) {
            delete[] m_apCoords[1];
        }
        delete[] m_apCoords;
    }

    multimaphistos::const_iterator it;
    for (it = m_mHistos.begin(); it != m_mHistos.end(); ++it) {
        maphistos::const_iterator ith;
        for (ith = it->second.begin(); ith != it->second.end(); ++ith) {
            delete[] ith->second;
        }
    }
}

//----------------------------------------------------------------------------
// constructor
//
AgentDataHistoPie_multi::AgentDataHistoPie_multi(std::string sQDFInputFile, std::string sPopName, stringvec &vDataItemNames, bool bVerbose) 
    : m_sQDFInputFile(sQDFInputFile),
      m_sPopName(sPopName),
      m_vDataItemNames(vDataItemNames),
      m_iNumBins(0),
      m_iNumCells(0),
      m_iNumDims(0),
      m_bSpherical(false),
      m_apCoords(NULL),
      m_bVerbose(bVerbose) {

}


//----------------------------------------------------------------------------
// init
//
int AgentDataHistoPie_multi::init() {
    int iResult = 0;

    iResult = extractData();
    if (iResult == 0) {
        //xha_printf("getting coords\n");
        m_apCoords = fillCoordMap(&m_iNumCells, &m_iNumDims, &m_bSpherical);
        if (m_apCoords != NULL) {
            if (m_bVerbose) xha_printf("Got %u cells with coords\n", m_iNumCells);
        }  else {
            if (m_bVerbose) xha_printf("Couldn't extract coordinates from [%s]\n", m_sQDFInputFile);
            iResult = -1;
        }    
    }
    return iResult;
}


//----------------------------------------------------------------------------
// extractData
//  get agent data items together with the cell id from the input file and
// save them in an indexedvals structure
//
int AgentDataHistoPie_multi::extractData() {
    int iResult = 0;


    for (uint i = 0; (iResult == 0) && (i < m_vDataItemNames.size()); i++) {
        std::string &sDataItemName = m_vDataItemNames[i];
      
        std::string sDSPath =  "/Populations/" + m_sPopName + "/AgentDataSet";
        Agent2DataExtractor *pADE = Agent2DataExtractor::createInstance(m_sQDFInputFile, sDSPath);
        if (pADE != NULL) {
            pADE->setVerbose(m_bVerbose);
            if (m_bVerbose) xha_printf("Extracting [%s]\n", sDataItemName);
            stringvec vItems;
            vItems.push_back("CellIdx");
            vItems.push_back(sDataItemName);
            /*
            // displaying items to be extracted
            for (uint i = 0; i < vItems.size(); i++) {
            xha_printf(" [%s]", vItems[i].c_str());
            }
            xha_printf("\n");
            */
            struct_manager *pSM = pADE->extractVarV(vItems);
            if (pSM != NULL) {
                int iNumItems = pADE->getNumItems();
                if (m_bVerbose) xha_printf("Extracted %d items.\n", iNumItems);
                indexedvals<double> vIndexedVals;
                pSM->makeIndexedVals(iNumItems, vIndexedVals);
                m_vIndexedVals[sDataItemName] = vIndexedVals;
                if (m_bVerbose) xha_printf("Got %zd indexed values.\n", vIndexedVals.size());
                delete pSM;
            } else {
                xha_printf("Couldn't create struct manager\n");
            } 
            
            iResult = 0;
            delete pADE;
        } else {
            xha_printf("Couldn't create AgentDataExtractor for [%s]\n", m_sQDFInputFile);
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//  get longitude and latitude arrays form the geogrid
//
double **AgentDataHistoPie_multi::fillCoordMap(uint *piNumCells, uint *piNumDims,  bool *pbSpherical) {
    double **apCoords = NULL;
    int iResult = -1;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
     
    std::string sType;
    iResult = qdf_getSurfType(m_sQDFInputFile, sType);
    if (iResult == 0) {
        *pbSpherical = (sType == GRID_STYPE_ICO) || (sType == GRID_STYPE_IEQ);
    }
    *piNumDims = *pbSpherical ? 3: 2; // ico grods require 3D pies, flat geometries need 2D pies

    QDFArray *pQA = QDFArray::create(m_sQDFInputFile);
    if (pQA != NULL) {

        // get the cell IDs
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            *piNumCells = pQA->getSize();
            pCellIDs = new int[*piNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, *piNumCells, GRID_DS_CELL_ID);
            if (iCount == *piNumCells) {
                //                fprintf(stderr, "Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                xha_fprintf(stderr, "Read bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)\n", m_sQDFInputFile, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, *piNumCells);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            xha_fprintf(stderr, "Couldn't open QDF array for [%s:%s/%s]\n", m_sQDFInputFile, GRIDGROUP_NAME, CELL_DATASET_NAME);
        }

        // get the cell Longitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == *piNumCells) {
                    pdLon = new double[*piNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, *piNumCells);
                    if (iCount == *piNumCells) {
                        //xha_fprintf(stderr, "Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "Read bad number of read longitudes from [%s:%s/%s]: %d instead of %d\n", m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, *piNumCells);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "Number of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, m_iNumCells, m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LONGITUDE);
                }
                pQA->closeArray();
            }
        }

        // get the cell Latitudes
        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LATITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == *piNumCells) {
                    pdLat = new double[*piNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, *piNumCells);
                    if (iCount == *piNumCells) {
                        //xha_fprintf(stderr, "Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        xha_fprintf(stderr, "Couldn't read latitudes from [%s:%s/%s]: %d instead of %d\n", m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL, *piNumCells);
                    }
                } else {
                    iResult = -1;
                    xha_fprintf(stderr, "Number of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]\n", iNumCellsL, *piNumCells, m_sQDFInputFile, GEOGROUP_NAME, GEO_DS_LATITUDE);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        xha_fprintf(stderr, "Couldn't create QDFArray\n");
    }
    
    // we assume that pdLon[i] and pdLat[i] are the longitude, respectively latitude, of cell number i
    // put everything into a map CellID => (lon, lat)
    if (iResult == 0) {
        // save coordinate values
        apCoords = new double*[2];
        apCoords[0] = new double[*piNumCells];
        apCoords[1] = new double[*piNumCells];
        for (uint i = 0; i < *piNumCells; i++) {
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
//   create a sampling object based on the data read from the file  sSamplingInfo
//
int AgentDataHistoPie_multi::createSampling(std::string sSamplingInfo)  {
    
    int iResult = 0;
    Sampling *pSamp = NULL;

    LineReader *pLR = LineReader_std::createInstance(sSamplingInfo, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        
        if (pLine != NULL) {
            std::string sType = trim(pLine);
            if (sType == SAMP_CELL_RANGE) {

                xha_printf("Using %s\n", SAMP_CELL_RANGE);
                pSamp = createCellRangeSampling(pLR);
                
            }  else if  (sType == SAMP_COORD_RANGE) {
                
                xha_printf("Using %s\n", SAMP_COORD_RANGE);
                pSamp = createCoordRangeSampling(pLR);
                
            }  else if  (sType == SAMP_FULL) {
                // FullSampling
                // no params needed - one group containing all cells
                
                xha_printf("Using %s\n", SAMP_FULL);
                pSamp = new FullSampling(m_iNumCells);

            }  else if  (sType == SAMP_EACH) {
                // EachSampling
                // no params needed - one group per cell
                
                xha_printf("Using %s\n", SAMP_EACH);
                pSamp = new EachSampling(m_iNumCells);

            }  else if  (sType == SAMP_GRID_RANGE) {
                // GridRangeSampling

                xha_printf("Using %s\n", SAMP_GRID_RANGE);
                pSamp = createGridRangeSampling(pLR);
                
            } else {
                xha_printf("Unknown sampling type [%s]\n", sType);
            }
        }
        delete pLR;
        
        multiindexedvals<double>::const_iterator it;
        for (it = m_vIndexedVals.begin(); (iResult == 0) && (it != m_vIndexedVals.end()); ++it) {
            if (pSamp != NULL) {
                pSamp->setVerbosity(m_bVerbose);
                indexedvals<double> vIndexedVals = it->second;
                groupedvals<double>  vGroupedVals;
                iResult = pSamp->groupValues(vIndexedVals, vGroupedVals);
                if (iResult == 0) {
                    m_vGroupedVals[it->first] = vGroupedVals;
                } else {
                    xha_printf("groupValues() failed\n");
                }
            }
        }
        //pSamp->showSamples();
        delete pSamp;

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
                            xha_printf("Expected '!' as last element [%s]\n", sBinInfo);
                            iResult = -1;
                        }
                    } else {
                        iResult = 0;
                    }
                } else {
                    xha_printf("Invalid value for numBins [%s}\n", vParts[2]);
                }
            } else {
                xha_printf("Invalid value for maxVal [%s}\n", vParts[1]);
            }
        } else {
            xha_printf("Invalid value for minVal [%s}\n", vParts[0]);
        }
    } else {
        xha_printf("Expected <minVal>:<maxVal>;<numBins> but got %d parts\n", iNumParts);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createHisto
//
int AgentDataHistoPie_multi::createHisto(std::string sBinInfo){
    int iResult = 0;

    m_dMinVal    = 0;
    m_dMaxVal    = 0;
    m_iNumBins   = 0;
    bool bStrict = false;

    iResult = splitBinInfo(sBinInfo, &m_dMinVal, &m_dMaxVal, &m_iNumBins, &bStrict);

    if (iResult == 0) {
        if ((m_iNumBins > 0) &&(m_dMinVal < m_dMaxVal)){
            
            multigroupedvals<double>::const_iterator it;
            for (it = m_vGroupedVals.begin(); (iResult == 0) && (it != m_vGroupedVals.end()); ++it) {
                const groupedvals<double> &vGroupedVals = it->second;
                HistoMaker_multi *pHM = new HistoMaker_multi(vGroupedVals);
                pHM->setVerbosity(m_bVerbose);

                int **pHistos = pHM->createHistos(m_dMinVal, m_dMaxVal, m_iNumBins, bStrict);
                            
                                                       
                int i = 0;
                maphistos mHistos;
                typename groupedvals<double>::const_iterator itg;
                for (itg = vGroupedVals.begin(); itg != vGroupedVals.end(); ++itg) {
                    mHistos[itg->first] = pHistos[i];  // itg->first is the reference cell
                    i++;
                }; 
                m_mHistos[it->first] = mHistos;        // it->first is the data item name

                if (m_bVerbose) {
                    printf("Histos:\n"); fflush(stdout);
                    pHM->showHistos(mHistos);
                }
                delete pHM; // this will not delete pHistos
            }
            printf("[AgentDataHistoPie_multi::createHisto] loop done\n");
        } else {
            xha_printf("minimum (%f) must be less than maximum (%f) and num bins (%u) mut be greater than 0\n", m_dMinVal, m_dMaxVal, m_iNumBins); 
            iResult = -1;
        }    
    }
    return iResult;
}

//------------------------------------ output methods ------------------------

int writeOutput(const std::string sWhat, const std::string sOutputFile, bool bstd) {
    int iResult = -1;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (sWhat == "txt>") {
        iResult = writeText(sQDFInputFile, bstd);
    } else if (sWhat == "csv") {
        iResult = writeCSV(sQDFInputFile, bstd);
    } else if (sWhat == "pie") {
        iResult = writePie(sQDFInputFile);
    } else {
        xha_printf("[AgentDataHistoPie_multi::writeOutput] Unknown output file [%s]\n", sWhat);
        iResult = -1:
    }
    return iResult;
}

//----------------------------------------------------------------------------
// writePie
//
int AgentDataHistoPie_multi::writePie(const std::string sQDFOutputFile) {
    int iResult = -1;

    if (m_bVerbose) xha_printf("writing pie output");

    //    PieWriter *pPW = PieWriter::createInstance(m_sDataItemName, m_mHistos, m_iNumBins, m_iNumDims);
    PieWriter_multi *pPW = PieWriter_multi::createInstance(m_mHistos, m_iNumBins, m_iNumDims);
    if (pPW != NULL) {
        pPW->setVerbosity(m_bVerbose);
        iResult = pPW->prepareData(m_iNumCells, m_apCoords[0], m_apCoords[1], m_bSpherical, 6371);
        if (iResult == 0) {
            iResult = pPW->writeToQDF(sQDFOutputFile);
            if (iResult == 0) {
                if (m_bVerbose) xha_printf("+++ successfully written pie to [%s]\n", sQDFOutputFile);
                
            } else {
                xha_printf("[AgentDataHistoPie_multi::writePie] Couldn't write data to qdf file [%s]\n", sQDFOutputFile);
            }
        } else {
            xha_printf("[AgentDataHistoPie_multi::writePie] Couldn't prepare data\n");
        }
        delete pPW;
    } else {
        xha_printf("[AgentDataHistoPie_multi::writePie] Couldn't create pie writer\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeText
//  simple text output: id [value]*
//
int AgentDataHistoPie_multi::writeText(const std::string sTextOutputFile, bool bstd) {
    int iResult = 0;

    uint iNameSize = 0;
    int iNumDigits = 1;
    int iNumDigitsVals = 1;
    printf("[AgentDataHistoPie_multi::writeText] determining column widths\n");
    multimaphistos::const_iterator it;
    for (it = m_mHistos.begin(); it != m_mHistos.end(); ++it) {
        const std::string &sDataItemName = it->first;
        const maphistos &mHistos = it->second;

        if (sDataItemName.size() > iNameSize) {
            iNameSize = sDataItemName.size();
        }
        iNameSize ++;
        // iNameSize determine number of digits of cell iDs and of values
 
        maphistos::const_iterator ith;
        for (ith = mHistos.begin(); (iResult == 0) && (ith != mHistos.end()); ++ith) {
            int iN = 0;
            if (ith->first > 0) {
                iN = 1+(int)(log(ith->first)/log(10));
            } else if (ith->first < 0) {
                iN = 2+(int)(log(fabs(ith->first))/log(10));
            } 
            if (iN > iNumDigits) {
                iNumDigits = iN;
            }
            
            int iNV = 0;
            for (uint i = 0; i < m_iNumBins; i++) {
                iNV = std::to_string(ith->second[i]).length();
                 
                /*
                int iVal = ith->second[i];
            
                if (iVal > 0) {
                    iNV = 1+(int)(log(iVal)/log(10));
                } else if (iVal < 0) {
                    iNV = 2+(int)(log(fabs(iVal))/log(10));
                } 
                */
                if (iNV > iNumDigitsVals) {
                    iNumDigitsVals = iNV;
                }
            }
        }
    }

    printf("[AgentDataHistoPie_multi::writeText] writing output file\n");
    FILE *fOut = fopen(sTextOutputFile.c_str(), "wt");
    if (fOut != NULL) {
        multimaphistos::const_iterator it;
        for (it = m_mHistos.begin(); it != m_mHistos.end(); ++it) {
            const maphistos &mHistos = it->second;
            std::string sOut = it->first;
            sOut.insert(sOut.end(), iNameSize - it->first.length(), ' ');
            maphistos::const_iterator ith;
            for (ith = mHistos.begin(); (iResult == 0) && (ith != mHistos.end()); ++ith) {

                std::string sN = std::to_string(ith->first);
                std::string s1(iNumDigits - sN.size(), '0');

                std::string sLine = sOut + s1 + sN;//std::to_string(ith->first);

                //std::string sLine = s1 + s0;
        
                for (uint i = 0; i < m_iNumBins; i++) {
                    
                    std::string s0v = std::to_string(ith->second[i]);
                    std::string s1v(iNumDigitsVals - s0v.size(), ' ');

                    sLine += " " + s1v + s0v;
                }
                xha_fprintf(fOut, "%s\n", sLine);
                if (bstd) {
                    xha_printf("%s\n", sLine);
                }
            }
        }
        fclose(fOut);
        if (m_bVerbose) xha_printf("+++ successfully written histograms to [%s]\n", sTextOutputFile);
    } else {
        xha_sprintf("Couldn't open [%s] for writing\n", sTextOutputFile);
        iResult = -1;
    }
    printf("[AgentDataHistoPie_multi::writeText] done\n");
    return iResult;
}


//----------------------------------------------------------------------------
// writeCSV
//   csv file
//     ID;longitude(x);latitude(y):minval;maxval;numbins;item_0;....;itemN
//
int AgentDataHistoPie_multi::writeCSV(const std::string sCSVOutputFile, bool bstd) {
    int iResult = 0;

    FILE *fOut = fopen(sCSVOutputFile.c_str(), "wt");
    if (fOut != NULL) {
        
        std::string sHeader = "DataItem;CellID";
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
        xha_fprintf(fOut, "%s\n", sHeader);
        if (bstd) {
            xha_printf("%s\n", sHeader);
        }

        multimaphistos::const_iterator it;
        for (it = m_mHistos.begin(); it != m_mHistos.end(); ++it) {
            const maphistos &mHistos = it->second;
            maphistos::const_iterator ith;
            for (ith = mHistos.begin(); (iResult == 0) && (ith != mHistos.end()); ++ith) {
                int iCellID = ith->first;
                if ((iCellID >= 0) && (iCellID < (int)m_iNumCells)) {
                    double dLon = m_apCoords[0][iCellID];
                    double dLat = m_apCoords[1][iCellID];
                    
                    std::string sLine = it->first;
                    sLine += ";" + std::to_string(iCellID);
                    sLine += ";" + std::to_string(dLon);
                    sLine += ";" + std::to_string(dLat);
                    sLine += ";" + std::to_string(m_dMinVal);
                    sLine += ";" + std::to_string(m_dMaxVal);
                    sLine += ";" + std::to_string(m_iNumBins);
                    for (uint i = 0; i < m_iNumBins; i++) {
                        sLine += ";" + std::to_string(ith->second[i]);
                    }
                    xha_fprintf(fOut, "%s\n", sLine);
                    if (bstd) {
                        xha_printf("%s\n", sLine);
                    }

                } else {
                    xha_printf("Invalid cellID [%d] should be in [0, %d]\n", iCellID, 0,  m_iNumCells-1);
                    iResult = -1;
                }
            }
        }

        fclose(fOut);
        xha_printf("+++ successfully written histogram to [%s]\n", sCSVOutputFile);
    } else {
        xha_sprintf("Couldn't open [%s] for writing\n", sCSVOutputFile);
        iResult = -1;
    }

    return iResult;
}



//------------------------------------ sampling creation utilities------------


//----------------------------------------------------------------------------
// createCellRangeSampling
//
//
//    expect lines of the form
//       <cellID> <range>
//
Sampling *AgentDataHistoPie_multi::createCellRangeSampling(LineReader *pLR) {
    // CellRangeSampling:
    // expect "<cellID> <range>
    int iResult = 0;
    Sampling *pSamp  =NULL; 

    cellrad cr;
    char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
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
                    cr.push_back(  std::pair<int, double>(i0, d));
                } else {
                    xha_printf("Expected double but got [%s}\n", vParts[1]);
                    iResult = -1;
                }
                
            } else {
                xha_printf("Expected int but got [%s}\n", vParts[0]);
                iResult = -1;
            }
        } else {
            xha_printf("Need at least 2 numbers for a cell range sampling entry\n");
        }
        
        pLine = pLR->getNextLine(GNL_IGNORE_ALL);
    }
    if (iResult ==0){
        pSamp = new CellRangeSampling(m_iNumCells, m_apCoords, 1.0, m_bSpherical);
        pSamp->setRangeDescription(&cr);
    }

    return pSamp;
}

//----------------------------------------------------------------------------
// createCoordRangeSampling
//
//
//    expect lines of the form
//       <coordX> <coordY> <range>
//
Sampling *AgentDataHistoPie_multi::createCoordRangeSampling(LineReader *pLR) {
    // CoordRangeSampling:
    // expect "<coordX> <coordY> <range>
    int iResult = 0;
    Sampling *pSamp  =NULL; 

    coordrad cr;
    char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
    while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
        iResult = -1; 
        stringvec vParts;
        uint iNumParts = splitString(pLine, vParts, " ", false);
        if (iNumParts == 3) {
            iResult = 0;
            double dX = 0;
            if (strToNum(vParts[0], &dX)) {
                double dY = 0;
                if (strToNum(vParts[1], &dY)) {
                    double d = 0.0;
                    if (strToNum(vParts[2], &d)) {
                        cr.push_back(coorddata(dX, dY, d));
                    } else {
                        xha_printf("Expected double but got [%s}\n", vParts[2]);
                        iResult = -1;
                    }
                } else {
                    xha_printf("Expected y-coord but got [%s}\n", vParts[1]);
                    iResult = -1;
                }
                
            } else {
                xha_printf("Expected x-cood but got [%s}\n", vParts[0]);
                iResult = -1;
            }
        } else {
            xha_printf("Need at least 3 numbers for a coord range sampling entry\n");
        }
        
        pLine = pLR->getNextLine(GNL_IGNORE_ALL);
    }
    if (iResult ==0){
        pSamp = new CoordRangeSampling(m_iNumCells, m_apCoords, 1.0, m_bSpherical);
        pSamp->setRangeDescription(&cr);
    }

    return pSamp;
}


//----------------------------------------------------------------------------
// createGridRangeSampling
//
//
//    expect lines of the form
//       <xmin> <min> <xstep> <ystep> <range>
//
Sampling *AgentDataHistoPie_multi::createGridRangeSampling(LineReader *pLR) {
    // GridRangeSampling
    // expect <xmin> <min> <xstep> <ystep> <range>
    int iResult = 0;
    Sampling *pSamp  =NULL; 

    xha_printf("Using grid range sampling\n");
    griddef gd;
    char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
    while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
        iResult = -1; 
        stringvec vParts;
        uint iNumParts = splitString(pLine, vParts, " ", false);
        if ((iNumParts == 7)|| (iNumParts == 4)) {
            iResult = 0;
            double dXMin = 0.0;
            if (strToNum(vParts[0], &dXMin)) {
                double dXMax = 0.0;
                if (strToNum(vParts[1], &dXMax)) {
                    double dStepX = 0.0;
                    if (strToNum(vParts[2], &dStepX)) {
                        double dYMin = 0;
                        double dYMax = 0;
                        double dStepY = 0;
                        double dRange = 0.0;
                        int iRangeIndex = 6;
                        if (iNumParts == 4) {
                            dYMin = dXMin;
                            dYMax = dXMax;
                            dStepY = dStepX;
                            iRangeIndex = 3;
                        } else {
                            
                            if (strToNum(vParts[3], &dYMin)) {
                                if (strToNum(vParts[4], &dYMax)) {
                                    if (strToNum(vParts[5], &dStepY)) {
                                        iResult = 0;
                                        
                                    } else {
                                        xha_printf("Expected stepY (double) but got [%s}\n", vParts[5]);
                                        iResult = -1;
                                    }
                                } else {
                                    xha_printf("Expected maxY (double) but got [%s}\n", vParts[4]);
                                    iResult = -1;
                                }
                            } else {
                                xha_printf("Expected minY (double) but got [%s}\n", vParts[3]);
                                iResult = -1;
                            }
                        }
                        if (iResult == 0) {
                            
                            if (strToNum(vParts[iRangeIndex], &dRange)) {
                                gd.push_back(griddata(dXMin, dXMax, dStepX, dYMin, dYMax, dStepY, dRange));
                            } else {
                                xha_printf("Expected range (double) but got [%s}\n", vParts[6]);
                                iResult = -1;
                            }
                        }
                    } else {
                        xha_printf("Expected stepX (double) but got [%s}\n", vParts[3]);
                        iResult = -1;
                    }
                } else {
                    xha_printf("Expected maxX (double) but got [%s}\n", vParts[1]);
                    iResult = -1;
                }
                
            } else {
                xha_printf("Expected minX (double) but got [%s}\n", vParts[0]);
                iResult = -1;
            }
        } else {
            xha_printf("Need at least 7 numbers for a grid range sampling entry\n");
        }
        
        pLine = pLR->getNextLine(GNL_IGNORE_ALL);
    }
    
    if (iResult == 0) {
        pSamp = new GridRangeSampling(m_iNumCells, m_apCoords, 1.0, m_bSpherical);
        pSamp->setRangeDescription(&gd);
    }
    return pSamp;
}
