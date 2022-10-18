#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <omp.h>

#include "svn_revision.h" // for revision string
#include "types.h"
#include "EventData.h"
#include "EventManager.h"
#include "EventConsts.h"
#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "colors.h"
#include "MessLoggerT.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "ParamReader.h"
#include "QDFUtils.h"



#include "GroupReader.h"
#include "SCell.h"
#include "SCellGrid.h"
#include "GridGroupReader.h"
#include "Geography.h"
#include "GeoGroupReader.h"
#include "Climate.h"
#include "ClimateGroupReader.h"
#include "Vegetation.h"
#include "VegGroupReader.h"
#include "Navigation.h"
#include "NavGroupReader.h"

#include "EnvInterpolator.h"
#include "AutoInterpolator.h"

#include "EventChecker.h"

#include "IcoGridNodes.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"

//@@@ Vegetation stuff needs rethink or remove
//#include "VegFactory.h"

#include "PopBase.h"
#include "PopLooper.h"
#include "PopReader.h"
#include "PopulationFactory.h"
#include "StatPopFactory.h"
#include "DynPopFactory.h"
#include "StatusWriter.h"
#include "IDGen.h"

#include "GridScrambler.h"

#include "SimParams.h"

#define DEF_LAYERSIZE 65536
#define MAX_LAYERSIZE 16777216
#define SEED_RANDOM "random"
#define SEED_SEQ    "seq:"
#define SEED_FILE   "file:"
#define SEED_PHRASE "phrase:"
#define SEED_HEADER "SEED"
#define SEED_FOOTER "SEED_END"


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
SimParams::SimParams() 
    : m_iNumIters(-1),
      m_iLayerSize(0),
      m_bHelp(false),
      m_hFile(H5P_DEFAULT),
      //@@ to be removed      m_pLRGrid(NULL),
      m_pPR(new ParamReader()),       
      m_pCG(NULL),  
      m_pGeo(NULL), 
      m_pCli(NULL),
      m_pVeg(NULL),
      m_pNav(NULL),
      m_pPopLooper(NULL),
      m_apIDG(NULL),
      m_pEnvInt(NULL),
      m_pAutInt(NULL),
      m_pSW(NULL),
      //      m_pEM(new EventManager()),
      m_pEM(NULL),
      m_pSurface(NULL),
      m_pPopFac(NULL),
      m_bZipOutput(false),
      m_bDeleteOrig(true),
      m_bResume(false),
      m_bMergePops(true),
      m_iStartStep(0),
      m_fStartTime(0),
      m_iInterpolStep(1),
      m_sHelpTopic(""),
      m_sOutputPrefix(DEF_OUT_PREFIX),
      m_sOutputDir("./"),
      m_sConfigOut(""),
      m_sInfoString(""),
      m_bTrackOcc(false),
      m_bDynPops(false),
      //@@      m_iGridSeed(-1),
      m_pGridScrambler(NULL) {

    m_vDataDirs.clear();
    m_vSODirs.clear();
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
    
    
}


//-----------------------------------------------------------------------------
// destructor
//
SimParams::~SimParams() {

    if (!m_bHelp && m_sHelpTopic.empty()) {
        LOG_DISP("-------------------------------------\n");
        MessLogger::showLog(MessLogger::SHOW_ALL);
    }
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
    if (m_pEnvInt != NULL) {
        delete m_pEnvInt;
    }
    if (m_pAutInt != NULL) {
        delete m_pAutInt;
    }
    if (m_pSW != NULL) {
        delete m_pSW;
    }
    if (m_pEM != NULL) {
        delete m_pEM;
    }
    //@@ to be removed
    /*
    if (m_pLRGrid != NULL) {
        delete m_pLRGrid;
    }
    */
    //@@ until here
    if (m_apIDG != NULL) {
        for (int iT = 0; iT < omp_get_max_threads(); iT++) {
            delete m_apIDG[iT];
        }
        delete[]  m_apIDG;
    }
    std::vector<eventdata*>::const_iterator it;
    for (it = m_vEvents.begin(); it != m_vEvents.end(); ++it) {
        delete *it;
    }
    

    if (m_pSurface != NULL) {
        delete m_pSurface;
    }
    if (m_pPopFac != NULL) {
        delete m_pPopFac;
    }
    if (m_pGridScrambler != NULL) {
        delete m_pGridScrambler;
    }

    qdf_closeFile(m_hFile);
}


//-----------------------------------------------------------------------------
// setHelp
//
int SimParams::setHelp(bool bHelp) {
    m_bHelp = bHelp;
    return 0;
}


//-----------------------------------------------------------------------------
// setHelpTopic
//
int SimParams::setHelpTopic(const std::string sHelpTopic) {
    m_sHelpTopic = sHelpTopic;
    return 0;
}


//-----------------------------------------------------------------------------
// helpParams
//
void SimParams::helpParams() {
    stdprintf("  -h,                        show this help\n");            
    stdprintf("  --help=<topic>             show deailed help for topic (use name of option or \"all\")\n");            
    stdprintf("  --log-file=<filename>      set name of log file (default:\"%s\")\n", DEF_LOG_FILE);            
    stdprintf("  --grid=<grid-file>         set grid file\n");     
    stdprintf("  --num-iters=<iters>        set number of iterations\n");  
    stdprintf("  --geo=<geo-file>           set geography file\n");       
    stdprintf("  --climate=<climate-file>   set climate file\n");    
    stdprintf("  --veg=<veg-file>           set vegetation file\n");    
    stdprintf("  --nav=<nav-file>           set navigation file\n");    
    stdprintf("  --pops=<pop-list>          set population files\n");       
    stdprintf("  --output-prefix=<name>     prefix for output files (default: \"%s\")\n", DEF_OUT_PREFIX);    
    stdprintf("  --output-dir=<dirname>     output directory (default: \"./\")\n");    
    stdprintf("  --data-dirs=<dirnames>     data directories (default: \"./\")\n");    
    stdprintf("  --read-config=<conf>       read config from file <conf>\n");
    stdprintf("  --write-config=<conf>      write config to file <conf>\n");
    stdprintf("  --events=<event-list>      set events\n");    
    stdprintf("  --start-time=<time>        set 'real' time for simulation step 0 (default: 0)\n");
    stdprintf("  --dyn-pops                 use populations from so director<y\n");    
    stdprintf("  --so-dirs=<dirnames>       search directories for population so-files (default: \"./\")\n");    
    stdprintf("  --interpol-step=<step>     set step size for interpolation (default: 1)\n");
    stdprintf("  --interpolation=<params>   set interpolation params\n");
    stdprintf("  --layer-size=<size>        set layer size (default: %d)\n", DEF_LAYERSIZE);
    stdprintf("  --shuffle=<num>            shift random generator's sequence (default: 0)\n");
    stdprintf("  --seed=<seedtype>          set seed for random number generators\n");
    stdprintf("  --pop-params=<paramstring> set special population parameters\n");
    stdprintf("  --zip-output               use gzip to zip all output qdf files\n");
    stdprintf("  --info-string=<infostring> information to be written to root group of output files\n");
    stdprintf("  --resume                   resume from previously dumped env and pop files\n");
    stdprintf("  --no-merge-pops            merge compatible populations (same IDs, same actions, same agent types)\n");
    stdprintf("  --dump-on-interrupt        set interrupt handler for Ctrl-C (SIG_INT): dump and exit\n");


}

#define ERRINFO(x,y) std::string((x))+std::string((y))

