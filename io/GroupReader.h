#ifndef __GROUPREADER_H__
#define __GROUPREADER_H__

#include <string>
#include <hdf5.h>

struct Attributes {
    uint m_iNumCells;
};

template<class T, typename U>
class GroupReader {

public:
    virtual ~GroupReader();

    virtual int readAttributes(U *pAttributes);
    
    // these should be implemented by derived classes
    virtual int readArray(T *pGroup, const std::string sArrayName)=0;
    virtual int readData(T *pGroup)=0;
 
protected:
    GroupReader();

    // these should be implemented by derived classes
    virtual int init(const std::string sFileName, const std::string sGroupName);
    virtual int init(hid_t hFile, const std::string sGroupName);
    virtual int tryReadAttributes(U *pAttributes);

    hid_t  m_hFile;
    hid_t  m_hGroup;
    uint   m_iNumCells;
    U     *m_pAttributes;
    bool   m_bCloseHFile;
   
};




#endif
