#ifndef __CLSUTILS_H__
#define __CLSUTILS_H__

#include "types.h"


template<typename T>
int getAttributeVal(const stringmap &mParams, const std::string sKey, T *pV);

template<typename T>
int getAttributeArr(const stringmap &mParams, const char* pKey, int iNum, T* pV);

inline int getAttributeStr(const stringmap &mParams, const std::string sKey, std::string &sVal);

inline int checkAttributesV(const stringmap &mParams, const stringvec &vNames, stringvec &vRequired, stringvec &vUnknown);

#endif
