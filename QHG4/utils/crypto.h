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

#include <openssl/evp.h>

#include <string>

namespace crypto {

    const int DIGEST_MD5 = 0;
    const int DIGEST_SHA = 1;

    const int MD5_SIZE = 16;
    const int SHA_SIZE = 32;
    //    const int RIP_SIZE = 20;

    // pSHAsum must point to buffer of size >= SHA_SIZE
    int shasum(const std::string sFile, unsigned char *pSHAsum);

    // pMD5sum must point to buffer of size >= MD5_SIZE
    int md5sum(const std::string sFile, unsigned char *pMD5sum);
    int md5sumNew(const std::string sFile, unsigned char *pMD5sum);
    
    // pRIPsum must point to buffer of size >= RIP_SIZE
    //    int ripsum(const std::string sFile, unsigned char *pRIPsum);

    // sha sum of single string, pSHAsum must point to buffer of size >= SHA_SIZE
    int shasumS(const std::string sString, unsigned char *pSHAsum);
    int shasumSNew(const std::string sString, unsigned char *pSHAsum);

    // md5 sum of single string, pMD5sum must point to buffer of size >= MD5_SIZE
    int md5sumS(const std::string sString, unsigned char *pMD5sum);
    int md5sumSNew(const std::string sString, unsigned char *pMD5sum);

    // rip sum of single string, pRIPsum must point to buffer of size >= RIP_SIZE
    //    int ripsumS(const std::string sString, unsigned char *pRIPsum);


    EVP_MD_CTX *evp_init(int iDigestType, const EVP_MD **ppDigestType );
    int evp_update(EVP_MD_CTX *mdctx, const EVP_MD *pDigestType, const unsigned char *pMessage, size_t iMessageLen);
    int evp_final(EVP_MD_CTX *mdctx, unsigned char **ppDigest, unsigned int *piDigestLen);
  
    void digest_message(const unsigned char *message, size_t message_len, unsigned char **digest, unsigned int *digest_len);


}

#endif
