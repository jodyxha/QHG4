#ifndef __CLSUTILS_CPP__
#define __CLSUTILS_CPP__

#include <cstdio>
#include <cstring>

#include "types.h"
#include "strutils.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "clsutils.h"


template<typename T>
int stringToArr(std::string sString, int iLen, T *pV) {
    int iResult = 0;

    stringvec vParts;
    uint iNum = splitString(sString, vParts, " \t,");
    if (iNum > iLen) {
        iNum = iLen;
    }
    for (uint i = 0; (iResult == 0) && (i < iNum); ++i) {
        T tDummy;
        if (strToNum(vParts[i], &tDummy)) {
            pV[i++] = tDummy;
        } else {
            stdprintf("Invalid number for array: [%s]\n", vParts[i]);
            iResult = -1;
        }
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// readKeyVal
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int readKeyVal(std::string sLine, const std::string sKey, const std::string sSep, T *pV) {
    int iResult = -1;
    std::string sVal = readKeyString(sLine, sKey, sSep);
  
    if (!sVal.empty()) {
        if (strToNum(sVal, pV)) {
            iResult = 0;
        } else {
            stdprintf("Invalid number for \"%s\" [%s]\n", sKey, sVal);
            iResult = -1;
        }
    } else {
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readKeyArr
//   gets the value from a line of form
//   <Key> <Sep> <Val>
template<typename T>
int readKeyArr(std::string sLine, const std::string sKey, const std::string sSep, int iNum,  T *pV) {
    int iResult = -1;
    std::string sVal = readKeyString(sLine, sKey, sSep);
  
    if (!sVal.empty()) {
        iResult = stringToArr(sVal, iNum, pV);

    } else {
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readKeyStr
//   gets the value from a line of form
//   <Key> <Sep> <Val>
int readKeyStr(std::string sLine, const std::string sKey, const std::string sSep, int iSize, std::string &sVal) {
    int iResult = -1;
    std::string sV = readKeyString(sLine, sKey, sSep);
    
    if (!sV.empty()) {
        if ((int)(sV.length()) < iSize) {
            sVal = sV;
            iResult = 0;
        } else {
            stdprintf("value string for [%s] is too long :len(%s) = %zd >= %d\n", sKey, sV, sV.length(), iSize);
            iResult = -1;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getParamVal
//   gets the value from a stringmap
//
template<typename T>
int getParamVal(const stringmap &mParams, const std::string sKey, T *pVal) {
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
// getParamArr
//   gets array of values from a stringmap
//
template<typename T> 
int getParamArr(const stringmap &mParams, const std::string sKey, int iNum, T* pVal) {
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
// getParamStr
//   gets array of values from a stringmap
//
int getParamStr(const stringmap &mParams, std::string sKey, std::string &sVal) {
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
// checkParams
//   check that mParams contains exactly the required entries
//   vNames will contain t 
int checkParamsV(const stringmap &mParams, const stringvec &vNames, stringvec &vRequired, stringvec &vUnknown) {
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

