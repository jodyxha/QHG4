#ifndef __AGENTDATAHISTOPIE_H__
#define __AGENTDATAHISTOPIE_H__

#include <string>
#include <vector>
#include <map>

#include "Sampling.h"
#include "PieWriter.h"


class AgentDataHistoPie {
public:
    static AgentDataHistoPie *createInstance(std::string sQDFInputFile, std::string sPopName, std::string sDataItemName, bool bVerbose=false);
    virtual ~AgentDataHistoPie();

    int createSampling(const std::string sSamplingInfo); 
    int createHisto(const std::string sBinInfo);
    int writePie(const std::string sQDFOutputFile);
    int writeText(const std::string sTxtOutputFile, bool bstd);
    int writeCSV(const std::string sCSVOutputFile, bool bstd);

    int setVerbosity(bool bVerbosity) { bool bOldV =m_bVerbose; m_bVerbose = bVerbosity; return bOldV;};
public:
    AgentDataHistoPie(std::string sQDFInputFile, std::string sPopName, std::string sDataItemName, bool bVerbose=false);

    int init();
    int extractData();

    double **fillCoordMap();
    std::string m_sQDFInputFile;
    std::string m_sPopName;
    std::string m_sDataItemName;

    double m_dMinVal;
    double m_dMaxVal;
    uint   m_iNumBins;


    indexedvals<double> m_vIndexedVals;;
    groupedvals<double> m_vGroupedVals;

    uint   m_iNumCells;
    uint   m_iNumDims;
    bool m_bSpherical;
                      
    double **m_apCoords;

    maphistos m_mHistos;

    bool m_bVerbose;
};


#endif
