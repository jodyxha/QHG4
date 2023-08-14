#include <cstdio>
#include <cstring>
#include <vector>

//#include "crypto.h"
#include "CryptoDigest.h"

const int   ID_SHA256  = 1;
const int   ID_SHA512  = 2;
const int   ID_MD5     = 4;
const int   ID_WHIRL   = 8;

const char *asNames[] = {"sha256", "sha512", "md5", "whirlpool"};
const int  aIDs[] = {ID_SHA256, ID_SHA512, ID_MD5, ID_WHIRL};

void showHex(unsigned char *pSum, int iLen) {
    for (int i = 0; i < iLen; i++) {
        printf("%02x", pSum[i]);
    }
    printf("\n");
}

int main (int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        int iWhich = 0;
        std::string sFileName(apArgV[1]);

        if (iArgC > 2) {
            int i = 2;
            while (i < iArgC) {
                for (unsigned int j = 0; j < sizeof(asNames)/sizeof(char *); j++) {
                    if (strcmp(apArgV[i], asNames[j]) == 0) {
                        iWhich |= aIDs[j];
                    }
                }
                i++;
            }
        } else {
            for (unsigned int j = 0; j < sizeof(asNames)/sizeof(char *); j++) {
                iWhich |= aIDs[j];
            }
        }
        

        if ((iResult == 0) && ((iWhich & ID_MD5) != 0)) {
            printf("--------------------------\n");
            printf("MD5\n");
            unsigned int iL     = 0;
            unsigned char *pBuf = NULL;
            pBuf = CryptoDigest::md5sum_file(sFileName, &iL);
            if (pBuf != NULL) {
                printf("digest length %u\n", iL);
                showHex(pBuf, iL); 
                free(pBuf);
            }
            /*
            unsigned char aMD5[crypto::MD5_SIZE];
            iResult = crypto::md5sum(apArgV[1], aMD5);
            if (iResult == 0) {
                showHex(aMD5, crypto::MD5_SIZE);
            } else {
                printf("couldn't open [%s]\n", apArgV[1]);
            }
            */
        }


        if ((iResult == 0) && ((iWhich & ID_SHA256) != 0)) {
            printf("--------------------------\n");
            printf("SHA256\n");

            unsigned int iL     = 0;
            unsigned char *pBuf = NULL;
            pBuf = CryptoDigest::sha256sum_file(sFileName, &iL);
            if (pBuf != NULL) {
                printf("digest length of %s:  %u\n", sFileName.c_str(), iL);
                showHex(pBuf, iL); 
                free(pBuf);
            }

            /*
              unsigned char aSHA[crypto::SHA_SIZE];
              iResult = crypto::shasum(apArgV[1], aSHA);
              if (iResult == 0) {
              showHex(aSHA, crypto::SHA_SIZE);
              } else {
              printf("couldn't open [%s]\n", apArgV[1]);
              }
            */
        }

        if ((iResult == 0) && ((iWhich & ID_SHA512) != 0)) {
            printf("--------------------------\n");
            printf("SHA512\n");

            unsigned int iL     = 0;
            unsigned char *pBuf = NULL;
            pBuf = CryptoDigest::sha512sum_file(sFileName, &iL);
            if (pBuf != NULL) {
                printf("digest length of %s:  %u\n", sFileName.c_str(), iL);
                showHex(pBuf, iL); 
                free(pBuf);
            }
        }

        if ((iResult == 0) && ((iWhich & ID_WHIRL) != 0)) {
            printf("--------------------------\n");
            printf("WHIRLPOOL\n");

            unsigned int iL     = 0;
            unsigned char *pBuf = NULL;
            pBuf = CryptoDigest::whirlpoolsum_file(sFileName, &iL);
            if (pBuf != NULL) {
                printf("digest length of %s:  %u\n", sFileName.c_str(), iL);
                showHex(pBuf, iL); 
                free(pBuf);
            }
        }

    } else {
        iResult = -1;
        printf("usage: %s <file> [\"sha256\" | \"sha512\" | \"md5\" | \"whirlpool\" ]*\n", apArgV[0]);
    }

    /*
    unsigned char sBuf1[256];
    unsigned char sBuf2[256];
    */
    unsigned char *pBuf3;
    unsigned char *pBuf4;
    std::string sMess = "In a cadda da vida";
    unsigned int iL;

    /*
    iResult = crypto::md5sumS(sMess, sBuf1);
    printf("old: ");
    showHex(sBuf1, crypto::MD5_SIZE); 
    iResult = crypto::md5sumSNew(sMess, sBuf2); 
    printf("new: ");
    showHex(sBuf2, crypto::MD5_SIZE); 
    printf("alt: ");
    crypto::digest_message((unsigned char *)sMess.c_str(),  sMess.length(), &pBuf3, & iL);
    showHex(pBuf3, iL); 
    free(pBuf3);
    */
    CryptoDigest *pCD = CryptoDigest::createInstance("md5");
    if (pCD != NULL) {

        printf("\n");
        pBuf4 = pCD->digestString(sMess, &iL);
        printf("CD method (%u):  ", iL);
        showHex(pBuf4, iL); 
        free(pBuf4);
        delete pCD;

        printf("\n");
        pBuf4 = CryptoDigest::md5sum_string(sMess, &iL);
        printf("CD static (%u): ", iL);
        showHex(pBuf4, iL); 
        free(pBuf4);

        /*
        pBuf4 = CryptoDigest::md5sum_string_old(sMess, &iL);
        printf("md5old (%d): ", iL);
        showHex(pBuf4, iL); 
        free(pBuf4);
        */
    }

    printf("file digest\n");
    std::string sFileName2 = "/home/jody/progs/QHG4/utils/crypto.cpp";
    /*
    crypto::md5sum(sFileName, sBuf1);
    printf("old: ");
    showHex(sBuf1, crypto::MD5_SIZE); 
    iResult = crypto::md5sumNew(sFileName, sBuf2); 
    printf("new: ");
    showHex(sBuf2, crypto::MD5_SIZE); 
    */
    pBuf3 = CryptoDigest::md5sum_file(sFileName2, &iL);
    printf("CD (%u): ", iL);
    showHex(pBuf3, iL); 

    printf("---- state stuff ----\n");

    int STATE_SIZE=16;
    std::string sPhrase = "Die Kamele bellen, Quark, die Hunde bellen, die Karawane zieht weiter\n";
    int k = 3;

    /*
    std::vector<unsigned int> vulState1;
    // modify the input string for each iteration
    std::string sPhrase2a = sPhrase + std::to_string(k);
    unsigned char md[crypto::MD5_SIZE];
    crypto::md5sumS(sPhrase2a.c_str(), md);
    for (uint i = 0; i < STATE_SIZE; i += sizeof(uint32_t)) {
        unsigned int u = 0;
        memcpy(&u, md + i, sizeof(uint32_t));
        vulState1.push_back(u);
    }

    printf("vulState1\n");
    for (unsigned int i = 0; i < vulState1.size(); i++) {
        printf("%02x", vulState1[i]);
    }
    printf("\n");
    */
    std::vector<unsigned int> vulState2;
    // modify the input string for each iteration
    std::string sPhrase2b = sPhrase + std::to_string(k);
    unsigned int iLen = 0;
    unsigned char *pDigest = CryptoDigest::md5sum_string(sPhrase2b, &iLen); 
    for (int i = 0; i < STATE_SIZE; i += sizeof(uint32_t)) {
        unsigned int u = 0;
        memcpy(&u, pDigest + i, sizeof(uint32_t));
        vulState2.push_back(u);
    }
    free(pDigest);

    printf("vulState2\n");
    for (unsigned int i = 0; i < vulState2.size(); i++) {
        printf("%02x", vulState2[i]);
    }
    printf("\n");

    return iResult;
}

