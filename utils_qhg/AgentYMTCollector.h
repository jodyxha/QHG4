#ifndef __AGENTYMTCOLLECTOR_H__
#define __AGENTYMTCOLLECTOR_H__

#include <map>
#include <string>
#include "hdf5.h"

#include "types.h"
#include "QDFUtils.h"

typedef struct aginfo_ymt {
    aginfo_ymt(): m_ulID(0), m_ulCellID(0), m_iGender(0), m_fHybridization(0), m_iYchr(0), m_imtDNA(0) {};
    aginfo_ymt(idtype ulID, gridtype ulCellID, uchar iGender, float fHyb, uchar iYchr, uchar imtDNA): m_ulID(ulID), m_ulCellID(ulCellID), m_iGender(iGender), m_fHybridization(fHyb), m_iYchr(iYchr), m_imtDNA(imtDNA) {};
    idtype   m_ulID;
    gridtype m_ulCellID;
    uchar    m_iGender;
    float    m_fHybridization;
    uchar    m_iYchr;
    uchar    m_imtDNA;
} aginfo_ymt;
    

class AgentYMTCollector {
public:
    static AgentYMTCollector *createInstance(const std::string sPopQDF, const std::string sSpecies);
    ~AgentYMTCollector();

    int init(const std::string sPopQDF, const std::string sSpecies);
    
    //    int getDistr(double dMin, double dMax, int iNumBins, const char *pItemName);

    
    uint getNumValues() { return m_iNumAgents;};
    uchar *getYchrValues() { return m_iYchrArr;};
    uchar *getmtDNAValues() { return m_imtDNAArr;};
    uchar *getGenderValues() { return m_iGenderArr;};
    int   *getCellIDValues() { return m_iCellIDArr;};

    uchar *getArray() { return m_pInfos;};

    int    getCurStep()   { return m_iCurStep;};
    float  getStartTime() { return m_fStartTime;};
protected:
    AgentYMTCollector();

    int loadAgentsCell(const std::string sPopQDF, const std::string sSpecies);

    hid_t createCompoundDataType();
    void separateValues();
  
    int getRootAttributes(hid_t hFile);

    uchar         *m_pInfos;
    
    
    uint  m_iNumAgents;

    std::string    m_sDataType;
    int           *m_iCellIDArr;
    uchar         *m_iGenderArr;
    uchar         *m_iYchrArr;
    uchar         *m_imtDNAArr;
    stringmap      m_mItemNames;

    int            m_iCurStep;
    float          m_fStartTime;
};




#endif
