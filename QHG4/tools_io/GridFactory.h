#ifndef __GRIDFACTORY_H__
#define __GRIDFACTORY_H__

#include <string>
#include <vector>
#include <map>
#include "types.h"
#include "LineReader.h"
#include "SCellGrid.h"
#include "Climate.h"
#include "Vegetation.h"
#include "Navigation.h"

typedef std::map<std::string, stringvec>   commandmap;

class NodeIndex;

class GridFactory {
public:
    GridFactory(const std::string sDefFile);
    GridFactory();
    ~GridFactory();


    //bool isReady() { return (m_pLR != NULL) || (m_pLine != NULL);};
    bool isReady() { return true;};
    int readDef();
    int applyShellCommands(const char *pQDFFile);
    

    SCellGrid* getCellGrid()  { return m_pCG; };
    Geography* getGeography() { return m_pGeo; };
    Climate* getClimate()     { return m_pClimate; };
    Vegetation* getVeg()      { return m_pVeg;};

    uint getNumShellCommands()  { return m_vShellCommands.size();};
    stringvec &getShellCommands()  { return m_vShellCommands;};

    bool isFromFile() {return m_bFromFile;}; 

protected:
    uint m_iNumCells;

    bool exists(const std::string sFile, std::string &sExists);
    
    int setDataDirs(const char *pDataDirs);
    int setDataDirs(const stringvec &vDataDirs);

    int groupLine(char *pLineList);
    int splitCommand(std::string sLine);
    int collectLines();

    int setGrid(const stringvec &vParams);
    int setGridIco(const stringvec &vParams);
    int setGridFlat(const stringvec &vParams);

    int createCells(NodeIndex *pNI);
    int createCells(int iW, int iH, uint iPeriodicity, bool bHex);
  
    int createCellsHex(uint iW, uint iH, uint iPeriodicity);
    int createCellsHexPeriodic(uint iW, uint iH);
 
    int createCellsRect(uint iW, uint iH, uint iPeriodicity);
    int createCellsRectPeriodic(uint iW, uint iH);
 
    int initializeGeography(NodeIndex *pNI);
    int initializeGeography(int iW, int iH, bool bHex);

    int createNETCDFCommands(stringvec &vParams, const std::string sApp, const std::string sDefaultFile, const std::string sCommandTemplate);

    int handleGridLine();
    int handleDDirLine();
    int handleAltLine();
    int handleIceLine();
    int handleNPPLine();
    int handleRainLine();
    int handleTempLine();

    int setDensity(int iIndex, double dD);
    int setNPP(int iIndex, double dA);

    SCellGrid  *m_pCG;
    Geography  *m_pGeo;
    Climate    *m_pClimate;
    Vegetation *m_pVeg;
    Navigation *m_pNav;

    stringvec   m_vDataDirs;
    LineReader *m_pLR;
    std::string m_sDef; 

    stringvec   m_vShellCommands;
    bool        m_bInterpol;
    commandmap  m_mSplitCommands;
    bool        m_bFromFile;

    double      m_dRadius;
};

#endif
