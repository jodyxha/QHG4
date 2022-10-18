#include <cstdio>
#include <cstring>

#include <hdf5.h>

#include "PopBase.h"
#include "SPopulation.h"

#include "types.h"
#include "strutils.h"
#include "WELL512.h"
#include "LineReader.h"
#include "PopLooper.h"
#include "PopulationFactory.h"
#include "IDGen.h"
#include "PopWriter.h"
#include "StatusWriter.h"
#include "GridGroupReader.h"
#include "GeoGroupReader.h"
#include "QDFUtils.h"
#include "ParamProvider2.h"

#define LAYER_SIZE 16777216
#define NUM_SEEDS  8

static unsigned int s_aulDefaultState[] = {
    0x2ef76080, 0x1bf121c5, 0xb222a768, 0x6c5d388b, 
    0xab99166e, 0x326c9f12, 0x3354197a, 0x7036b9a5, 
    0xb08c9e58, 0x3362d8d3, 0x037e5e95, 0x47a1ff2f, 
    0x740ebb34, 0xbf27ef0d, 0x70055204, 0xd24daa9a,
};

static const char *sInfoString = "created by PopFiller";

// Longitude;Latitude;LifeState;AgentID;BirthTime;Gender
#define NUM_TO_SKIP 6

//----------------------------------------------------------------------------
// readGeo
//  from QDF file
//
int readGeo(hid_t hFile, SCellGrid *pCG) {
    Geography *pGeo = NULL;
    int iResult = -1;
    
    GeoGroupReader *pGR = GeoGroupReader::createGeoGroupReader(hFile);
    if (pGR != NULL) {
        GeoAttributes geoatt;
        iResult = pGR->readAttributes(&geoatt);
        if (iResult == 0) {
            if (geoatt.m_iMaxNeighbors == (uint)pCG->m_iConnectivity) {
                if (geoatt.m_iNumCells == (uint)pCG->m_iNumCells) {
                    pGeo = new Geography(geoatt.m_iNumCells, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
                    iResult = pGR->readData(pGeo);
                    if (iResult == 0) {
                        pCG->setGeography(pGeo);
                        printf("[setGeo] GeoReader readData succeeded - Geo: %p, CG: %p!\n", pGeo, pCG);
                    } else {
                        printf("[setGeo] Couldn't read data\n");
                    }
                } else {
                    iResult = -2;
                    printf("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", pCG->m_iNumCells, geoatt.m_iNumCells);
                }
            } else {
                iResult = -3;
                printf("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
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
// readGeoGrid
//  from QDF file
//
SCellGrid *readGeoGrid(hid_t hFile) {
    SCellGrid *pCG = NULL;
    int iResult = -1;
    GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
    if (pGR != NULL) {
        GridAttributes gridatt;
        char sTime[32];
        // get the timestamp of the initial qdf file (grid)
        iResult = qdf_extractSAttribute(hFile,  ROOT_STEP_NAME, 31, sTime);
        if (iResult != 0) {
            printf("Couldn't read time attribute from grid file\n");
            iResult = 0;
        } else {
            int iStartStep=-1;
            if (strToNum(sTime, &iStartStep)) {
                iResult = 0;
                printf("Have timestamp %d\n", iStartStep);
            } else {
                printf("Timestamp not valid [%s]\n", sTime);
                iResult = -1;
            }
	}
        iResult = pGR->readAttributes(&gridatt);

        if (iResult == 0) {
            pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
            pCG->m_aCells = new SCell[gridatt.m_iNumCells];

            iResult = pGR->readData(pCG);
            if (iResult == 0) {
                printf("[setGrid] Grid read successfully: %p\n", pCG);
                iResult = readGeo(hFile, pCG);
                if (iResult == 0) {
                    // ok
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
    if ((iResult != 0) && (pCG != NULL)) {
        delete pCG;
        pCG = NULL;
    }
    return pCG;
}


//----------------------------------------------------------------------------
// fillNodes
//
int fillNodes(Geography *pGeo, PopBase *pPop, char *pData, int iNumAgentsPerNode) {
    int iResult = 0;
    int iC = 0;

    for (uint iCell = 0; iCell < pGeo->m_iNumCells; iCell++) {
        if (pGeo->m_adAltitude[iCell] > 0) {
            iC++; 
            for (int j = 0; (iResult == 0) && (j < iNumAgentsPerNode); j++) {
                char *pUseIt = pData;
                int iAgentIndex = pPop->getFreeIndex();
                pPop->createAgentAtIndex(iAgentIndex, iCell);
            
                iResult = pPop->addPopSpecificAgentData(iAgentIndex, &pUseIt);
            }
            pPop->updateTotal();
        }
    }
    printf("Seeded %d nodes, total %ld agents\n", iC, pPop->getNumAgentsEffective());
    return iResult;
}

//----------------------------------------------------------------------------
// getDatLine
//
char *getDatLine(char *pDatFile) {
    char *pLine = new char[1024];

    LineReader *pLR = LineReader_std::createInstance(pDatFile, "rt");
    if (pLR != NULL) {
        char *pData = pLR->getNextLine();
        if (pData != NULL) {
            int iResult = 0;
            char *pRest = pData;
         
            for (int i = 0; (iResult == 0) && (i < NUM_TO_SKIP); i++) {
                pRest = strpbrk(pRest, ";, ");
                if (pRest != NULL) {
                    pRest++;
                } else {
                    iResult = -1;
                    printf("Expected mor words in [%s]\n", pData);
                }
            }
            if (iResult == 0) {
                pLine = new char[strlen(pRest)+1];
                strcpy(pLine, pRest);
                printf("Found dat line [%s]\n", pLine);
            }
        } else {
            printf("Couldn't read line from [%s]\n", pDatFile);
        }
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pDatFile);
    }
    return pLine;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 5) {
        // XML + template dat, GeoQDF
        char *pGeoQDF  = apArgV[1];
        char *pXMLFile = apArgV[2];
        char *pDATFile = apArgV[3];
        char *pQDFOut  = apArgV[5];
        int iNumAgentsPerNode = 0;
        if (strToNum(apArgV[4], &iNumAgentsPerNode)) {

            hid_t hGeoGrid = qdf_openFile(pGeoQDF);
            if (hGeoGrid != H5P_DEFAULT) {
                SCellGrid *pCG = readGeoGrid(hGeoGrid);
                if (pCG != NULL) {
                    printf("Have grid with %d cells\n", pCG->m_iNumCells);
                    IDGen *pIDG = new IDGen(0, 0, 1);
                
                    uint      aiSeeds[NUM_SEEDS];
                    memset(aiSeeds, 0, NUM_SEEDS*sizeof(uint));
                    uint32_t  aulState[STATE_SIZE];
                    memcpy(aulState, s_aulDefaultState, STATE_SIZE*sizeof(uint32_t));
            
                    PopLooper *pPopLooper = new PopLooper();
            
                    PopulationFactory *pPopFac = new PopulationFactory(pCG, pPopLooper, LAYER_SIZE, &pIDG, aulState, aiSeeds);
                    if (pPopFac != NULL) {
                        iResult = 0;
                        
                        PopBase *pPop = NULL;
                        ParamProvider2 *pPP = ParamProvider2::createInstance(pXMLFile);
                        if (pPP != NULL) {
                            printf("ParamProvider:::::::::::\n");
                            pPP->showTree();
                            const stringvec &vClassNames = pPP->getClassNames();
                            // we will consider only the first class definition
                            iResult = pPP->selectClass(vClassNames[0]);
                            if (iResult == 0) {
                                pPop = pPopFac->readPopulation(pPP);
                                pPop->setAgentDataType();
                                popvec vPops;
                                vPops.push_back(pPop);

                                StatusWriter *pSW = StatusWriter::createInstance(pCG, vPops);

                                if (pPop != NULL) {
                                    char *pData = getDatLine(pDATFile);
                                    if (pData != NULL) {
                                        printf("pData:[%s]\n", pData);
                                        

                                        iResult = fillNodes(m_pCG->m_pGeography, pPop, pData, iNumAgentsPerNode);
                                        
                                        //int iWhat = WR_POP | WR_GRID | WR_GEO;
                                        int iWhat = WR_POP;
                                        std::vector<std::pair<std::string, int>> vSubs;
                                        vSubs.push_back(std::pair<std::string, int>(pPop->getSpeciesName(), PW_ALL));
                                        iResult = pSW->write(pQDFOut, 0, 0, sInfoString, iWhat, vSubs,  -1); // -1: DUMP_MODE_NONE);

                                        delete[] pData;
                                    }
                                    delete pSW;
                                    //delete pPop;
                                } else {
                                    printf("Couldn't create pop for [%s]\n", pXMLFile);
                                }
                            } else {
                                printf("ParamProvider couldn't select class [%s]\n", vClassNames[0].c_str());
                            }
                    
                        } else {
                            printf("Couldn't crate ParamProvider2\n");
                        }
                
                        delete pPopFac;
                        delete pPopLooper;
                    } else {
                        printf("Couldn't crate PoulationFactory\n");
                    }
            
                    delete pIDG;

                    delete m_pCG->m_pGeography;
                    delete pCG;
                } else {
            
                    printf("Couldn't GeoGrid from [%s]\n", pGeoQDF);
                }
                qdf_closeFile(hGeoGrid);
            } else {
                printf("Couldn't open [%s]\n", pGeoQDF);
            }
        } else {
            printf("Expected a number, not [%s]\n", apArgV[4]);
        }
    } else {
        printf("usage: %s <GeoQDF> <popXML> <popDat> <numnodespernode> <outQDF>\n", apArgV[0]);
    }
    return iResult;

}
