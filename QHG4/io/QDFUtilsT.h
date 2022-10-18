#ifndef __QDFUTILST_H__
#define __QDFUTILST_H__

#include <hdf5.h>
#include <string>
#include <cstdio>
#include <sstream>
#include <vector>
#include "types.h"
#include "QDFUtils.h"


//----------------------------------------------------------------------------
// qdf_typeSelector
//    Returns the HDF data type corresponding to T
//
template<typename T>
hid_t qdf_typeSelector() {
    if constexpr (std::is_same_v<T, char>) return H5T_NATIVE_CHAR;
    if constexpr (std::is_same_v<T, uchar>) return H5T_NATIVE_UCHAR;
    if constexpr (std::is_same_v<T, short int>) return H5T_NATIVE_SHORT;
    if constexpr (std::is_same_v<T, ushort>) return H5T_NATIVE_USHORT;
    if constexpr (std::is_same_v<T, int>) return H5T_NATIVE_INT32;
    if constexpr (std::is_same_v<T, uint>) return H5T_NATIVE_UINT32;
    if constexpr (std::is_same_v<T, long>) return H5T_NATIVE_LONG;
    if constexpr (std::is_same_v<T, ulong>) return H5T_NATIVE_ULONG;
    if constexpr (std::is_same_v<T, long long>) return H5T_NATIVE_LLONG;
    if constexpr (std::is_same_v<T, float>) return H5T_NATIVE_FLOAT;
    if constexpr (std::is_same_v<T, double>) return H5T_NATIVE_DOUBLE;
    if constexpr (std::is_same_v<T, long double>) return H5T_NATIVE_LDOUBLE;
    if constexpr (std::is_same_v<T, bool>) return H5T_NATIVE_HBOOL;

    printf("*** WARNING: type selector can't handle unknown type! ***\n");
    
    return H5P_DEFAULT;
}

//----------------------------------------------------------------------------
// qdf_insertSAttribute
//   char
//
template<typename T>
int qdf_insertSAttribute(hid_t hLoc, const std::string sName, const T &tValue) {
    std::stringstream ss;
    ss << tValue;
    return qdf_insertSAttribute(hLoc, sName, ss.str());
}


//----------------------------------------------------------------------------
// qdf_insertAttribute
//    Use qdf_typeSelector to get HDF type corresponding to T
//
template<typename T>
int qdf_insertAttribute(hid_t hLoc, const std::string sName, const uint iNum, T *tValue) {
    int iResult = -1;
    hid_t hType = qdf_typeSelector<T>();
    if (hType != H5P_DEFAULT) {
        iResult = qdf_insertAttribute(hLoc, sName, iNum, tValue, hType);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// qdf_extractAttribute
//    Use qdf_typeSelector to get HDF type corresponding to T
//
template<typename T>
int qdf_extractAttribute(hid_t hLoc, const std::string sName, const uint iNum, T *tValue) {
    int iResult = -1;
    hid_t hType = qdf_typeSelector<T>();
    if (hType != H5P_DEFAULT) {
        iResult = qdf_extractAttribute(hLoc, sName, iNum, tValue, hType);
    }
    return iResult;}


//----------------------------------------------------------------------------
// qdf_readArray
//    Use qdf_typeSelector to get HDF type corresponding to T
//
template<typename T>
int qdf_readArray(hid_t hGroup, const std::string sName, const uint iNum, T *pData) {
    int iResult = -1;
    hid_t hType = qdf_typeSelector<T>();
    if (hType != H5P_DEFAULT) {
        iResult = qdf_readArray(hGroup, sName, iNum, pData, hType);
    }
    return iResult;}


//----------------------------------------------------------------------------
// qdf_writeArray
//    Use qdf_typeSelector to get HDF type corresponding to T
//
template<typename T>
int qdf_writeArray(hid_t hGroup, const std::string sName, const uint iNum, T *pData) {
    int iResult = -1;
    hid_t hType = qdf_typeSelector<T>();
    if (hType != H5P_DEFAULT) {
        iResult = qdf_writeArray(hGroup, sName, iNum, pData, hType);
    }
    return iResult;}


//----------------------------------------------------------------------------
// qdf_replaceArray
//    Use qdf_typeSelector to get HDF type corresponding to T
//
template<typename T>
int qdf_replaceArray(hid_t hGroup, const std::string sName, const uint iNum, T *pData) {
    int iResult = -1;
    hid_t hType = qdf_typeSelector<T>();
    if (hType != H5P_DEFAULT) {
        iResult = qdf_replaceArray(hGroup, sName, iNum, pData, hType);
    }
    return iResult;}

#endif
