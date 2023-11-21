#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <string>

#include "utils.h"
#include "types.h"
#include "geomutils.h"
#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "ExecuteCommand.h"
#include "SCell.h"
#include "Geography.h"
#include "Climate.h"
#include "Vegetation.h"
#include "Navigation.h"
#include "IcoNode.h"
#include "NodeIndex.h"
#include "VertexLinkage.h"
#include "EQsahedron.h"
#include "SCellGrid.h"
#include "MessLoggerT.h"
#include "GridFactory.h"
#include "SeasonProvider.h"

static const int PR_LINEAR         = 7;
static const int MAX_ICO_NEIGHBORS = 6;
static const int HEX_NEIGHBORS     = 6;
static const int RECT_NEIGHBORS    = 4;

static std::string KEY_DDIR = "DATA_DIR";
static std::string KEY_GRID = "GRID_TYPE";
static std::string KEY_ALT  = "ALT";
static std::string KEY_ICE  = "ICE";
static std::string KEY_RAIN = "RAIN";
static std::string KEY_TEMP = "TEMP";
static std::string KEY_NPP  = "NPP";

const std::string PLACEHOLDER = "##QDF##";

const static stringvec s_vKeys = {KEY_DDIR, KEY_GRID, KEY_ALT, KEY_ICE, KEY_RAIN, KEY_TEMP, KEY_NPP};

//static std::string INTERP_DIR     = "/home/jody/progs/multi_spc_QHG3/useful_stuff/gridpreparation/";
static std::string APP_PYTHON     = "/usr/bin/python";
static std::string APP_ALT_INTERP = "alt_interpolator.py";
static std::string APP_SEA_INTERP = "sealevel_changer.py";
static std::string APP_ICE_INTERP = "ice_gano_interpolator.py";
static std::string APP_NPP_INTERP = "npp_interpolator.py";
static std::string APP_CLI_INTERP = "climate_interpolator.py";

static std::string DEF_ALT_FILE  = "ETOPO1_Bed_g_gmt4.grd";
static std::string DEF_SEA_FILE  = "sealevel_GanopolskiR.dat";
#define DEF_SEA_HCOL  1 
#define DEF_SEA_TCOL  0 

static std::string DEF_ICE_FILE  = "icesheet_Ganopolski.nc";

static std::string DEF_CLI_DIR   = "CLIMATE/BRIDGE";
static std::string DEF_CLI_PAT   = "([0-9]*)ka\\.ann\\.nc";

static std::string DEF_NPP_FILE  = "npp_timmermann.nc";
static std::string DEF_NPP_LON   = "XAXLEVITR";
static std::string DEF_NPP_LAT   = "YAXLEVITR";
static std::string DEF_NPP_TIME  = "TAXI";
static std::string DEF_NPP_NPP   = "NPPHRS";

static uint PERIODIC_NONE = 0;
static uint PERIODIC_X    = 1;
static uint PERIODIC_Y    = 2;
static uint PERIODIC_XY   = 3;

static std::string sPeriodicityNames[] = {
    "PERIODIC_NONE",
    "PERIODIC_X",
    "PERIODIC_Y",
    "PERIODIC_XY"
};

#define RADIUS 6371.3



//-----------------------------------------------------------------------------
// constructor
//
    GridFactory::GridFactory(const std::string sDefFile) 
    : m_iNumCells(0),
      m_pCG(NULL),
      m_pGeo(NULL),
      m_pClimate(NULL),
      m_pVeg(NULL),
      m_pNav(NULL),
      m_pLR(NULL),
      m_sDef(sDefFile),
      m_bInterpol(false),
      m_bFromFile(false),
      m_dRadius(RADIUS) {
    

}


//-----------------------------------------------------------------------------
// constructor
//
GridFactory::GridFactory() 
    : m_iNumCells(0),
      m_pCG(NULL),
      m_pGeo(NULL),
      m_pClimate(NULL),
      m_pVeg(NULL),
      m_pNav(NULL),
      m_pLR(NULL),
      m_sDef(""),
      m_bInterpol(false),
      m_bFromFile(false),
      m_dRadius(RADIUS)   {
    
}

//-----------------------------------------------------------------------------
// destructor
//
GridFactory::~GridFactory() {

    // the GridFactory creates the cell grid, geo and climate
    // but does not delete them - they are used elsewhere
    delete m_pNav;
    delete m_pVeg;
    delete m_pClimate;
    delete m_pGeo;
    delete m_pCG;
}



//-----------------------------------------------------------------------------
// createEQGrid
//    pVL = EQsahedron::getLinkage()
//
NodeIndex *createEQGrid(VertexLinkage *pVL) {
    
    NodeIndex *pNI = new NodeIndex();
    
    std::map<gridtype, Vec3D *>::const_iterator iti;
    for (iti = pVL->m_mI2V.begin(); iti != pVL->m_mI2V.end(); iti++) {
        Vec3D *pV = iti->second;

        // calc theta and phi
        double dPhi = 0;
        double dTheta = 0;
        if (pV != NULL) {
            double dArea = 0; // get it from a vertexlinkage thingy
            cart2Sphere(pV, &dTheta, &dPhi);
            IcoNode *pIN = new IcoNode(iti->first, dTheta, dPhi, dArea);
            intset vLinks = pVL->getLinksFor(iti->first);
            if (vLinks.size() > MAX_ICO_NEIGHBORS) {
                intset::iterator st;
                stdprintf("Vertex #%d (%f,%f) has %zd links\n", iti->first, pV->m_fX, pV->m_fY,vLinks.size());
                for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                    stdprintf("  %d", *st);
                }
                stdprintf("\n");
            }
            
                    
            intset::iterator st;
            for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                Vec3D *pN = pVL->getVertex(*st);
                double dDist = spherdist(pV, pN, 1.0);
                pIN->m_dArea = pVL->calcArea(pIN->m_lID);

                pIN->addLink(*st, dDist);

            }
            pNI->m_mNodes[iti->first] = pIN;
        } else {
            pNI = NULL;
        }
    }

    return pNI;
}


