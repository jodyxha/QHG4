#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <omp.h>

#include "svn_revision.h" // for revision string
#include "types.h"
#include "colors.h"
#include "ParamReader.h"
#include "QDFUtils.h"


#include "L2List.h"
#include "GroupReader.h"
#include "SCell.h"
#include "SCellGrid.h"
#include "GridFactory.h"
#include "GridGroupReader.h"
#include "Geography.h"
#include "GeoGroupReader.h"
#include "Climate.h"
#include "ClimateGroupReader.h"
#include "Vegetation.h"
#include "VegGroupReader.h"
#include "Navigation.h"
#include "NavGroupReader.h"

#include "IcoGridNodes.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"

//@@@ Vegetation stuff needs rethink or remove
//#include "VegFactory.h"
#include "MessLoggerT.h"
#include "PopBase.h"
#include "PopReader.h"
#include "PopLooper.h"
#include "PopulationFactory.h"
#include "StatPopFactory.h"
#include "StatusWriter.h"
#include "IDGen.h"
#include "PopWriter.h"
#include "EventConsts.h"

#include "OccTracker.h"

#include "SimLoader.h"

// use random numbers from a table for default state
static unsigned int s_aulDefaultState[] = {
    0x2ef76080, 0x1bf121c5, 0xb222a768, 0x6c5d388b, 
    0xab99166e, 0x326c9f12, 0x3354197a, 0x7036b9a5, 
    0xb08c9e58, 0x3362d8d3, 0x037e5e95, 0x47a1ff2f, 
    0x740ebb34, 0xbf27ef0d, 0x70055204, 0xd24daa9a,
};


//-----------------------------------------------------------------------------
// constructor
//
SimLoader::SimLoader() 
    : m_iLayerSize(LAYERSIZE),
      m_hFile(H5P_DEFAULT),
      m_pLRGrid(NULL),
      m_pPR( new ParamReader()),       
      m_pCG(NULL),  
      m_pGeo(NULL), 
      m_pCli(NULL),
      m_pVeg(NULL),
      m_pNav(NULL),
      m_pPopLooper(NULL),
      m_apIDG(NULL),
      m_pSW(NULL),
      m_pSurface(NULL),
      m_pPopFac(NULL),
      m_pOcc(NULL) {
    
    m_sOutputQDF = "";
    m_sDesc      = "";
    m_sHelpTopic = "";
    m_vDataDirs.clear();
    m_bHelp = false;
    int iNumThreads =  omp_get_max_threads();
    // the IDGen are allocated and created (valid offset and step)
    // the real base and offset are set once the max id of all populations is known
    // (must be done here, sothey exust whne restores values are read fron populations)
    m_apIDG = new IDGen*[iNumThreads];
    for (int iT = 0; iT < iNumThreads; ++iT) {
        m_apIDG[iT] = new IDGen(0, iT, iNumThreads);
    }      
    
    // set deault state for WELLs
    memcpy(m_aulState, s_aulDefaultState, STATE_SIZE*sizeof(uint32_t));
    memset(m_aiSeeds, 0, NUM_SEEDS*sizeof(uint));

}


//-----------------------------------------------------------------------------
// destructor
//
SimLoader::~SimLoader() {
    if (m_pPR != NULL) {
        delete m_pPR;
    }
    if (m_pCG != NULL) {
        delete m_pCG;
    }
    if (m_pGeo != NULL) {
        delete m_pGeo;
    }
    if (m_pCli != NULL) {
        delete m_pCli;
    }
    if (m_pVeg != NULL) {
        delete m_pVeg;
    }
    if (m_pNav != NULL) {
        delete m_pNav;
    }
    if (m_pPopLooper != NULL) {
        delete m_pPopLooper;
    }
    if (m_pSW != NULL) {
        delete m_pSW;
    }
    if (m_pLRGrid != NULL) {
        delete m_pLRGrid;
    }
    if (m_apIDG != NULL) {
        for (int iT = 0; iT < omp_get_max_threads(); iT++) {
            delete m_apIDG[iT];
        }
        delete[]  m_apIDG;
    }

    if (m_pSurface != NULL) {
        delete m_pSurface;
    }
    if (m_pPopFac != NULL) {
        delete m_pPopFac;
    }

    qdf_closeFile(m_hFile);
    MessLogger::free();
}


//-----------------------------------------------------------------------------
// setHelp
//
int SimLoader::setHelp(bool bHelp) {
    m_bHelp = bHelp;
    return 0;
}


//-----------------------------------------------------------------------------
// setHelpTopic
//
int SimLoader::setHelpTopic(const std::string sHelpTopic) {
    m_sHelpTopic = sHelpTopic;
    return 0;
}


//-----------------------------------------------------------------------------
// helpParams
//
void SimLoader::helpParams() {
    stdprintf("  -h,                        show help\n");            
    stdprintf("  --help=<topic>             show help for topic (use name of option or \"all\")\n");            
    stdprintf("  --grid=<grid-file>         set grid file\n");     
    stdprintf("  --geo=<geo-file>           set geography file\n");       
    stdprintf("  --climate=<climate-file>   set climate file\n");    
    stdprintf("  --veg=<veg-file>           set vegetation file\n");    
    stdprintf("  --nav=<nav-file>           set navigation file\n");    
    stdprintf("  --pops=<pop-list>          set population files\n");       
    stdprintf("  --output-name=<name>       set output file na,e\n");
    stdprintf("  --data-dirs=<dirnames>     data directories (default: \"./\")\n");    
    stdprintf("  --info-string              information to be written to root group of output files\n");
    stdprintf("  --select=<desc>            string describing which groups t write\n");


}

//-----------------------------------------------------------------------------
// mergePops
//
int SimLoader::mergePops() { 
    return m_pPopLooper->tryMerge();
}


#define ERRINFO(x,y) std::string((x))+std::string((y))

