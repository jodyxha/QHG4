#ifndef __CRYPTO_DIGEST_H__
#define __CRYPTO_DIGEST_H__

#include <string>
#include <openssl/evp.h>


class CryptoDigest {

public: 
    static CryptoDigest *createInstance(std::string sType);
    virtual ~CryptoDigest();

    int addData(unsigned char *pData, unsigned int iDataLen);
    unsigned char *getDigest(unsigned int *piDigestLen);

    unsigned char *digestString(std::string sData, unsigned int *piDigestLen);

    void showDigestNames();

    static unsigned char *md5sum_string(std::string sData, unsigned int *piDigestLen);
    static unsigned char *md5sum_string_old(std::string sData, unsigned int *piDigestLen);
    static unsigned char *sha512sum_string(std::string sData, unsigned int *piDigestLen);
    static unsigned char *sha256sum_string(std::string sData, unsigned int *piDigestLen);
    static unsigned char *whirlpoolsum_string(std::string sData, unsigned int *piDigestLen);


    static unsigned char *md5sum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen);
    static unsigned char *sha512sum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen);
    static unsigned char *sha256sum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen);
    static unsigned char *whirlpoolsum_data(unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen);

    static unsigned char *md5sum_file(std::string sData, unsigned int *piDigestLen);
    static unsigned char *sha512sum_file(std::string sData, unsigned int *piDigestLen);
    static unsigned char *sha256sum_file(std::string sData, unsigned int *piDigestLen);
    static unsigned char *whirlpoolsum_file(std::string sData, unsigned int *piDigestLen);

protected:
    CryptoDigest();
    int init(std::string sType);

    static unsigned char *genericsum_string(std::string sDigestName, std::string sData, unsigned int *piDigestLen);
    static unsigned char *genericsum_file(std::string sDigestName, std::string sFileName, unsigned int *piDigestLen);
    static unsigned char *genericsum_data(std::string sDigestName, unsigned char *pData, unsigned int iDataLen, unsigned int *piDigestLen);
    
    
    EVP_MD_CTX   *m_mdctx;
    const EVP_MD *m_pDigestType;
    
    /*
    static const char *names[] = {
        "md5", "blowfish", "sha256", "sha512", "whirlpool"}; 
    */
};


#endif