//-----------------------------------------------------------------------------
// setDataDirs
//  setting data dirs: directories to searcg infor app names or file names
//
int GridFactory::setDataDirs(const stringvec &vDataDirs) {
    int iResult = 0;

    for (uint i = 0; (iResult == 0) && (i < vDataDirs.size()); i++) {
        std::string sDir = vDataDirs[i];
        std::string sCur = "";
        iResult = replaceEnvVars(sDir, sCur);
        if (iResult == 0) {
        
            if (dirExists(sCur)) {
                if (sCur.back() != '/') {
                    sCur = sCur + '/';
                }
                m_vDataDirs.push_back(sCur);
            } else {
                iResult = -1;
                stdfprintf(stderr, "[%s] does not exist as directory\n", sCur);
            }
        } else {
            stdfprintf(stderr, "unknown env var [%s] in data dirs\n", sDir);
        }
    }
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
bool GridFactory::exists(const std::string sFile, std::string &sExists) {
    bool bExists = false;
    //    struct stat statbuf;
    
    //    printf("Have %lu datadirs\n",  m_vDataDirs.size());
    //    printf("Errore1: [%s]\n", strerror(iResult));
    if (fileExists(sFile))  {
        sExists = sFile;
        bExists = true;
    } else {
        if (!m_vDataDirs.empty()) {
            
            sExists = "";
            for (uint i = 0; (sExists.empty()) && (i < m_vDataDirs.size()); i++) {
                std::string sTest = stdsprintf("%s%s", m_vDataDirs[i], sFile);
                if (fileExists(sTest)) {
                    sExists = sTest;
                    bExists = true;
                    //printf("Foubd!\n");
                }
            }
        } else {
            // pFile doesn't exist, and we don't have a datadir
            // so that's it
            bExists = false;
        } 
    }
    if (!bExists) {
        sExists = "";
        stdprintf("[GridFactory::exists] [%s] not found\n", sFile);
    }
    
    return bExists;
}


//-----------------------------------------------------------------------------
// splitCommand
//
int GridFactory::splitCommand(std::string sLine) {
    int iResult = -1;

    stringvec vParts;
    uint iNum = splitString(sLine, vParts, ";, \t\n");
    if (iNum > 0) {
        stringvec::const_iterator it = std::find(s_vKeys.begin(), s_vKeys.end(), vParts[0]);
        if (it != s_vKeys.end()) {

            m_mSplitCommands[vParts[0]] = stringvec(vParts.begin()+1, vParts.end());
            iResult = 0;
        } else {
            stdfprintf(stderr, "Unknown key [%s]\n", vParts[0]);
        }
    } else {
        stdfprintf(stderr, "empty command ?\n");
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// collectLines
//
int GridFactory::collectLines() {
    int iResult = -1;
    // let's see if this is 
    std::string sRealName;
    if (exists(m_sDef, sRealName)) {
        m_pLR = LineReader_std::createInstance(sRealName, "rt");
        if (m_pLR != NULL) {
            iResult = 0;
            m_bFromFile = true;
            while ((iResult == 0) && (!m_pLR->isEoF())) {
                char *pLine = m_pLR->getNextLine(GNL_IGNORE_ALL);
                if (pLine != NULL) {
                    iResult = splitCommand(pLine);
                }
            } 
            delete m_pLR;
        }
    }

    if (iResult != 0)  {
        iResult = 0;
        m_bFromFile = false;

        stringvec vParts;
        uint iNum = splitString(m_sDef, vParts, ";"); 
        for (uint i =0; (iResult == 0) && (i < iNum); ++i) {
             iResult = splitCommand(vParts[i]);
        }
    }

    // just checking
    for (uint i = 0; i < s_vKeys.size(); i++)  {
        commandmap::const_iterator it = m_mSplitCommands.find(s_vKeys[i]);
        printf("%10s ", s_vKeys[i].c_str());
        stdprintf("[%s]\n", (it != m_mSplitCommands.end())?(join(it->second, " ")):"---");
    }

    return iResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// line handlers
//////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// readDef
//
int GridFactory::readDef() {
    int iResult = -1;

    iResult = collectLines();
    printf("Collected lines: %d\n", iResult);

    //----- data dirs
    if (iResult == 0) {
        iResult = handleDDirLine();
    }

    //----- grid
    if (iResult == 0) {
        iResult = handleGridLine();
    }

    //----- altitude
    if (iResult == 0) {
        iResult = handleAltLine();
    }

    //----- ice
    if (iResult == 0) {
        iResult = handleIceLine();
    }

    //----- temp
    if (iResult == 0) {
        iResult = handleTempLine();
    }

    //----- rain
    if (iResult == 0) {
        iResult = handleRainLine();
    }


    //----- npp
    if (iResult == 0) {
        iResult = handleNPPLine();
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// handleDDirLine
//
int GridFactory::handleDDirLine() {
    int iResult = 0;

    commandmap::const_iterator it = m_mSplitCommands.find(KEY_DDIR);
    if (it != m_mSplitCommands.end()) {
        stdprintf("----- setting data dirs [%s]\n", join(it->second, ":"));
        iResult = setDataDirs(it->second);
    }         
    if (iResult != 0) stdfprintf(stderr, "handleDDirLine: %d\n", iResult);
    return iResult;
}


//-----------------------------------------------------------------------------
// handleGridLine
//
int GridFactory::handleGridLine() {
 
  int iResult = 0;

    commandmap::const_iterator it = m_mSplitCommands.find(KEY_GRID);
    if (it != m_mSplitCommands.end()) {
        stdprintf("----- creating grid [%s]\n", join(it->second, "|"));
        if (it->second.size() > 1) {
            iResult = setGrid(it->second);
            if (iResult == 0) {
                // printf("Grid OK\n");
                m_bInterpol = (m_pCG->m_iType==GRID_TYPE_ICO) || (m_pCG->m_iType==GRID_TYPE_IEQ);
            }
        } else {
            stdfprintf(stderr, "expected arguments\n");
        }
    }
    if (iResult != 0) stdfprintf(stderr, "handleGridLine: %d\n", iResult);
    return iResult;
}

//-----------------------------------------------------------------------------
// handleAltLine
//
int GridFactory::handleAltLine() {
    int iResult = -1;
 
    commandmap::const_iterator it = m_mSplitCommands.find(KEY_ALT);
    if (it != m_mSplitCommands.end()) {
        stringvec vParams = it->second;
        stdprintf("----- handling ALT [%s]\n", join(vParams, ":"));
        
        if (vParams[0].compare("NETCDF") == 0) {
            stdprintf("----- have %d elements >=2: %d\n", vParams.size(), vParams.size()>=2);
            if  (vParams.size() > 1) {
                std::string sTemplate = stdsprintf("%s %%s %%s %%d %s", APP_PYTHON, PLACEHOLDER);
                iResult = createNETCDFCommands(vParams, APP_ALT_INTERP, DEF_ALT_FILE, sTemplate);
                if ((iResult == 0) && (vParams.size() > 2)) {
                    vParams[1] = DEF_SEA_FILE;
                    sTemplate = stdsprintf("%s %%s %%s %d %d %%d %s", APP_PYTHON, DEF_SEA_TCOL, DEF_SEA_HCOL, PLACEHOLDER);
                    iResult = createNETCDFCommands(vParams, APP_SEA_INTERP, DEF_SEA_FILE, sTemplate);
                }
            } else {
                iResult = -1;
                stdfprintf(stderr, "Expected 2 parmeters [%s]\n", join(vParams, " "));
            }
                
        } else if ((vParams[0].compare("FLAT") == 0)  && (vParams.size() > 1)) {
            double dAlt;
            stdfprintf(stderr, "converting [%s] to uint\n", vParams[1]);
            if (strToNum(vParams[1], &dAlt)) {
                stdfprintf(stderr, "-> %f (for all %d cells\n", dAlt, m_iNumCells);
                for (unsigned int i = 0; i < m_iNumCells; i++) {
                    m_pGeo->m_adAltitude[i]    = dAlt;
                }
                iResult = 0;
            } else {
                stdfprintf(stderr, "Expected float value in command [%s]\n", join(vParams, " "));
            }


        } else {
            stdfprintf(stderr, "Invalid ALT line [%s]\n",  join(vParams, " "));
            iResult =-1;
        }
    } else {
        // nothing to do is ok
        iResult = 0;
    }
    if (iResult != 0) stdfprintf(stderr, "handleAltLine: %d\n", iResult);

    return iResult;
}


//-----------------------------------------------------------------------------
// handleIceLine
//
int GridFactory::handleIceLine() {
    int iResult = -1;
    
    commandmap::const_iterator it = m_mSplitCommands.find(KEY_ICE);
    if (it != m_mSplitCommands.end()) {
        stringvec vParams = it->second;
        stdprintf("----- handling ICE [%s]\n", join(vParams, ":"));

        if (vParams[0].compare("NETCDF") == 0){
            if (vParams.size() > 2) {
                std::string sTemplate = stdsprintf("%s %%s %%s %%d %s", APP_PYTHON, PLACEHOLDER);
                iResult = createNETCDFCommands(vParams, APP_ICE_INTERP, DEF_ICE_FILE, sTemplate);

            }   else {
                stdfprintf(stderr, "Unknown format for \"NETCDF\" [%s]\n", join(vParams, " "));
                iResult = -1;
            }
            
        } else  if (vParams[0].compare("FLAT") == 0) {
            if (vParams.size() > 1) {
                double dIce = 0;
                if (strToNum(vParams[1], &dIce)) {
                    for (unsigned int i = 0; i < m_iNumCells; i++) {
                        m_pGeo->m_abIce[i]  = (dIce > 0)?1:0;
                    }
                    iResult = 0;
                } else {
                    stdfprintf(stderr, "Ice value should be a number: [%s]\n", join(vParams, " "));
                    iResult = -1;
                    
                }
            } else {
                stdfprintf(stderr, "Expected value parameter for \"FLAT\":[%s]\n", join(vParams, " "));
                iResult = -1;
            }
                


        } else {
            stdprintf("Invalid ICE line [%s]\n", join(vParams, " "));
            iResult =-1;
        }
    } else {
        // nothing to do is ok
        iResult = 0;
    } 

    if (iResult != 0) stdfprintf(stderr, "handleIceLine: %d\n", iResult);
    return iResult;
}


//-----------------------------------------------------------------------------
// handleTempLine
//
int GridFactory::handleTempLine() {
    int iResult = -1;
 
    commandmap::const_iterator it = m_mSplitCommands.find(KEY_TEMP);
    if (it != m_mSplitCommands.end()) {
        stringvec vParams = it->second;
        stdprintf("----- handling TEMP [%s]\n", join(vParams, ":"));

        if (vParams[0].compare("NETCDF") == 0){
            if (vParams.size() > 2) {

                std::string sTemplate = stdsprintf("%s %%s %%s %s %s %%d %s", APP_PYTHON, DEF_CLI_PAT, "temp", PLACEHOLDER);
                iResult = createNETCDFCommands(vParams, APP_CLI_INTERP, DEF_CLI_DIR, sTemplate);

                    
            } else {
                stdfprintf(stderr, "Unknown format for \"NETCDF\" [%s]\n", join(vParams, " "));
                iResult = -1;
            }

        } else  if (vParams[0].compare("FLAT") == 0) {
            if (vParams.size() > 1) {
                double fVal = 0;
                if (strToNum(vParams[1], &fVal)) {
                    //printf("Filling Veg arrays with %f\n", fVal); 
                    for (unsigned int i = 0; i < m_iNumCells; i++) {
                        m_pClimate->m_adActualTemps[i]    = fVal;
                        m_pClimate->m_adAnnualMeanTemp[i] = fVal;
                    }
                    iResult = 0;
                } else {
                    stdfprintf(stderr, "Temp value should be a number: [%s]\n", join(vParams, " "));
                    iResult = -1;
                }
            } else {
                stdfprintf(stderr, "Expected value parameter for \"FLAT\"_ [%s]\n", join(vParams, " "));
                iResult = -1;
            }
        } else {
            stdfprintf(stderr, "Unknown command [%s]\n", join(vParams, " "));
            iResult = -1;
        }
    } else {
        // nothing to do is ok
        iResult = 0;
    }

    if (iResult != 0) stdfprintf(stderr, "handleTempLine: %d\n", iResult);
    return iResult;
}


//-----------------------------------------------------------------------------
// handleRainLine
//
int GridFactory::handleRainLine() {
    int iResult = -1;

    commandmap::const_iterator it = m_mSplitCommands.find(KEY_RAIN);
    if (it != m_mSplitCommands.end()) {
        stringvec vParams = it->second;
        stdprintf("----- handling RAIN [%s]\n", join(vParams, ":"));

        if (vParams[0].compare("NETCDF") == 0){
            if (vParams.size() > 2) {
                std::string sTemplate = stdsprintf("%s %%s %%s %s %s %%d %s", APP_PYTHON, DEF_CLI_PAT, "rain", PLACEHOLDER);
                iResult = createNETCDFCommands(vParams, APP_CLI_INTERP, DEF_CLI_DIR, sTemplate);

            } else {
                stdfprintf(stderr, "Unknown format for \"NETCDF\" [%s]\n", join(vParams, " "));
                iResult = -1;
            }


        } else  if (vParams[0].compare("FLAT") == 0) {
            if (vParams.size() > 1) {
                double fVal = 0;
                if (strToNum(vParams[1], &fVal)) {
                    //printf("Filling Veg arrays with %f\n", fVal); 
                    for (unsigned int i = 0; i < m_iNumCells; i++) {
                        m_pClimate->m_adActualRains[i]    = fVal;
                        m_pClimate->m_adAnnualRainfall[i] = fVal;
                    }
                    iResult = 0;
                } else {
                    stdfprintf(stderr, "Temp value should be a number: [%s]\n", join(vParams, " "));
                    iResult = -1;
                }
            } else {
                stdfprintf(stderr, "Expected value parameter for \"FLAT\"_ [%s]\n", join(vParams, " "));
                iResult = -1;
            }

        } else {
            stdfprintf(stderr, "Unknown command [%s]\n", join(vParams, " "));
            iResult = -1;
        }
    } else {
        // nothing to do is ok
        iResult = 0;
    }

    if (iResult != 0) stdfprintf(stderr, "handleRainLine: %d\n", iResult);

    return iResult;
}


//-----------------------------------------------------------------------------
// handleNPPLine
//
int GridFactory::handleNPPLine() {
    int iResult = -1;

    commandmap::const_iterator it = m_mSplitCommands.find(KEY_NPP);
    if (it != m_mSplitCommands.end()) {
        stringvec vParams = it->second;

        stdprintf("----- handling KEY_NPP: [%s]\n", join(vParams, ":"));
        iResult = -1;
        if (vParams[0].compare("NETCDF") == 0) {
            if (vParams.size() > 2) {
                std::string sTemplate = stdsprintf("%s %%s %%s %s %s %s %s %%d %s", APP_PYTHON, DEF_NPP_LON, DEF_NPP_LAT, DEF_NPP_TIME, DEF_NPP_NPP, PLACEHOLDER);
                iResult = createNETCDFCommands(vParams, APP_NPP_INTERP, DEF_NPP_FILE, sTemplate);
            } else {
                stdfprintf(stderr, "Unknown format for \"NETCDF\" [%s]\n", join(vParams, " "));
                iResult = -1;
            }
 
        } else  if (vParams[0].compare("FLAT") == 0) {
            double fVal = 0;
            if (strToNum(vParams[1], &fVal)) {
                //printf("Filling Veg arrays with %f\n", fVal); 
                for (unsigned int i = 0; i < m_iNumCells; i++) {
                    m_pVeg->m_adBaseANPP[i] = fVal;
                    m_pVeg->m_adTotalANPP[i] = fVal;
                }
                iResult = 0;
            } else {
                stdfprintf(stderr, "Expected value parameter for \"FLAT\"\n");
                iResult = -1;
            }
        } else {
            stdfprintf(stderr, "Unknown command [%s]\n", join(vParams, " "));
            iResult = -1;
        }
    } else {
        // nothing to do is ok
        iResult = 0;
    }
    if (iResult != 0) stdfprintf(stderr, "handleNPPLine: %d\n", iResult);

    return iResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// grid creation
//////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// setGrid
//
int GridFactory::setGrid(const stringvec &vParams) {
    int iResult = -1;

    if (vParams[0].compare("ICO") == 0)  {
        iResult = setGridIco(vParams);
    } else  {
        iResult = setGridFlat(vParams);
    }

    if (iResult == 0) {
        m_pClimate = new Climate(m_pCG, m_iNumCells, 0);
        m_pCG->setClimate(m_pClimate);
        m_pVeg = new Vegetation(m_pCG, m_iNumCells, 0);
        m_pCG->setVegetation(m_pVeg);
        m_pNav = new Navigation(m_pCG);
        m_pCG->setNavigation(m_pNav);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// setGridIco
//
int GridFactory::setGridIco(const stringvec &vParams) {
    int iResult = -1;

    NodeIndex *pNI = NULL;
    int iSubDivs = -1;
    
    bool bEqual = true;
    
    double dRadius = RADIUS;
    
    if (vParams.size() > 1) {

        if (vParams.size() >= 2) {
            // <ignfile> or <type>:<subdiv>
            stringvec vArg;
            int iNum = splitString(vParams[1], vArg, ":");
            if (iNum == 1) {
               
            } else {
                if (strToNum(vArg[1], &iSubDivs)) {
                    iResult = 0;
                    if (vArg[0].compare("std") == 0)  {
                        bEqual = false;
                        stdprintf("Without tegmark\n");
                    } else if (vArg[0].compare("eq") == 0)  {
                        bEqual = true;
                        stdprintf("With tegmark\n");
                    } else {
                        stdprintf("expected  \"<type>:<subdivs>\"\n");
                        iResult = -1;
                    }
                } else {
                    stdprintf("expected  number for \"<subdivs>\", not [%s]\n", vArg[1]);
                    iResult = -1;
                }
            }
            if (vParams.size() == 3) {
                // trailing <radius>
                if (strToNum(vParams[2], &dRadius)) {
                    m_dRadius = dRadius;
                }
            } else if (vParams.size() > 3) {
                stdprintf("too many arguments\n");
                iResult = -1;
            } 
            
        } else {
            // too many parameters
            iResult = -1;
        }
         
       
    } else {
        stdprintf("need parameters\n");
        iResult = -1;
    }


    if (iResult == 0) {
        if (iSubDivs > 0)  {
            EQsahedron *pEQNodes = EQsahedron::createInstance(iSubDivs, bEqual);
            pEQNodes->relink();
            VertexLinkage *pVL = pEQNodes->getLinkage();
            pNI = createEQGrid(pVL);
            m_iNumCells = pNI->getNumNodes();
            delete pEQNodes;
            iResult = 0;
        }

        // build grid
        if (iResult == 0) {
            stringmap smSurfaceData;
            smSurfaceData[SURF_TYPE] = SURF_EQSAHEDRON;
            smSurfaceData[SURF_IEQ_SUBDIVS] = stdsprintf("%d", iSubDivs);
            m_pCG = new SCellGrid(0, m_iNumCells, smSurfaceData);
            iResult = createCells(pNI);
            
        }
        
        // build geography
        if (iResult == 0) {
            m_pGeo = new Geography(m_pCG, m_iNumCells, m_pCG->m_iMaxNeighbors, m_dRadius);  // create geography
            m_pCG->setGeography(m_pGeo);
            initializeGeography(pNI);
        }
        
        delete pNI;

    }


    return iResult;
}


//-----------------------------------------------------------------------------
// setGridFlat
//   ("HEX" | "RECT") <iW>"x"<iH> ["PERIODIC"]
//
int GridFactory::setGridFlat(const stringvec &vParams) {
    int iResult = -1;

    uint iPeriodicity = PERIODIC_NONE;
    bool bHex = false;

    m_iNumCells = 0;

    printf("params (%zd):", vParams.size());
    for (uint i = 0; i < vParams.size(); i++) {
        printf("  %s", vParams[i].c_str());
    }
    printf("\n");
    if (vParams.size() > 1) {
        int iW = 0;
        int iH = 0;

        if (vParams.size() > 2) { 
            if (vParams[2].compare("PERIODIC_NONE") == 0) {
                iPeriodicity = PERIODIC_NONE;
            } else if (vParams[2].compare("PERIODIC_X") == 0) {
                iPeriodicity = PERIODIC_X;
            } else if (vParams[2].compare("PERIODIC_Y") == 0) {
                iPeriodicity = PERIODIC_Y;
            } else if (vParams[2].compare("PERIODIC_XY") == 0) {
                iPeriodicity = PERIODIC_XY;
            } else if (vParams[2].compare("PERIODIC") == 0) {
                iPeriodicity = PERIODIC_XY;
            }
        }

        // find size
        stringvec vParts;
        uint iNum = splitString(vParams[1], vParts, "x");
        if (iNum == 2) {
            if (strToNum(vParts[0], &iW) && strToNum(vParts[1], &iH)) {
                m_iNumCells = iW*iH;
                iResult = 0;
                stdfprintf(stderr, "got iW %d, iH %d -> %d\n", iW, iH, m_iNumCells);

            } else {
                stdfprintf(stderr, "invalid number in  size: [%s]\n", vParams[2]);
            }
        } else {
            stdfprintf(stderr, "invalid size: [%s]\n", vParams[2]);
        }

        // find type 
        if (iResult == 0) {
            if (vParams[0].compare("HEX") == 0) {
                bHex = true;
                if (((iPeriodicity & PERIODIC_Y) != 0)&&((iH%2) == 1)) {
                    stdfprintf(stderr,"[GridFactory::readDef] WARNING: You need an even of rows for a regularY-periodic HEX grid\n");
                    stdfprintf(stderr,"[GridFactory::readDef] Using an odd number of rows results in a grid with torsion!\n");
                    
                }
            } else if (vParams[0].compare("RECT") == 0) {
                bHex = false;
            } else {
                stdfprintf(stderr,"[GridFactory::readDef] unknown grid type  [%s]\n", vParams[1]);
                iResult = -1;
            }
        }

        // build grid
        if (iResult == 0) {

            stringmap smSurfaceData;

            smSurfaceData[SURF_TYPE] = "LTC";
            smSurfaceData[SURF_LTC_W] = stdsprintf("%d", iW);
            smSurfaceData[SURF_LTC_H] = stdsprintf("%d", iH);
            smSurfaceData[SURF_LTC_LINKS] = stdsprintf("%d", bHex?HEX_NEIGHBORS:RECT_NEIGHBORS);
            smSurfaceData[SURF_LTC_PERIODIC] = sPeriodicityNames[iPeriodicity];

            smSurfaceData[SURF_LTC_PROJ_TYPE] = stdsprintf("%d [Linear] %f %f %d", PR_LINEAR, 0.0, 0.0, 0);

            double dW = (bHex) ? (iW - 0.5) : (iW - 1);
            double dH = (bHex) ? (iH - 1) * 0.8660254 : (iH - 1);
            smSurfaceData[SURF_LTC_PROJ_GRID] = stdsprintf("%d %d %lf %lf %lf %lf %lf", iW-1, iH-1, dW, dH, 0.0, 0.0, 1.0);

            m_pCG = new SCellGrid(0, m_iNumCells, smSurfaceData);
            // initialize the cells depending for flat grids
            iResult = createCells(iW, iH, iPeriodicity, bHex);
            
        }

        // build geography
        if (iResult == 0) {
            geonumber dRadius = 0.0;
            int iMaxNeigh =bHex ? HEX_NEIGHBORS : RECT_NEIGHBORS;
            m_pGeo = new Geography(m_pCG, m_iNumCells, iMaxNeigh, dRadius);  // create geography
            m_pCG->setGeography(m_pGeo);
            initializeGeography(iW, iH, bHex);
            
        }
    } else {
        stdfprintf(stderr, "Invalid flat grid definition [%s]\n", join(vParams, " "));
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// createCells
//   create cells
//   link cells
//
int GridFactory::createCells(int iW, int iH, uint iPeriodicity, bool bHex) {  // THIS IS FOR RECT OR HEX GRID
    //printf("[GridFactory::createCells] %dx%d, p:%d, t:%s\n", iW, iH, iPeriodicity, bHex?"Hex":"Rect");

    int iResult = 0;
    m_pCG->m_aCells = new SCell[m_iNumCells];
    
    // use simplest ID scheme
    for(uint i=0; i<m_iNumCells; ++i) {
        m_pCG->m_aCells[i].m_iGlobalID = i;
        m_pCG->m_mIDIndexes[i] = i;
    }
    
    if (bHex) {
        iResult = createCellsHex(iW, iH, iPeriodicity);
    } else {
        iResult = createCellsRect(iW, iH, iPeriodicity);
    }

    return iResult;
 
}



//-----------------------------------------------------------------------------
// createCellsRectPeriodic
//   link cells
//
int GridFactory::createCellsRect(uint iW, uint iH, uint iPeriodicity) {
    int iResult = 0;
    printf("[GridFactory::createCellsRect] %dx%d, p:%d\n", iW, iH, iPeriodicity);
    
    iResult =  createCellsRectPeriodic(iW, iH);
    
    // now remove bad neighbors if nnot periodic 
    //x, y, W and H should be ints, because some intermediate results can be negative-
    int W = (int)(iW);
    int H = (int)(iH);
   
    // remove links forbidden by non-periodicity in x direction (left and right border elements=
    printf("[GridFactory::createCellsRect] iPeriodicity:%d, PERIODIC_X:%d. &:%d\n", iPeriodicity, PERIODIC_X, iPeriodicity & PERIODIC_X);
    if ((iPeriodicity & PERIODIC_X) == 0) {
        printf("[GridFactory::createCellsRect] removing horizontal border links\n");
        
        for (int y = 0; y < H; y++) {
            int x = W-1;
            int i = y*W + x;
            m_pCG->m_aCells[i].m_aNeighbors[0] = -1; 
            m_pCG->m_aCells[i].m_iNumNeighbors--;
            
            x = 0;
            i = y*W + x;
            m_pCG->m_aCells[i].m_aNeighbors[2] = -1; 
            m_pCG->m_aCells[i].m_iNumNeighbors--;
        }
    }
    // remove links forbidden by non-periodicity in y direction (top and bottom border elements=
    // here we must make sure we don't double-delete

    printf("[GridFactory::createCellsRect] iPeriodicity:%d, PERIODIC_Y:%d, &:%d\n", iPeriodicity, PERIODIC_Y, iPeriodicity & PERIODIC_Y);
    if ((iPeriodicity & PERIODIC_Y) == 0) {
        printf("[GridFactory::createCellsRect] removing vertical horizontal border links\n");
        // top
        int y = H - 1;   
        for (int x = 0; x < W; x++) {
            int i = y*W + x;
            m_pCG->m_aCells[i].m_aNeighbors[1] = -1; 
            m_pCG->m_aCells[i].m_iNumNeighbors--;   
        }
   
        // bottom
        y = 0;   
        for (int x = 0; x < W; x++) {
            int i = y*W + x;
            m_pCG->m_aCells[i].m_aNeighbors[3] = -1; 
            m_pCG->m_aCells[i].m_iNumNeighbors--;
        }   
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// createCellsRectPeriodic
//   link cells
//
int GridFactory::createCellsRectPeriodic(uint iW, uint iH) {
    printf("[GridFactory::createCellsRectPeriodic] %dx%d\n", iW, iH);
    for(uint i=0; i<m_iNumCells; i++) {

        // clear all neighbor links
        m_pCG->m_aCells[i].m_iNumNeighbors = (uchar)RECT_NEIGHBORS;
        for (int j = 0; j < m_pCG->m_iMaxNeighbors; j++) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }

        int x  = i % iW;
        int y  = i / iW;
        int x1 = x;
        int y1 = y;

        //    1
        //  2 X 0
        //    3

        // dir 0: x' = (x+1)%W, y' = y
        // dir 1: x' = x, y' = (y+1)%H     
        // dir 2: x' = (x-1+W)%W, y' = y   ("+iW" to make the result positive)
        // dir 3: x' = x, y' = (y-1+H)%H   ("+iH" to make the result positive)
        // let's first deal with boundary rows and up-down neighbors:
        
        // dir 0:
        x1 = (x+1)%iW;
        y1 = y;
        m_pCG->m_aCells[i].m_aNeighbors[0] = y1*iW + x1;
        // dir 1:
        x1 = x;
        y1 = (y+1)%iH;
        m_pCG->m_aCells[i].m_aNeighbors[1] = y1*iW + x1;
        // dir 2:
        x1 = (x-1+iW)%iW;
        y1 = y;
        m_pCG->m_aCells[i].m_aNeighbors[2] = y1*iW + x1;
        // dir 3:
        x1 = x;
        y1 = (y-1+iH)%iH;
        m_pCG->m_aCells[i].m_aNeighbors[3] = y1*iW + x1;

    }
    return 0;
}


//-----------------------------------------------------------------------------
// createCellsHex
//   create hexagonal grid of cells with optionl non-periodicity
//
//   PERIODIC_X: the links 1 and 2 in the top row and 
//               the links 4 and 5 in the bottom row must be deleted
//
//   PERIODIC_Y: in even-numbered rows the links 2,3 and 4 of the leftmost elements
//               and link 0 of the rightmost elements must be deleted.
//               in odd-numbered rows the link 3 of the leftmodt element,
//               and the links 0, 1 and 5 of the rightmst element must be deleted.
//
int GridFactory::createCellsHex(uint iW, uint iH, uint iPeriodicity) {
  
    int iResult = 0;
    //printf("[GridFactory::createCellsHex] %ux%u, p:%d\n", iW, iH, iPeriodicity);

    iResult = createCellsHexPeriodic(iW, iH);

    // now remove bad neighbors if nnot periodic 
    //x, y, W and H should be ints, because some intermediate results can be negative-
    int W = (int)(iW);
    int H = (int)(iH);
   
    
    //  . . . . 
    //  3rd row:     X X X X X X 
    //  2nd row:    X X X X X X 
    //  1st row:     X X X X X X
    //  0th row:    X X X X X X 


    // remove links forbidden by non-periodicity in x direction (left and right border elements=
    if ((iPeriodicity & PERIODIC_X) == 0) {
        printf("[GridFactory::createCellsHex] removing horizontal border links\n");
        
        for (int y = 0; y < H; y++) {
            int x = 0;
            int i = y*W + x;
            
            if ((y%2) == 1) {
                m_pCG->m_aCells[i].m_aNeighbors[3] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
            } else {
                m_pCG->m_aCells[i].m_aNeighbors[2] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
                m_pCG->m_aCells[i].m_aNeighbors[3] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
                m_pCG->m_aCells[i].m_aNeighbors[4] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
            }
            x = W-1;
            i = y*W + x;
            if ((y%2) == 1) {
                m_pCG->m_aCells[i].m_aNeighbors[0] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
                m_pCG->m_aCells[i].m_aNeighbors[1] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
                m_pCG->m_aCells[i].m_aNeighbors[5] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
            } else {
                m_pCG->m_aCells[i].m_aNeighbors[0] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
            }
        }
    }

    // remove links forbidden by non-periodicity in y direction (top and bottom border elements=
    // here we must make sure we don't double-delete

    if ((iPeriodicity & PERIODIC_Y) == 0) {
        printf("[GridFactory::createCellsHex] removing vertical border links\n");
        // top
        int y = H - 1;   
        for (int x = 0; x < W; x++) {
            int i = y*W + x;
            if (m_pCG->m_aCells[i].m_aNeighbors[1]>= 0) {
                m_pCG->m_aCells[i].m_aNeighbors[1] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;   
            }
            if (m_pCG->m_aCells[i].m_aNeighbors[2]>= 0) {
                m_pCG->m_aCells[i].m_aNeighbors[2] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;   
            }
        }   
        // bottom
        y = 0;   
        for (int x = 0; x < W; x++) {
            int i = y*W + x;
            if (m_pCG->m_aCells[i].m_aNeighbors[4]>= 0) {
                m_pCG->m_aCells[i].m_aNeighbors[4] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
            }
            if (m_pCG->m_aCells[i].m_aNeighbors[5]>= 0) {
                m_pCG->m_aCells[i].m_aNeighbors[5] = -1; 
                m_pCG->m_aCells[i].m_iNumNeighbors--;
            }  
        }   
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// createCellsHexPeriodic
//   create cells
//   link cells
//
//    2 1       NW NE
//   3 X 0     W  X  E
//    4 5       SW SE
// 
// connecctions:
// row 4   - X - X - X - X-       E - X - W       |
//           |\  |\  |\  |\           |\          |
//          \| \ | \ | \ |            | \         |
// row 2   - X - X - X - X -      NW  NE          |
//           |  /|  /|  /|          \ |           |
//         \ | / | / | / |/          \|           |
// row 1   - X - X - X - X-       E - X - W       |
//           |\  |\  |\  |\           |\          |
//           | \ | \ | \ | \          | \         |
// row 0   - X - X - X - X -         SW  SE       |
//          /|  /|  /|  /|
//
//               even rows     odd rows
//         0     x+1, y        x+1, y         0,0
//         1     x,   y+1      x+1, y+1       1,0  
//         2     x-1, y+1      x,   y+1       1,0
//         3     x-1, y        x-1, y         1,0
//         4     x-1, y-1      x,   y-1       1,0
//         5     x,   y-1      x+1, y-1     
//
int GridFactory::createCellsHexPeriodic(uint iW, uint iH) {

    //printf("[GridFactory::createCellsHexPeriodic] %ux%u\n", iW, iH);
    //x, y, W and H should be ints, because some intermediate results can be negative-
    int W = (int)(iW);
    int H = (int)(iH);
   
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

            int i = y*W + x;
            m_pCG->m_aCells[i].m_iNumNeighbors = 6;
            int iX;
            int iY;

            // E and W are the same for all rows
            iX = (x+1) % W;
            iY = y;
            m_pCG->m_aCells[i].m_aNeighbors[0] = iY*W + iX;

            iX = (x-1+W) % W;
            iY = y;
            m_pCG->m_aCells[i].m_aNeighbors[3] = iY*W + iX;

            // for the other directions, odd and even differ
            iX = (x +   W + y%2) % W;
            iY = (y+1 + H) % H;
            m_pCG->m_aCells[i].m_aNeighbors[1] = iY*W + iX;

            iX = (x-1 + W + y%2) % W;
            iY = (y+1 + H) % H;
            m_pCG->m_aCells[i].m_aNeighbors[2] = iY*W + iX;

            iX = (x-1 + W + y%2) %W;
            iY = (y-1 + H) % H;
            m_pCG->m_aCells[i].m_aNeighbors[4] = iY*W + iX;

            iX = (x +   W + y%2) % W;
            iY = (y-1 + H) % H;
            m_pCG->m_aCells[i].m_aNeighbors[5] = iY*W + iX;
        }
    }
    return 0;
}










//-----------------------------------------------------------------------------
// createNETCDFCommands
//
int GridFactory::createNETCDFCommands(stringvec &vParams, const std::string sApp, const std::string sDefaultFile, const std::string sCommandTemplate) {
    int iResult = -1;
    int    iTime;

    if  (vParams.size() > 2) {
        if (strToNum(vParams[2], &iTime)) {
            std::string sRealFile = "";
            if (vParams[1].compare("DEFAULT") == 0) {
                vParams[1] = sDefaultFile;
            }
            if (exists(vParams[1], sRealFile)) {
                // call alt interpolator
                std::string sCommand;
                std::string sRealCommand;
                if (exists(sApp, sRealCommand)) {
                    sCommand = stdsprintf(sCommandTemplate, sRealCommand, sRealFile, iTime);
                    m_vShellCommands.push_back(sCommand);
                    iResult = 0;
                } else {
                    stdfprintf(stderr, "Couldnt find path for [alt_interpolator]\n");
                    iResult = -1;
                }
            } else {
                iResult = -1;
                stdfprintf(stderr, "File [%s] does not exist\n", sRealFile);
            }
        } else {
            iResult = -1;
            stdfprintf(stderr, "Expected integer time in command [%s]\n", join(vParams, " "));
        }
    } else {
        iResult = -1;
        stdfprintf(stderr, "Expected 3 parameters [%s]\n", join(vParams, " "));
    }        
    return iResult;
}




//-----------------------------------------------------------------------------
// applyShellCommands
//   apply the various netcdf-interpolation commands
//   to the existing QWDF file
//
int GridFactory::applyShellCommands(const char *pQDFFile) {
    int iResult = 0;
    if (m_vShellCommands.size() > 0) {
        for (uint i = 0; (iResult == 0) && (i <m_vShellCommands.size()); i++) {
            std::string sComm(m_vShellCommands[i]);
            size_t start_pos = sComm.find(PLACEHOLDER);
            if(start_pos != std::string::npos) {
                sComm.replace(start_pos, PLACEHOLDER.length(), pQDFFile);
                stdprintf("Command %02u: [%s]\n", i, sComm);
                stringvec vOutputLines;
                iResult =  executeCommand(sComm, vOutputLines);
                if (iResult != 0) {
                    for (unsigned int i = 0; i < vOutputLines.size(); i++) {
                        stdprintf("%s\n", vOutputLines[i]);
                    }
                }
            }
        }    
    }
    return iResult;
}




//-----------------------------------------------------------------------------
// initializeGeography
// for HEX and RECT grids
int GridFactory::initializeGeography(int iX, int iY, bool bHex) {
    int iResult = 0;

    float fDeltaY = bHex ? 0.8660254 : 1;
    float fOddRowOffset = bHex ? 0.5 : 0;

    for (uint i=0; i<m_iNumCells; i++) {
        unsigned int y = i / iX;
        unsigned int x = i % iX;
        m_pGeo->m_adLongitude[i] = (double)x + (y % 2) * fOddRowOffset;;
        m_pGeo->m_adLatitude[i] = (double)y * fDeltaY;
        // the neighbor arrays are arranged sequentially into a big 1-d array
        int i0 = m_pGeo->m_iMaxNeighbors*i;
        for (uint j=0; j< m_pGeo->m_iMaxNeighbors; j++) {
                m_pGeo->m_adDistances[i0+j] = 1;
        }

        m_pGeo->m_adAltitude[i] = 0;
        m_pGeo->m_adArea[i] = 1;
        m_pGeo->m_abIce[i] = false;
        m_pGeo->m_adWater[i] = 0;
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initializeGeography
//  create Geography, set
//   Longitude
//   Latitude
//   Distances
//   Area
//
int GridFactory::initializeGeography(NodeIndex *pNI) {

    int iResult = 0;

    bool bDeg2Rad = true;
    // rectangular grids with linear "projection" should not 
    // have their coordinates modified
    // printf("Testing type of IGN surface:[%s]\n", m_pCG->m_smSurfaceData[SURF_TYPE].c_str());
    if (m_pCG->m_smSurfaceData[SURF_TYPE].compare(SURF_LATTICE) == 0) {
        stdprintf("  --> is lattice\n");
        iResult = -1;
        const std::string &sPT = m_pCG->m_smSurfaceData[SURF_LTC_PROJ_TYPE];
        stringvec vParts;
        uint iNum = splitString(sPT, vParts, " ");
        stdprintf("PROJ type  --> [%s]\n", sPT);
        
        int iPT = 0;
        if ((iNum > 0) && (strToNum(vParts[0], &iPT))) { 
            if (iPT == PR_LINEAR) {
                stdprintf("have LINEAR\n");
                bDeg2Rad = false;
            }
        }
    }
    
    if (iResult == 0) {
        for (uint i=0; i<m_iNumCells; ++i) {
            gridtype iIndex = m_pCG->m_aCells[i].m_iGlobalID;  // for each cell find its ID
            IcoNode* pIN = pNI->m_mNodes[iIndex];           // get the corresponding iconode in pNI

       
            if(pIN != NULL) {
                m_pGeo->m_adAltitude[iIndex] = 0;

                m_pGeo->m_adLatitude[iIndex]  =  pIN->m_dLat;
                m_pGeo->m_adLongitude[iIndex] =  pIN->m_dLon;
                if (bDeg2Rad) {
                    m_pGeo->m_adLatitude[iIndex]  *=  180/M_PI;
                    m_pGeo->m_adLongitude[iIndex] *=  180/M_PI;
                }
                // the neighbor arrays are arranged sequentially into a big 1-d array
                int i0 = m_pGeo->m_iMaxNeighbors*iIndex;
                for (int j = 0; j < pIN->m_iNumLinks; j++) {
                    m_pGeo->m_adDistances[i0+j] = pIN->m_adDists[j];
                }
                m_pGeo->m_adArea[iIndex] = pIN->m_dArea;
                m_pGeo->m_abIce[i] = false;
                m_pGeo->m_adWater[i] = 0;
                m_pGeo->m_abCoastal[i] = false;

            } else {
                stdfprintf(stderr,"[GridFactory::setGeography] node of index %d not found\n",iIndex);
                iResult = -1;
            }
        }
    } else {
        stdfprintf(stderr,"[GridFactory::setGeography] couldn't read projection details\n");
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// setDensity
//
 int GridFactory::setDensity(int iIndex, double dD) {
    int iResult = 0;

    // nothing to do
    
    return iResult;
}

//-----------------------------------------------------------------------------
// setNPP
//
 int GridFactory::setNPP(int iIndex, double dN) {
    int iResult = 0;

    // nothing to do
    

    return iResult;
}





//-----------------------------------------------------------------------------
// createCells
//   create cells
//   link cells
//
int GridFactory::createCells(NodeIndex *pNI) { // THIS IS FOR ICOSAHEDRON GRID
 
    LOG_STATUS("[GridFactory::createCells] allocating %d cells\n", m_iNumCells);
    
    uint iC = 0;
    m_pCG->m_aCells = new SCell[m_iNumCells];
    std::map<gridtype, IcoNode*>::const_iterator it;
    for (it = pNI->m_mNodes.begin(); it != pNI->m_mNodes.end(); ++it) {
        m_pCG->m_mIDIndexes[it->first]=iC;

        m_pCG->m_aCells[iC].m_iGlobalID    = it->first;
        m_pCG->m_aCells[iC].m_iNumNeighbors = (uchar)it->second->m_iNumLinks;
        //        pCF->setGeography(m_pGeography, iC, it->second);
        iC++;
    }
    if (iC != m_iNumCells) {
        stdfprintf(stderr, "[GridFactory::createCells] numcells: %d, actually set %dn", m_iNumCells, iC);
    }
    LOG_STATUS("[GridFactory::createCells] linking cells\n");

    // linking and distances
    for (uint i =0; i < m_iNumCells; ++i) {
        // get link info from IcCell
        IcoNode *pIN = pNI->m_mNodes[m_pCG->m_aCells[i].m_iGlobalID];
        for (int j = 0; j < pIN->m_iNumLinks; ++j) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = m_pCG->m_mIDIndexes[pIN->m_aiLinks[j]];
        }
        for (int j = pIN->m_iNumLinks; j < m_pCG->m_iMaxNeighbors; ++j) {
            m_pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }
    }
    return 0;
}

