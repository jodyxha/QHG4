/*============================================================================
| crypto 
| 
|  Wrappers for hash algorithms
|  - sha-1
|  - md5
|  - ripemd160
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <string>

namespace crypto {

    const int MD5_SIZE = 16;
    const int SHA_SIZE = 20;
    const int RIP_SIZE = 20;

    // pSHAsum must point to buffer of size >= SHA_SIZE
    int shasum(const std::string sFile, unsigned char *pSHAsum);

    // pMD5sum must point to buffer of size >= MD5_SIZE
    int md5sum(const std::string sFile, unsigned char *pMD5sum);
    
    // pRIPsum must point to buffer of size >= RIP_SIZE
    int ripsum(const std::string sFile, unsigned char *pRIPsum);

    // sha sum of single string, pSHAsum must point to buffer of size >= SHA_SIZE
    int shasumS(const std::string sString, unsigned char *pSHAsum);

    // md5 sum of single string, pMD5sum must point to buffer of size >= MD5_SIZE
    int md5sumS(const std::string sString, unsigned char *pMD5sum);

    // rip sum of single string, pRIPsum must point to buffer of size >= RIP_SIZE
    int ripsumS(const std::string sString, unsigned char *pRIPsum);

}

#endif