//-----------------------------------------------------------------------------
// readOptions
//
int SimLoader::readOptions(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sHelpTopic[SHORT_INPUT];
    char sGridFile[MAX_PATH];
    char sPops[MAX_PATH];
    char sGeoFile[MAX_PATH];
    char sClimateFile[MAX_PATH];
    char sVegFile[MAX_PATH];
    char sNavFile[MAX_PATH];
    // this might be to big to place on the buffer
    //    char sEvents[2048*MAX_PATH];
    // better use a string allocated by ParamReader instead
    char sDataDirs[MAX_PATH];
    char sDummyLog[MAX_PATH];
    char sOutputQDF[MAX_PATH];
 
    *sHelpTopic    = '\0';
    *sGridFile     = '\0';
    *sPops         = '\0';
    *sGeoFile      = '\0';
    *sClimateFile  = '\0';
    *sVegFile      = '\0';
    *sNavFile      = '\0';
    *sOutputQDF    = '\0';
    *sDataDirs     = '\0';
    *sDummyLog     = '\0';
    m_pInfoString  = NULL;

    bool bHelp = false;
    m_bCalcGeoAngles = false;

    bool bOK = m_pPR->setOptions(14,
                                 "-h:0",                  &bHelp,
                                 "--help:s",              sHelpTopic,
                                 "--log-file:s",          sDummyLog,
                                 "--grid:s!",             sGridFile,
                                 "--geo:s",               sGeoFile,
                                 "--climate:s",           sClimateFile,
                                 "--veg:s",               sVegFile,
                                 "--nav:s",               sNavFile,
                                 "--pops:s",              sPops,
                                 "--output-name:s!",      sOutputQDF,
                                 "--data-dirs:s",         sDataDirs,
                                 "--calc-geoangles:0",   &m_bCalcGeoAngles,
                                 "--select:s!",           m_sDesc,
                                 "--info-string:S",      &m_pInfoString);
    if (bOK) {  
        // checkConfig(&iArgC, &apArgV);
        bool bIntermediateResult = true;
        iResult = m_pPR->getParams(iArgC, apArgV);
        if (*sHelpTopic != '\0') {
             setHelpTopic(sHelpTopic);
             stdprintf("HelpTopic [%s]\n", m_sHelpTopic);
             iResult = 2;
        } else  if (bHelp) {
            setHelp(bHelp);
            iResult = 3;
        } else {
            if (iResult >= 0) {
                if (iResult > 0) {
                    LOG_WARNING("ParamReader Warning:\n%s", m_pPR->getErrorMessage(iResult).c_str());
                    stdprintf("ParamReader Warning:\n%s", m_pPR->getErrorMessage(iResult).c_str());

                    iResult = 0;
                }
                stringvec vsOptions;
                m_pPR->collectOptions(vsOptions);
                LOG_DISP("%s (r%s) on %d threads called with\n", apArgV[0], REVISION, omp_get_max_threads());
                for (uint j = 0; j < vsOptions.size(); j++) {
                    LOG_DISP("  %s\n", vsOptions[j].c_str());
                }
                LOG_DISP("-----------------------------------------\n");
                LOG_DISP("replicate executable command:\n");
                LOG_DISP("  head -n %d %s | tail -n %d | sed 's/D://g' | gawk '{ print $1 }' | xargs\n", 4+vsOptions.size(), MessLogger::getFile(), 1+vsOptions.size());

                if (iResult > 0) {
                    stringvec vUnknown;
                    int iNum = m_pPR->getUnknownParams(vUnknown);
                    LOG_WARNING("[SimLoader::readOptions] %d unknown param%s:\n", iNum, (iNum>1)?"s":"");
                    for (int i = 0; i < iNum; i++) {
                        LOG_WARNING("  %s\n", vUnknown[i].c_str());
                    }
                }
                
           
            
                int iRes2=0;
                // m_pPR->display();
                stringvec vsErrorInfo;

                if (*sDataDirs != '\0') {
                    iRes2   = setDataDirs(sDataDirs);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setDataDirs");
                    }
                }
                if (bIntermediateResult) stdprintf("After setDataDir %d\n", iResult);
                
                if (*sGridFile != '\0') {
                    iRes2   = setGrid(sGridFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setGrid");
                    }
                }
                if (bIntermediateResult) stdprintf("After setGrid %d\n", iResult);

                if (*sGeoFile != '\0') {
                    iRes2   = setGeo(sGeoFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setGeo");
                    }
                }
                if (bIntermediateResult) stdprintf("After setGeo %d\n", iResult);

                if (*sClimateFile != '\0') {
                    iRes2   = setClimate(sClimateFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setClimate");
                    }
                }
                if (bIntermediateResult) stdprintf("After setClimate %d\n", iResult);

                if (*sVegFile != '\0') {
                    iRes2   = setVeg(sVegFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setVeg");
                    }
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) stdprintf("After setVeg %d\n", iResult);

                if (*sNavFile != '\0') {
                    iRes2   = setNav(sNavFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setNav");
                    }
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) stdprintf("After setNav %d\n", iResult);

                if (*sPops != '\0') {
                    iRes2   = setPopList(sPops);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setPopList");
                    }
                }
                if (bIntermediateResult) stdprintf("After setPopList %d\n", iResult);



                if (*sOutputQDF != '\0') {
                    iRes2   = setOutputQDF(sOutputQDF);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setOutputDir");
                    }
                }
                if (bIntermediateResult) stdprintf("After seOutputQDF %d\n", iResult);


                if (iResult != 0) {
                    stdprintf("[SimLoader::readOptions] Errors in the following method%s:\n", (vsErrorInfo.size()!=1)?"s":"");
                    LOG_ERROR("[SimLoader::readOptions] Errors in the following method%s:\n", (vsErrorInfo.size()!=1)?"s":"");
                    for (uint i = 0; i < vsErrorInfo.size(); i++) {
                        LOG_ERROR("  %s\n", vsErrorInfo[i].c_str());
                        stdprintf("  %s\n", vsErrorInfo[i].c_str());
                    }
                }
            } else  {
                if (iResult == PARAMREADER_ERR_MISSING_PARAM) {
                    LOG_ERROR("[SimLoader::readOptions] Missing parameter for option [%s]\n", m_pPR->getBadArg().c_str());
                } else if (iResult == PARAMREADER_ERR_OPTION_SET) {
                    LOG_ERROR("[SimLoader::readOptions] Error setting option [%s] to [%s]\n", m_pPR->getBadArg().c_str(), m_pPR->getBadVal().c_str());
                } else {
                    iResult = -2;
                    std::string sTempErr = stdsprintf("[SimLoader::readOptions] Missing mandatory params. Required:\n");
                    LOG_ERROR("[SimLoader::readOptions] Missing mandatory params. Required:\n");
                    
                    stringvec vMand;
                    int iNum = m_pPR->getMandatoryParams(vMand);
                    for (int i = 0; i < iNum; i++) {
                        LOG_ERROR("  %s\n", vMand[i]);
                        sTempErr += vMand[i];
                    }
                }
            }
        }
    } else {
        LOG_ERROR("[SimLoader::readOptions] Error setting options\n");
        iResult = -3;
    }
    
    //    LOG_STATUS("****************  readOptions exited with %d\n", iResult);
    
    if (iResult == 0) {
        LOG_STATUS("Succesfully read params\n");
        LOG_DISP("-------------------------------------\n");
        

        /*
        stdprintf("Random seed:\n");
        LOG_STATUS("Random seed:\n");
        
        for (uint i = 0; i < STATE_SIZE/4; i++) {
            std::string sState = "";
            for (uint j = 0; j < 4; j++) {
                sState += stdsprintf(" %08x", m_aulState[4*i+j]);
            }
            LOG_STATUS("    %s\n", sState);
            stdprintf("    %s\n", sState);
        }
        stdprintf("\n");
        */
        stdprintf("Layer Size: %d\n", m_iLayerSize);
        LOG_STATUS("Layer Size: %d\n", m_iLayerSize);
        
    } else if ((iResult != 2) && (iResult != 3)) {
        /* done in the destructor
           MessLogger::showLog(SHOW_ERROR | SHOW_WARNING);
        */
        iResult = -1;
    
    }

    fflush(stdout);
    return iResult;
   
}


//----------------------------------------------------------------------------
// showInputs
//  write output
//
void SimLoader::showInputs() {
    stdprintf("--------Inputs--------\n"); 
    if (m_pCG != NULL) {
        stdprintf("Grid %d cells\n", m_pCG->m_iNumCells);
    }
    if (m_pGeo != NULL) {
        stdprintf("Geography\n");
    }
    if (m_pCli != NULL) {
        stdprintf("Climate\n");
    }
    if (m_pVeg != NULL) {
        stdprintf("Vegetation\n");
    }
    if (m_pPopLooper != NULL) {
        stdprintf("Populations: %zd\n   ", m_pPopLooper->getNumPops());
        popmap::const_iterator it_pop;
        for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
            stdprintf("%s  ", it_pop->second->getSpeciesName());
        }
        stdprintf("\n");
    }
    stdprintf("----------------------\n"); 
}


//----------------------------------------------------------------------------
// exists
//  if pFile exists in current directory, 
//    true is returned and pExists is set to pFile
//  otherwise, if data dirs are given, and pFile exists in a data dir,
//    true is returned and pExists is set to DataDirX/pFile, 
//    where DataDirX is the first directory in the list containg 
//    a file named pFile 
//  otherwise,
//    false is returned  
//
bool SimLoader::exists(const std::string sFile, std::string &sExists) {
    struct stat statbuf;
    
    int iResult = stat(sFile.c_str(), &statbuf);
    //    stdprintf("Errore1: [%s]\n", strerror(iResult));
    if (iResult != 0)  {
        if (!m_vDataDirs.empty()) {
            sExists= "";
            for (uint i = 0; sExists.empty() && (i < m_vDataDirs.size()); i++) {
                std::string sTest = stdsprintf("%s%s", m_vDataDirs[i], sFile);
                iResult = stat(sTest.c_str(), &statbuf);
                if (iResult == 0) {
                    sExists = sTest;
                }
            }
        } else {
            // pFile doesn't exist, and we don't have a datadir
            // so that's it
        } 
    } else {
        sExists = sFile;
    }
    if (iResult == 0) {
        //        stdprintf("[exists] [%s] -> [%s]\n", sFile, sExists);
        //        LOG_STATUS("[exists] [%s] -> [%s]\n", sFile, sExists);
    } else {
        sExists = "";
        stdprintf("[exists] [%s] not found\n", sFile);
        LOG_STATUS("[exists] [%s] not found\n", sFile);
    }
    
    return (0 == iResult);
}


