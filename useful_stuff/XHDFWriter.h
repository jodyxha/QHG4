#ifndef __XHDFWRITER_H__
#define __XHDFWRITER_H__

#include <map>
#include <string>

#include <hdf5.h>

#include "AgentXItemCollector.h"
#include "CellSampler.h"

typedef std::map<std::string, std::pair<int,int>> loc_landwater; 

template<typename A>
class XHDFWriter {
public:
    static XHDFWriter<A> *createInstance(const std::string sFileName, uint iNumRegions, field_data_vec &vFieldInfos);

    ~XHDFWriter();
    int writeData(const std::string sSimName, uchar *pData, int iNumVals);
    int writeDataRegionized(const std::string sSimName, int iStep, float fStartTime, uchar *pData, uint iNumAgs, const loc_cells& mLocCells, loc_landwater &mvLandWater);
protected:
    XHDFWriter(field_data_vec &vFieldInfo);
    int init(const std::string sFileName, uint iNumArrays);
    hid_t createCompoundDataType();

    hid_t m_hFile;
    hid_t m_hRoot;
    hid_t m_hAgentDataType;    
    bool  m_bVerbose;

    field_data_vec m_vFieldInfos;

};




#endif



