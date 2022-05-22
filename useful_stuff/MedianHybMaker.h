#ifndef __MEDIANHYBMAKER_H__
#define __MEDIANHYBMAKER_H__

#include <string>
#include <vector>

#include "hdf5.h"

#include "types.h"


class MedianHybMakerBase {
public:
    MedianHybMakerBase():m_iNumCells(0),m_pMedians(NULL){};
    virtual ~MedianHybMakerBase(){if (m_pMedians != NULL) {delete[] m_pMedians;} };

    virtual int calc_medians()   = 0;
    virtual int write_medians()  = 0;

    float *getMedians() { return m_pMedians;};
    uint getNumCells() { return m_iNumCells;};
protected:
    int   m_iNumCells;
    float *m_pMedians;
};



template<typename T>
class MedianHybMaker : public MedianHybMakerBase {
public:
    static MedianHybMaker<T> *create_instance(const std::string sQDFFile, const std::string sSpecies);
    
    virtual ~MedianHybMaker();

    virtual int collect_agent_hybs();
    virtual int calc_medians();
    virtual int write_medians();

    enum class agent_type {TYPE_HYB, TYPE_YMT};
protected:
    MedianHybMaker();
    int init(const std::string sQDFFile, const std::string sSpecies);

    hid_t create_compound_data_type();
    int   merge_vectors();
  
    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;
    int   m_iNumThreads;

    std::vector<float> *m_avHybValues; //  numcells array of float vector 

    uchar         *m_pAgents;
    
};


#endif
