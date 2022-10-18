/*============================================================================
| stdstrutils.cpp
| 
|  Regular functions std::string-capable string utilities.
|  
|  Also see stdstrutilsT.h, stdstrutils.h
|
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __STDSTRUTILS_CPP__
#define __STDSTRUTILS_CPP__

#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <sys/types.h>
#include <sys/stat.h>
 
#include "types.h"
#include "stdstrutils.h"

//----------------------------------------------------------------------------
// splitString
//  Splits the string sString at every character matching one of sSeps.
//  The resulting substrings are stored in vParts. 
//
uint splitString(const std::string sString, stringvec &vParts, std::string sSeps, bool bKeepEmpty/*=true*/) {

    size_t iPos0 = 0;
    if (!sString.empty()) {
    
        if (sString.at(iPos0) == '\'') {
            iPos0++;
        }
        size_t iPos = iPos0;
        long   iPrevPos = (long)iPos0 - 1;
        while (iPos != std::string::npos) {
            size_t iPos1 = sString.find_first_of(sSeps, iPos);
            if (iPos1 != std::string::npos) {
                if ((iPos1 > (uint)(iPrevPos+1))) {
                    vParts.push_back(sString.substr(iPos, iPos1-iPos));
                }
                iPrevPos = iPos1;
                iPos = iPos1+1;
            } else {
                std::string sTemp = sString.substr(iPos, std::string::npos);
                if (sTemp[sTemp.size()-1] == '\'') {
                    sTemp = sTemp.substr(0, sTemp.size()-1);
                }
                if (bKeepEmpty || (sTemp.size() > 0)) {
                    vParts.push_back(sTemp);
                }
                iPos = iPos1;
            }
        }
    }
    return vParts.size();
}


//----------------------------------------------------------------------------
// trim
//
std::string trim(const std::string& str) {
    std::string sOut = "";
    size_t first = str.find_first_not_of(" \t\n");
    if (std::string::npos == first) {
        sOut = "";
    } else {
        // we know that last will not be std::string::npos
        size_t last = str.find_last_not_of(" \t\n");
        sOut = str.substr(first, (last - first + 1));
    }
    return sOut;
}


//----------------------------------------------------------------------------
// endsWith
//   returns true if sBig ends with the string sEnd
//
bool endsWith(const std::string sBig, const std::string sEnd) {
    bool bEnds = false;
    if (sBig.length() > sEnd.length()) {
        bEnds = (sBig.find(sEnd) == (sBig.size() - sEnd.size()));
    }
    return bEnds;
}


//----------------------------------------------------------------------------
// startsWith
//   returns true if sBig starts with the string seck
//
bool startsWith(const std::string sBig, const std::string sCheck) {
    return (sBig.find(sCheck) == 0);
}


//----------------------------------------------------------------------------
// strReplace
//
bool strReplace(std::string &sBig, const std::string sEnd, const std::string sNew) {
    bool bReplaced = false;
    size_t iPos = sBig.find(sEnd);
    if (iPos != std::string::npos) {
        sBig = sBig.replace(iPos, sEnd.size(), sNew);
        bReplaced = true;
    }
    return bReplaced;
}


//----------------------------------------------------------------------------
// join
//   join the strings in vStrings with sSep
//
std::string join(const stringvec &vStrings, const std::string &sSep) {
    std::string sJoin  = "";
    for (uint i = 0; i < vStrings.size(); i++) {
        if (!sJoin.empty()) {
            sJoin = sJoin + sSep;
        }
        sJoin = sJoin + vStrings[i];
    }
    return sJoin;
}


//----------------------------------------------------------------------------
// replaceEnvVars
//   replace all environment variables of form  "${XXX}" ins sIn with their
//   values and places the result in sOut. If an unknown environment variable
//   is encountered the unknown name is placed in sOUt and -1 is returned.
//
int replaceEnvVars(const std::string sIn, std::string & sOut) {
    int iResult = 0;

    sOut = "";
    size_t p1 = 0;
    bool bGoOn = true;

    while (bGoOn && (iResult == 0) && (p1 < sIn.length())) {
        size_t p2 = sIn.find("${", p1);
        if (p2 != std::string::npos) {
            size_t p3 = sIn.find("}", p2);
            if (p3 != std::string::npos) {
                std::string sVar = sIn.substr(p2+2, p3-p2-2);
                char *pVal = std::getenv(sVar.c_str());
                if (pVal != NULL) {
                    sOut += sIn.substr(p1, p2 - p1) + std::string(pVal);
                    p1 = p3 + 1;
                } else {
                    // unknown envvar - error
                    sOut = std::string("${") + sVar + "}";
                    iResult = -1;
                }
            } else {
                // no closing bracket - error
                sOut = sIn.substr(p2).c_str();
                iResult = -1;
            }
        } else {
            // no more envvars - leave loop
            bGoOn = false;
        }
    }
    if (iResult == 0) {
        sOut += sIn.substr(p1);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// fileExists
//
bool fileExists(const std::string sFile) {
    struct stat statbuf;
    return ((stat(sFile.c_str(), &statbuf) == 0) && (S_ISREG(statbuf.st_mode)));
}


//----------------------------------------------------------------------------
// dirExists
//
bool dirExists(const std::string sPath) {
    struct stat statbuf;
    return ((stat(sPath.c_str(), &statbuf) == 0) && (S_ISDIR(statbuf.st_mode)));
}


//-----------------------------------------------------------------------------
// readKeyString
//   gets the string value from a line of form
//   <Key> <Sep> <Val>
std::string readKeyString(std::string sLine, const std::string sKey, const std::string sSep) {
    std::string sVal = "";
    sLine = trim(sLine);
  
    if (startsWith(sLine, sKey)) {
        if (!sSep.empty()) {
            size_t iPos = sLine.find(sSep);
            if (iPos != std::string::npos) {
                sVal = trim(sLine.substr(iPos));
            } else {
                sVal = "";
            }
        }
    }

    return std::string(sVal);
}

// These are the "termination" functions for the recursive template functions around stdsprintf.
// Somehow i can't have these versions of recursiveFormat() and simpleFormat() in stdstrutilsT.h -
// they have no template arguments, and when declared 'static' i always get 'unused' warnings


//----------------------------------------------------------------------------
// recursiveFormat
//   termination version of stdstrutils::recursiveFormat
//
std::string recursiveFormat(stringvec &vParts, stringvec &vFormats, uint i) {
    return vParts[i];
}


//----------------------------------------------------------------------------
// simpleFormat
//  special case for single string a char using sFormat
//
std::string simpleFormat(const std::string sFormat, const std::string t) {
    size_t required = snprintf(NULL, 0, sFormat.c_str(), t.c_str());
    char *sTemp = new char[required+1];

    sprintf(sTemp, sFormat.c_str(), t.c_str());

    std::string sOut = sTemp;
    delete[] sTemp;
    return std::string(sOut);
} 


//----------------------------------------------------------------------------
// fetchNextParam
//  termination function for fetchNextParam
//
int fetchNextParam(){
    throw std::runtime_error("Wrong number of argument");
}


//----------------------------------------------------------------------------
// fetchNextParam
//  termination function for fetchNextParam
//
int fetchNextParam2(){
    throw std::runtime_error("Wrong number of argument");
}

#endif
