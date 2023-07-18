#include <cstdio>
#include <cstring>
#include <vector>

#include "crypto.h"
#include "CryptoDigest.h"

const int   ID_SHA  = 1;
const int   ID_MD5  = 2;
/*const int   ID_RIP  = 4;*/

const char *asNames[] = {"sha", "md5"/*, "rip"*/};
const int  aIDs[] = {ID_SHA, ID_MD5/*, ID_RIP*/};

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
            printf("MD5\n");
            unsigned char aMD5[crypto::MD5_SIZE];
            iResult = crypto::md5sum(apArgV[1], aMD5);
            if (iResult == 0) {
                showHex(aMD5, crypto::MD5_SIZE);
            } else {
                printf("couldn't open [%s]\n", apArgV[1]);
            }
        }


        if ((iResult == 0) && ((iWhich & ID_SHA) != 0)) {
            printf("SHA\n");
            unsigned char aSHA[crypto::SHA_SIZE];
            iResult = crypto::shasum(apArgV[1], aSHA);
            if (iResult == 0) {
                showHex(aSHA, crypto::SHA_SIZE);
            } else {
                printf("couldn't open [%s]\n", apArgV[1]);
            }
        }

        /*
        if ((iResult == 0) && ((iWhich & ID_RIP) != 0)) {
            printf("RIP\n");
            unsigned char aRIP[crypto::RIP_SIZE];
            iResult = crypto::ripsum(apArgV[1], aRIP);
            if (iResult == 0) {
                showHex(aRIP, crypto::RIP_SIZE);
            } else {
                printf("couldn't open [%s]\n", apArgV[1]);
            }
        }
        */

    } else {
        iResult = -1;
        printf("usage: %s <file> [\"sha\" | \"md5\" | \"rip\"]*\n", apArgV[0]);
    }


    unsigned char sBuf1[256];
    unsigned char sBuf2[256];
    unsigned char *pBuf3;
    unsigned char *pBuf4;

    std::string sMess = "In a cadda da vida";
    iResult = crypto::md5sumS(sMess, sBuf1);
    printf("old: ");
    showHex(sBuf1, crypto::MD5_SIZE); 
    iResult = crypto::md5sumSNew(sMess, sBuf2); 
    printf("new: ");
    showHex(sBuf2, crypto::MD5_SIZE); 
    printf("alt: ");
    unsigned int iL;
    crypto::digest_message((unsigned char *)sMess.c_str(),  sMess.length(), &pBuf3, & iL);
    showHex(pBuf3, iL); 
    free(pBuf3);

    CryptoDigest *pCD = CryptoDigest::createInstance("md5");
    if (pCD != NULL) {
        pBuf4 = pCD->digestString(sMess, &iL);
        printf("CD (%u):  ", iL);
        showHex(pBuf4, iL); 
        free(pBuf4);
        delete pCD;
        printf("\n");
        pBuf4 = CryptoDigest::md5sum_string(sMess, &iL);
        printf("CDs (%d): ", iL);
        showHex(pBuf4, iL); 
        free(pBuf4);

        pBuf4 = CryptoDigest::md5sum_string_old(sMess, &iL);
        printf("md5old (%d): ", iL);
        showHex(pBuf4, iL); 
        free(pBuf4);
    }

    printf("file digest\n");
    std::string sFileName = "/home/jody/progs/QHG4/utils/crypto.cpp";
    crypto::md5sum(sFileName, sBuf1);
    printf("old: ");
    showHex(sBuf1, crypto::MD5_SIZE); 
    iResult = crypto::md5sumNew(sFileName, sBuf2); 
    printf("new: ");
    showHex(sBuf2, crypto::MD5_SIZE); 

    pBuf3 = CryptoDigest::md5sum_file(sFileName, &iL);
    printf("CD (%u): ", iL);
    showHex(pBuf3, iL); 

    printf("---- state stuff ----\n");

    int STATE_SIZE=16;
    std::string sPhrase = "Die Kamele bellen ... Quark .. Die hunde bellen\n";
    int k = 3;

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

    std::vector<unsigned int> vulState2;
    // modify the input string for each iteration
    std::string sPhrase2b = sPhrase + std::to_string(k);
    unsigned int iLen = 0;
    unsigned char *pDigest = CryptoDigest::md5sum_string(sPhrase2b, &iLen); 
    for (uint i = 0; i < STATE_SIZE; i += sizeof(uint32_t)) {
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

