/*============================================================================
| gzutils 
| 
|  Wrappers for gzip und gunzip functionality
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __GZUTILS_H__
#define __GZUTILS_H__

#include <string>
#include "types.h"


class gzUtils {
public:
    
    static gzUtils *createInstance(uint iBlockSize);
    int init(uint iBlockSize);
    ~gzUtils();

    int do_gzip(const std::string sInput, const std::string sOutput);
    int do_gunzip(const std::string sInput, const std::string sOutput);

    //static const uint  BLOCK_SIZE{0x7fffffff}; // == 2147483647
    static const uint  BLOCK_SIZE{0x70000000}; // == 1879048192


protected:
    gzUtils();
    uint   m_iBlockSize;
    uchar *m_pBlock;
};

#endif
