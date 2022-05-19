#ifndef __CLSUTILS_H__
#define __CLSUTILS_H__

#include "types.h"

template<typename T>
int readKeyVal(std::string sLine, const std::string sKey, const std::string sSep, T *pV);
template<typename T>
int readKeyArr(std::string sLine, const std::string sKey, const std::string sSep, int iNum, T *pV);

std::string readKeyString(std::string sLine, const std::string sKey, const std::string sSep);

inline int readKeyStr(std::string sLine, const std::string sKey, const std::string sSep, int iSize, std::string &sV);

template<typename T>
    int getParamVal(const stringmap &mParams, const std::string sKey, T *pV);
template<typename T>
    int getParamArr(const stringmap &mParams, const char* pKey, int iNum, T* pV);

inline int getParamStr(const stringmap &mParams, const std::string sKey, std::string &sVal);

inline int checkParamsV(const stringmap &mParams, const stringvec &vNames, stringvec &vRequired, stringvec &vUnknown);

#endif
