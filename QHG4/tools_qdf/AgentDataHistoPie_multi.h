#ifndef __AGENTDATAHISTOPIE_MULTI_H__
#define __AGENTDATAHISTOPIE_MULTI_H__

#include <string>
#include <vector>
#include <map>

#include "LineReader.h"
#include "Sampling.h"
#include "PieWriter_multi.h"

const std::string SAMP_CELL_RANGE  = "CellRangeSampling";
const std::string SAMP_COORD_RANGE = "CoordRangeSampling";
const std::string SAMP_GRID_RANGE  = "GridRangeSampling";
const std::string SAMP_FULL        = "FullSampling";
const std::string SAMP_EACH        = "EachSampling";

template<typename T>
using multiindexedvals = std::map<std::string, indexedvals<T>>;

template<typename T>
using multigroupedvals = std::map<std::string, groupedvals<T>>;

class AgentDataHistoPie_multi {
public:
    static AgentDataHistoPie_multi *createInstance(std::string sQDFInputFile, std::string sPopName, stringvec &vDataItemName, bool bVerbose=false);
    virtual ~AgentDataHistoPie_multi();

    int createSampling(const std::string sSamplingInfo); 
    int createHisto(const std::string sBinInfo);

    int writeOutput(const std::string sWhat, const std::string sOutputFile, bool bstd);

    int setVerbosity(bool bVerbosity) { bool bOldV =m_bVerbose; m_bVerbose = bVerbosity; return bOldV;};
protected:
    AgentDataHistoPie_multi(std::string sQDFInputFile, std::string sPopName, stringvec &vDataItemName, bool bVerbose=false);

    int init();
    int extractData();

    Sampling *createCellRangeSampling(LineReader *pLR);
    Sampling *createCoordRangeSampling(LineReader *pLR);
    Sampling *createGridRangeSampling(LineReader *pLR);

    int writePie(const std::string sQDFOutputFile);
    int writeTxt(const std::string sTxtOutputFile, bool bstd);
    int writeCSV(const std::string sCSVOutputFile, bool bstd);

    double **fillCoordMap(uint *piNumCells, uint *piNumDims,  bool *pbSpherical);
    std::string m_sQDFInputFile;
    std::string m_sPopName;
    stringvec m_vDataItemNames;

    double m_dMinVal;
    double m_dMaxVal;
    uint   m_iNumBins;


    multiindexedvals<double> m_vIndexedVals;;
    multigroupedvals<double> m_vGroupedVals;

    uint   m_iNumCells;
    uint   m_iNumDims;
    bool m_bSpherical;
                      
    double **m_apCoords;

    multimaphistos m_mHistos;

    bool m_bVerbose;
};


#endif
