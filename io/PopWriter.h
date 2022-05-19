#ifndef __POPWRITER_H__
#define __POPWRITER_H__

#include <map>
#include <vector>
#include <string>
#include <hdf5.h>

class PopBase;

const int PW_NONE            = 0;
const int PW_AGENTS_ONLY     = 1;
const int PW_STATS_ONLY      = 2;
const int PW_ADDITIONAL_ONLY = 4;
const int PW_ALL             = 7;

typedef std::map<PopBase *, hid_t> poptypes; 

class PopWriter {
public:
    PopWriter(std::vector<PopBase *> vPops);
    ~PopWriter();
    
    int write(const std::string sFilename, int iStep, float fStartTime, const std::string sInfoString, const std::string sSub, int iWSpecial, int iDumpMode=-1);
    int write(hid_t hFile, const std::string sSub, int iWSpecial, int iDumpMode=-1); 
    int openPopulationGroup() { return opencreatePopGroup();};
    void closePopulationGroup() { H5Gclose(m_hPopGroup);};
protected:
    poptypes m_mDataTypes;
    hid_t    m_hFile;

    
    hid_t    m_hPopGroup;
    hid_t    m_hSpeciesGroup;
   
    int opencreatePopGroup();
    int opencreateSpeciesGroup(PopBase *pPB, int iDumpMode);
    
};


#endif
