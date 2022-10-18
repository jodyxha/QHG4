#ifndef __POPWRITER_H__
#define __POPWRITER_H__

#include <map>
#include <vector>
#include <string>
#include <hdf5.h>

class PopBase;

enum popwrite_flags {
    PW_NONE            = 0,
    PW_AGENTS_ONLY     = 1,
    PW_STATS_ONLY      = 2,
    PW_ADDITIONAL_ONLY = 4,
    PW_ALL             = 7,
};

inline popwrite_flags operator|(popwrite_flags &a, popwrite_flags b) {
    return static_cast<popwrite_flags>(static_cast<int>(a) | static_cast<int>(b));
}

inline popwrite_flags& operator|=(popwrite_flags &a, popwrite_flags b) {
    a = a | b;

    return a;
}


typedef std::map<PopBase *, hid_t> poptypes; 

class PopWriter {
public:
    PopWriter(std::vector<PopBase *> vPops);
    ~PopWriter();
    
    int write(const std::string sFilename, int iStep, float fStartTime, const std::string sInfoString, const std::string sSub, popwrite_flags iWSpecial, int iDumpMode=-1);
    int write(hid_t hFile, const std::string sSub, popwrite_flags iWSpecial, int iDumpMode=-1); 
    /*
    int openPopulationGroup() { return opencreatePopGroup();};
    void closePopulationGroup() { H5Gclose(m_hPopGroup);};
    */
protected:
    poptypes m_mDataTypes;
    hid_t    m_hFile;

    
    hid_t    m_hPopGroup;
    hid_t    m_hSpeciesGroup;
   
    int opencreatePopGroup();
    int opencreateSpeciesGroup(PopBase *pPB, int iDumpMode);
    
};


#endif
