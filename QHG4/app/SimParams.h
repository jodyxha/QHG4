#ifndef __SIMPARAMS_H__
#define __SIMPARAMS_H__


#include <string>
#include "utils.h"
#include "WELL512.h"
#include "SPopulation.h"
 
class ParamReader;
class EventData;
class EventManager;
class LineReader;

class SCellGrid;
class Geography;
class Climate;  
class Vegetation;
class Navigation;
class PopulationFactory;
class PopBase;
class PopLooper;
class StatusWriter;
class IDGen;
class EnvInterpolator;
class AutoInterpolator;
class GridScrambler;


const std::string DEF_LOG_FILE              = "SimTest.log";
const std::string DEF_OUT_PREFIX            = "output_";

// structure to hold event data
typedef struct eventdata {
    char *pEventType;
    char *pEventParams;
    char *pEventTimes;
    eventdata(char *pType, char *pParams, char *pTimes) {
        pEventType = new char[strlen(pType)+1];
        strcpy(pEventType,  pType);
        pEventParams = new char[strlen(pParams)+1];
        strcpy(pEventParams,  pParams);
        pEventTimes = new char[strlen(pTimes)+1];
        strcpy(pEventTimes,  pTimes);
    };
    ~eventdata() {
        delete[] pEventType;
        delete[] pEventParams;
        delete[] pEventTimes;
    };
} eventdata;


class Surface;

class SimParams {
public:
    SimParams();
    virtual ~SimParams();

    int readOptions(int iArgC, char *apArgV[]);
    static void helpParams();
    static void showTopicHelp(const std::string sTopic);

    const std::string &getHelpTopic() { return m_sHelpTopic;};

    int addEventTriggers(char *pEventDescription, bool bUpdateEventList);

    static std::string addEventName(std::string &sEventString);

protected:
    int          m_iNumIters;
    int          m_iLayerSize;
    bool m_bHelp;
    hid_t m_hFile;
    //@@  to be removed    LineReader *m_pLRGrid;

    ParamReader *m_pPR;       
    SCellGrid   *m_pCG;       
    Geography   *m_pGeo;      
    Climate     *m_pCli;      
    Vegetation  *m_pVeg;      
    Navigation  *m_pNav;      
    PopLooper   *m_pPopLooper;
    IDGen      **m_apIDG;

    EnvInterpolator *m_pEnvInt;
    AutoInterpolator *m_pAutInt;

    StatusWriter *m_pSW;
    EventManager *m_pEM;
    
    Surface           *m_pSurface;
    PopulationFactory *m_pPopFac;

    bool m_bZipOutput;
    bool m_bDeleteOrig;

    bool  m_bResume;
    bool  m_bMergePops;

    int   m_iStartStep;
    float m_fStartTime;
    int   m_iInterpolStep;

    std::string m_sHelpTopic;
    std::string m_sOutputPrefix;
    std::string m_sOutputDir;
    std::string m_sConfigOut;
    
    std::string m_sInfoString;
    
    uint32_t  m_aulState[STATE_SIZE];
    uint      m_aiSeeds[NUM_SEEDS];
    stringvec m_vDataDirs;
    stringvec m_vSODirs;
    std::vector<eventdata*>   m_vEvents;

    bool m_bCalcGeoAngles;
    bool m_bTrackOcc;
    bool m_bDynPops;
    //@@    int  m_iGridSeed;

    GridScrambler *m_pGridScrambler;
    int    setHelp(bool bHelp);
    int    setHelpTopic(const std::string sHelpTopic);


    int setGrid(const std::string sFile);
    
    int setGeo(const std::string sFile);

    int setClimate(const std::string sFile);

    int setVeg(const std::string sFile);

    int setNav(const std::string sFile);

    int setPopList(const std::string sList);
    int setPops(const std::string sFile);


    // -- event parsing --
    //int processEvents(bool bUpdateEventList);
    //int readEventsFromFile(char *pEventFile);
    //int readEventFromString(char *pEventString);
    //int checkEvents();

    // -- input data --
    int setGrid(hid_t hFile, bool bUpdate = false);
    //@@ to be removed     int setGridFromDefFile(const std::string sDefFile);

    int setGeo(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setGeoFromDefFile(const char *pDefFile);

    int setClimate(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setClimateFromDefFile(const char *pDefFile);

    int setVeg(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setVegFromDefFile(const char *pDefFile);

    int setNav(hid_t hFile, bool bUpdate);

    int setPops(hid_t hFile, const std::string sPopName, bool bRequired);
    int setPopsFromPopFile(const std::string sClsFile, const std::string sDataFile);
    int setPopsFromXMLFile(const std::string sXMLFile, const std::string sDataFile);

    int setPopParams(const std::string sParams);

    int setOutputPrefix(const std::string sOutputPrefix);
    int setOutputDir(const std::string sOutputDir);
    int setDataDirs(const std::string sDataDirs);
    int setSODirs(const std::string sSODirs);
    int setConfigOut(const std::string sConfigOut);

    int setInterpolationData(std::string sInterpolationData);

    int setSeed(const std::string sSeed);

    int readSeed(const std::string sFileName, std::vector<uint32_t> &vulState);
    int setShuffles(const std::string sShuffles);


    bool exists(const std::string sFile, std::string &sExists);
    int  createDir(const std::string sDirName);

    int createSurface();
    int readAgentData(PopBase *pPop, const std::string sAgentDataFile);
 
    int setLayerSize(uint iLayerSize);
};


#endif

