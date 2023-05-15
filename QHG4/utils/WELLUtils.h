#ifndef __WELLUTILS_H__
#define __WELLUTILS_H__

#include "WELL512.h"

namespace WELLUtils {

    int stringToSeed(const std::string sSequence, std::vector<uint32_t> &vulState);
    int phraseToSeed(const std::string sPhrase, std::vector<uint32_t> &vulState);
    int randomSeed(int iSeed, std::vector<uint32_t> &vulState);
    
    WELL512 *createWELL(const std::string sPhrase);
    
    WELL512 **buildWELLs(int iNum, uint iSeed);
    void      destroyWELLs(WELL512 **pWELLs, int iNum);
    
    void      showState(WELL512 *pWELL);
    void      showStates(WELL512 **pWELL, int iNum, bool bFull);
    std::string strState(WELL512 *pWELL);
    std::string strStateHash(WELL512 *pWELL);

};

#endif
