/*============================================================================
| xha_strutils.h
| 
|  Regular functions std::string-capable string utilities.
|  
|  Also see xha_strutilsT.h, xha_strutils.cpp
|
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __XHA_STRUTILS_H__
#define __XHA_STRUTILS_H__

#include <string>
#include <vector>
#include "types.h"

uint splitString(const std::string sString, stringvec &vParts, const std::string sSeps, bool bKeepEmpty=true);

std::string trim(const std::string& str);

bool endsWith(const std::string sBig, const std::string sEnd);

bool startsWith(const std::string sBig, const std::string sStart);

bool strReplace(std::string &sBig, const std::string sEnd, const std::string sNew);

std::string join(const stringvec &vStrings, const std::string &sSep);

std::string readKeyString(std::string sLine, const std::string sKey, const std::string sSep);

int replaceEnvVars(const std::string sIn, std::string & sOut);

bool fileExists(const std::string sFile);
bool dirExists(const std::string sDir);

// somehow can't have these in xha_strutilsT.h

//----------------------------------------------------------------------------
// recursiveFormat
//   termination version (of xha_strutilsT::recursiveFormat)
//
std::string recursiveFormat(stringvec &vParts, stringvec &vFormats, uint i);


//----------------------------------------------------------------------------
// simpleFormat
//  format a char using sFormat
//
std::string simpleFormat(const std::string sFormat, const std::string t);



//----------------------------------------------------------------------------
// fetchNextParam
//  termination function for fetchNextParam
//
int fetchNextParam();

//----------------------------------------------------------------------------
// fetchNextTwoParams
//  termination function for fetchNextTwoParams
//
std::pair<int,int> fetchNextTwoParams();

#endif
