#ifndef __CLSUTILS_CPP__
#define __CLSUTILS_CPP__

#include <cstdio>
#include <cstring>

#include "types.h"
#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "clsutils.h"


//-----------------------------------------------------------------------------
// getAttrbuteVal
//   gets the value from a stringmap
//
template<typename T>
int getAttributeVal(const stringmap &mParams, const std::string sKey, T *pVal) {
    int iResult = -1;
    stringmap::const_iterator it = mParams.find(sKey);
    if (it != mParams.end()) {
        if (strToNum(it->second, pVal)) {
            iResult = 0;
        } else {
            stdprintf("Expected param [%s] to be a number: [%s]\n", sKey, it->second);
        }
    } else {
        stdprintf("Couldn't find param [%s]\n", sKey);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// getAttributeArr
//   gets array of values from a stringmap
//
template<typename T> 
int getAttributeArr(const stringmap &mParams, const std::string sKey, int iNum, T* pVal) {
    int iResult = -1;
    stringmap::const_iterator it = mParams.find(sKey);
    if (it != mParams.end()) {
        std::string sCopy = it->second;
        iResult = stringToArr(sCopy, iNum, pVal);

    } else {
        stdprintf("Couldn't find param [%s]\n", sKey);
    }

    return iResult;
}

 
//-----------------------------------------------------------------------------
// getAttributeStr
//   gets array of values from a stringmap
//
int getAttributeStr(const stringmap &mParams, std::string sKey, std::string &sVal) {
    int iResult = -1;
    stringmap::const_iterator it = mParams.find(sKey);
    if (it != mParams.end()) {
        sVal = it->second;
        iResult = 0;
    } else {
        stdprintf("Couldn't find param [%s]\n", sKey);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// checkAttributes
//   check that mParams contains exactly the required entries
//   vNames will contain t 
int checkAttributesV(const stringmap &mParams, const stringvec &vNames, stringvec &vRequired, stringvec &vUnknown) {
    int iResult = -1;

    stringmap::const_iterator itm;
    stringvec::const_iterator itv;
    for (itm = mParams.begin(); itm != mParams.end(); ++itm) {
        bool bSearching = true;
        for (itv = vNames.begin(); bSearching && (itv != vNames.end()); ++itv) {
            if (itm->first == *itv) {
                bSearching = false;
            }
        }
        if (bSearching) {
            vUnknown.push_back(itm->first);
        }
    }

    for (itv = vNames.begin(); itv != vNames.end(); ++itv) {
        itm = mParams.find(*itv);
        if (itm == mParams.end()) {
            vRequired.push_back(*itv);
        }
    }

    if ((vRequired.size() == 0) && (vUnknown.size() == 0)) {
        iResult = 0;
    }
    return iResult;
}

#endif

