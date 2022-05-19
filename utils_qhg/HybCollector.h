#ifndef __HYBCOLLECTOR_H__
#define __HYBCOLLECTOR_H__

#include <string>
#include <hdf5.h>



#include "types.h"

// agent structure for Y chromosome + MT dna  
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

// agent structure for simple hybridisation
typedef struct aginfo_t {
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fHybridization;
} aginfo;


class HybCollector {
public:
    static HybCollector  *create_instance(const std::string sFile, uint iNumBins);

    ~HybCollector();

    int collect_hybridisations();

    int *getData() { return m_aiHisto;};
    uint getNumBins()    {return m_iNumBins;};    
    uint getNumRegions() {return m_vRegionNames.size();};
    uint getNumSteps()   {return m_vStepNames.size();};    
    uint getNumSims()    {return m_vSimNames.size();};    
    const stringvec getRegionNames() {return m_vRegionNames;};
    const stringvec getStepNames()   {return m_vStepNames;};    
    const stringvec getSimNames()   {return m_vSimNames;};    

    void show_names();
    void show_data();
protected:
    HybCollector(uint iNumBins);
    int init(const std::string sFile);
    int collect_sim_names();
    int collect_step_names();
    int collect_region_names(hid_t hStep);
    int readAgentDataHDF(uint iStep, uint iRegion, hid_t hDataSet, void *pBuffer, uint iBufSize);

    
    uint      m_iNumBins;
    uint      m_iNumSteps;
    uint      m_iNumRegions;
    stringvec m_vStepNames;
    stringvec m_vRegionNames;
    stringvec m_vSimNames;


    int    *m_aiHisto; // one array for each thread
    hid_t m_hFile;
    hid_t m_hSim;

    hid_t m_hAgentType;
    float m_fMin;
    float m_fMax;
};


#endif
