#ifndef __QDFARRAY_H__
#define __QDFARRAY_H__

#include <vector>
#include <string>
#include <hdf5.h>

#include "types.h"
#include "QDFUtilsT.h"

#define NUM_GROUPS 4

class QDFArray {
public:
    static QDFArray *create(const std::string sQDFFile);
    static QDFArray *create(hid_t hFile);

    ~QDFArray();

    int init(const std::string sQDFFile);
    int init(hid_t hFile);

    int openArray(const std::string sPathToDataset);
    int openArray(const std::string sGroup,  const std::string sDataset);
    int openArray(const std::string sGroup1, const std::string sGroup2, const std::string sDataset);
    int openArray(stringvec &vGroups, const std::string sDataSet);

    uint getSize() { return m_iArraySize;};


    template<typename T>
    int getFirstSlab(T *pBuffer, const uint iSize, const std::string sFieldName="");
    
    template <typename T>
    int getNextSlab(T *pBuffer, const uint iSize);


    void  closeArray();
    float getTimeStep() {return m_iStep;};



protected:
    QDFArray();


    //    template<typename T>
    //    int getFirstSlab(T *pBuffer, const uint iSize,/* hid_t hBaseType,*/ const std::string sFieldName);

    template <typename T>
    int readSlab(T *pBuffer);

    int setDataType(const std::string sFieldName, const hid_t hBaseType, const uint iSize);

    hid_t m_hFile;
    hid_t m_hRoot;
    hid_t m_hDataSet;
    hid_t m_hDataSpace;
    hid_t m_hMemSpace;
    hid_t m_hDataType;
    std::vector<hid_t> m_vhGroups;

    uint   m_iArraySize;
    
    hsize_t m_offset;
    hsize_t m_count;  
    
    hsize_t m_stride;
    hsize_t m_block;
    hsize_t m_memsize;
    int m_iBufSize;
    bool m_bDeleteDataType;
    int    m_iStep;

    bool m_bCloseHFile;
};

#endif
