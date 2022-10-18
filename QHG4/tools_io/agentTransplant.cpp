#include <cstdio>

#include "strutils.h"
#include "SPopulation.h"
#include "SCellGrid.h"
#include "Geography.h"
#include "PopulationFactory.h"
#include "QDFUtils.h"
#include "PopReader.h"
#include "StatusWriter.h"
#include "PopWriter.h"
#include "GridGroupReader.h"
#include "GeoGroupReader.h"
#include "PopLooper.h"


// use random numbers from a table for default state
static unsigned int s_aulDefaultState[] = {
    0x2ef76080, 0x1bf121c5, 0xb222a768, 0x6c5d388b, 
    0xab99166e, 0x326c9f12, 0x3354197a, 0x7036b9a5, 
    0xb08c9e58, 0x3362d8d3, 0x037e5e95, 0x47a1ff2f, 
    0x740ebb34, 0xbf27ef0d, 0x70055204, 0xd24daa9a,
};

void usage(char *pApp) {
    printf("  %s <PosQDF>:<speciesPos> <RefQDF>:<speciesRef> <definitstring> <OutputQDF> \n", pApp);
}

//----------------------------------------------------------------------------
// setGeo
//  from QDF file
//
Geography *createGeo(hid_t hFile, SCellGrid *pCG) {
    int iResult = -1;
    Geography *pGeo = NULL;
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

    if (iResult < 0) {
        delete pGeo;
        pGeo = NULL;
    }

    return pGeo;
}

