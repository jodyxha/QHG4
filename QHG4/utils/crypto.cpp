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

//#include <cstdio>
#include <istream>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include "crypto.h"
#include <string>
#include <fstream>

const int BLOCK_SIZE = 32768;



//----------------------------------------------------------------------------
// shasum
//
int crypto::shasum(const std::string sFile, unsigned char *pSHAsum) {
    int iResult = -1;

    char aBuf[BLOCK_SIZE];

    std::ifstream fIn (sFile, std::ifstream::binary);
    if (fIn.good()) {
        SHA_CTX c;
        SHA1_Init(&c);
        while (fIn.good()) {
            memset(aBuf, 0, BLOCK_SIZE);
            fIn.read(aBuf, BLOCK_SIZE);
            SHA1_Update(&c, aBuf, fIn.gcount());
        }
        
        SHA1_Final(pSHAsum, &c);
        iResult = 0;
        fIn.close();
    } else {
        memset(pSHAsum, 0, SHA_SIZE);
        iResult = -1;
    }


 
    return iResult;

}


//----------------------------------------------------------------------------
// shasumS
//
int crypto::shasumS(const std::string sString, unsigned char *pSHAsum) {
    int iResult = -1;

    SHA_CTX c;
    SHA1_Init(&c);
    SHA1_Update(&c, sString.c_str(), sString.length());
    SHA1_Final(pSHAsum, &c);
    iResult = 0;

    return iResult;

}


//----------------------------------------------------------------------------
// md5sum
//
int crypto::md5sum(const std::string sFile, unsigned char *pMD5sum) {
    int iResult = -1;

    char aBuf[BLOCK_SIZE];

    std::ifstream fIn(sFile, std::ifstream::binary);
    if (fIn.good()) {
        MD5_CTX c;
        MD5_Init(&c);
        while (fIn.good()) {
            fIn.read(aBuf, BLOCK_SIZE);
            MD5_Update(&c, aBuf, fIn.gcount());
        }

        MD5_Final(pMD5sum, &c);
        iResult = 0;
        fIn.close();
    } else {
        memset(pMD5sum, 0, MD5_SIZE);
        iResult = -1;
    }

    return iResult;

}


//----------------------------------------------------------------------------
// md5sumS
//
int crypto::md5sumS(const std::string sString, unsigned char *pMD5sum) {
    int iResult = -1;

    MD5_CTX c;
    MD5_Init(&c);
    MD5_Update(&c, sString.c_str(), sString.length());
    MD5_Final(pMD5sum, &c);
    iResult = 0;

    return iResult;

}


//----------------------------------------------------------------------------
// ripsum
//
int crypto::ripsum(const std::string sFile, unsigned char *pRIPsum) {
    int iResult = -1;

    char aBuf[BLOCK_SIZE];
    std::ifstream fIn(sFile, std::ifstream::binary);
    if (fIn.good()) {
        RIPEMD160_CTX c;
        RIPEMD160_Init(&c);
        while (fIn.good()) {
            fIn.read(aBuf, BLOCK_SIZE);
            RIPEMD160_Update(&c, aBuf, fIn.gcount());
        }

        RIPEMD160_Final(pRIPsum, &c);
        iResult = 0;
        fIn.close();
    } else {
        memset(pRIPsum, 0, RIP_SIZE);
        iResult = -1;
    }

    return iResult;

}


//----------------------------------------------------------------------------
// ripsumS
//
int crypto::ripsumS(const std::string sString, unsigned char *pRIPsum) {
    int iResult = -1;

    RIPEMD160_CTX c;
    RIPEMD160_Init(&c);
    RIPEMD160_Update(&c, sString.c_str(), sString.length());
    RIPEMD160_Final(pRIPsum, &c);
    iResult = 0;

    return iResult;

}


   
