#include <istream>
#include <cstring>
#include <openssl/evp.h>

#include <string>
#include <fstream>
#include "CryptoDigest.h"

const int BLOCK_SIZE = 32768;


//----------------------------------------------------------------------------
// createInstance
//
CryptoDigest *CryptoDigest::createInstance(std::string sType) {
    CryptoDigest *pCD = new CryptoDigest;
    int iResult = pCD->init(sType);
    if (iResult != 0) {
        delete pCD;
        pCD = NULL;
    }
    return pCD;
}


//----------------------------------------------------------------------------
// constructor
//
CryptoDigest::CryptoDigest() 
    : m_mdctx(NULL),
      m_pDigestType(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
CryptoDigest::~CryptoDigest() {
    
    EVP_MD_CTX_free(m_mdctx);

}


//----------------------------------------------------------------------------
// init
//
int CryptoDigest::init(std::string sType) {
    int iResult = 0;
    // determine type
        
    if (sType == "md5") {
        m_pDigestType = EVP_md5();
    } else if (sType == "sha256") {
        m_pDigestType = EVP_sha256();
    } else if (sType == "sha512") {
        m_pDigestType = EVP_sha512();
    } else if (sType == "whirlpool") {
        m_pDigestType = EVP_whirlpool();
    } else {
        m_pDigestType = NULL;
        iResult = -1;
    }

    if (m_pDigestType != NULL) {

        m_mdctx =  EVP_MD_CTX_new();
        
        if (m_mdctx != NULL) {
            int iTempRes = EVP_DigestInit_ex(m_mdctx, m_pDigestType, NULL);
            if (iTempRes == 1) {
                // everything ok; do nothing
                iResult = 0;
            } else {
                // couldn't initialize; free resources
                EVP_MD_CTX_free(m_mdctx);
                m_mdctx = NULL;
                iResult = -1;
            }
        } else {
            // couldn't create context
            iResult = -1;
        }
    }
    return iResult;
}



//----------------------------------------------------------------------------
// addData
//  add a block of data()
//
int CryptoDigest::addData(unsigned char *pData, unsigned int iDataLen) {

    int iResult = -1;
    int iTempRes =  EVP_DigestUpdate(m_mdctx, pData, iDataLen);
    if (iTempRes == 1) {
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getDigest
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::getDigest(unsigned int *piDigestLen) {
    unsigned char *pOut = (unsigned char *)OPENSSL_malloc(EVP_MD_size(m_pDigestType));
    if (pOut != NULL) {
        int iTempRes =  EVP_DigestFinal_ex(m_mdctx, pOut, piDigestLen);
        if (iTempRes == 1) {
            // evrything ok - do nothing
        } else {
            // digest failed
            free(pOut);
            pOut = NULL;
        }   
    } else {
        // couldn't allocate
    }
    return pOut;
}


//----------------------------------------------------------------------------
// digestString
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::digestString(std::string sData, unsigned int *piDigestLen) {
    addData((unsigned char *)sData.c_str(), sData.length());
    return getDigest(piDigestLen);
}

// some static methods

//----------------------------------------------------------------------------
// md5sum_string
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::md5sum_string(std::string sData, unsigned int *piDigestLen) {
    return genericsum_string("md5", sData, piDigestLen);
}


//----------------------------------------------------------------------------
// sha256sum_string
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::sha256sum_string(std::string sData, unsigned int *piDigestLen) {
    return genericsum_string("sha256", sData, piDigestLen);
}


//----------------------------------------------------------------------------
// sha512sum_string
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::sha512sum_string(std::string sData, unsigned int *piDigestLen) {
    return genericsum_string("sha512", sData, piDigestLen);
}


//----------------------------------------------------------------------------
// whirlpoolsum_string
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::whirlpoolsum_string(std::string sData, unsigned int *piDigestLen) {
    return genericsum_string("whirlpool", sData, piDigestLen);
}


//----------------------------------------------------------------------------
// md5sum_string_old
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::md5sum_string_old(std::string sData, unsigned int *piDigestLen) {
    unsigned char *pOut = NULL;
    *piDigestLen = 0;

    CryptoDigest *pCD = CryptoDigest::createInstance("md5");
    if (pCD != NULL) {
        printf("CD:  ");
        pOut = pCD->digestString(sData, piDigestLen);
        delete pCD;
    }
    return pOut;
}


//----------------------------------------------------------------------------
// md5sum_file
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::md5sum_file(std::string sFileName, unsigned int *piDigestLen) {
    return genericsum_file("md5", sFileName, piDigestLen);
}


//----------------------------------------------------------------------------
// sha256sum_file
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::sha256sum_file(std::string sFileName, unsigned int *piDigestLen) {
    return genericsum_file("sha256", sFileName, piDigestLen);
}


//----------------------------------------------------------------------------
// sha512sum_file
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::sha512sum_file(std::string sFileName, unsigned int *piDigestLen) {
    return genericsum_file("sha512", sFileName, piDigestLen);
}


//----------------------------------------------------------------------------
// whirlpoolsum_file
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::whirlpoolsum_file(std::string sFileName, unsigned int *piDigestLen) {
    return genericsum_file("whirlpool", sFileName, piDigestLen);
}


//----------------------------------------------------------------------------
// md5sum_data
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::md5sum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen) {
    return genericsum_data("md5", pData, iDataLen, piDigestLen);
}


//----------------------------------------------------------------------------
// sha256sum_data
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::sha256sum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen) {
    return genericsum_data("sha256", pData, iDataLen, piDigestLen);
}


//----------------------------------------------------------------------------
// sha512sum_data
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::sha512sum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen) {
    return genericsum_data("sha512", pData, iDataLen, piDigestLen);
}