//----------------------------------------------------------------------------
// setGrid
//  from QDF file
//
SCellGrid *createGrid(char *pFile) {
    int iResult = -1;
    SCellGrid *pCG = NULL;
    hid_t hFile = qdf_openFile(pFile, false);
    if (hFile != H5P_DEFAULT) {

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
                int iStartStep = 0;
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
                    // ok
                    Geography *pGeo = createGeo(hFile, pCG);
                    if (pGeo != NULL) {
                        pCG->setGeography(pGeo);
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
    } else {
        printf("[setGrid] Couldn't open %s as QDF\n", pFile);
    }
    if (iResult < 0) {
        delete pCG;
        pCG = NULL;
    }
    return pCG;
}


PopBase *openPop(char *pInputQDF, PopulationFactory *pPopFac, SCellGrid *pCG) {
    int iResult = 0;
    PopBase *pPop0 = NULL;
    PopReader *pPR = PopReader::create(pInputQDF);
    if (pPR != NULL) {
        const popinfolist &pil = pPR->getPopList();
        if (pil.size() > 0) {
            for (uint i = 0; i < pil.size(); i++) {
                if (pPop0 == NULL) {
                    PopBase *pPop = pPopFac->createPopulation(pil[i].m_sClassName);
                    if (pPop != NULL) { 
                        pPop->setAgentDataType();
                        printf("[openPop] have pop [%s]\n", pil[i].m_sClassName);
                        
                        iResult = pPR->read(pPop, pil[i].m_sSpeciesName, pCG->m_iNumCells, false);
                        if (iResult == 0) {
                            pPop0 = pPop;
                            
                        } else {
                            if (iResult == POP_READER_ERR_CELL_MISMATCH) {
                                printf("[openPop] Cell number mismatch: CG(%d), pop[%s](%d)\n",  
                                       pCG->m_iNumCells, pil[i].m_sSpeciesName, pPop->getNumCells());
                            } else if (iResult == POP_READER_ERR_READ_SPECIES_DATA) {
                                printf("[openPop] Couldn't read species data for [%s]\n",  pil[i].m_sSpeciesName);
                            } else if (iResult == POP_READER_ERR_NO_SPECIES_GROUP) { 
                                printf("[openPop] No species group for [%s] found in QDF file\n",  pil[i].m_sSpeciesName);
                            } else if (iResult == POP_READER_ERR_NO_POP_GROUP) { 
                                printf("[openPop] No pop group for [%s] found in QDF file\n",  pil[i].m_sSpeciesName);
                            }
                            delete pPop;
                            
                        }
                    } else {
                        printf("[setPops] Couldn't create Population %s %s\n", 
                               pil[i].m_sSpeciesName, pil[i].m_sClassName);
                    }
                }
            }
        } else {
            printf("[openPop] poplist size %zd\n", pil.size());
            iResult = -1;
        }
    } else {
        printf("[openPops] Couldn't create PopReader:(\n");
        iResult = -1;
    }
    if (iResult != 0) {
        delete pPop0;
        pPop0 = NULL;
    }
    return pPop0;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    
    char* pPosQDF;
    char* pGridQDF;
    char* pSpeciesPos;
    char* pRefCls;
    char* pSpeciesOut;
    char* pOutputQDF;
    char* pDefInitString;

    if (iArgC > 5) {
        pPosQDF        = apArgV[1];
        pGridQDF       = apArgV[2];
        pRefCls        = apArgV[3];
        pDefInitString = apArgV[4];
        pOutputQDF     = apArgV[5];
        
        pSpeciesPos = strchr(pPosQDF, ':');
        if (pSpeciesPos != NULL) {
            *pSpeciesPos++ = '\0';
        }
        iResult = 0;
        int iNumThreads = omp_get_max_threads();
        int iLayerSize = 65536;
        IDGen **apIDGen;
        apIDGen = new IDGen*[iNumThreads];
        for (int iT = 0; iT < iNumThreads; ++iT) {
            apIDGen[iT] = new IDGen(0, iT, iNumThreads);
        } 
        uint32_t *aulState = new uint32_t[STATE_SIZE];
        memcpy(aulState, s_aulDefaultState, STATE_SIZE*sizeof(uint32_t));
        uint     aiSeeds[NUM_SEEDS];
        
        SCellGrid *pCG = createGrid(pGridQDF);
        PopLooper *pPopLooper = new PopLooper();
        
        PopulationFactory *pPopFac = new PopulationFactory(pCG, pPopLooper, iLayerSize, apIDGen, aulState, aiSeeds);
        
        printf("definit [%s]\n", pDefInitString);
        fflush(stdout);
        printf("+++++++++ creating PopPos *******\n");
        PopBase *pPopPos = openPop(pPosQDF, pPopFac, pCG);
        printf("+++++++++ creating PopRef *******\n");
        PopBase *pPopRef = pPopFac->readPopulation(pRefCls);
        printf("Posfile: %lu agents\n", pPopPos->getNumAgentsTotal());
        pPopPos->compactData();
        SPopulation<Agent> *pPopRefS = static_cast<SPopulation<Agent> *>(pPopRef);
        ulong iNumAgents = pPopPos->getNumAgentsTotal();
        printf("Posfile: %lu agents\n", iNumAgents);
        printf("Reffile: %lu agents\n", pPopRef->getNumAgentsTotal());
        int iStart = pPopRefS->reserveAgentSpace(iNumAgents);
        pPopRefS->setAgentDataType();
        if (iStart >=  0) {
            for (uint i = 0; (iResult == 0) && (i < iNumAgents); i++) {
                char *pCurDef = pDefInitString;
                Agent a;
                a.m_iLifeState = pPopPos->getAgentLifeState(i);
                a.m_iCellIndex   = pPopPos->getAgentCellIndex(i);
                a.m_ulID         = pPopPos->getAgentID(i);
                a.m_ulCellID     = pPopPos->getAgentCellID(i);
                a.m_fBirthTime   = pPopPos->getAgentBirthTime(i);
                a.m_iGender      = pPopPos->getAgentGender(i);
                
                int iCIdx = a.m_iCellIndex;
                pPopRefS->createAgentAtIndex(i,  iCIdx);
                iResult = pPopRefS->addAgentData(iCIdx, i, &pCurDef);
                if (iResult == 0) {
                    pPopRefS->setAgentBasic(i, &a);
                    //                    printf("Fir agent@%d: LS %u, CI %d(%d), AID %lu, bt %f, g%u\n", i, iLS, iCIdx, iCID, uID, fBT, uG);
                    
                } else {
                    printf("Couldn't add agent data\n");
                    iResult = -1;
                    
                }
            }
            if (iResult == 0) {
                pPopRefS->updateTotal();  

                printf("Reffile at end: %lu agents\n", pPopRefS->getNumAgentsTotal());
                
                std::vector<PopBase *> vPops;
                vPops.push_back(pPopRefS);
                std::vector<std::pair<std::string, popwrite_flags>> vSubs;
                vSubs.push_back(std::pair<std::string, popwrite_flags>(pPopRefS->getSpeciesName(),popwrite_flags::PW_ALL));
                StatusWriter *pSW = StatusWriter::createInstance(pCG, vPops);
                char sInfo[256];
                sprintf(sInfo, "transplanted from %s", pPosQDF);

                pSW->write(pOutputQDF,0, 0, sInfo, WR_POP, vSubs);
                delete pSW;
                
            } else {
                printf("--- fail ---\n");
            }

        } else {
            printf("Couldn't reserve space\n");
            iResult = -1;
        }
        
        
        // create N new agents wit default init string
        // overwrite lifestate, cellIndex, cellID, birthtime gender
        
        
        
        delete pPopPos;
        delete pPopRef;
        delete pCG;
        
        for (int iT = 0; iT < iNumThreads; ++iT) {
            delete apIDGen[iT];
        } 
        delete[] apIDGen;
        delete pPopLooper;
        delete[] aulState;
    } else {
        usage(apArgV[0]);
    }
    
    return iResult;
}
