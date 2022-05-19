/*============================================================================
| gzutils 
| 
|  Wrappers for gzip und gunzip functionality
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstdio>
#include <zlib.h>

#include "gzutils.h"

//----------------------------------------------------------------------------
// constructor
//
gzUtils::gzUtils()
    : m_iBlockSize(0),
      m_pBlock(NULL) {
}


//----------------------------------------------------------------------------
// destructor
//
gzUtils::~gzUtils() {
    if (m_pBlock != NULL) {
        delete[] m_pBlock;
    }
}


//----------------------------------------------------------------------------
// createInstance
//
gzUtils *gzUtils::createInstance(uint iBlockSize) {
    gzUtils *pgz = new gzUtils;
    if (pgz != NULL) {
        int iResult = pgz->init(iBlockSize);
        if (iResult != 0) {
            delete pgz;
            pgz = NULL;
        }
    }
    return pgz;
}


//----------------------------------------------------------------------------
// init
//
int gzUtils::init(uint iBlockSize) {
    int iResult = 0;
    m_iBlockSize = iBlockSize;
    m_pBlock = new uchar[m_iBlockSize];
    if (m_pBlock == NULL) {
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// do_gzip
//
int gzUtils::do_gzip(const std::string sInput, const std::string sOutput) {
    int iResult = 0;
    FILE *fIn = fopen(sInput.c_str(), "rb");
    if (fIn != NULL) {
        gzFile fOut = gzopen(sOutput.c_str(), "wb");
        if (fOut != NULL) {
            while ((iResult == 0) && !feof(fIn)) {
                uint iRead = fread(m_pBlock, 1, m_iBlockSize, fIn);
                if (iRead > 0) {
                    uint iWrite = gzwrite(fOut, m_pBlock, iRead);
                    if (iWrite != iRead) {
                        printf("Write error: read [%d] but wrote[%d]\n", iRead, iWrite);
                        iResult = -1;
                    }
                }
            }
            gzclose(fOut);
        } else {
            printf("Couldn't open [%s] for writing\n", sOutput.c_str());
            iResult = -1;
        }
        fclose(fIn);
    } else {
        printf("Couldn't open [%s] for reading\n", sInput.c_str());
        iResult = -1;
    }

    return iResult;
}



//----------------------------------------------------------------------------
// do_gunzip
//
int gzUtils::do_gunzip(const std::string sInput, const std::string sOutput) {
    int iResult = 0;

    gzFile fIn = gzopen(sInput.c_str(), "rb");
    if (fIn != NULL) {
        FILE *fOut = fopen(sOutput.c_str(), "wb");
        if (fOut != NULL) {
            while ((iResult == 0) && !gzeof(fIn)) {
                uint iRead = gzread(fIn, m_pBlock, m_iBlockSize);
                if (iRead > 0) {
                    uint iWrite = fwrite(m_pBlock, 1, iRead, fOut);
                    if (iWrite != iRead) {
                        printf("Write error: read [%d] but wrote[%d]\n", iRead, iWrite);
                        iResult = -1;
                    }
                }
            }
            fclose(fOut);
        } else {
            printf("Couldn't open [%s] for writing\n", sOutput.c_str());
            iResult = -1;
        }
        gzclose(fIn);
    } else {
        printf("Couldn't open [%s] for reading\n", sInput.c_str());
        iResult = -1;
    }

    return iResult;
}