//----------------------------------------------------------------------------
// whirlpoolsum_data
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::whirlpoolsum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen) {
    return genericsum_data("whirlpool", pData, iDataLen, piDigestLen);
}


//----------------------------------------------------------------------------
// genericsum_string
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::genericsum_string(std::string sDigestName, std::string sData, unsigned int *piDigestLen) {
    unsigned char *pOut = NULL;
    *piDigestLen = 0;

    CryptoDigest *pCD = CryptoDigest::createInstance(sDigestName);
    if (pCD != NULL) {
        pOut = pCD->digestString(sData, piDigestLen);
        delete pCD;
    }
    return pOut;
}


//----------------------------------------------------------------------------
// genericsum_file
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::genericsum_file(std::string sDigestName, std::string sFileName, unsigned int *piDigestLen) {
    int iResult = -1;
    unsigned char *pOut = NULL;
    *piDigestLen = 0;

    char aBuf[BLOCK_SIZE];


    std::ifstream fIn(sFileName, std::ifstream::binary);
    if (fIn.good()) {
        

        CryptoDigest *pCD = CryptoDigest::createInstance(sDigestName);
        if (pCD != NULL) {
            iResult = 0;
            
            while ((iResult == 0) && (fIn.good())) {
                fIn.read(aBuf, BLOCK_SIZE);
                pCD->addData((unsigned char *) aBuf, fIn.gcount());
                
            }
            if (iResult == 0) {
                pOut =  pCD->getDigest(piDigestLen);
            }
            delete pCD;
        }
            
        fIn.close();
    } else {
        fprintf(stderr, "Couldn't open file for reading [%s]\n", sFileName.c_str());
        iResult = -1;
    }
        return pOut;
}


//----------------------------------------------------------------------------
// genericsum_data
//  the returned pointer must be destroyed with free()
//
unsigned char *CryptoDigest::genericsum_data(std::string sDigestName, unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen) {
    unsigned char *pOut = NULL;
    *piDigestLen = 0;

    CryptoDigest *pCD = CryptoDigest::createInstance(sDigestName);
    if (pCD != NULL) {
        int iResult = pCD->addData(pData, iDataLen);
        if (iResult == 0) {
            pOut = pCD->getDigest(piDigestLen);
        } else {
            // failure 
        }

        delete pCD;
    }
    return pOut;
}
