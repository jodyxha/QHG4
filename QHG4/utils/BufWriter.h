/*============================================================================
|  BufWriter
|
|   A BufWriter object provides buffered file output.
|   Characters, character arrays and szrings can be added to the buffer.
|   Once the buffer is full, it is writtem to file.
|   
\===========================================================================*/

#ifndef __BUFWRITER_H__
#define __BUFWRITER_H__

#include <cstdio>
#include <string>

const int DEF_BUF_SIZE = 2048;

class BufWriter {
public:
    static BufWriter *createInstance(const std::string sOut, int iBufSize=DEF_BUF_SIZE);

    ~BufWriter();
    int addChars(const char *pData, int iNum);
    int addChar(char c);
    int addLine(const char *pLine);
    long getPos() { return m_lPos;};

protected:
    BufWriter();

    int initialize(const std::string sOut, int iBufSize);
    int dump();
    
    char *m_pBuffer;
    int   m_iBufSize;
    int   m_iCur;
    FILE *m_fOut;
    long  m_lPos;
};


#endif