////////////////////////////////////////////////////////////////////////////
//// INPUT DATA HANDLING 
////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------
// setGrid
//  if File is QDF file, use GridGroupReader to create Grid
//  otherwise try the old way (def file)
//
int SimLoader::setGrid(const std::string sFile) {
    int iResult = -1;
    std::string sExistingFile;
    if (m_pCG == NULL) {
        if (exists(sFile, sExistingFile)) {
            m_hFile = qdf_openFile(sExistingFile);
            if (m_hFile > 0) {
                iResult = setGrid(m_hFile);
            } else {
                stdprintf("use old way\n");
                // do it the "old" way
                m_pLRGrid = LineReader_std::createInstance(sExistingFile, "rt");
                if (m_pLRGrid != NULL) {
                    iResult = setGridFromDefFile(sExistingFile);
                }
            }
            if (iResult == 0) {

                /* moved top setPops
                m_pPopFac = new PopulationFactory(m_pCG, m_iLayerSize, m_apIDG, m_aulState);
                */
            }
        } else {
            // err doesn't exist
            stdprintf("doesn't exist [%s]\n", sFile);
            LOG_ERROR("Gridfile [%s] doesn't exist\n", sFile);
        }
    } else {
        stdprintf("Can't have second gridfile [%s]\n", sFile);
        LOG_ERROR("Can't have second gridfile [%s]\n", sFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setGrid
//  from QDF file
//
int SimLoader::setGrid(hid_t hFile, bool bUpdate /* = false */) {
    int iResult = -1;
    GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
    if (pGR != NULL) {
        /*
        int iGridType = GRID_TYPE_NONE;
        int aiFormat[2];
        aiFormat[0] = -1;
        aiFormat[1] = -1;
        bool bPeriodic = false;
        */
        GridAttributes gridatt;
        std::string sTime =  qdf_extractSAttribute(hFile,  ROOT_STEP_NAME);
        if (sTime.empty() != 0) {
            stdprintf("Couldn't read time attribute from grid file\n");
            LOG_STATUS("Couldn't read time attribute from grid file\n");
            iResult = 0;
	}
        iResult = pGR->readAttributes(&gridatt);

        if (bUpdate) {
            if (m_pCG == NULL) {
                stdprintf("[setGrid] updating non-existent grid!!!\n");
                iResult = -1;
            }
            if ((uint)gridatt.m_iNumCells != m_pCG->m_iNumCells) {
                stdprintf("[setGrid] updating grid failed: different cell number!!!\n");
                iResult = -1;
            }
        }

        if (iResult == 0) {
            if (!bUpdate) {
                m_pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
                m_pCG->m_aCells = new SCell[gridatt.m_iNumCells];
            } else {
                stdprintf("[setGrid] updating grid...\n");
            }
            iResult = pGR->readData(m_pCG);
            if (iResult == 0) {
                /* moved to setPops
                if (!bUpdate) {
                    m_pPopLooper = new PopLooper(iNumCells);
                }
                */
                stdprintf("[setGrid] Grid read successfully: %p\n", m_pCG);
                LOG_STATUS("[setGrid] Grid read successfully: %p\n", m_pCG);
                int iRes = setGeo(hFile, false, bUpdate);
                if (iRes == 0) {
                    iRes = setClimate(hFile, false, bUpdate);
                    if (iRes == 0) {
                        iRes = setVeg(hFile, false, bUpdate);
                        if (iRes == 0) {

                            // ok                            
                        } else {
                            stdprintf("[setGrid] No Vegetation found in QDF\n");
                            LOG_STATUS("[setGrid] No Vegetation found in QDF\n");
                        }
                    } else {
                        stdprintf("[setGrid] No Climate found in QDF\n");
                        LOG_STATUS("[setGrid] No Climate found in QDF\n");
                    }
                } else {
                    stdprintf("[setGrid] No Geography found in QDF\n");
                    LOG_STATUS("[setGrid] No Geography found in QDF\n");
                }

                if (!bUpdate) {
                    iRes = setPops(hFile, NULL, false); // NULL: read all pops, false: not required
                    if (iRes == 0) {
                        // ok
                    } else {
                        stdprintf("[setGrid] No Populations found in QDF\n");
                        LOG_STATUS("[setGrid] No Populations found in QDF\n");
                    }
                }

                iRes = setNav(hFile, bUpdate);
                if (iRes == 0) {
                    // ok
                } else {
                    stdprintf("[setGrid] No Navigation found in QDF\n");
                    LOG_STATUS("[setGrid] No Navigation found in QDF\n");
                }
                
 
            } else {
                stdprintf("[setGrid] GridReader couldn't read data\n");
                LOG_ERROR("[setGrid] GridReader couldn't read data\n");
            }
        } else {
            stdprintf("[setGrid] GridReader couldn't read attributes\n");
            LOG_ERROR("[setGrid] GridReader couldn't read attributes\n");
        }
        delete pGR;
    } else {
        stdprintf("[setGrid] Couldn't create GridReader\n");
        LOG_ERROR("[setGrid] Couldn't create GridReader\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setGridFromDefFile
//  try the old way (def file)
//
int SimLoader::setGridFromDefFile(const std::string sFile) {
    int iResult = -1;
    
    GridFactory *pGF = new GridFactory(sFile);
    iResult = pGF->readDef();
    if (iResult == 0) {
        m_pCG = pGF->getCellGrid();
    // not here:    m_pPopLooper = new PopLooper(m_pCG->m_iNumCells);
    }
    delete pGF;
    return iResult;
}


//----------------------------------------------------------------------------
// setGeo
//  if File is QDF file, use GeoGroupReader to create Geography
//  otherwise try the old way (def file)
//
int SimLoader::setGeo(const std::string sFile) {
    int iResult = -1;
    std::string sExistingFile;
    if (exists(sFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(sExistingFile);
        if (hFile > 0) {
            iResult = setGeo(hFile, true);
        } else {
            // read from deffile
            iResult = setGeoFromDefFile(sExistingFile);
        }
    } else {
        // err doesn't exist
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setGeo
//  from QDF file
//
int SimLoader::setGeo(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
    int iResult = -1;
    
     GeoGroupReader *pGR = GeoGroupReader::createGeoGroupReader(hFile);
     if (pGR != NULL) {
         GeoAttributes geoatt;
         iResult = pGR->readAttributes(&geoatt);
         if (iResult == 0) {
             if (geoatt.m_iMaxNeighbors == (uint)m_pCG->m_iConnectivity) {
                 if (geoatt.m_iNumCells == (uint)m_pCG->m_iNumCells) {
                     if (!bUpdate) {
                         m_pGeo = new Geography(geoatt.m_iNumCells, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
                     } else {
                         m_pGeo = m_m_pCG->m_pGeography;
                     }
                     iResult = pGR->readData(m_pGeo);
                     if (iResult == 0) {
                         if (!bUpdate) {
                             m_pCG->setGeography(m_pGeo);
                         }
                         stdprintf("[setGeo] GeoReader readData succeeded - Geo: %p, CG: %p!\n", m_pGeo, m_pCG);
                         LOG_STATUS("[setGeo] GeoReader readData succeeded : %p!\n", m_pGeo);
                     } else {
                         stdprintf("[setGeo] Couldn't read data\n");
                         LOG_ERROR("[setGeo] Couldn't read data\n");
                     }
                 } else {
                     iResult = -2;
                     stdprintf("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, geoatt.m_iNumCells);
                     LOG_ERROR("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, geoatt.m_iNumCells);
                 }
             } else {
                 iResult = -3;
                 stdprintf("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
                 LOG_ERROR("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
             }
             
         } else {
             stdprintf("[setGeo] Couldn't read attributes\n");
             LOG_ERROR("[setGeo] Couldn't read attributes\n");
         }
         
         delete pGR;
     } else {
         stdprintf("[setGeo] Couldn't create GeoGroupReader: did not find group [%s]\n", GEOGROUP_NAME);
         if (bRequired) {
             LOG_ERROR("[setGeo] Couldn't create GeoGroupReader: did not find group [%s]\n", GEOGROUP_NAME);
         }
     }

    if (m_bCalcGeoAngles && (iResult == 0)) {
        m_pGeo->calcAngles(m_pCG);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setGeoFromDefFile
//  [try the old way (def file)] we don't do def files for geo anymore
//
int SimLoader::setGeoFromDefFile(const std::string sFile) {
    int iResult = -1;

    return iResult;
}


//----------------------------------------------------------------------------
// setClimate
//  if File is QDF file, use ClimateGroupReader to create Climate
//  otherwise try the old way (def file)
//
int SimLoader::setClimate(const std::string sFile) {
    int iResult = -1;
    
    std::string sExistingFile;
    if (exists(sFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(sExistingFile);
        if (hFile > 0) {
            iResult = setClimate(hFile, true);
        } else {
            // set from def file
            iResult = setClimateFromDefFile(sExistingFile);
        }
    } else {
        // err doesn't exist
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setClimate
//  from QDF file
//
int SimLoader::setClimate(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
    int iResult = -1;
    
    ClimateGroupReader *pCR = ClimateGroupReader::createClimateGroupReader(hFile);
    if (pCR != NULL) {
        ClimateAttributes climatt;
        iResult = pCR->readAttributes(&climatt);
        if (iResult == 0) {
            if (climatt.m_iNumCells == m_pCG->m_iNumCells) {

                if (!bUpdate) {
                    m_pCli = new Climate(climatt.m_iNumCells, climatt.m_iNumSeasons, m_pGeo);
                } else {
                    m_pCli = m_pCG->getClimate();
                }
                
                iResult = pCR->readData(m_pCli);
                if (iResult == 0) {
                    if (!bUpdate) {
                        m_pCG->setClimate(m_pCli);
                    }
                    stdprintf("[setClimate] ClimateReader readData succeeded : %p!\n", m_pCli);
                    LOG_STATUS("[setClimate] ClimateReader readData succeeded : %p!\n", m_pCli);
                } else {
                    stdprintf("[setClimate] Couldn't read climate data\n");
                    LOG_ERROR("[setClimate] Couldn't read climate data\n");
                    delete m_pCli;
                }
               
            } else {
                iResult = -2;
                stdprintf("[setClimate] Cell number mismatch: CG(%d) Cli(%d)\n", m_pCG->m_iNumCells, climatt.m_iNumCells);
                LOG_ERROR("[setClimate] Cell number mismatch: CG(%d) Cli(%d)\n", m_pCG->m_iNumCells, climatt.m_iNumCells);
            }
            
        } else {
            stdprintf("[setClimate] Couldn't read attributes\n");
            LOG_ERROR("[setClimate] Couldn't read attributes\n");
        }
        
        
        delete pCR;
    } else {
        stdprintf("[setClimate] Couldn't create ClimateGroupReader: did not find group [%s]\n", CLIGROUP_NAME);
        if (bRequired) {
            LOG_ERROR("[setClimate] Couldn't create ClimateGroupReader: did not find group [%s]\n", CLIGROUP_NAME);
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// setClimateFromDefFile
//  try the old way (def file)
//
int SimLoader::setClimateFromDefFile(const std::string sFile) {
    int iResult = -1;
    

    return iResult;
}


//----------------------------------------------------------------------------
// setVeg
//  if File is QDF file, use VegGroupReader to create Vegetation
//  otherwise try the old way (def file)
//
int SimLoader::setVeg(const std::string sFile) {
    int iResult = -1;
    
    std::string sExistingFile;
    if (exists(sFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(sFile);
        if (hFile > 0) {
            iResult = setVeg(hFile, true);
        } else {
            iResult = setVegFromDefFile(sFile);
        }
    } else {
        // err doesn't exist
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setVeg
//  use QDF file
//
int SimLoader::setVeg(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
    int iResult = -1;
    
    VegGroupReader *pVR = VegGroupReader::createVegGroupReader(hFile);
    if (pVR != NULL) {
        VegAttributes vegatt;
        iResult = pVR->readAttributes(&vegatt);
        if (iResult == 0) {
            //@@            stdprintf("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", vegatt.m_iNumVegSpc, vegatt.m_iNumCells);
            //@@            LOG_STATUS("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);

            if (vegatt.m_iNumCells == m_pCG->m_iNumCells) {
                if (!bUpdate) {
                    m_pVeg = new Vegetation(vegatt.m_iNumCells, vegatt.m_iNumVegSpc, m_m_pCG->m_pGeography, m_pCG->getClimate());
                } else {
                    m_pVeg = m_pCG->getVegetation();
                }
                iResult = pVR->readData(m_pVeg);
                if (iResult == 0) {
                    if (!bUpdate) {
                        m_pCG->setVegetation(m_pVeg);
                    }
                    stdprintf("[setVeg] VegReader readData succeeded!\n");
                    LOG_STATUS("[setVeg] VegReader readData succeeded!\n");
                } else {
                    stdprintf("[setVeg] Couldn't read data\n");
                    LOG_ERROR("[setVeg] Couldn't read data\n");
                }
            } else {
                iResult = -2;
                stdprintf("[setVeg] Cell number mismatch: CG(%d) Veg(%d)\n", m_pCG->m_iNumCells, vegatt.m_iNumCells);
                LOG_ERROR("[setVeg] Cell number mismatch: CG(%d) Veg(%d)\n", m_pCG->m_iNumCells, vegatt.m_iNumCells);
            }
        } else {
            stdprintf("[setVeg] Couldn't read attributes\n");
            LOG_ERROR("[setVeg] Couldn't read attributes\n");
        }


        delete pVR;
    } else {
        if (bRequired) {
            stdprintf("[setVeg] Couldn't create VegGroupReader: did not find group [%s]\n", VEGGROUP_NAME);
            LOG_ERROR("[setVeg] Couldn't create VegGroupReader: did not find group [%s]\n", VEGGROUP_NAME);
        }
     }

    
    return iResult;
}


//----------------------------------------------------------------------------
// setVegFromDefFile
//   try the old way (def file)
//
int SimLoader::setVegFromDefFile(const std::string sFile) {
    int iResult = -1;
    /*@@@ Vegetation stuff needs rethink or remove
    if (m_pCG != NULL) {
        VegFactory *pVF = new VegFactory(pFile, m_pCG->m_iNumCells,  m_m_pCG->m_pGeography, m_pCG->getClimate());
        iResult = pVF->readDef();
        if (iResult == 0) {
            m_pVeg =pVF->getVeg();
            m_pCG->setVegetation(m_pVeg);
        } else {
            delete m_pVeg;
            m_pVeg = NULL;
        }
        delete pVF;
    }
    @@@*/
    return iResult;
}


//----------------------------------------------------------------------------
// setNav
//  if File is QDF file, use NavGroupReader to create Navigation
//  otherwise fail
//
int SimLoader::setNav(const std::string sFile) {
    int iResult = -1;
    
    std::string sExistingFile;
    if (exists(sFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(sFile);
        if (hFile > 0) {
            iResult = setNav(hFile, true);
        }
	qdf_closeFile(hFile);
    } else {
        // err: doesn't exist
    }
    return iResult;
}



//----------------------------------------------------------------------------
// setNav
//  use QDF file
//
int SimLoader::setNav(hid_t hFile, bool bUpdate) {
    int iResult = -1;
    
    NavGroupReader *pNR = NavGroupReader::createNavGroupReader(hFile);
    if (pNR != NULL) {
        NavAttributes navatt;
        iResult = pNR->readAttributes(&navatt);
        if (iResult == 0) {
            //@@            stdprintf("[setNav] NavReader attributes read (numports: %d, numdests:%d, numdists:%d, sampledist:%f)\n", navatt.m_iNumPorts, navatt.m_iNumDests, navatt.m_iNumDists, navatt.m_dSampleDist);
            //@@            LOG_STATUS("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);
            
            if (!bUpdate) {
                m_pNav = new Navigation();
            } else {
                m_pNav = m_pCG->getNavigation();
            }

            
            iResult = pNR->readData(m_pNav);
            if (iResult == 0) {
                stdprintf("[setNav] NavGroupReader readData succeeded!\n");
                LOG_STATUS("[setNav] NavGroupReader readData succeeded!\n");
                iResult = pNR->readBridges(m_pNav);
                if (iResult == 0) {
                    stdprintf("[setNav] NavGroupReader readBridges succeeded!\n");
                    LOG_STATUS("[setNav] NavGroupReader readBridges succeeded!\n");
                    if (!bUpdate) {
                        m_pCG->setNavigation(m_pNav);
                    }
                } else {
                    stdprintf("[setNav] Couldn't read bridges\n");
                    LOG_ERROR("[setNav] Couldn't read bridges\n");
                }
            } else {
                stdprintf("[setNav] Couldn't read data\n");
                LOG_ERROR("[setNav] Couldn't read data\n");
            }
            
        } else {
            stdprintf("[setNav] Couldn't read attributes\n");
            LOG_ERROR("[setNav] Couldn't read attributes\n");
        }
        
        
        delete pNR;
    } else {
        stdprintf("[setNav] Couldn't create NavGroupReader: did not find group [%s]\n", NAVGROUP_NAME);
        LOG_WARNING("[setNav] Couldn't create NavGroupReader: did not find group [%s]\n", NAVGROUP_NAME);
    }
    
    
    return iResult;
}


//----------------------------------------------------------------------------
// setPoplist
//  assume comma-separated list of pop files
//
int SimLoader::setPopList(std::string sList) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sList, vParts, ",");
    for (uint i = 0; (i < iNum) && (iResult == 0); i++) {
        iResult = setPops(vParts[i]);
    }


    return iResult;
}


//----------------------------------------------------------------------------
// setPops
//  if file contains a ':' try <clsfile>:<datafile> (as in Pop2QDF)
//  otherwise try qdf and  use PopReader to create Populations
//
int SimLoader::setPops(const std::string sFile) {
    int iResult = -1;
    //    LOG_ERROR("[setPops] Looking at [%s]\n", pFile);
    
    // create poplooper and popfactory  if they don't exist
    // 
    if (m_pPopLooper == NULL) {
        m_pPopLooper = new PopLooper();
    }
    if (m_pPopFac == NULL) {
        m_pPopFac = new StatPopFactory(m_pCG, m_pPopLooper, m_iLayerSize, m_apIDG, m_aulState, m_aiSeeds);
    }
    
    stringvec vParts;
    uint iNum = splitString(sFile, vParts, ":");
    if (iNum == 2) {
        std::string sClsFile;
        std::string sDataFile;
        if (exists(vParts[0], sClsFile) && exists(vParts[1], sDataFile)) {
	    stdprintf("[setPops] trying [%s] as XML\n", sClsFile);
            iResult = setPopsFromXMLFile(sClsFile, sDataFile);
        } else {
            if (sClsFile.empty()) {
                LOG_ERROR("[setPops] File [%s] does not exist\n", vParts[0]);
            }
            if (sDataFile.empty()) {
                LOG_ERROR("[setPops] File [%s] does not exist\n", vParts[1]);
            }
        }
    } else {
        //@@        LOG_STATUS("[setPops] pop file [%s] must be qdf\n", sFile);
        std::string sExistingFile;
        if (exists(sFile, sExistingFile)) {
            hid_t hFile = qdf_openFile(sExistingFile);
            if (hFile > 0) {
                iResult = setPops(hFile, NULL, true);
            } else {
                LOG_ERROR("[setPops] COuldn't open [%s] as QDF file\n", sFile);
            }
        } else {
            // file does not exist
            LOG_ERROR("[setPops] File [%s] does not exist\n", sFile);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setPops
//  use QDF file
//  if pPopName is NULL: read all  populations in the QDF file,
//  otherwise read population named pPopName
//
int SimLoader::setPops(hid_t hFile, const std::string sPopName, bool bRequired) {
    int iResult = -1;
    PopReader *pPR = PopReader::create(hFile);
    if (pPR != NULL) {
        if (m_pPopFac != NULL) {
            const popinfolist &pil = pPR->getPopList();
            if (pil.size() > 0) {
                for (uint i = 0; i < pil.size(); i++) {
                    
                    if (!sPopName.empty() || (sPopName == pil[i].m_sClassName)) {
                        PopBase *pPop = m_pPopFac->createPopulationByName(pil[i].m_sClassName);
                        if (pPop != NULL) {
                            pPop->setAgentDataType();
                            stdprintf("[setPops] have pop [%s]\n", pil[i].m_sClassName);
                            LOG_STATUS("[setPops] have pop [%s]\n", pil[i].m_sClassName);
                            
                            iResult = pPR->read(pPop, pil[i].m_sSpeciesName, m_pCG->m_iNumCells, false); // false: don't resume
                            if (iResult == 0) {
                                
                                stdprintf("[setPops] have read pop\n");
                                LOG_STATUS("[setPops] have read pop\n");
                                iResult = m_pPopLooper->addPop(pPop);
                                if (iResult == 0) {
                                    stdprintf("[setPops] successfully added Population: %s: %ld agents\n", pPop->getSpeciesName(), pPop->getNumAgentsTotal());
                                    LOG_STATUS("[setPops] successfully added Population: %s: %ld agents\n", pPop->getSpeciesName(), pPop->getNumAgentsTotal());
                                    if (m_aiSeeds[0] > 0) {
                                        stdprintf("Randomizing(1) with %u\n", m_aiSeeds[0]);
                                        pPop->randomize(m_aiSeeds[0]);
                                    } else {
                                        stdprintf("No seeds???\n");
                                    }
                                    
                                } else {
                                    stdprintf("[setPops] Couldn't add population [%s]\n",  pil[i].m_sSpeciesName);
                                    LOG_ERROR("[setPops] Couldn't add population [%s]\n",  pil[i].m_sSpeciesName);
                                    
                                }
                            } else {
                                if (iResult == POP_READER_ERR_CELL_MISMATCH) {
                                    LOG_ERROR("[setPops] Cell number mismatch: CG(%d), pop[%s](%d)\n",  
                                              m_pCG->m_iNumCells, pil[i].m_sSpeciesName, pPop->getNumCells());
                                } else if (iResult == POP_READER_ERR_READ_SPECIES_DATA) {
                                    LOG_ERROR("[setPops] Couldn't read species data for [%s]\n",  pil[i].m_sSpeciesName);
                                } else if (iResult == POP_READER_ERR_NO_SPECIES_GROUP) { 
                                    LOG_ERROR("[setPops] No species group for [%s] found in QDF file\n",  pil[i].m_sSpeciesName);
                                } else if (iResult == POP_READER_ERR_NO_POP_GROUP) { 
                                    LOG_ERROR("[setPops] No pop group for [%s] found in QDF file\n",  pil[i].m_sSpeciesName);
                                }
                                delete pPop;
                                
                            }
                        } else {
                            stdprintf("[setPops] Couldn't create Population %s %s\n", 
                                   pil[i].m_sSpeciesName, pil[i].m_sClassName);
                            LOG_ERROR("[setPops] Couldn't create Population %s %s\n", 
                                      pil[i].m_sSpeciesName, pil[i].m_sClassName);
                        }
                    } else {
                        stdprintf("[setPops] No population name given\n");
                        LOG_ERROR("[setPops] No population name given\n");
                    }
                }
            } else {
                LOG_ERROR("[setPops] poplist size %zd\n", pil.size());
                iResult = -1;
            }

            delete pPR;
        } else {
            stdprintf("[setPops] no PopulationFactory (this should not happen!)\n");
            LOG_ERROR("[setPops] no PopulationFactory (this should not happen!)\n");
        }
    } else {
        if (bRequired) { 
            stdprintf("[setPops] Couldn't create PopReader:(\n");
            LOG_ERROR("[setPops] Couldn't create PopReader\n");
        }
    }
    
    return iResult;
}

/*
//----------------------------------------------------------------------------
// setPopsFromPopFile
//  read data from Pop file with real coordinates (lon, lat)
//  use GridInformation to build a surface to call findNode(lon, lat)  
//
int SimLoader::setPopsFromPopFile(const std::string sClsFile, const std::string sDataFile) {
    int iResult = 0;
    // if m_pSurface doesnot exist
    if (m_pSurface == NULL) {
        //    create m_pSurface from CellGrid info
        iResult = createSurface();
    }

    if (iResult == 0) {
        iResult = -1;
        if (m_pPopFac != NULL) {
            PopBase *pPop = m_pPopFac->readPopulation(sClsFile);
            if (pPop != NULL) {
                stdprintf("doing species [%s]\n", pPop->getSpeciesName());
                // read agent data
                iResult = readAgentData(pPop, sDataFile);
                if (iResult == 0) {
                    m_pPopLooper->addPop(pPop);
                    LOG_STATUS("[setPopsFromPopFile] successfully added Population: %s\n", pPop->getSpeciesName());
                    if (m_aiSeeds[0] > 0) {
                        stdprintf("Randomizing(1) with %u\n", m_aiSeeds[0]);
                        pPop->randomize(m_aiSeeds[0]);
                    } else {
                        stdprintf("No seeds???\n");        
                    }
                }
            } else {
                stdprintf("[setPopsFromPopFile] Couldn't create data from [%s]\n", pClsFile);
                LOG_ERROR("[setPopsFromPopFile] Couldn't create data from [%s]\n", pClsFile);
            }

        } else {
            stdprintf("[setPopsFromPopFile] no PopulationFactory (this should not happen!)\n");
            LOG_ERROR("[setPopsFromPopFile] no PopulationFactory (this should not happen!)\n");
        }

    } 

    // use  readAgentData from Pop2QDF to create population
    return iResult;
}
*/

//----------------------------------------------------------------------------
// setPopsFromXMLFile
//  read data from Pop file with real coordinates (lon, lat)
//  use GridInformation to build a surface to call findNode(lon, lat)  
//
int SimLoader::setPopsFromXMLFile(const std::string sXMLFile, const std::string sDataFile) {
    int iResult = 0;
    // if m_pSurface doesnot exist
    if (m_pSurface == NULL) {
        //    create m_pSurface from CellGrid info
        iResult = createSurface();
    }

    if (iResult == 0) {
        iResult = -1;
        if (m_pPopFac != NULL) {
            iResult = 0;
            ParamProvider2 *pPP = ParamProvider2::createInstance(sXMLFile);
            if (pPP != NULL) {
                //@@ stdprintf("ParamProvider:::::::::::\n");
                //@@ pPP->showTree();
                const stringvec &vClassNames = pPP->getClassNames();
                for (uint i = 0; (iResult == 0) && (i < vClassNames.size()); ++i){
                    iResult = pPP->selectClass(vClassNames[i].c_str());
                    PopBase *pPop = m_pPopFac->readPopulation(pPP);
                    if (pPop != NULL) {
                        pPop->setAgentDataType();

                        stdprintf("doing species [%s]\n", pPop->getSpeciesName());
                        // read agent data
                        iResult = readAgentData(pPop, sDataFile);
                        if (iResult == 0) {
                            m_pPopLooper->addPop(pPop);
                            LOG_STATUS("[setPopsFromXMLFile] successfully added Population: %s\n", pPop->getSpeciesName());
                            if (m_aiSeeds[0] > 0) {
                                stdprintf("Randomizing(1) with %u\n", m_aiSeeds[0]);
                                pPop->randomize(m_aiSeeds[0]);
                            } else {
                            stdprintf("No seeds???\n");        
                            }
                        }
                    } else {
                        stdprintf("[setPopsFromXMLFile] Couldn't create data from [%s]\n", sXMLFile);
                        LOG_ERROR("[setPopsFromXMLFile] Couldn't create data from [%s]\n", sXMLFile);
                    }
                    
                }
                delete pPP;
            } else {
                iResult = -1;
            }
        } else {
            stdprintf("[setPopsFromXMLFile] no PopulationFactory (this should not happen!)\n");
            LOG_ERROR("[setPopsFroXMLFile] no PopulationFactory (this should not happen!)\n");
        }

    } 

    // use  readAgentData from Pop2QDF to create population
    return iResult;
}



//-----------------------------------------------------------------------------
// setDataDirs
//
int SimLoader::setDataDirs(const std::string sDataDirs) {
    stringvec vParts;
    uint iNum = splitString(sDataDirs, vParts, ",;:");
    LOG_STATUS("[setDataDirs]Data directories:\n");
    for (uint i = 0; i < iNum; i++) {
        std::string s = vParts[i];
        if (s.back() != '/') {
            s += '/';
        }
        LOG_STATUS("[setDataDirs]  %s\n", s);
        m_vDataDirs.push_back(s);
    }
    return 0;
}


//-----------------------------------------------------------------------------
//  setOutputQDF
// 
int SimLoader::setOutputQDF(const std::string sOutputQDF) {
    int iResult = 0;

    m_sOutputQDF =  sOutputQDF;

    return iResult;
}



//-----------------------------------------------------------------------------
// createSurface
//
int SimLoader::createSurface() {
    int iResult = -1;

    if (m_pSurface == NULL) {
        if (m_pCG != NULL) {
            stringmap &smSurf = m_pCG->getSurfaceData();
            std::string sSurfType = smSurf["SURF_TYPE"];

            if (sSurfType == SURF_LATTICE) {
                int iLinks;
                std::string sLinks = smSurf[SURF_LTC_LINKS];
                if (strToNum(sLinks, &iLinks)) {
                    if ((iLinks == 4) || (iLinks == 6)) {
                        const std::string sProjT = smSurf[SURF_LTC_PROJ_TYPE];
                        const std::string sProjG = smSurf[SURF_LTC_PROJ_GRID];
                        if (!sProjT.empty() &&  !sProjG.empty()) {
                            Lattice *pLat = new Lattice();
                            iResult = pLat->create(iLinks, sProjT, sProjG);
                            m_pSurface = pLat;
                            if (iResult == 0) {
                                LOG_STATUS("[createSurface] Have lattice\n");
                            } else {
                                LOG_ERROR("[createSurface] cioulnd't create lattice\n");
                            }
                        } else {
                            LOG_ERROR("[createSurface] projection data incomplete:Type [%s], Grid[%s]\n", sProjT, sProjG);
                        }
                    } else {
                        LOG_ERROR("[createSurface] number of links must be 4 or 6 [%s]\n", sLinks);
                    }
                } else {
                    LOG_ERROR("[createSurface] number of links is not  a number [%s]\n", sLinks);
                }

            } else if (sSurfType == SURF_EQSAHEDRON) {
                int iSubDivs = -1;
                const std::string sSubDivs = smSurf[SURF_IEQ_SUBDIVS];
                if (strToNum(sSubDivs, &iSubDivs)) {
                    if (iSubDivs >= 0) {
                        EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, true);
                        if (pEQ != NULL) {
                            //@@ i think we don't need to relink for eqsahedrons
                            //pEQ->relink();
                            m_pSurface = pEQ;
                            iResult = 0;
                            LOG_STATUS("[createSurface] Have EQsahedron\n");
                        }
                    } else {
                        LOG_ERROR("[createSurface] subdivs must be positive [%s]\n", sSubDivs);
                    }
                } else {
                    LOG_ERROR("[createSurface] subdivs is not a number [%s]\n", sSubDivs);
                }

            } else if (sSurfType == SURF_ICOSAHEDRON) {
                int iSubLevel = -1;
                const std::string sSubLevel = smSurf[SURF_ICO_SUBLEVEL];
                if (strToNum(sSubLevel, &iSubLevel)) {
                    if (iSubLevel >= 0) {
                        Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
                        pIco->setStrict(true);
                        pIco->setPreSel(false);
                        pIco->subdivide(iSubLevel);
                        m_pSurface = pIco;
                        LOG_STATUS("[createSurface] Have Icosahedron\n");
                    } else {
                        LOG_ERROR("[createSurface] sublevels must be positive [%s]\n", sSubLevel);
                    }
                } else {
                    LOG_ERROR("[createSurface] subdivs is not a number [%s]\n", sSubLevel);
                }
                    
            } else {
                LOG_ERROR("[createSurface] unknown surface type [%s]\n", sSurfType);
            }
            
        } else {
            LOG_ERROR("[createSurface] can't create surface without CellGrid data\n");
        }
    }
    return iResult;
};


//-----------------------------------------------------------------------------
// readAgentData
//   we expect lines containing longitude and latitude followed by a colon ':'
//   and  specific agent data
//
int SimLoader::readAgentData(PopBase *pPop, const std::string sAgentDataFile) {
    int iResult = 0;
    
    LineReader *pLR = LineReader_std::createInstance(sAgentDataFile, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
            if (pLine != NULL) {

                double dLon;
                double dLat;
                //we don't use sscanf so we can handle different kinds of separators
                stringvec vParts;
                uint iNum = splitString(pLine, vParts, ",;:");
                if (iNum == 2) {
                    if (strToNum(vParts[0], &dLon)) {
                        if (strToNum(vParts[1], &dLat)) {
                            iResult = 0;
                        } else {
                            stdprintf("Not a valid number [%s]\n", vParts[1]);
                            LOG_ERROR("Not a valid number [%s]\n", vParts[1]);
                            iResult = -1;
                            }
                    } else {
                        stdprintf("Not a valid number [%s]\n", vParts[0]);
                        LOG_ERROR("Not a valid number [%s]\n", vParts[0]);
                        iResult = -1;
                    }
                } else {
                    stdprintf("Expected two double (lon, lat) [%s]\n", pLine);
                    LOG_ERROR("Expected two double (lon, lat) [%s]\n", pLine);
                    iResult = -1;
                }

                if (iResult == 0) {
                //                int iNum = sscanf(pLine, "%lf %lf", &dLon, &dLat);
                //                if (iNum == 2) {
                    gridtype lNode = m_pSurface->findNode(dLon, dLat);
                    if (lNode >= 0) {
                        //                        stdprintf("%f %f -> %d\n", dLon, dLat, lNode);
                        int iIndex = m_pCG->m_mIDIndexes[lNode];
                        iResult = pPop->addAgent(iIndex, pLine, false); //don't update counts
                        /*
                        char *pData = strchr(pLine, ':');
                        if (pData != NULL) {
                            *pData = '\0';
                            pData++;
                            // here we assume the index is equal to the ID
                            int iIndex = m_pCG->m_mIDIndexes[lNode];
                            iResult = pPop->addAgent(iIndex, pData, false); //don't update counts
                        } else {
                            stdprintf("[readAgentData] No colon ':' in line:[%s]\n", pLine);
                            LOG_ERROR("[readAgentData] No colon ':' in line:[%s]\n", pLine);
                            iResult = -1;
                        }
                        */
                    } else {
                        stdprintf("[readAgentData] coordinates (%f, %f) could not be mapped to a node\n", dLon ,dLat);
                        LOG_WARNING("[readAgentData] coordinates (%f, %f) could not be mapped to a node\n", dLon ,dLat);
                    }
                } else {
                    iResult = -1;
                    stdprintf("[readAgentData] Couldn't extract Lon, Lat from [%s]\n", pLine);
                    LOG_ERROR("[readAgentData] Couldn't extract Lon, Lat from [%s]\n", pLine);
                }
            } else {
                // we have reached the end of the file - nop error
            }
        }
        if (iResult == 0) {
            pPop->updateTotal();
            pPop->updateNumAgentsPerCell();
        }
        delete pLR;
    } else {
        stdprintf("[readAgentDataCouldn't open [%s] for reading\n", sAgentDataFile);
        LOG_ERROR("[readAgentData] Couldn't open [%s] for reading\n", sAgentDataFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// handleWriteEvent
//
int SimLoader::write() {
    int iResult = -1;
    std::string sTemp{""};
    std::string sPops{""};
    std::string sOther{""};
    std::vector<std::pair<std::string, popwrite_flags>> vSubs;
    
    stringvec vParts;
    uint iNum = splitString(m_sDesc, vParts, "+");
    in iWhat = WR_NONE;

    for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
        uint iWhat = WR_NONE;
        std::string sCur = vParts[i];
        std::string sSub;
        if (sCur.substr(EVENT_PARAM_WRITE_POP.size()) == EVENT_PARAM_WRITE_POP) {
            stringvec vSub;
            uint iNumSub = splitString(sCur, vSub, ":");
            if (iNumSub >= 2) {
                sSub = vSub[1];
            }
        } else if (sCur == EVENT_PARAM_WRITE_GRID) {
            iWhat |= WR_GRID;
            sOther += "S";
        } else if (sCur == EVENT_PARAM_WRITE_GEO) {
            iWhat |= WR_GEO;
            sOther += "G";
        } else if (sCur == EVENT_PARAM_WRITE_CLIMATE) {
            iWhat |= WR_CLI;
            sOther += "C";
        } else if (sCur == EVENT_PARAM_WRITE_VEG) {
            iWhat |= WR_VEG;
            sOther += "V";
        } else if (sCur == EVENT_PARAM_WRITE_NAV) {
            iWhat |= WR_NAV;
            sOther += "N";
        } else if (sCur == EVENT_PARAM_WRITE_ENV) {
            iWhat |= WR_ALL;
            sOther += "env";
        } else if (sCur == EVENT_PARAM_WRITE_OCC) {
            iWhat |= WR_OCC;
            sOther += "O";
        } else {
            iWhat = WR_NONE;
            iResult = -1;
            stdprintf("Unknown output type [%s] (%s)\n", sCur, m_sDesc);
            LOG_ERROR("Unknown output type [%s] (%s)\n", sCur, m_sDesc);
        }
        
        if  (iWhat != WR_NONE) {
            if (iResult == 0) {
                popwrite_flags iWS = popwrite_flags::PW_NONE;
                
                if  (!sSub.empty() && (iWhat >= WR_POP)) {
                    sPops += "_pop-";
                    size_t iPosSpecial = sSub.find_first_of("#%~*");
                    std::string sD = "_";

                    if (iPosSpecial != std::string::npos) {
                        size_t iStar = sSub.find_first_of('*', iPosSpecial);
                        if (iStar != std::string::npos) {
                            iWS = PW_ALL;
                            sD+= "PMA";
                        } else {
                            size_t iPos = sSub.find_first_of("#%~");
                            while (iPos != std::string::npos) {
                                char c = sSub.at(iPos);
                                switch(c) {
                                case '#':
                                    sD += "P";
                                    iWS |= popwrite_flags::PW_AGENTS_ONLY;
                                    break;
                                case '%':
                                    sD += "M";
                                    iWS |= popwrite_flags::PW_STATS_ONLY;
                                    break;
                                case '~':
                                    sD += "A";
                                    iWS |= popwrite_flags::PW_ADDITIONAL_ONLY;
                                    break;
                                }
                                iPos = sSub.find_first_of("#%~", iPos+1);
                            }
                        }
                    } else {
                        iWS = PW_ALL;
                        sD = "";
                    }
                        
                    sPops += sSub;
                    sPops += sD;
                    stdprintf("current sPops is [%s])\n", sPops);
                    stdprintf("pushing back (%s, %d)\n", sSub, iWS);

                    vSubs.push_back(std::pair<std::string, popwrite_flags>(sSub, iWS));
                }

                if (iWS == PW_NONE) {
                    iWS = PW_ALL;
                }
                    
            }
        }
    } 

    if (iResult == 0) {
        stdprintf("[Simulator::handleWriteEvent] vSubs has %zd elements\n", vSubs.size());
        for (uint i = 0; i < vSubs.size(); i++) {
            stdprintf("[Simulator::handleWriteEvent]  %s -> %d\n", vSubs[i].first.c_str(), vSubs[i].second);
        }
 
        iResult = writeState(iWhat);
    } 
    return iResult;
}


//----------------------------------------------------------------------------
// writeState
//  write output
//
int SimLoader::writeState(int iWhat) {
    int iResult = 0;
    
   
    double dStartW = omp_get_wtime();
    int iCurStep = 0;
    popvec vPops;
    std::vector<std::pair<std::string, int>> vSubs; 
    popmap::const_iterator it_pop;
    for (it_pop = m_pPopLooper->begin(); (iResult == 0) && (it_pop != m_pPopLooper->end()); ++it_pop) {
        it_pop->second->setQDFVersionOut(4); 
        vSubs.push_back(std::pair<std::string, int>(it_pop->second->getSpeciesName(), PW_ALL));
        vPops.push_back(it_pop->second);
    }
    m_pSW = StatusWriter::createInstance(m_pCG, vPops);


    iResult = m_pSW->write(m_sOutputQDF, iCurStep, 0, "converted", iWhat, vSubs, DUMP_MODE_NONE);

    double dEndW = omp_get_wtime();
    stdprintf("[%d] Writing of [%s] took %fs\n", iCurStep, m_sOutputQDF, dEndW - dStartW);
    LOG_WARNING("[%d] Writing of [%s] took %fs\n", iCurStep, m_sOutputQDF, dEndW - dStartW);
    
    if (iResult != 0) {
        stdprintf("StatusWriter [%s]\n", m_pSW->getError().c_str());
        if (iResult < 0) {
            LOG_ERROR("StatusWriter [%s]\n", m_pSW->getError().c_str());
        } else {
            LOG_WARNING("StatusWriter [%s]\n", m_pSW->getError().c_str());
        }
    }

    return iResult;
}



/*********************************
Help
*********************************/


//-----------------------------------------------------------------------------
// printHeaderLine
//
void printHeaderLine(uint iL, const char *pTopic) {

   
    std::string sDash{iL, '-'};
    std::string sDashi;
    if (*pTopic != '\0') {
        std::string sTemp1 = stdsprintf(" Help for topic: %s ",  pTopic);
        size_t i1 = sTemp1.size();
        std::string sTemp2 = stdsprintf(" Help for topic: %s%s%s%s ",  colors::BOLDBLUE, pTopic, colors::OFF, colors::BOLD);
        size_t i2 = sTemp2.size();
        size_t iPos = (iL-i1)/2;
        sDash.replace(iPos, i2, sTemp2, i2);
    }
    stdprintf("\n%s%s%s%s\n\n", colors::BOLD, sDash, sDashi, colors::OFF);
}


//-----------------------------------------------------------------------------
// showTopicHelp
//
void SimLoader::showTopicHelp(const std::string sTopic) {
    bool bAll = (sTopic == "all");
    bool bFound = false;

    uint iL = 80;
    //-- topic "help"
    if (bAll || (sTopic == "help")) {
        printHeaderLine(iL, "help");
        stdprintf("  %s--help=<topic>%s    show detailed help for specified topic\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    Use 'all' to see info for all options.\n");
        bFound = true;
    }

    //-- topic "log-file"
    if (bAll || (sTopic == "log-file")) {
        printHeaderLine(iL, "log-file");
        stdprintf("  %s--log-file=<filename>%s    set name of logfile (default: \"%s\")\n", colors::BOLDBLUE, colors::OFF, DEF_LOG_FILE);
        bFound = true;
    }

    //-- topic "grid"
    if (bAll || (sTopic == "grid")) {
        printHeaderLine(iL, "grid");
        stdprintf("  %s--grid=<grid-file>%s    set grid file (required option)\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    grid-file is a QDF file which may also contain geography, climate, \n");
        stdprintf("    and vegetation data.\n");
        stdprintf("    In this case the corresponding geo, cilmate, or veg options can be omitted.\n");
        bFound = true;
    }

    //-- topic "geo"
    if (bAll || (sTopic == "geo")) {
        printHeaderLine(iL, "geo");
        stdprintf("  %s--geo=<geo-file>%s    set geography file\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    geo-file is a QDF-file containing geography data.\n");
        stdprintf("    It must contain data for the same number of cells as the grid file\n");
        bFound = true;
    } 

    //-- topic "climate"
    if (bAll || (sTopic == "climate")) {
        printHeaderLine(iL, "climate");
        stdprintf("  %s--climate=<climate-file>%s    set climate file\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    climate-file is a QDF-file containing climate data.\n");
        stdprintf("    It must contain data for the same number of cells as the grid file\n");
        bFound = true;
    } 

    //-- topic "veg"
    if (bAll || (sTopic == "veg")) {
        printHeaderLine(iL, "veg");
        stdprintf("  %s--veg=<veg-file>%s    set vegetation file\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    veg-file is a QDF-file containing vegetation data.\n");
        stdprintf("    It must contain data for the same number of cells as the grid file\n");
        bFound = true;
    } 

    //-- topic "pops"
    if (bAll || (sTopic  == "pops")) {
        printHeaderLine(iL, "pops");
        stdprintf("  %s--pops=<pop-list>%s    set populations data\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    pop-list is a comma-separated list of QDF-files, each containing population data.\n");
        stdprintf("    It must contain data for the same number of cells as the grid file\n");
        stdprintf("    Alternatively you may use\n");
        stdprintf("      --pops=<cls-file>:<data-file>\n");
        stdprintf("    where cls-file is a class definition file,\n");
        stdprintf("    and data-file is a file containing agent data.\n");
        bFound = true;
    } 

    //-- topic "data-dirs"
    if (bAll || (sTopic == "data-dirs")) {
        printHeaderLine(iL, "data-dirs");
        stdprintf("  %s--data-dirs=<dir-names>%s    search-directories for input files (default: \"./\")\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    dir-names is a comma-separated list of directories.\n");
        stdprintf("    The order of the directores in the list defines the search order.\n");
        bFound = true;
    } 


    //-- topic "info-string"
    if (bAll || (sTopic == "info-string")) {
        printHeaderLine(iL, "info-string");
        stdprintf("  %s--info-string%s    <infostring>\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    Add  <infostring> as the value of the attribute \"info\" of the root group of every output qdf\n");
        bFound = true;
    }

    //-- unknown topic
    if (!bFound) {
        printHeaderLine(iL, "");
        stdprintf("  %s%s%s: %sUnknown topic%s\n", colors::BOLDBLUE, sTopic, colors::OFF, colors::RED, colors::OFF);
        stdprintf("    select an option from this list\n");
        helpParams();
    }

     printHeaderLine(iL, "");
}


