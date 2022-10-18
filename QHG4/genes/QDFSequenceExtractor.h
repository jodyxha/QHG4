#ifndef __QDF_SEQUENCE_EXTRACTOR_H__
#define __QDF_SEQUENCE_EXTRACTOR_H__

#include <string>
#include <map>
#include <hdf5.h>
#include "types.h"

#include "SequenceProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"


template<typename T>
class QDFSequenceExtractor : public SequenceProvider<T> {

public:
    typedef std::map<idtype, T *>  sequencemap;

  
    virtual ~QDFSequenceExtractor();
    
    // use number & distance from file
    virtual int createSelection(const std::string sLocList, const std::string sRefFile, bool bDense, int iNumPerBuf, const std::string sSampIn, const std::string sSampOut);
    // specify number and distance
    virtual int createSelection(const std::string sLocList, int iNumSamp, double dSampDist, const std::string sRefFile, bool bDense, int iNumPerBuf, const std::string sSampIn, const std::string sSampOut);

    virtual int getSequenceSize();
    virtual const T *getSequence(idtype iID);
    virtual int setRange(int iFromBlock, int iNumBlocks);


    int                  getNumSelected() { return m_iNumSelected;};
    
    const loc_data      &getLocData() { return m_mLocData;};
    const sequencemap   &getSequences() { return m_mSequences;};
    const idset         &getSelectedIDs() { return m_sSelected;};
    const arrpos_ids    &getIndexIDMap() { return m_mSelected;};
    int                  getNumRefSelected() { return m_iNumRefSelected;};
    int                  getPloidy() { return m_iPloidy;};

    const loc_ids       &getLocIDs() {return m_mLocIDs;};

    const IDSample *getSample() const { return m_pCurSample;};
    const IDSample *getRefSample() const { return m_pRefSample;};
    
    void setVerbose(bool bVerbose) {m_bVerbose = bVerbose;};
protected:
    QDFSequenceExtractor(WELL512 *pWELL, bool bCartesian);
    int init(const std::string sGeoFile, 
             const std::string sQDFFile, 
             const std::string sSpeciesName, 
             const std::string sAttrSequenceSize, 
             const std::string sSequenceDataSetName);

    
    int getSelectedSequencesDense(const std::string sSequenceDataSetName, int iNumPerBuf);
    int getSelectedSequencesSparse(const std::string sSequenceDataSetName, int iNumPerBuf);

    virtual int extractAdditionalAttributes() = 0;
    virtual int calcNumBlocks() = 0;
    virtual int calcNumItemsInBlock() = 0;
    virtual hid_t readQDFSequenceSlab(hid_t hDataSet, hid_t hMemSpace, hid_t hDataSpace, T*aBuf)=0;

    std::string m_sPopName;
    std::string m_sQDFGeoFile;
    stringvec   m_vQDFPopFiles;
    std::string m_sSequenceDataSetName;

    int        m_iNumSelected;
    idset      m_sSelected;
    arrpos_ids m_mSelected;

    int        m_iNumRefSelected;
    idset      m_sRefSelected;
    arrpos_ids m_mRefSelected;

    IDSample  *m_pCurSample;
    IDSample  *m_pRefSample;

    loc_data   m_mLocData;    // location name -> (lon, lat, dist, num)
    

    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    int  m_iSequenceSize;
    uint m_iNumBlocks;
    int  m_iPloidy;

    sequencemap m_mSequences;
    loc_ids     m_mLocIDs;

    bool m_bCartesian;
    WELL512 *m_pWELL;
    bool m_bVerbose;

};


#endif