//-----------------------------------------------------------------------------
// readOptions
//
int SimParams::readOptions(int iArgC, char *apArgV[]) {
    int iResult = -1;

    std::string sHelpTopic("");
    std::string sGridFile("");
    std::string sPops("");
    std::string sGeoFile("");
    std::string sClimateFile("");
    std::string sVegFile("");
    std::string sNavFile("");
    // this might be to big to place on the buffer
    //    char sEvents[2048*MAX_PATH];
    // better use a string allocated by ParamReader instead
    char *pEvents;
    char *pInterpolationData = NULL;;
    std::string sOutputPrefix("");
    std::string sOutputDir("");
    std::string sDataDirs("");
    std::string sSODirs("");
    std::string sConfigIn(""); 
    std::string sConfigOut("");
    std::string sSeed("");
    std::string sDummyLog("");
    std::string sPopParams("");
    std::string sShuffles("");

    uint iLayerSize = DEF_LAYERSIZE;
 
    //    *sEvents       = '\0';
    pEvents = NULL;

    char *pInfoString   = NULL;
    bool bNoMerge = false;

    bool bHelp = false;
    m_bCalcGeoAngles = false;
    m_bTrackOcc = false;
    m_bZipOutput = false;

    bool bOK = m_pPR->setOptions(31, 
                                 "-h:0",                  &bHelp,
                                 "--help:s",              &sHelpTopic,
                                 "--log-file:s",          &sDummyLog,
                                 "--grid:s!",             &sGridFile,
                                 "--geo:s",               &sGeoFile,
                                 "--climate:s",           &sClimateFile,
                                 "--veg:s",               &sVegFile,
                                 "--nav:s",               &sNavFile,
                                 "--pops:s",              &sPops,
                                 "--output-prefix:s",     &sOutputPrefix,
                                 "--output-dir:s",        &sOutputDir,
                                 "--data-dirs:s",         &sDataDirs,
                                 "--so-dirs:s",           &sSODirs,
                                 "--read-config:s",       &sConfigIn,
                                 "--write-config:s",      &sConfigOut,
                                 "--events:S",           &pEvents,
                                 "--num-iters:i",        &m_iNumIters,
                                 "--start-time:f",       &m_fStartTime,
                                 "--interpol-step:i",    &m_iInterpolStep,
                                 "--interpolation:S",    &pInterpolationData,
                                 "--layer-size:i",       &iLayerSize,
                                 "--shuffle:s",           &sShuffles,
                                 "--seed:s",              &sSeed,
                                 "--pop-params:s",        &sPopParams,
                                 "--calc-geoangles:0",   &m_bCalcGeoAngles,
                                 "--zip-output:0",       &m_bZipOutput,
                                 "--resume:0",           &m_bResume,
                                 "--no-merge-pops:0",    &bNoMerge,
                                 "--track-occ:0",        &m_bTrackOcc,
                                 "--dyn-pops:0",         &m_bDynPops,
                                 "--info-string:S",      &pInfoString);
    if (bOK) {  
        // checkConfig(&iArgC, &apArgV);
        bool bOnProbation = false;
        bool bIntermediateResult = true;
        iResult = m_pPR->getParams(iArgC, apArgV);
        if (sHelpTopic != "") {
             setHelpTopic(sHelpTopic);
             stdprintf("HelpTopic [%s]\n", m_sHelpTopic);
             iResult = 2;
        } else  if (bHelp) {
            setHelp(bHelp);
            iResult = 3;
        } else {
            if ((sConfigIn != "") && (iResult == PARAMREADER_ERR_MANDATORY_MISSING)) {
                iResult = PARAMREADER_OK;
                bOnProbation = true;
            }
            if (iResult >= 0) {
                stringvec vsOptions;
                m_pPR->collectOptions(vsOptions);
                LOG_DISP("%s (r%s) on %d threads called with\n", apArgV[0], REVISION, omp_get_max_threads());
                for (uint j = 0; j < vsOptions.size(); j++) {
                    if (startsWith(vsOptions[j], "--events")) {
		        std::string ssub = vsOptions[j].substr(9);
                        LOG_DISP("  --events='%s'\n", ssub);

		    } else {
                        LOG_DISP("  %s\n", vsOptions[j]);
		    }
                }
                LOG_DISP("-----------------------------------------\n");
                LOG_DISP("replicate executable command:\n");
                LOG_DISP("  head -n %d %s | tail -n %d | sed 's/D://g' | gawk '{ print $1 }' | xargs\n", 4+vsOptions.size(), MessLogger::getFile(), 1+vsOptions.size());

                if (iResult > 0) {
                    LOG_WARNING("ParamReader Warning:\n%s", m_pPR->getErrorMessage(iResult));
                    stdprintf("ParamReader Warning:\n%s", m_pPR->getErrorMessage(iResult));

                    iResult = 0;
                }

                if (iResult > 0) {
                    stringvec vUnknown;
                    int iNum = m_pPR->getUnknownParams(vUnknown);
                    LOG_WARNING2("[SimParams::readOptions] %d unknown param%s:\n", iNum, (iNum>1)?"s":"");
                    for (int i = 0; i < iNum; i++) {
                        LOG_WARNING("  %s\n", vUnknown[i]);
                    }
                }
                
           
            
                int iRes2=0;
                stringvec vsErrorInfo;

                iRes2 = setLayerSize(iLayerSize);
                if (iRes2 != 0) {
                    iResult = iRes2;
                    vsErrorInfo.push_back("setLayerSIze");
                }

                // m_pPR->display();
                // use config file if given
                if (sConfigIn != "") {
                    std::string sTemp;
                    if (exists(sConfigIn, sTemp)) {
                        LOG_STATUS2("[SimParams::readOptions] using config file [%s]\n", sConfigIn);
                        int iResult2 = m_pPR->getParams(sConfigIn);
                        if (bOnProbation && (iResult2 < 0)) {
                            iResult = iResult2;
                        }
                    } else {
                        LOG_ERROR2("The config file [%s] does not exist\n", sConfigIn);
                        vsErrorInfo.push_back("readConfig");
                    }
                }

                stdprintf("numiters: %d\n", m_iNumIters);
                if (m_iNumIters < 0) {
                    iRes2   = -1;
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                    LOG_ERROR2("Negative number of loops (%d)\n", m_iNumIters);
                }

                if (sDataDirs != "") {
                    iRes2   = setDataDirs(sDataDirs);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setDataDirs");
                    }
                }
                if (bIntermediateResult) stdprintf("After setDataDir %d\n", iResult);

                if (sSODirs != "") {
                    iRes2   = setSODirs(sSODirs);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setSODirs");
                    }
                }
                
                if (sSeed != "") {
                    iRes2   = setSeed(sSeed);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setSeed");
                    }
                }
                if (bIntermediateResult) stdprintf("After setSeed %d\n", iResult);

                if (sShuffles != "") {
                    iRes2   = setShuffles(sShuffles);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setShuffles");
                    }
                }
                if (bIntermediateResult) stdprintf("After setShuffles %d\n", iResult);
                
                if (sGridFile != "") {
                    iRes2   = setGrid(sGridFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setGrid");
                    } else {
                        m_pEnvInt = new EnvInterpolator(m_pCG);
                    }
                }
                if (bIntermediateResult) stdprintf("After setGrid %d\n", iResult);

                if (sGeoFile != "") {
                    iRes2   = setGeo(sGeoFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setGeo");
                    }
                }
                if (bIntermediateResult) stdprintf("After setGeo %d\n", iResult);

                if (sClimateFile != "") {
                    iRes2   = setClimate(sClimateFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setClimate");
                    }
                }
                if (bIntermediateResult) stdprintf("After setClimate %d\n", iResult);

                if (sVegFile != "") {
                    iRes2   = setVeg(sVegFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setVeg");
                    }
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) stdprintf("After setVeg %d\n", iResult);

                if (sNavFile != "") {
                    iRes2   = setNav(sNavFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setNav");
                    }
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) stdprintf("After setNav %d\n", iResult);

                if (sPops != "") {
                    iRes2   = setPopList(sPops);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setPopList");
                    }
                }
                if (bIntermediateResult) stdprintf("After setPopList %d\n", iResult);

                if (sPopParams != "") {
                    iRes2   = setPopParams(sPopParams);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setPopParams");
                    }
                }
                if (bIntermediateResult) stdprintf("After setPopParams %d\n", iResult);

                if (sOutputPrefix != "") {
                    iRes2   = setOutputPrefix(sOutputPrefix);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setOutputPrefix");
                    }
                }
                if (bIntermediateResult) stdprintf("After setOutputPrefix %d\n", iResult);

                if (sOutputDir != "") {
                    iRes2   = setOutputDir(sOutputDir);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setOutputDir");
                    }
                }
                if (bIntermediateResult) stdprintf("After setOutputDir %d\n", iResult);

                if ((pEvents != NULL) && (*pEvents != '\0')) {
                    iRes2   = addEventTriggers(pEvents, true); // true: update event list
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("addEventTriggers");
                    }
                }
                if (bIntermediateResult) stdprintf("After addEventTriggers %d\n", iResult);


                if ((pInfoString != NULL) && (*pInfoString != '\0')) {
                    m_sInfoString = pInfoString;
                }
                if (bIntermediateResult) stdprintf("After input string %d\n", iResult);


                if ((pInterpolationData != NULL) && (*pInterpolationData != '\0')) {
                    iRes2   = setInterpolationData(pInterpolationData);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setInterpolationData");
                    }
                }
                if (bIntermediateResult) stdprintf("After setInterpolationData %d\n", iResult);

                if (sConfigOut != "") {
                    iRes2   = setConfigOut(sConfigOut);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setConfigOut");
                    }
                }
                if (bIntermediateResult) stdprintf("After setConfigOut %d\n", iResult);

                m_bMergePops = !bNoMerge;

                if (iResult != 0) {
                    LOG_ERROR2("[SimParams::readOptions] Errors in the following method%s:\n", (vsErrorInfo.size()!=1)?"s":"");
                    for (uint i = 0; i < vsErrorInfo.size(); i++) {
                        LOG_ERROR2("  %s\n", vsErrorInfo[i]);
                    }
                }
            } else {
                if (iResult == PARAMREADER_ERR_MISSING_PARAM) {
                    LOG_ERROR2("[SimParams::readOptions] Missing parameter for option [%s]\n", m_pPR->getBadArg());
                } else if (iResult == PARAMREADER_ERR_OPTION_SET) {
                    LOG_ERROR2("[SimParams::readOptions] Error setting option [%s] to [%s]\n", m_pPR->getBadArg(), m_pPR->getBadVal());
                } else if (iResult == PARAMREADER_ERR_BAD_CONFIG_FILE) {
                    LOG_ERROR2("[SimParams::readOptions] Config file [%s] does not exist\n", sConfigIn);
                } else {
                    iResult = -2;
                    LOG_ERROR2("[SimParams::readOptions] %sissing mandatory params. Required:\n", ((sConfigIn != "")?"Bad config file or m":"M"));
                    
                    stringvec vMand;
                    int iNum = m_pPR->getMandatoryParams(vMand);
                    for (int i = 0; i < iNum; i++) {
                        std::string sPar = stdsprintf("  %s\n", vMand[i]);
                        LOG_ERROR2(sPar, "  %s\n", vMand[i]);
                    }
                }
            }
        }
    } else {
        LOG_ERROR2("[SimParams::readOptions] Error setting options\n");
        iResult = -3;
    }
    
    //    LOG_STATUS("****************  readOptions exited with %d\n", iResult);
    
    if (iResult == 0) {
        LOG_STATUS("Succesfully read params\n");
        LOG_DISP("-------------------------------------\n");
        
        if (!m_sConfigOut.empty()) {
            std::string sConfPath = stdsprintf("%s%s", m_sOutputDir, m_sConfigOut);
            LOG_STATUS("writing to config file [%s]\n", sConfPath);
            // omit the option  "--write-config"
            m_pPR->writeConfigFile(sConfPath, "--write-config");
        }

        /*
        stdprintf("Random seed:\n");
        LOG_STATUS("Random seed:\n");
        std::string sState("");
        for (uint i = 0; i < STATE_SIZE/4; i++) {
            sState = "";                
            for (uint j = 0; j < 4; j++) {
                std::string sDig = stdsprintf(" %08x", m_aulState[4*i+j]);
                sState += sDig;
            }
            LOG_STATUS("    %s\n", sState);
            stdprintf("    %s\n", sState);
        }
        stdprintf("\n");
        */
        LOG_STATUS2("Layer Size: %d\n", m_iLayerSize);
        
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
bool SimParams::exists(const std::string sFile, std::string &sExists) {
    bool bExists = false;
    stdprintf("exists: [%s]?\n", sFile);
    if (!fileExists(sFile))  {
        if (!m_vDataDirs.empty()) {
            std::string sTest;
            sExists = "";
            for (uint i = 0; (sExists.empty() && (i < m_vDataDirs.size())); i++) {
                std::string  sTest0 = stdsprintf("%s%s", m_vDataDirs[i], sFile);
                int iRes = replaceEnvVars(sTest0, sTest);
                if (iRes == 0) {
                    stdprintf("exists: [%s]? - ", sTest);
                    if (fileExists(sTest)) {
                        sExists = sTest;
                        bExists = true;
                        stdprintf("yes\n");
                    } else {
                        stdprintf("no\n");
                    }
                } else {
                    stdprintf("unknown env var: [%s]?\n", sTest);
                    bExists = false;
                }
            }
        } else {
            // pFile doesn't exist, and we don't have a datadir
            // so that's it
            bExists = false;
        } 
    } else {
        sExists = sFile;
        bExists = true;
    }
    if (!bExists) {
        sExists = "";
        LOG_STATUS2("[exists] [%s] not found\n", sFile);
    }
    
    return (bExists);
}


////////////////////////////////////////////////////////////////////////////
//// INPUT DATA HANDLING 
////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------
// setGrid
//  if File is QDF file, use GridGroupReader to create Grid
//  otherwise try the old way (def file)
//
int SimParams::setGrid(const std::string sFile) {
    int iResult = -1;
    std::string sRealFile;
    if (m_pCG == NULL) {
        if (exists(sFile, sRealFile)) {
            m_hFile = qdf_openFile(sRealFile);
            if (m_hFile > 0) {
                iResult = setGrid(m_hFile);
            } else {
                // err couldn't read
                LOG_ERROR2("Couldn't read [%s] as QDF grid file\n", sFile);
                /*
                stdprintf("use old way\n");
                // do it the "old" way
                m_pLRGrid = LineReader_std::createInstance(sRealFile, "rt");
                if (m_pLRGrid != NULL) {
                    iResult = setGridFromDefFile(sRealFile);
                }
                */
            }
            if (iResult == 0) {
                m_pGridScrambler = GridScrambler::createInstance(m_pCG, 12321/*m_iGridSeed*/);
            }
        } else {
            // err doesn't exist
            LOG_ERROR2("Gridfile [%s] doesn't exist\n", sFile);
        }
    } else {
        LOG_ERROR2("Can't have second gridfile [%s]\n", sFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setGrid
//  from QDF file
//
int SimParams::setGrid(hid_t hFile, bool bUpdate /* = false */) {
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
        std::string sTime;
        // get the timestamp of the initial qdf file (grid)
        iResult = qdf_extractSAttribute(hFile,  ROOT_STEP_NAME, sTime);
        if (iResult != 0) {
            LOG_STATUS2("Couldn't read time attribute from grid file\n");
            iResult = 0;
        } else {
            if (strToNum(sTime, &m_iStartStep)) {
                iResult = 0;
                stdprintf("Have timestamp %d\n", m_iStartStep);
            } else {
                stdprintf("Timestamp not valid [%s]\n", sTime);
                iResult = -1;
            }
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
                LOG_STATUS2("[setGrid] Grid read successfully: %p\n", m_pCG);
                int iRes = setGeo(hFile, false, bUpdate);
                if (iRes == 0) {
                    iRes = setClimate(hFile, false, bUpdate);
                    if (iRes == 0) {
                        iRes = setVeg(hFile, false, bUpdate);
                        if (iRes == 0) {

                            // ok                            
                        } else {
                            LOG_STATUS2("[setGrid] No Vegetation found in QDF\n");
                        }
                    } else {
                        LOG_STATUS2("[setGrid] No Climate found in QDF\n");
                    }
                } else {
                    LOG_STATUS2("[setGrid] No Geography found in QDF\n");
                }

                if (!bUpdate) {
                    iRes = setPops(hFile, "", false); // NULL: read all pops, false: not required
                    if (iRes == 0) {
                        // ok
                    } else {
                        LOG_STATUS2("[setGrid] No Populations found in QDF\n");
                    }
                }

                iRes = setNav(hFile, bUpdate);
                if (iRes == 0) {
                    // ok
                } else {
                    LOG_STATUS2("[setGrid] No Navigation found in QDF\n");
                }
                
 
            } else {
                LOG_ERROR2("[setGrid] GridReader couldn't read data\n");
            }
        } else {
            LOG_ERROR2("[setGrid] GridReader couldn't read attributes\n");
        }
        delete pGR;
    } else {
        LOG_ERROR2("[setGrid] Couldn't create GridReader\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setPopParams
//  set special population parameters
//
int SimParams::setPopParams(const std::string sParams) {
    int iResult = 0;
    
    stringvec vParams;

    
    stringmap mNamedParams;

    uint iNum = splitString(sParams, vParams, ",");
    if (iNum > 0) {
        for (uint i = 0; (iResult == 0) && (i < iNum); i++) {
            stringvec vPar;
            uint iNum2 = splitString(vParams[i], vPar, ":");
            if (iNum2 == 2) {
                mNamedParams[vPar[0]] = vPar[1];
                
            } else {
                LOG_ERROR2("[setPopParams] expected a ':' in pop-param definition: [%s]\n", vParams[i]);
            }
            
        } 
    } else {
        LOG_ERROR2("[setPopParams] empty param string?\n");
    }

    // loop through population names
    if (iResult == 0) {
        // now loop through populations and check if name is present
        popmap::const_iterator it_pop;
        for (it_pop = m_pPopLooper->begin(); it_pop != m_pPopLooper->end(); ++it_pop) {
            PopBase *pPop = it_pop->second;
            stringmap::const_iterator it = mNamedParams.find(pPop->getSpeciesName());
            // pass parameter string to population
            if (it != mNamedParams.end()) {
                iResult = pPop->setParams(it->second);
            }
        }

    }

    return iResult;

}

/*
//----------------------------------------------------------------------------
// setGridFromDefFile
//  try the old way (def file)
//
int SimParams::setGridFromDefFile(const std::string sFile) {
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
*/

//----------------------------------------------------------------------------
// setGeo
//  if File is QDF file, use GeoGroupReader to create Geography
//  otherwise try the old way (def file)
//
int SimParams::setGeo(const std::string sFile) {
    int iResult = -1;

    std::string sRealGeo;
    if (exists(sFile, sRealGeo)) {
        hid_t hFile = qdf_openFile(sRealGeo);
        if (hFile > 0) {
            iResult = setGeo(hFile, true);
        } else {
            // no more def file for geo
            iResult = -1; 
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
int SimParams::setGeo(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
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
                         m_pGeo = m_pCG->m_pGeography;
                     }
                     iResult = pGR->readData(m_pGeo);
                     if (iResult == 0) {
                         if (!bUpdate) {
                             m_pCG->setGeography(m_pGeo);
                         }
                         LOG_STATUS2("[setGeo] GeoReader readData succeeded : %p!\n", m_pGeo);
                     } else {
                         LOG_ERROR2("[setGeo] Couldn't read data\n");
                     }
                 } else {
                     iResult = -2;
                     LOG_ERROR2("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, geoatt.m_iNumCells);
                 }
             } else {
                 iResult = -3;
                 LOG_ERROR2("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
             }
             
         } else {
             LOG_ERROR2("[setGeo] Couldn't read attributes\n");
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
// setClimate
//  if File is QDF file, use ClimateGroupReader to create Climate
//  otherwise try the old way (def file)
//
int SimParams::setClimate(const std::string sFile) {
    int iResult = -1;
    
    std::string sRealClimate;
    if (exists(sFile, sRealClimate)) {
        hid_t hFile = qdf_openFile(sRealClimate);
        if (hFile > 0) {
            iResult = setClimate(hFile, true);
        } else {
            // no more def file for climat
            iResult = -1;
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
int SimParams::setClimate(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
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
                    m_pCli = m_pCG->m_pClimate;
                }
                
                iResult = pCR->readData(m_pCli);
                if (iResult == 0) {
                    if (!bUpdate) {
                        m_pCG->setClimate(m_pCli);
                    }
                    LOG_STATUS2("[setClimate] ClimateReader readData succeeded : %p!\n", m_pCli);
                } else {
                    LOG_ERROR2("[setClimate] Couldn't read climate data\n");
                    delete m_pCli;
                }
               
            } else {
                iResult = -2;
                LOG_ERROR2("[setClimate] Cell number mismatch: CG(%d) Cli(%d)\n", m_pCG->m_iNumCells, climatt.m_iNumCells);
            }
            
        } else {
            LOG_ERROR2("[setClimate] Couldn't read attributes\n");
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
// setVeg
//  if File is QDF file, use VegGroupReader to create Vegetation
//  otherwise try the old way (def file)
//
int SimParams::setVeg(const std::string sFile) {
    int iResult = -1;
    
    std::string sRealVeg;
    if (exists(sFile, sRealVeg)) {
        hid_t hFile = qdf_openFile(sRealVeg);
        if (hFile > 0) {
            iResult = setVeg(hFile, true);
        } else {
            //no more def file for veg
            iResult = -1;
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
int SimParams::setVeg(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
    int iResult = -1;
    
    VegGroupReader *pVR = VegGroupReader::createVegGroupReader(hFile);
    if (pVR != NULL) {
        VegAttributes vegatt;
        iResult = pVR->readAttributes(&vegatt);
        if (iResult == 0) {
            //@@            LOG_STATUS1("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);

            if (vegatt.m_iNumCells == m_pCG->m_iNumCells) {
                if (!bUpdate) {
                    m_pVeg = new Vegetation(vegatt.m_iNumCells, vegatt.m_iNumVegSpc, m_pCG->m_pGeography, m_pCG->m_pClimate);
                } else {
                    m_pVeg = m_pCG->m_pVegetation;
                }
                iResult = pVR->readData(m_pVeg);
                if (iResult == 0) {
                    if (!bUpdate) {
                        m_pCG->setVegetation(m_pVeg);
                    }
                    LOG_STATUS2("[setVeg] VegReader readData succeeded!\n");
                } else {
                    LOG_ERROR2("[setVeg] Couldn't read data\n");
                }
            } else {
                iResult = -2;
                LOG_ERROR2("[setVeg] Cell number mismatch: CG(%d) Veg(%d)\n", m_pCG->m_iNumCells, vegatt.m_iNumCells);
            }
        } else {
            LOG_ERROR2("[setVeg] Couldn't read attributes\n");
        }


        delete pVR;
    } else {
        if (bRequired) {
            LOG_ERROR2("[setVeg] Couldn't create VegGroupReader: did not find group [%s]\n", VEGGROUP_NAME);
        }
     }

    
    return iResult;
}


//----------------------------------------------------------------------------
// setNav
//  if File is QDF file, use NavGroupReader to create Navigation
//  otherwise fail
//
int SimParams::setNav(const std::string sFile) {
    int iResult = -1;
    
    std::string sRealNav;
    if (exists(sFile, sRealNav)) {
        hid_t hFile = qdf_openFile(sRealNav);
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
int SimParams::setNav(hid_t hFile, bool bUpdate) {
    int iResult = -1;
    
    NavGroupReader *pNR = NavGroupReader::createNavGroupReader(hFile);
    if (pNR != NULL) {
        NavAttributes navatt;
	memset(&navatt, 0, sizeof(NavAttributes));
        iResult = pNR->readAttributes(&navatt);
        if (iResult == 0) {
            //@@            LOG_STATUS2("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);
            
            if (!bUpdate) {
                m_pNav = new Navigation();
            } else {
                m_pNav = m_pCG->m_pNavigation;
            }

            
            iResult = pNR->readData(m_pNav);
            if (iResult == 0) {
                LOG_STATUS2("[setNav] NavGroupReader readData succeeded!\n");
                iResult = pNR->readBridges(m_pNav);
                if (iResult == 0) {
                    LOG_STATUS2("[setNav] NavGroupReader readBridges succeeded!\n");
                    if (!bUpdate) {
                        m_pCG->setNavigation(m_pNav);
                    }
                } else {
                    LOG_ERROR2("[setNav] Couldn't read bridges\n");
                }

            } else {
                LOG_ERROR2("[setNav] Couldn't read data\n");
            }
            
        } else {
            LOG_ERROR2("[setNav] Couldn't read attributes\n");
        }
        
        
        delete pNR;
    } else {
        LOG_WARNING2("[setNav] Couldn't create NavGroupReader: did not find group [%s]\n", NAVGROUP_NAME);
    }
    
    
    return iResult;
}


//----------------------------------------------------------------------------
// setPoplist
//  assume comma-separated list of pop files
//
int SimParams::setPopList(const std::string sList) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sList, vParts, ",");
    if (iNum > 0) {
        for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
            iResult = setPops(vParts[i]);
        }
    } else {
        LOG_ERROR2("[setPopList] empty poplist ?\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setPops
//  if file contains a ':' try <clsfile>:<datafile> (as in Pop2QDF)
//  otherwise try qdf and  use PopReader to create Populations
//
int SimParams::setPops(const std::string sFile) {
    int iResult = 0;
    //    LOG_ERROR("[setPops] Looking at [%s]\n", pFile);
    
    // create poplooper and popfactory  if they don't exist
    // 
    if (m_pPopLooper == NULL) {
        m_pPopLooper = new PopLooper();
    }

    if (m_pPopFac == NULL) {
        if (m_bDynPops) {
            m_pPopFac = DynPopFactory::createInstance(m_vSODirs, m_pCG, m_pPopLooper, m_iLayerSize, m_apIDG, m_aulState, m_aiSeeds);
            if (m_pPopFac == NULL) {
                stdprintf("[setPops} couldn't create DynPopFactory\n");
                iResult = -1;
            } else {
                iResult = 0;
            }
        }  else {
            m_pPopFac = new StatPopFactory(m_pCG, m_pPopLooper, m_iLayerSize, m_apIDG, m_aulState, m_aiSeeds);
            iResult = 0;
        }
    }
    
    if (iResult == 0) {
        stringvec vDefs;
        uint iNum = splitString(sFile, vDefs, ":");
        if (iNum == 2) {
            std::string sXMLFile  = "";
            std::string sDataFile = "";
            if (exists(vDefs[0], sXMLFile) && exists(vDefs[1], sDataFile)) {
                stdprintf("[setPops] trying [%s] as XML\n", sXMLFile);
                iResult = setPopsFromXMLFile(sXMLFile, sDataFile);
                if (iResult != 0) {
                    stdprintf("[setPops] [%s] seems not to be a QHG XML file\n", sXMLFile);
                }
            } else {
                if (sXMLFile.empty()) {
                    LOG_ERROR("[setPops] File [%s] does not exist\n", sXMLFile);
                }
                if (sDataFile.empty() == '\0') {
                    LOG_ERROR("[setPops] File [%s] does not exist\n", sDataFile);
                }
            }
        } else if (iNum == 1) {
            //@@        LOG_STATUS("[setPops] pop file [%s] must be qdf\n", pFile);
            std::string sExistingPopFile;
            if (exists(sFile, sExistingPopFile)) {
                hid_t hFile = qdf_openFile(sExistingPopFile);
                if (hFile > 0) {
                    iResult = setPops(hFile, "", true);
                } else {
                    LOG_ERROR("[setPops] COuldn't open [%s] as QDF file\n", sExistingPopFile);
                }
            } else {
                // file does not exist
                LOG_ERROR("[setPops] File [%s] does not exist\n", sFile);
            }
        } else {
            LOG_ERROR("[setPops] expected \"<pop-qdf>\" or \"<xml-file>:<data-file>\": [%s]\n", sFile);
            iResult = -1;
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
int SimParams::setPops(hid_t hFile, const std::string sPopName, bool bRequired) {
    int iResult = -1;
    PopReader *pPR = PopReader::create(hFile);
    if (pPR != NULL) {
        if (m_pPopFac != NULL) {
            const popinfolist &pil = pPR->getPopList();
            if (pil.size() > 0) {
                for (uint i = 0; i < pil.size(); i++) {
                    
                    if ((sPopName.empty()) || (sPopName == pil[i].m_sClassName)) {
                        PopBase *pPop = m_pPopFac->createPopulationByName(pil[i].m_sClassName);
                        if (pPop != NULL) {
                            pPop->setAgentDataType();
                            LOG_STATUS2("[setPops] have pop [%s]\n", pil[i].m_sClassName);
                            
                            iResult = pPR->read(pPop, pil[i].m_sSpeciesName, m_pCG->m_iNumCells, m_bResume);
                            if (iResult == 0) {
                                
                                LOG_STATUS2("[setPops] have read pop\n");
                                iResult = m_pPopLooper->addPop(pPop);
                                if (iResult == 0) {
                                    LOG_STATUS2("[setPops] successfully added Population: %s: %ld agents\n", pPop->getSpeciesName(), pPop->getNumAgentsTotal());
                                    if (!m_bResume) {
                                        if (m_aiSeeds[0] > 0) {
                                            stdprintf("Randomizing(1) with %u\n", m_aiSeeds[0]);
                                            pPop->randomize(m_aiSeeds[0]);
                                        } else {
                                            stdprintf("No seeds???\n");
                                        }
                                    }
                                } else {
                                    LOG_ERROR2("[setPops] Couldn't add population [%s]\n",  pil[i].m_sSpeciesName);
                                    
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
                            LOG_ERROR2("[setPops] Couldn't create Population %s %s\n", 
                                      pil[i].m_sSpeciesName, pil[i].m_sClassName);
                        }
                    } else {
                        LOG_ERROR2("[setPops] No population name given\n");
                    }
                }
            } else {
                LOG_ERROR("[setPops] poplist size %zd\n", pil.size());
                iResult = -1;
            }

            delete pPR;
        } else {
            LOG_ERROR2("[setPops] no PopulationFactory (this should not happen!)\n");
        }
    } else {
        if (bRequired) {
            LOG_ERROR2("[setPops] Couldn't create PopReader\n");
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// setPopsFromXMLFile
//  read data from Pop file with real coordinates (lon, lat)
//  use GridInformation to build a surface to call findNode(lon, lat)  
//
int SimParams::setPopsFromXMLFile(const std::string sXMLFile, const std::string sDataFile) {
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
                stdprintf("ParamProvider:::::::::::\n");
                pPP->showTree();
                const stringvec &vClassNames = pPP->getClassNames();
                for (uint i = 0; (iResult == 0) && (i < vClassNames.size()); ++i){
                    iResult = pPP->selectClass(vClassNames[i]);
                    PopBase *pPop = m_pPopFac->readPopulation(pPP);
                    if (pPop != NULL) {
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
                        LOG_ERROR2("[setPopsFromXMLFile] Couldn't create data from [%s]\n", sXMLFile);
                    }
                    
                }
                delete pPP;
            } else {
                iResult = -1;
            }
        } else {
            LOG_ERROR2("[setPopsFroXMLFile] no PopulationFactory (this should not happen!)\n");
        }

    } 

    // use  readAgentData from Pop2QDF to create population
    return iResult;
}


//----------------------------------------------------------------------------
// addEventTriggers
//  write output
//
int SimParams::addEventTriggers(char *pEventDescription, bool bUpdateEventList){
    int iResult = 0;
    // vEvents: vector of std::string
    // split events string and loop through events
    //   if event is 'file'
    //      readEvents(arg, vEvents)
    //   else
    //      vEvents.push_back(event string)


    m_pEM = EventChecker::createEventManager(pEventDescription, m_pPopLooper, m_vDataDirs, m_iNumIters, m_fStartTime, bUpdateEventList);
    if (m_pEM != NULL) {
        m_pEM->start();
        iResult = 0;
    } else {
        iResult = -1;
    }

 
    return iResult;
}


//----------------------------------------------------------------------------
// setInterpolationData
//    expect string
//      intstr ::= <shortarr>["+"<shortarr>]":"<listfile>":"<step> 
//
int SimParams::setInterpolationData(const std::string sInterpolationData){
    int iResult = -1;;

    std::string sEvtFile;
    stringvec vParts;

    uint iNum = splitString(sInterpolationData, vParts, ":");
    if (iNum == 3) {
        if (exists(vParts[1], sEvtFile)) {
            int iStep = 0;
            if (strToNum(vParts[2], &iStep)) {
                m_iInterpolStep = iStep;
                
                stdprintf("[SimParams::setInterpolationData] have step [%d]\n", m_iInterpolStep);
                iResult = 0;
            } else {
                stdprintf("[SimParams::setInterpolationData] expected a number [%s] not found\n", vParts[2]);
            }
                
        } else {
            stdprintf("[SimParams::setInterpolationData] file [%s] not found\n", vParts[1]);
        }
        
    } else {
        stdprintf("[SimParams::setInterpolationData] expected 3 ':'-separated terms\n");
    }

    
    target_list vTargets;
    if (iResult == 0)  {
        stringvec vShorts;
        uint iNumShorts = splitString(vParts[0], vShorts, "+");
        
        if (iNumShorts > 0) {
            for (uint i = 0; i < iNumShorts; ++i) {
                if (vShorts[i] == "alt") {
                    //                    vTargets.push_back(string_combi("Geography/Altitude", string_pair("Geography", "Altitude")));
                    vTargets.push_back(target_info("Geography/Altitude", "Geography", "Altitude"));
                } else if (vShorts[i] == "npp") {
                    //                    vTargets.push_back(string_combi("Vegetation/NPP", string_pair("Vegetation", "NPP")));
                    vTargets.push_back(target_info("Vegetation/NPP", "Vegetation", "NPP"));
                } else {
                    stdprintf("[SimParams::setInterpolationData] Unknown array name [%s]\n", vShorts[i]);
                    iResult = -1;
                }
            }
        } else {
            stdprintf("[SimParams::setInterpolationData] Expected at least one array short name\n");
        }
    }


    if (iResult == 0) {
        m_pAutInt = AutoInterpolator::createInstance(m_pCG, sEvtFile, m_fStartTime, vTargets);
        if (m_pAutInt != NULL) {
            stdprintf("[SimParams::setInterpolationData] AutoInterpolator created ok\n");
        } else {
            iResult = -1;
            stdprintf("[SimParams::setInterpolationData] Failed to create AutoInterpolator!\n");
        }
    }

        /*
    // old:
    char *pFile = NULL;
    char sEvtFile[MAX_PATH];
    char *pShorts = strtok(pInterpolationData, ":");
    if (pShorts != NULL) {
        pFile = strtok(NULL, ":");
        if (pFile != NULL) {
            stdprintf("[SimParams::setInterpolationData] have file [%s]\n", pFile);
                if (exists(pFile, sEvtFile)) {
                stdprintf("[SimParams::setInterpolationData] using filke [%s]\n", sEvtFile);
                char *pStep =  strtok(NULL, ":");
                if (pStep != NULL) {
                    int iStep = 0;
                    if (strToNum(pStep, &iStep)) {
                        m_iInterpolStep = iStep;
                        stdprintf("[SimParams::setInterpolationData] have step [%d]\n", m_iInterpolStep);
                        iResult = 0;
                    } else {
                        stdprintf("[SimParams::setInterpolationData] expected number, not [%s]\n", pStep);
                    }
                } else {
                    stdprintf("[SimParams::setInterpolationData] expected step size\n");
                }
            } else {
                stdprintf("[SimParams::setInterpolationData] fil [%s] not found\n", pFile);
            }      
        } else {
            stdprintf("[SimParams::setInterpolationData] expected file name\n");
        }
        
    } else {
        stdprintf("[SimParams::setInterpolationData] empty string\n");
    }
    

    target_list vTargets;
    if (iResult == 0)  {
        char *pArr = strtok(pShorts,"+");
        while ((iResult == 0) && (pArr != NULL)) {
            if (strcmp(pArr, "alt") == 0) {
                vTargets.push_back(string_combi("Geography/Altitude", string_pair("Geography", "Altitude")));
            } else if (strcmp(pArr, "npp") == 0) {
                vTargets.push_back(string_combi("Vegetation/NPP", string_pair("Vegetation", "NPP")));
            } else {
                stdprintf("[SimParams::setInterpolationData] Unknown array name [%s]\n", pArr);
                iResult = -1;
            }
            pArr = strtok(NULL, "+");
        }
    }
    if (iResult == 0) {
        m_pAutInt = AutoInterpolator::createInstance(m_pCG, sEvtFile, m_fStartTime, vTargets);
        if (m_pAutInt != NULL) {
            stdprintf("[SimParams::setInterpolationData] AutoInterpolator created ok\n");
        } else {
            iResult = -1;
            stdprintf("[SimParams::setInterpolationData] Failed to create AutoInterpolator!\n");
        }
    }
*/
    return iResult;
}



//-----------------------------------------------------------------------------
// setOutputPrefix
//
int SimParams::setOutputPrefix(const std::string sOutputPrefix) {
    m_sOutputPrefix = sOutputPrefix;
    return 0;
}


//-----------------------------------------------------------------------------
// setOutputDir
//
int SimParams::setOutputDir(const std::string sOutputDir) {
    m_sOutputDir = sOutputDir;

    if (!endsWith(m_sOutputDir, "/")) {
        m_sOutputDir += "/";
    }
    int iResult = createDir(m_sOutputDir);
    if (iResult == 0) {
        LOG_STATUS2("Created output directory[%s]\n", m_sOutputDir);
    } else if (iResult == 1) {
        iResult = 0;
        LOG_WARNING2("[setOutputDir] Directory [%s] already exists\n", m_sOutputDir);
    } else if (iResult == -2) {
        LOG_ERROR2("[setOutputDir] [%s] exists, but is not a directory\n", m_sOutputDir);
    } else if (iResult == -3) {
        LOG_ERROR2("[setOutputDir] couldn't stat name [%s]: %s\n", m_sOutputDir, strerror(errno));
    } else if (iResult == -4) {
        LOG_ERROR2("[setOutputDir] couldn't create directory [%s]:%s\n", m_sOutputDir, strerror(errno));
    } else {
        LOG_ERROR2("Couldn't create output directory [%s]\n", m_sOutputDir);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// setDataDirs
//
int SimParams::setDataDirs(const std::string sDataDirs) {
    int iResult = 0;
    uint iNum = splitString(sDataDirs, m_vDataDirs, ",:", false);
    if (iNum == 0) {
        // no datadir?
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// setSODirs
//
int SimParams::setSODirs(const std::string sSODirs) {
    int iResult = 0;
    uint iNum = splitString(sSODirs, m_vSODirs, ",:", false);
    if (iNum == 0) {
        // no datadir?
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  setConfigOut
// 
int SimParams::setConfigOut(const std::string sConfigOut) {
    int iResult = 0;

    m_sConfigOut =  sConfigOut;

    return iResult;
}


//-----------------------------------------------------------------------------
//  readSeed
//    creates a 16 array of long from an arbitrary string
//
int SimParams::readSeed(const std::string sFileName, std::vector<uint32_t> &vulState) {
    int iResult = -1;

    // later: check if it is a file; read first line
    std::string sSeedFile;
    if (exists(sFileName, sSeedFile)) {
        LineReader *pLR = LineReader_std::createInstance(sSeedFile, "rt");
        if (pLR != NULL) {
            char *p = pLR->getNextLine();
            if (p != NULL) {
                if (strstr(p, SEED_HEADER) == p) {
                    iResult = 0;
                    char *p2 = strchr(p, ':');
                    if (p2 != NULL) {
                        *p2 = '\0';
                        p2++;
                        LOG_STATUS("Ignoring seed destination [%s]\n", p2);
                        // pDest = p;
                    }
                    p = pLR->getNextLine();
                    while ((p != NULL) && (iResult == 0) && (strcasecmp(p, SEED_FOOTER) != 0)) {
                        //@@stdprintf("checking [%s]\n", p);
                        iResult = WELLUtils::stringToSeed(p, vulState);
                        if (iResult == 0) {
                            p = pLR->getNextLine();
                        }
                    }

                    if (strcasecmp(p, SEED_FOOTER) == 0) {
                        iResult = 0;
                    } else {
                        if (p == NULL) {
                            LOG_ERROR2("Expected seed footer [%s]\n", SEED_FOOTER);
                            iResult = -1;
                        }
                    }
                } else {
                    LOG_ERROR2("Expected seed header [%s]\n", SEED_HEADER);
                    iResult = -1;
                }
            } else {
                LOG_ERROR2("Seed file [%s] empty?\n", sSeedFile);
                iResult = -1;
            }
            delete pLR;
        } else {
            LOG_ERROR2("Couldn't open seed file [%s]\n", sSeedFile);
            iResult = -1;
        }
    } else {
        LOG_ERROR2("File [%s] does not exist\n", sFileName);
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  setSeed
// 
int SimParams::setSeed(const std::string sSeed) {
    int iResult = -1;

    if (!m_bResume) {
        std::vector<uint32_t> vulState;
    
        if (sSeed == SEED_RANDOM) {
            // create random seeds
            LOG_STATUS("[setSeed]Creating random seeds\n");
            time_t t = time(NULL);
            iResult = WELLUtils::randomSeed((int)t, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] created random seed sequence with rand()\n");
            }
            
        } else if (startsWith(sSeed, SEED_SEQ)) {
            std::string sSeq = sSeed.substr(strlen(SEED_SEQ));
            
            // let's see if it is a sequence
            LOG_STATUS("[setSeed] trying to read seed from string");
            iResult = WELLUtils::stringToSeed(sSeq, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] read seed sequence from string\n");
            }
            
        } else if (startsWith(sSeed, SEED_FILE)) {
            std::string sSeq = sSeed.substr(strlen(SEED_FILE));
            
            // maybe it is a file?
            LOG_STATUS("[setSeed] trying to read seed from file");
            iResult = readSeed(sSeq, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] read seed sequence from file\n");
            }
            
        } else if (startsWith(sSeed, SEED_PHRASE)) {
            std::string sSeq = sSeed.substr(strlen(SEED_PHRASE));
            
            // ok - let's use it as pass phrase
            LOG_STATUS("Use [%s] as phrase to create seed\n", sSeed);
            iResult = WELLUtils::phraseToSeed(sSeq, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] used passphrase [%s] to create seed sequence\n", sSeed);
            }
        } else {
            LOG_ERROR2("Unknown seed type [%s]\n", sSeed);
        }


        if (iResult == 0) {
            // line or file have a vecor of uints
            if (vulState.size() >= STATE_SIZE) {
                iResult = 0;
                if (vulState.size() > STATE_SIZE) {
                    LOG_WARNING("Got %zd hex numbers; only using first %d of them\n", vulState.size(), STATE_SIZE);
                }
                for (uint i = 0; i < STATE_SIZE; i++) {
                    m_aulState[i] = vulState[i];
                }
            } else {
                LOG_ERROR2("Require %d hexnumbers but only got %zd\n", STATE_SIZE, vulState.size());
                iResult = -1;
            }
        }
    } else {
        // don't change random states if we are resuming
        iResult = 0;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
//  setShuffles
// 
int SimParams::setShuffles(const std::string sShuffles) {
    int iResult = 0;
    memset(m_aiSeeds, 0, NUM_SEEDS*sizeof(uint));
    
    stringvec vParts;
    uint iNum = splitString(sShuffles, vParts, ",");
    if (iNum > 0) {
        uint *po = m_aiSeeds;
        uint i = 0;
        while ((iResult == 0) && (i < iNum)) {
            if (strToNum(vParts[i], po)) {
                po++;
                i++;
            } else {
                iResult = -1;
            }
        }
 
        // fill up remaining spaces with the last value
        uint iLastVal = m_aiSeeds[i-1];
        while (i < NUM_SEEDS) {
            m_aiSeeds[i++] = iLastVal;
        }
        
    } else {
        stdprintf("[SimParams::setShuffles] empty shuffle string?\n");
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  setShuffles
// 
int SimParams::setLayerSize(const uint iLayerSize) {
    int iResult = 0;
    uint i = 0;
    uint j = iLayerSize;
    while (j > 1) {
        j >>= 1;
        i++;
    }
    if ((iLayerSize <= MAX_LAYERSIZE) && ((1u << i) == iLayerSize)) {
        m_iLayerSize = iLayerSize;
        
        stdprintf("[SimParams::setLayerSize} m_iLayerSize  is set to %u\n", m_iLayerSize);
    } else {
        stdprintf("[SimParams::setLayerSize] layerSize should be a power of 2 less than %u, but is %u\n", MAX_LAYERSIZE, iLayerSize);
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// createDir
//
int SimParams::createDir(const std::string sDirName) {
    int iResult = -1;
    // create dir if doesn't exist
    mode_t m = S_IRWXU | S_IRGRP | S_IROTH;  // 744
    //struct stat statbuf;

    iResult = mkdir(sDirName.c_str(), m);

    if (iResult != 0) {
        if (errno == EEXIST) {
            //            iResult = stat(sDirName.c_str(), &statbuf);
            if (dirExists(sDirName)) {
                    iResult = 1;
            } else {
                iResult = -3;
            }
        } else {
            iResult = -4;
        }
    } else {
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// createSurface
//
int SimParams::createSurface() {
    int iResult = -1;

    if (m_pSurface == NULL) {
        if (m_pCG != NULL) {
            stringmap &smSurf = m_pCG->m_smSurfaceData;

            std::string &sSurfType = smSurf["SURF_TYPE"];

            if (sSurfType == SURF_LATTICE) {
                int iLinks;
                std::string &sLinks = smSurf[SURF_LTC_LINKS];
                if (strToNum(sLinks, &iLinks)) {
                    if ((iLinks == 4) || (iLinks == 6)) {
                        const std::string sProjT = smSurf[SURF_LTC_PROJ_TYPE];
                        const std::string sProjG = smSurf[SURF_LTC_PROJ_GRID];
                        if ((!sProjT.empty()) &&  (!sProjG.empty())) {
                            Lattice *pLat = new Lattice();
                            iResult = pLat->create(iLinks, sProjT, sProjG);
                            m_pSurface = pLat;
                            if (iResult == 0) {
                                LOG_STATUS2("[createSurface] Have lattice\n");
                            } else {
                                LOG_ERROR2("[createSurface] cioulnd't create lattice\n");
                            }
                        } else {
                            LOG_ERROR2("[createSurface] projection data incomplete:Type [%s], Grid[%s]\n", sProjT, sProjG);
                        }
                    } else {
                        LOG_ERROR2("[createSurface] number of links must be 4 or 6 [%s]\n", sLinks);
                    }
                } else {
                    LOG_ERROR2("[createSurface] number of links is not  a number [%s]\n", sLinks);
                }

            } else if (sSurfType == SURF_EQSAHEDRON) {
                int iSubDivs = -1;
                std::string sSubDivs = smSurf[SURF_IEQ_SUBDIVS];
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
                        LOG_ERROR2("[createSurface] subdivs must be positive [%s]\n", sSubDivs);
                    }
                } else {
                    LOG_ERROR2("[createSurface] subdivs is not a number [%s]\n", sSubDivs);
                }

            } else if (sSurfType == SURF_ICOSAHEDRON) {
                int iSubLevel = -1;
                std::string sSubLevel = smSurf[SURF_ICO_SUBLEVEL];
                if (strToNum(sSubLevel, &iSubLevel)) {
                    if (iSubLevel >= 0) {
                        Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
                        pIco->setStrict(true);
                        pIco->setPreSel(false);
                        pIco->subdivide(iSubLevel);
                        m_pSurface = pIco;
                        LOG_STATUS("[createSurface] Have Icosahedron\n");
                    } else {
                        LOG_ERROR2("[createSurface] sublevels must be positive [%s]\n", sSubLevel);
                    }
                } else {
                    LOG_ERROR2("[createSurface] subdivs is not a number [%s]\n", sSubLevel);
                }
                    
            } else {
                LOG_ERROR2("[createSurface] unknown surface type [%s]\n", sSurfType);
            }
            
        } else {
            LOG_ERROR2("[createSurface] can't create surface without CellGrid data\n");
        }
    }
    return iResult;
};


//-----------------------------------------------------------------------------
// readAgentData
//   we expect lines containing longitude and latitude followed by a colon ':'
//   and  specific agent data
//
int SimParams::readAgentData(PopBase *pPop, const std::string sAgentDataFile) {
    int iResult = 0;
    
    LineReader *pLR = NULL;
    if (sAgentDataFile.ends_with(".gz")) {
        pLR = LineReader_gz::createInstance(sAgentDataFile, "rt");
    } else {
        pLR = LineReader_std::createInstance(sAgentDataFile, "rt");
    }
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
            if (pLine != NULL) {

                double dLon;
                double dLat;
                //we don't use sscanf so we can handle different kinds of separators
                char *pTemp = nextWord(&pLine, " ,;:");
                if (*pTemp != '\0') {
                    if (strToNum(pTemp, &dLon)) {
                        pTemp = nextWord(&pLine, " ,;:");
                        if (*pTemp != '\0') {
                            if (strToNum(pTemp, &dLat)) {
                                iResult = 0;
                            } else {
                                LOG_ERROR2("Not a valid number [%s]\n", pTemp);
                                iResult = -1;
                            }
                        } else {
                            LOG_ERROR2("Expected more than 1 token [%s]\n", pLine);
                            iResult = -1;
                        }
                    } else {
                        LOG_ERROR2("Not a valid number [%s]\n", pTemp);
                        iResult = -1;
                    }
                } else {
                    LOG_ERROR2("No tokens [%s]\n", pLine);
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
                        LOG_WARNING2("[readAgentData] coordinates (%f, %f) could not be mapped to a node\n", dLon ,dLat);
                    }
                } else {
                    iResult = -1;
                    LOG_ERROR2("[readAgentData] Couldn't extract Lon, Lat from [%s]\n", pLine);
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
        LOG_ERROR2("[readAgentData] Couldn't open [%s] for reading\n", sAgentDataFile);
    }
    return iResult;
}


/*********************************
Help
*********************************/


//-----------------------------------------------------------------------------
// printHeaderLine
//
void printHeaderLine(int iL, const std::string sTopic) {

   
    //    char sDash[256];
    //    memset(sDash, '-', iL);
    //    sDash[iL] = '\0';
    //    char sDashi[256];
    //    *sDashi = '\0';    
    if (!sTopic.empty()) {
        
        std::string sTemp1 = stdsprintf(" Help for topic: %s ",  sTopic);
        size_t i1 = sTemp1.size();
        //char sTemp2[256];
        std::string sTemp2 = stdsprintf(" Help for topic: %s%s%s%s ",  colors::BOLDBLUE, sTopic, colors::OFF, colors::BOLD);
        size_t i2 = sTemp2.size();
        size_t iPos = (iL-i1)/2;
        std::string sDash(iPos, '-');
        stdprintf("i1:%zd, i2:%zd\n", i1, i2);
        std::string sDashi((i2-i1), '-');
        //sDash[i2-i1] = '\0';
        //strcat(sDash, sDashi);
        //sDash += sDashi;
        //memcpy(sDash+iPos, sTemp2, i2);
        stdprintf("\n%s%s%s%s%s\n\n", colors::BOLD, sDash, sTemp2, sDash, colors::OFF);
    } else {
    }
}


//-----------------------------------------------------------------------------
// showTopicHelp
//
void SimParams::showTopicHelp(const std::string sTopic) {
    bool bAll = (sTopic == "all");
    bool bFound = false;

    int iL = 80;
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
        stdprintf("    grid-file is a QDF file which must contain a grid group, but may also contain geography, climate, \n");
        stdprintf("    and vegetation data.\n");
        stdprintf("    In this case the corresponding \"--geo\", \"--climate\", and/or \"--veg\" options can be omitted.\n");
        bFound = true;
    }

    //-- topic "num-iters"
    if (bAll || (sTopic == "num-iters")) {
        printHeaderLine(iL, "num-iters");
        stdprintf("  %s--num-iters=<iters>%s    set number of iterations (required option)\n", colors::BOLDBLUE, colors::OFF);
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
    if (bAll || (sTopic == "pops")) {
        printHeaderLine(iL, "pops");
        stdprintf("  %s--pops=<pop-list>%s    set populations data\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    pop-list is a comma-separated list of QDF-files, each containing population data.\n");
        stdprintf("    It must contain data for the same number of cells as the grid file\n");
        stdprintf("    Alternatively you may use\n");
        stdprintf("      --pops=<xml-file>:<dat-file>\n");
        stdprintf("    where xml--file is a class definition xml file,\n");
        stdprintf("    and dat-file is a file containing agent position and other data.\n");
        bFound = true;
    } 

    //-- topic "output-prefix"
    if (bAll || (sTopic == "output-prefix")) {
        printHeaderLine(iL, "output-prefix");
        stdprintf("  %s--output-prefix=<prefix>%s    prefix for output files (default: \"%s\")\n", colors::BOLDBLUE, colors::OFF, DEF_OUT_PREFIX);    
        stdprintf("    This prefix is prepended to all output files\n");
        bFound = true;
    } 

    //-- topic "output-dir"
    if (bAll || (sTopic == "output-dir")) {
        printHeaderLine(iL, "output-dir");
        stdprintf("  %s--output-dir=<dir-name>%s    set output directory (default: \"./\")\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    If the output directory does not exist, it is created\n");
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

    //-- topic "dyn-pops"
    if (bAll || (sTopic == "dyn-pops")) {
        stdprintf("  %s--dyn-pops%s                 use populations from so directory specified in \"--so-dirs\"\n", colors::BOLDBLUE, colors::OFF);    
        bFound = true;
    } 

    //-- topic "so-dirs"
    if (bAll || (sTopic == "so-dirs")) {
        printHeaderLine(iL, "so-dirs");
        stdprintf("  %s--so-dirs=<dir-names>%s    search-directories for population so files (default: \"./\")\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    dir-names is a comma-separated list of directories.\n");
        stdprintf("    The order of the directores in the list defines the search order.\n");
        bFound = true;
    } 

    //-- topic "read-config"
    if (bAll || (sTopic == "read-config")) {
        printHeaderLine(iL, "read-config");
        stdprintf("  %s--read-config=<conf>%s    read options from configuration file.\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    A configuration file must consist of lines\n");
        stdprintf("    each of which is a option setting of the form\n");
        stdprintf("      --<option-name>=value\n");
        stdprintf("    Options passed as parameters to the application supercede the settings in\n");
        stdprintf("    the config file\n");
        bFound = true;
    } 

    //-- topic "write-config"
    if (bAll || (sTopic == "write-config")) {
        printHeaderLine(iL, "write-config");
        stdprintf("  %s--write-config=<conf>%s    write configuration file.\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    The configuration file created with this call can be used\n");
        stdprintf("    with the option \"--read-config\"\n");
        bFound = true;
    }

    //-- topic "events"
    if (bAll || (sTopic == "events")) {
        printHeaderLine(iL, "events");
        stdprintf("  %s--events=<event-list>%s    set event list\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    The event-list string is a comma-separated list of events:\n");
        stdprintf("      %sevent-list%s       ::= \"%s'%s\" <event> [\"%s,%s\" <event>]*\"%s'%s\"\n", 
               colors::BOLD, colors::OFF, colors::BOLD, colors::OFF, colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("    The entire event list must be enclosed in quotes;\n");
        stdprintf("    if the event list contains environment variables, use double quotes.\n");
        stdprintf("      %sevent%s            ::= <event-type> \"%s|%s\" <event-params> \"%s@%s\" <event-times>\n", 
               colors::BOLD, colors::OFF, colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("      %sevent-type%s       ::= \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \n", colors::BOLD, colors::OFF, 
               colors::BOLD, EVENT_TYPE_WRITE, colors::OFF, 
               colors::BOLD, EVENT_TYPE_ENV, colors::OFF, 
               colors::BOLD, EVENT_TYPE_ARR, colors::OFF, 
               colors::BOLD, EVENT_TYPE_POP, colors::OFF, 
               colors::BOLD, EVENT_TYPE_FILE, colors::OFF);
        stdprintf("                           \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" |  \"%s%s%s\" \n",  
               colors::BOLD, EVENT_TYPE_DUMP, colors::OFF,
               colors::BOLD, EVENT_TYPE_INTERPOL, colors::OFF,
               colors::BOLD, EVENT_TYPE_COMM, colors::OFF,
               colors::BOLD, EVENT_TYPE_CHECK, colors::OFF,
               colors::BOLD, EVENT_TYPE_USER, colors::OFF,
               colors::BOLD, EVENT_TYPE_SCRAMBLE, colors::OFF);
        stdprintf("    <event-times> defines at which times the event should be triggered.\n"); 
        stdprintf("      %sevent-times%s      ::= <full-trigger> [ \"%s+%s\" <full-trigger>]*\n", 
               colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("      %sfull-trigger%s     ::= [\"%sS%s\"|\"%sT%s\"]<full-trigger-sub>\n", colors::BOLD, colors::OFF, colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("      If \"S\" or nothing is prefixed, the following numbers are interpreted as steps.\n");
        stdprintf("      If \"T\" is prefixed, the following numbers are interpreted as 'real' times.\n");
        stdprintf("      %sfull-trigger-sub%s ::= <normal-trigger> | <point-trigger> | <final-trigger>\n", colors::BOLD, colors::OFF);
        stdprintf("      %snormal-trigger%s   ::= [<trigger-interval>] <step-size>\n", colors::BOLD, colors::OFF);
        stdprintf("      %strigger-interval%s ::= \"%s[%s\"[<minval>] \"%s:%s\" [<maxval>] \"%s]%s\"\n",  
               colors::BOLD, colors::OFF, colors::BOLD, colors::OFF,  colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("      This form causes events to be triggered at time <minval>+k*<step-size>, for\n");
        stdprintf("      k = 0 ... (<maxval>-<minval>)/<step-size>.\n");
        stdprintf("      If <minval> is omitted, it is set to fNegInf,\n");
        stdprintf("      if <maxval> is omitted, it is set to fPosInf.\n");
        stdprintf("      If <trigger-interval> is omitted, it is set to [fNegInf:fPosInf].\n");
        stdprintf("      %spoint-trigger%s    ::= \"%s[%s\"<time>\"%s]%s\"\n", colors::BOLD, colors::OFF, colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("      This form causes a single event at the specified time.\n");
        stdprintf("      %sfinal-trigger%s    ::= \"%sfinal%s\"\n", colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("      This form causes a single event at the end of the simulation.\n");
        stdprintf("      Trigger Example:\n");
        stdprintf("         %s20+[305:]500+[250:600]10+[37]%s\n", colors::BOLD, colors::OFF);
        stdprintf("         fire events every 20 steps,\n");
        stdprintf("         and every 500 steps starting from step 305,\n");
        stdprintf("         and every 10 steps between steps 250 and 600,\n");
        stdprintf("         and a single event at step 37.\n");
        stdprintf("\n");
        stdprintf("    The event parameters differ for the event types:\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_WRITE, colors::OFF);
        stdprintf("      %sevent-params%s ::= <type> [\"+\" <type>]*\n", colors::BOLD, colors::OFF);
        stdprintf("      %stype%s         ::= \"%s%s%s\" | \"%s%s%s\"   | \"%s%s%s \" | \"%s%s%s \" |\n", colors::BOLD, colors::OFF, 
               colors::BOLD, EVENT_PARAM_WRITE_GRID, colors::OFF, 
               colors::BOLD, EVENT_PARAM_WRITE_GEO, colors::OFF, 
               colors::BOLD, EVENT_PARAM_WRITE_CLIMATE, colors::OFF, 
               colors::BOLD, EVENT_PARAM_WRITE_VEG, colors::OFF);
        stdprintf("                       \"%s%s%s\"  | \"%s%s%s\" |  \"%s%s%s\" |  \"%s%s:%s\"<speciesname>[<filter>]\n", 
               colors::BOLD, EVENT_PARAM_WRITE_NAV, colors::OFF, 
               colors::BOLD, EVENT_PARAM_WRITE_OCC, colors::OFF, 
               colors::BOLD, EVENT_PARAM_WRITE_STATS, colors::OFF, 
               colors::BOLD, EVENT_PARAM_WRITE_POP, colors::OFF);
        stdprintf("    Write the specified data sets to file.\n");
        stdprintf("    In case of \"%s%s%s\", an optional filter character can be appendef to the populaton name to filter the output:.\n", colors::BOLD, EVENT_PARAM_WRITE_POP, colors::OFF);
        stdprintf("       '%s#%s'   only write agent data\n", colors::BOLD, colors::OFF);
        stdprintf("       '%s%%%s'   only write status arrays\n", colors::BOLD, colors::OFF);
        stdprintf("       '%s~%s'   only write additional data\n", colors::BOLD, colors::OFF);
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_ENV, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= <type> [\"+\" <type>]* \":\" <qdf-file>\n", colors::BOLD, colors::OFF);
        stdprintf("      %stype%s           ::= \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\"\n", colors::BOLD, colors::OFF,             
               colors::BOLD, EVENT_PARAM_NAME_GEO, colors::OFF, 
               colors::BOLD, EVENT_PARAM_NAME_CLIMATE, colors::OFF, 
               colors::BOLD, EVENT_PARAM_NAME_VEG, colors::OFF,
               colors::BOLD, EVENT_PARAM_NAME_NAV, colors::OFF),
        stdprintf("    Read the specified data sets from file.\n");
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_ARR, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= <type> \":\" <arrayname> \":\" <qdf-file>\n", colors::BOLD, colors::OFF);
        stdprintf("      %stype%s           ::= \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\"\n", colors::BOLD, colors::OFF,             
               colors::BOLD, EVENT_PARAM_NAME_GEO, colors::OFF, 
               colors::BOLD, EVENT_PARAM_NAME_CLIMATE, colors::OFF, 
               colors::BOLD, EVENT_PARAM_NAME_VEG, colors::OFF),
        stdprintf("    Read the specified arrays from file.\n");
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_POP, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= <speciesname> [\"+\" <speciesname>]* \":\" <qdf-file>\n", colors::BOLD, colors::OFF);
        stdprintf("    Read the specified populations from file.\n");
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_FILE, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= <filename>\n", colors::BOLD, colors::OFF);
        stdprintf("      The file must contain lines of the form\n");
        stdprintf("      %sline%s            ::= <event-type> \"%s|%s\" <event-params> \"%s@%s\" <event-times>\n", 
               colors::BOLD, colors::OFF, colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("    Add the events listed in the file to the event manager.\n");
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_INTERPOL, colors::OFF); 
        stdprintf("      %sevent-params%s     ::= <interpol-data> | <command> \n", colors::BOLD, colors::OFF);
        stdprintf("      %sinterpol-data%s    ::= \"file:\" <interpol-file> \n", colors::BOLD, colors::OFF);
        stdprintf("      %scommand%s          ::= \"cmd:\" <interpol-command> \n", colors::BOLD, colors::OFF);
        stdprintf("      %sinterpol-command%s ::= \"start\" | \"stop\" \n", colors::BOLD, colors::OFF);
        stdprintf("    where\n");
        stdprintf("      interpol-file : a QDF file containing interpolation steps.\n");
        stdprintf("      \"stop\"      : stop interpolation\n");  
        stdprintf("      \"start\"     : restart interpolation\n");  
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_DUMP, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= \"flat\" | \"smart\" | \"free\" \n", colors::BOLD, colors::OFF);
        stdprintf("    Dump the entire state of the simulation.\n");
        stdprintf("    The dump modes:\n");
        stdprintf("      flat    copy the linked lists as arrays\n");
        stdprintf("      smart   save the linked lists by only saving the starts and ends of the active regions (not fully tested)\n");
        stdprintf("      free    use \"smart\" mode if number of holes exceeds a threshold, other wiswe use \"flat\" mode (not fully tested)\n");
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_COMM, colors::OFF);
        stdprintf("      %sevent-params%s   ::= <cmd-file>  | <command>\n", colors::BOLD, colors::OFF);
        stdprintf("      %scommand%s        ::= <iter_cmd> | <del_action_cmd> | <mod_pop_cmd> | <event>\n", colors::BOLD, colors::OFF);
        stdprintf("      %siter_cmd%s       ::= \"%s%s%s:\"<num_iters>\n", colors::BOLD, colors::OFF, colors::BOLD, CMD_SET_ITERS, colors::OFF);
        stdprintf("      %sdel_action_cmd%s ::= \"%s%s%s:\"<population>:<action_name>\n", colors::BOLD, colors::OFF, colors::BOLD, CMD_REMOVE_ACTION, colors::OFF);
        stdprintf("      %smod_pop_cmd%s    ::= \"%s%s%s:\"<population>:<param_name>:<value>\n", colors::BOLD, colors::OFF, colors::BOLD, CMD_MOD_POP, colors::OFF);
        stdprintf("      %sevent%s          : any event description; see definition above\n", colors::BOLD, colors::OFF);
        stdprintf("      The format of <cmd-file>\n");
        stdprintf("        %scmd-file%s       ::= <command-line>*\n", colors::BOLD, colors::OFF);
        stdprintf("        %scommand-line%s   ::= <command> <NL>\n", colors::BOLD, colors::OFF);
        stdprintf("      If the <cmd-file> has changed since the last triggered time the contents will be executed:\n");
        stdprintf("        iter_cmd:        set new iteration limit\n");
        stdprintf("        del_action_cmd:  remove action from prio list\n");
        stdprintf("        mod_pop_cmd:     hange parameter of population\n");
        stdprintf("        event:           add event to event manager's list\n");
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_CHECK, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= <what> [\"+\" <what]*\n", colors::BOLD, colors::OFF);
        stdprintf("      %swhat%s           ::= \"%s%s%s\"\n",  colors::BOLD, colors::OFF, colors::BOLD, EVENT_PARAM_CHECK_LISTS, colors::OFF);
        stdprintf("    (Debugging only) \"lists\": check linked lists at specified times\n");
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_USER, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= <eventid>:<stringdata>\n", colors::BOLD, colors::OFF);
        stdprintf("      %seventid%s        : an event id in [%d, %d]\n",  colors::BOLD, colors::OFF, EVENT_ID_USR_MIN, EVENT_ID_USR_MAX);
        stdprintf("      %sstringdata%s     : a string\n",  colors::BOLD, colors::OFF);
        stdprintf("\n");

        stdprintf("    for <event-type> == \"%s%s%s\":\n", colors::BOLD, EVENT_TYPE_SCRAMBLE, colors::OFF); 
        stdprintf("      %sevent-params%s   ::= \"%s\" | \"%s\"\n", colors::BOLD, colors::OFF, EVENT_PARAM_SCRAMBLE_CONN, EVENT_PARAM_SCRAMBLE_ALL);
        stdprintf("      Scrambles the order of connections between nodes\n");
        stdprintf("\n");

        stdprintf("    Full example:\n");
        stdprintf("      %s--events='env|geo+climate:map120.qdf@[120],write|pop:GrassLover|grid@5+[20:30]1,comm|REMOVE ACTION sapiens:ConfinedMove@[3000]'%s\n", colors::BOLD, colors::OFF); 
        stdprintf("    This loads a QDF file containing new geography and climate data\n");
        stdprintf("    from 'map120.qdf' at step 120, and writes a QDF file containing grid data\n");
        stdprintf("    and population data for species \"GrassLover\" every 5 steps\n");
        stdprintf("    and additiionally every step between steps 20 and 30.\n");
        stdprintf("    At step 3000 the 'ConfinedMove' action of species 'sapiens' is disabled.\n");
        bFound = true;
    } 

    //-- topic "start-time"
    if (bAll || (sTopic == "start-time")) {
        printHeaderLine(iL, "start-time");
        stdprintf("  %s--start-time=<time>%s    set time at step 0\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("                                                     (default: 0)\n");
        stdprintf("    This way you can set the time for events in natural form, e.g. -85000\n");
        bFound = true;
    }

    //-- topic "interpol-step"
    if (bAll || (sTopic == "interpol-step")) {
        printHeaderLine(iL, "interpol-step");
        stdprintf("  %s--interpol-step=<step>%s    set interval for interpolation updates 0\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("                                                     (default: 1)\n");
        stdprintf("    This option only works if you have some interpolation events\n");
        bFound = true;
    }

    //-- topic "interpolation"
    if (bAll || (sTopic == "interpolation")) {
        printHeaderLine(iL, "interpolation");
        stdprintf("  %s--interpolation=<params%s    set interpolation parametersn", colors::BOLDBLUE, colors::OFF);
        stdprintf("    This option prepares interpolation of the specified arrays set  at the time steps specified in the file\n");
        stdprintf("    %sparams%s   ::= <arrays>\":\"<evtfile>\":\"<inter-step>\n", colors::BOLD, colors::OFF);
        stdprintf("    %sarrays%s   ::= <arrname> [\"+\"<arrname>]*\n", colors::BOLD, colors::OFF);
        stdprintf("    %sarrname%s  ::= \"alt\" | \"npp\"\n", colors::BOLD, colors::OFF);
        
        stdprintf("    If 'alt' is specified, the altitudes will be interpolated\n");
        stdprintf("    If 'npp' is specified, the NPP will be interpolated\n");
        stdprintf("    The evtfile is a 'normal' event file (usually the same as used for events)\n");
        stdprintf("    inter-step is the interpolation interval\n");  
        bFound = true;
    }

    //-- topic "layer-size"
    if (bAll || (sTopic == "layer-size")) {
        printHeaderLine(iL, "layer-size");
        stdprintf("  %s--layer-size=<size>%s    %s(technical)%s set layer size of data structures\n", colors::BOLDBLUE, colors::OFF, colors::BOLDRED, colors::OFF);
        stdprintf("                                                     (default: %d)\n", DEF_LAYERSIZE);
        stdprintf("    Only change this option if you know what you are doing\n");
        bFound = true;
    }

    //-- topic "shuffle"
    if (bAll || (sTopic == "shuffle")) {
        printHeaderLine(iL, "shuffle");
        stdprintf("  %s--shuffle=<num>%s    %s(technical)%s shift random generators' sequence by <num>\n", colors::BOLDBLUE, colors::OFF, colors::BOLDRED, colors::OFF);
        stdprintf("    This is done by generating <num> random numbers on each random\n");
        stdprintf("    number generator before the start of the simulation.\n");
        stdprintf("    I.e., this option can be used to change the random number sequence\n");
        bFound = true;
    } 

    //-- topic "seed"
    if (bAll || (sTopic == "seed")) {
        printHeaderLine(iL, "seed");
        stdprintf("  %s--seed=<seedtype>%s    %s(technical)%s seed the random generators\n", colors::BOLDBLUE, colors::OFF, colors::BOLDRED, colors::OFF);
        stdprintf("    %sseedtype%s   ::= \"%s%s%s\" | \"%s%s%s\"<sequence> | \"%s%s%s\"<seed-file> | \"%s%s%s\"<arbitrary>\n", colors::BOLD, colors::OFF,
               colors::BOLD, SEED_RANDOM, colors::OFF, 
               colors::BOLD, SEED_SEQ, colors::OFF,
               colors::BOLD, SEED_FILE, colors::OFF,
               colors::BOLD, SEED_PHRASE, colors::OFF);
        stdprintf("    %ssequence%s   ::= (<hexnumber> \"%s,%s\")*\n", colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("    %sarbitrary%s  : arbitrary string (enclose in quotes if it contains spaces)\n", colors::BOLD, colors::OFF);
        stdprintf("    %sseed-file%s  : name of a seed file\n", colors::BOLD, colors::OFF);
        stdprintf("    If seedType is \"%srandom%s\", the seeds are filled with random numbers\n", colors::BOLD, colors::OFF);
        stdprintf("    based on the current time\n");
        stdprintf("    A sequence must be consist of %d comma-separated 8-digit hexnumbers.\n", STATE_SIZE);
        stdprintf("    If sequence is given, it is passed directly to the random nuber generators.\n");
        stdprintf("    If an arbitrary string is provided, it is turned into a seed sequence using\n");
        stdprintf("    a hash function (SHA-512)\n");
        stdprintf("    If a seed file is specified it must conform to the following format.\n");
        stdprintf("    The general format of a seed file:\n");
        stdprintf("      %sseed-file%s ::= <seed-def>*\n", colors::BOLD, colors::OFF);
        stdprintf("      %sseed-def%s  ::= <header> <seed-line>* <footer>\n", colors::BOLD, colors::OFF);
        stdprintf("      %sheader%s    ::= \"%s%s%s\"[:<dest>]\n", colors::BOLD, colors::OFF, colors::BOLD, SEED_HEADER, colors::OFF);
        stdprintf("      %sseed-line%s ::= (<hexnumber> \"%s,%s\")*\n", colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("      %sdest%s      :   A string describing which RNG the sequence should be\n", colors::BOLD, colors::OFF);
        stdprintf("                   passed to (%sNOT IMPLEMENTED YET%s)\n", colors::BOLDRED, colors::OFF);
        stdprintf("      %sfooter%s    ::= \"%s%s%s\"\n", colors::BOLD, colors::OFF, colors::BOLD, SEED_FOOTER, colors::OFF);
        stdprintf("    In total, %d 8-digit hex numbers must be given between header and footer\n", STATE_SIZE);
        stdprintf("    Lines starting with \"%s#%s\" are ignored\n", colors::BOLD, colors::OFF);
        bFound = true;
    } 

    //-- topic "pop-params"
    if (bAll || (sTopic == "pop-params")) {
        printHeaderLine(iL, "pop-params");
        stdprintf("  %s--pop-params=<param-strings>%s    set special population parameters\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    %sparam-strings%s   ::= <param-string> [\"%s,%s\" <param-string>]*\n", colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("    %sparam-string%s    ::= <popname> \"%s:%s\" <pop-param>\n", colors::BOLD, colors::OFF, colors::BOLD, colors::OFF);
        stdprintf("    The  %spop-param%s strings are passed to the specified populations;\n", colors::BOLD, colors::OFF);
        stdprintf("    the format of %spop-param%s is specific for the particular population\n", colors::BOLD, colors::OFF);
        stdprintf("    Data provided this way will be passed to the parameter of the population's setParam(const char *pParam) method.\n");
        bFound = true;
    } 

    //-- topic "zip-output"
    if (bAll || (sTopic == "zip-output")) {
        printHeaderLine(iL, "zip-output");
        stdprintf("  %s--zip-output%s    zip all output files\n", colors::BOLDBLUE, colors::OFF);
        bFound = true;
    }

    //-- topic "info-string"
    if (bAll || (sTopic == "info-string")) {
        printHeaderLine(iL, "info-string");
        stdprintf("  %s--info-string%s=<infostring>\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    Add  <infostring> as the value of the attribute \"info\" of the root group of every output qdf\n");
        bFound = true;
    }

    /*@@
    //-- topic "scramble"
    if (bAll || (sTopic == "scramble")) {
        printHeaderLine(iL, "scramble");
        stdprintf("  %s--scramble=<what>%s      %s(technical)%s scramble order of node connections\n", colors::BOLDBLUE, colors::OFF, colors::BOLDRED, colors::OFF);
        stdprintf("    %swhat%s  : a string\n", colors::BOLD, colors::OFF);
        stdprintf("    Scramble the order of neighbors of a node using <what> as random seed\n");
        bFound = true;
    }
    */

    //-- topic "resume"
    if (bAll || (sTopic == "resume")) {
        printHeaderLine(iL, "resume");
        stdprintf("  %s--resume%s    resume simulation from previously dumped env and pop files (created by a \"dump\" event)\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    Use the full environment dump as initial grid, and all dumped population qdfs for '--pops='\n");
        stdprintf("    To have totally exact resume, the number of threads must be the same as in the dumped simulation\n");
        bFound = true;
    }

    //-- topic "no-merge-pops"
    if (bAll || (sTopic == "no-merge-pops")) {
        printHeaderLine(iL, "no-merge-pops");
        stdprintf("  %s--no-merge-pop%s    Don't merge compatiple populations.\n", colors::BOLDBLUE, colors::OFF);
        stdprintf("    Two populations are compatible if they have the same class name, the same species name, the same actions , and the same agent structure.\n");
        stdprintf("    ATTENTION: currently the population read in more recently overwrites the olde one\n"); 
        bFound = true;
    }

    //-- topic "dump-on-interrupt"
    if (bAll || (sTopic ==  "dump-on-interrupt")) {
        printHeaderLine(iL,  "dump-on-interrupt");
        stdprintf("  %s--dump-on-interrupt%s  set handler for SIG_INT (Ctrl-C): write dump and exit gracefullly\n", colors::BOLDBLUE, colors::OFF);
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


//----------------------------------------------------------------------------
// addEventName
//
std::string SimParams::addEventName(std::string &sEventString) {
    int iStart = sEventString.find('(');
    int iEnd = sEventString.find(')', iStart);
    std::string s1=sEventString.substr(iStart+1, iEnd-iStart-1);
    int iType = 0;
    std::string s = "(unknown)";
    if (strToNum(s1, &iType)) {
        switch (iType) {
        case EVENT_ID_WRITE:
            s = EVENT_TYPE_WRITE;
        break;
        case EVENT_ID_ENV:
            s = EVENT_TYPE_ENV;
            break;
        case EVENT_ID_COMM:
            s = EVENT_TYPE_COMM;
            break;
        case EVENT_ID_DUMP:
            s = EVENT_TYPE_DUMP;
            break;
        case EVENT_ID_ARR:
            s = EVENT_TYPE_ARR;
            break;
        }
    }
    return s + sEventString;
}
