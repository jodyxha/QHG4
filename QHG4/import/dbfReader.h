#ifndef __DBFHEADER_H__
#define __DBFHEADER_H__

#include <cstdio>
#include <string>
#include <map>
#include <vector>

// the size of a shape buffer

const int DBF_HEADER_SIZE  = 32;
const int DBF_DESC_SIZE    = 32;

typedef std::vector<double> vecdouble;
typedef std::map<std::string, std::pair<int, int> > nameoffsets;

class dbfReader {
public:
    dbfReader(FILE *fIn);

    int read(const std::string sFieldName, vecdouble &vVals);
    void display(const char *pCaption);

    int   getNumRecords() const { return m_iNumRecords;};
    short getHeaderSize() const { return m_iHeaderSize;};
    short getRecordSize() const { return m_iRecordSize;};
    const nameoffsets &getOffsets() { return m_mOffsets; };
protected:
    int readHeader();
    int readFieldDescriptors();
    int readRecords(int iFieldOffset, int iLen, vecdouble &vVals);
    FILE *m_fIn;

    int    m_iNumRecords;
    short  m_iHeaderSize;
    short  m_iRecordSize;
    
    nameoffsets m_mOffsets;
};

#endif

