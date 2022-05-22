#ifndef __AGENTXITEMCOLLECTOR_H__
#define __AGENTXITEMCOLLECTOR_H__

#include <map>
#include <string>
#include "hdf5.h"

#include "types.h"
#include "QDFUtils.h"

class field_data {
public:
    std::string m_sNameIn;
    std::string m_sNameOut;
    size_t      m_iOffset;
    hid_t       m_hType;
    field_data(std::string sNameIn, std::string sNameOut,  size_t iOffset, hid_t hType) : m_sNameIn(sNameIn), m_sNameOut(sNameOut), m_iOffset(iOffset), m_hType(hType) {};
};

typedef std::vector<field_data> field_data_vec;

class AgentXItemCollector {
public:
    static AgentXItemCollector *createInstance(const std::string sPopQDF, const std::string sSpecies, field_data_vec &vFieldInfo, size_t iStructSize);
    ~AgentXItemCollector();

    int init(const std::string sPopQDF, const std::string sSpecies, field_data_vec &vFieldInfo, size_t iStructSize);    

    uchar *getArray() { return m_pInfos;};
    uint getNumValues() { return m_iNumAgents;};


    int    getCurStep()   { return m_iCurStep;};
    float  getStartTime() { return m_fStartTime;};

protected:
    AgentXItemCollector();

    int loadAgentsCell(const std::string sPopQDF, const std::string sSpecies, field_data_vec &vFieldInfo, size_t iStructSize);

    hid_t createCompoundDataType(field_data_vec &vFieldInfo, size_t iStructSize);
    uchar *createItemArray();
    int getItemNames(const std::string sQDFPop, const std::string sSpecies);
    int checkItemType(const std::string sPopQDF, const std::string sSpecies, const std::string sItemName, const  std::string sDType);

    int getRootAttributes(hid_t hFile);

    uchar         *m_pInfos;
    
    stringmap      m_mItemNames;


    uint  m_iNumAgents;
 
    int            m_iCurStep;
    float          m_fStartTime;
};




#endif
