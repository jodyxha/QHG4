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
#include <openssl/evp.h>

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
// shasumSNew
//
int crypto::shasumSNew(const std::string sString, unsigned char *pSHAsum) {
    int iResult = -1;

    EVP_MD_CTX *mdctx    = NULL;
    const EVP_MD *pDigestType;

    mdctx = evp_init(DIGEST_SHA, &pDigestType); 
    if (mdctx != NULL) {

        iResult = evp_update(mdctx, pDigestType, (unsigned char *)sString.c_str(), sString.length());
        if (iResult == 0) {
            unsigned int iLen = -1;
            iResult = evp_final(mdctx, &pSHAsum, &iLen);
        } else {
            //
        }
    }
                     

    return iResult;

}



//----------------------------------------------------------------------------
// md5sum
//
int crypto::md5sum(const std::string sFile, unsigned char *pMD5sum) {
    int iResult = -1;

    char aBuf[BLOCK_SIZE];

    std::ifstream fIn(sFile, std::ifstream::binary);
    if (fIn.good())  {
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
// md5sumNew
//
int crypto::md5sumNew(const std::string sFile, unsigned char *pMD5sum) {
    int iResult = -1;

    EVP_MD_CTX *mdctx    = NULL;
    const EVP_MD *pDigestType;
    
    unsigned int iLen = 0;
    char aBuf[BLOCK_SIZE];
    memset(pMD5sum, 0, MD5_SIZE);

    mdctx = evp_init(DIGEST_MD5, &pDigestType); 
    if (mdctx != NULL) {
        
        std::ifstream fIn(sFile, std::ifstream::binary);
        if (fIn.good()) {
            iResult = 0;
            while ((iResult == 0) && (fIn.good())) {
                fIn.read(aBuf, BLOCK_SIZE);
                evp_update(mdctx, pDigestType, (unsigned char *) aBuf, fIn.gcount());
                iLen += fIn.gcount();
            }
            if (iResult == 0) {
                evp_final(mdctx, &pMD5sum, &iLen);
                fIn.close();
            }
        } else {
            fprintf(stderr, "Couldn't open file for reading [%s]\n", sFile);
            iResult = -1;
        }

    } else {
        fprintf(stderr, "Couldn't create digest context");
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
// md5sumSNew
//
int crypto::md5sumSNew(const std::string sString, unsigned char *pMD5sum) {
    int iResult = -1;

    EVP_MD_CTX *mdctx    = NULL;
    const EVP_MD *pDigestType;

    mdctx = evp_init(DIGEST_MD5, &pDigestType); 
    if (mdctx != NULL) {

        iResult = evp_update(mdctx, pDigestType, (unsigned char *)sString.c_str(), sString.length());
        if (iResult == 0) {
            unsigned int iLen = -1;
            iResult = evp_final(mdctx, &pMD5sum, &iLen);
        } else {
            //
        }
    }
                     

    return iResult;

}

/*
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
*/


//----------------------------------------------------------------------------
// evp_init
//
EVP_MD_CTX *crypto::evp_init(int iDigestType, const EVP_MD **ppDigestType ) {
    EVP_MD_CTX *mdctx = NULL;
    //*ppDigestType = NULL;

    printf("in: %d, MD5:  %d, SHA: %d\n", iDigestType, DIGEST_MD5, DIGEST_SHA);
    switch (iDigestType) {
    case DIGEST_MD5: 
        *ppDigestType = EVP_md5();
        break;
    case DIGEST_SHA: 
        *ppDigestType = EVP_sha256();
        break;
    default:
        // unhandled digest
        // do nothing
        *ppDigestType = NULL;
    }

    if (*ppDigestType != NULL) {
        mdctx = EVP_MD_CTX_new();

	if (mdctx != NULL) {
            int iTempRes = EVP_DigestInit_ex(mdctx, *ppDigestType, NULL);
            if (iTempRes == 1) {
                // everything ok; do nothing
            } else {
                // couldn't initialize; free resources
                EVP_MD_CTX_free(mdctx);
                mdctx = NULL;
            }
        } else {
            // couldn't create conmtext
        }
    }
    return  mdctx;
}

//----------------------------------------------------------------------------
// evp_update
//
int crypto::evp_update(EVP_MD_CTX *mdctx, const EVP_MD *pDigestType, const unsigned char *pMessage, size_t iMessageLen) {
    int iResult = -1;
    int iTempRes =  EVP_DigestUpdate(mdctx, pMessage, iMessageLen);
    if (iTempRes == 1) {
        iResult = 0;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// evp_final
//
int crypto::evp_final(EVP_MD_CTX *mdctx, unsigned char **ppDigest, unsigned int *piDigestLen) {
    int iResult = -1;
    //    *ppDigest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha256()));
    if (*ppDigest != NULL) {
        int iTempRes =  EVP_DigestFinal_ex(mdctx, *ppDigest, piDigestLen);
        if (iTempRes == 1) {
            iResult = 0;
        }
    } else {
        // couldn't allocate
    }
    EVP_MD_CTX_free(mdctx);
    return iResult;
}

/*
int digest_message(const unsigned char *pMessage, size_t iMessageLen, unsigned char **ppDigest, unsigned int *piDigestLen, int iDigestType) {
    int iResult = 0;

    EVP_MD *pDigestType = NULL;

    switch (iDigestType) {
    DIGEST_MD5: 
        pDigestType = EVP_md5();
        break;
    DIGEST_SHA: 
        pDigestType = EVP_sha256();
        break;
    default:
        // unhandled digest
        iResult = -1;
    }
        
    if (iResult == 0) {
	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

	if (mdctx != NULL) {
            int iTempRes = EVP_DigestInit_ex(mdctx, pDigestType, NULL);
            if (iTempRes == 1) {
                iTempRes = EVP_DigestUpdate(mdctx, pMessage, iMessageLen);
                if (iTempRes == 1) {
                    *ppDigest =  (unsigned char *)OPENSSL_malloc(EVP_MD_size(pDigestType));
                    if (*ppDigest != NULL)  {

                        iTempRes = EVP_DigestFinal_ex(mdctx, *ppDigest, piDigestLen);
                        if (iTempRes == 1) {
                            iResult = 0;
                        } else {
                            // couldn't do DigestFinal
                            iResult = -1;
                        }
                    } else {
                        // couldn't allocate space
                        iResult = -1;
                    }
                } else {
                    // couldn't update digest 
                    iResult = -1;
                }
            } else {
            // couldn't initialize 
                iResult = -1;
            }

            EVP_MD_CTX_free(mdctx);

        } else {
            // couldn't create context
            iResult = -1;
        }
    }

}

*/
void handleErrors() {

}


void crypto::digest_message(const unsigned char *message, size_t message_len, unsigned char **digest, unsigned int *digest_len)
{
	EVP_MD_CTX *mdctx;

	if((mdctx = EVP_MD_CTX_new()) == NULL)
		handleErrors();

	if(1 != EVP_DigestInit_ex(mdctx, EVP_md5(), NULL))
		handleErrors();

	if(1 != EVP_DigestUpdate(mdctx, message, message_len))
		handleErrors();

	if((*digest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_md5()))) == NULL)
		handleErrors();

	if(1 != EVP_DigestFinal_ex(mdctx, *digest, digest_len))
		handleErrors();

	EVP_MD_CTX_free(mdctx);
}
