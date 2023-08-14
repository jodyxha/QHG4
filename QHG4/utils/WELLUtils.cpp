#include <cstdio>
#include <cstring>

#include <vector>

#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "types.h"
#include "crypto.h"
#include "CryptoDigest.h"
#include "WELL512.h"
#include "WELLUtils.h"

//-----------------------------------------------------------------------------
//  stringToSeed
// 
int WELLUtils::stringToSeed(const std::string sSequence, std::vector<uint32_t> &vulState) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sSequence, vParts, ",; ");

    if (iNum == STATE_SIZE) {
        
        for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
            int iX = 0;
            if (strToHex(vParts[i], &iX)) {
                vulState.push_back(iX & 0xffffffff);
            } else {
                stdprintf("[WELLUtils::stringToSeed] Invalid hexnumber given in seed sequence [%s]\n", vParts[i]);
                iResult = -1;
            }
        }
    } else {
        stdprintf("[WELLUtils::stringToSeed] need %d elements in sequence\n", STATE_SIZE);
        iResult = -1;
    }


    return iResult;
}


//-----------------------------------------------------------------------------
//  phraseToSeed
//    creates a 16 array of int from an arbitrary string
//    we use md5sum (which fills calculates 16bytes from a string) four times
//    with slightly changed phrase

int WELLUtils::phraseToSeed(const std::string sPhrase, std::vector<uint32_t> &vulState) {
    int iResult = -1;


    if (sPhrase.length() > 1) {

        // md5sumS creates an array of 16 unsigned chars.
        // For anentire WELL512 state we need 4 such results
        for (uint k = 0; k < 4; k++) {
            /*
            // modify the input string for each iteration
            std::string sPhrase2 = sPhrase + std::to_string(k);
            uchar md[crypto::MD5_SIZE];
            crypto::md5sumS(sPhrase.c_str(), md);
            for (uint i = 0; i < STATE_SIZE; i += sizeof(uint32_t)) {
                uint32_t u = 0;
                memcpy(&u, md + i, sizeof(uint32_t));
                vulState.push_back(u);
            }
*/
            // modify the input string for each iteration
            std::string sPhrase2 = sPhrase + std::to_string(k);
            unsigned int iLen = 0;
            unsigned char *pDigest = CryptoDigest::md5sum_string(sPhrase2, &iLen); 
            for (uint i = 0; i < STATE_SIZE; i += sizeof(uint32_t)) {
                uint32_t u = 0;
                memcpy(&u, pDigest + i, sizeof(uint32_t));
                vulState.push_back(u);
            }
            free(pDigest);
        }
        iResult = 0;
        
    } else {
        stdprintf("Phrase for seed must not be empty\n");
        iResult = -1;
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
//  random seed
//    creates a 16 array of long using the rand() function
//
int WELLUtils::randomSeed(int iSeed, std::vector<uint32_t> &vulState) {
    
    for (uint i = 0; i < STATE_SIZE; i++) {
        vulState.push_back(rand()%0xffffffff);
    }
    return 0;
}


//-----------------------------------------------------------------------------
//  createWELL
//    creates a WELL and sets seed from phrase
//
WELL512 *WELLUtils::createWELL(const std::string sPhrase) {

    WELL512 *pWELL = NULL;


    std::vector<uint32_t> vState;
    if (sPhrase.length() > 1) {
        int iResult = phraseToSeed(sPhrase, vState);
        if (iResult == 0) {
            uint32_t aState[STATE_SIZE];
            for (uint i = 0; i < STATE_SIZE; i++) {
                aState[i] = vState[i];
            }

            pWELL = new WELL512(aState);
        }
    } else {
        stdprintf("Phrase for seed must not be empty\n");
    }
    
    return pWELL;
}


//-----------------------------------------------------------------------------
//  buildWELLs
//    creates an array WELL and sets seed
//
WELL512 **WELLUtils::buildWELLs(int iNum, uint iSeed) {
    uint32_t temp[STATE_SIZE];
    WELL512 **pWELL = new WELL512 *[iNum];

    for  (int iT = 0; iT < iNum; iT++) {
        int c = 0;
        for (uint j = 0; j < STATE_SIZE/4; j++) {
            std::string sPhrase = stdsprintf("seed for %d[%d]:%u", iT, j, iSeed);
            //uchar md[crypto::MD5_SIZE];
            //crypto::md5sumS(sPhrase.c_str(), md);
            unsigned int iLen = 0;
            unsigned char *pDigest = CryptoDigest::md5sum_string(sPhrase, &iLen); 
        

            for (uint i = 0; i < 4; i++) {
                temp[c++] = *((uint32_t *)(pDigest+sizeof(uint32_t)*i));
            }
        }
        pWELL[iT] = new WELL512(temp);
    }


    return pWELL;
}

//-----------------------------------------------------------------------------
//  destroyWELLs
//    creates an array WELL and sets seed
//
void WELLUtils::destroyWELLs(WELL512 **pWELLs, int iNum) {
    for  (int iT = 0; iT < iNum; iT++) {
        delete pWELLs[iT];
    }
    delete[] pWELLs;
}


//-----------------------------------------------------------------------------
//  showState
//    show cur index and state
//
void WELLUtils::showState(WELL512 *pWELL) {
    stdprintf("[%08x] ", pWELL->getIndex());
    const uint32_t *p = pWELL->getState();
    for (uint j = 0; j < STATE_SIZE;j++) {
        stdprintf("%08x ", p[j]);
    }
    stdprintf("\n");
}


//-----------------------------------------------------------------------------
//  showStates
//    show cur index and state
//
void WELLUtils::showStates(WELL512 **apWELL, int iNum, bool bFull) {
    
    std::string sStates = "";
    for (int i = 0; i < iNum; i++) {
        sStates +=  stdsprintf("[%08x] ", apWELL[i]->getIndex());
        char sTemp[256];

        apWELL[i]->state2String(sTemp);
        //strState(apWELL[i], sTemp);
        sStates += sTemp;
        sStates += "\n";
    }
    /*
    uchar umd5[crypto::MD5_SIZE];
    std::string smd5("");
    
    crypto::md5sumS(sStates.c_str(), umd5);
    for (int i = 0; i < crypto::MD5_SIZE; i++) {
        std::string s = stdsprintf("%02x", umd5[i]);
        smd5 += s;
    }
    */
 
    unsigned int iLen = 0;
    std::string smd5("");
    
    unsigned char *pDigest = CryptoDigest::md5sum_string(sStates, &iLen); 
    for (unsigned int i = 0; i < iLen; i++) {
        std::string s = stdsprintf("%02x", pDigest[i]);
        smd5 += s;
    }

 
    stdprintf("WELL hash %s\n", smd5);
    if (bFull) {
        stdprintf("%s", sStates);
    }
}


//-----------------------------------------------------------------------------
//  strState
//    pString should have a size of at least 173
//
std::string WELLUtils::strState(WELL512 *pWELL) {
    std::string sTemp = "";
    sTemp += stdsprintf("[%08x]g[%f] ", pWELL->getIndex(), pWELL->getPrevNormal());
    const uint32_t *p = pWELL->getState();
    for (uint j = 0; j < STATE_SIZE;j++) {
        sTemp += stdsprintf("%08x ", p[j]);
    }
    return std::string(sTemp);
}

//-----------------------------------------------------------------------------
//  strStateHash
//    pString should have a size of at least 173
//
std::string WELLUtils::strStateHash(WELL512 *pWELL) {
    std::string sState = strState(pWELL);

    std::string smd5("");
    /*
    uchar umd5[crypto::MD5_SIZE];
    crypto::md5sumS(sState.c_str(), umd5);
    for (int i = 0; i < crypto::MD5_SIZE; i++) {
        std::string s = stdsprintf("%02x", umd5[i]);
        smd5 += s;
    }
    */
    unsigned int iLen = 0;
    unsigned char *pDigest = CryptoDigest::md5sum_string(sState, &iLen);
    for (unsigned int i = 0; i < iLen; i++) {
        std::string s = stdsprintf("%02x", pDigest[i]);
        smd5 += s;
    }
    free(pDigest);
    return std::string(smd5);
}
