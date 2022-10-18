#ifndef __YMTHDFWRITER_H__
#define __YMTHDFWRITER_H__

#include <map>
#include <string>

#include <hdf5.h>

#include "AgentYMTCollector.h"
#include "CellSampler.h"

typedef std::map<std::string, std::pair<int,int>> loc_landwater; 

class YMTHDFWriter {
public:
    static YMTHDFWriter *createInstance(const std::string sFileName, uint iNumRegions);

    ~YMTHDFWriter();
    int writeData(const std::string sSimName, uchar *pData, int iNumVals);
    int writeDataRegionized(const std::string sSimName, int iStep, float fStartTime, uchar *pData, uint iNumAgs, const loc_cells& mLocCells, loc_landwater &mvLandWater);
protected:
    YMTHDFWriter();
    int init(const std::string sFileName, uint iNumArrays);
    hid_t createCompoundDataType();

    hid_t m_hFile;
    hid_t m_hRoot;
    hid_t m_hAgentDataType;    
    bool  m_bVerbose;
};




#endif



