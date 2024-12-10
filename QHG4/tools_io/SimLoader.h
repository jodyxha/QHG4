#ifndef __SIMLOADER_H__
#define __SIMLoader_H__

#include <string>
#include "qhg_consts.h"
#include "SPopulation.h"

const int         LAYERSIZE    =  65536;
const std::string DEF_LOG_FILE =  "SimSimp.log";

class ParamReader;
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

class OccTracker;

class Surface;

class SimLoader {
public:
    SimLoader();
    virtual ~SimLoader();

    int readOptions(int iArgC, char *apArgV[]);
    static void helpParams();
    static void showTopicHelp(const std::string sTopic);

    const std::string &getHelpTopic() { return m_sHelpTopic;};
    void showInputs();
    int write();

    int mergePops();

protected:
    int          m_iLayerSize;
    bool m_bHelp;
    hid_t m_hFile;
    LineReader *m_pLRGrid;

    ParamReader *m_pPR;       
    SCellGrid   *m_pCG;       
    Geography   *m_pGeo;      
    Climate     *m_pCli;      
    Vegetation  *m_pVeg;      
    Navigation  *m_pNav;      
    PopLooper   *m_pPopLooper;
    IDGen      **m_apIDG;

    StatusWriter *m_pSW;
    
    Surface           *m_pSurface;
    PopulationFactory *m_pPopFac;
    OccTracker        *m_pOcc;




    std::string m_sDesc;
    std::string m_sOutputQDF;
    std::string m_sHelpTopic;
    
    char *m_pInfoString;
    
    uint32_t  m_aulState[STATE_SIZE];
    uint      m_aiSeeds[NUM_SEEDS];

    stringvec m_vDataDirs;

    bool m_bCalcGeoAngles;

    int    setHelp(bool bHelp);
    int    setHelpTopic(const std::string sHelpTopic);

    int setOutputQDF(const std::string sFile);

    int setGrid(const std::string sFile);
    
    int setGeo(const std::string sFile);

    int setClimate(const std::string sFile);

    int setVeg(const std::string sFile);

    int setNav(const std::string sFile);

    int setPopList(std::string sList);
    int setPops(const std::string sFile);


    // -- input data --
    int setGrid(hid_t hFile, bool bUpdate = false);
    int setGridFromDefFile(const std::string sDefFile);

    int setGeo(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setGeoFromDefFile(const std::string sDefFile);

    int setClimate(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setClimateFromDefFile(const std::string sDefFile);

    int setVeg(hid_t hFile, bool bRequired, bool bUpdate = false);
    int setVegFromDefFile(const std::string sDefFile);

    int setNav(hid_t hFile, bool bUpdate);

    int setPops(hid_t hFile, const std::string sPopName, bool bRequired);
    //    int setPopsFromPopFile(const std::string sClsFile, const std::string sDataFile);
    int setPopsFromXMLFile(const std::string sXMLFile, const std::string sDataFile);

    int setPopParams(const std::string sParams);

    int setDataDirs(const std::string sDataDirs);


    bool exists(const std::string sFile, std::string &sExists);
    int  createDir(const std::string sDirName);

    int createSurface();
    int readAgentData(PopBase *pPop, const std::string sAgentDataFile);

    int writeState(int iWhat);
 
};


#endif

