/*============================================================================
| ArrayShare
|
|  A singleton managing a collection of named (void *)-arrays.
|  It is used to share arrays between different SPopulations.
|   
|  Author: Jody Weissmann
| 
\===========================================================================*/

#include <cstdio>
#include <string>
#include <map>
#include <regex>

#include "ArrayShare.h"

// singleton declaration
ArrayShare *ArrayShare::s_pAS = NULL;


//-----------------------------------------------------------------------------
// getInstance
//
ArrayShare *ArrayShare::getInstance() {
    if (s_pAS == NULL) {
        s_pAS = new ArrayShare();
    }
    return s_pAS;
}


//-----------------------------------------------------------------------------
// freeInstance
//
void ArrayShare::freeInstance() {
    if (s_pAS != NULL) {
        delete s_pAS;
        s_pAS = NULL;
    }
}


//-----------------------------------------------------------------------------
// constructor
//
ArrayShare::ArrayShare() {
}


//-----------------------------------------------------------------------------
// destructor
//  deleting the arraystruct elements
//
ArrayShare::~ArrayShare() {
    arraymap::iterator it;
    printf("deleting arrayshare\n");
    for (it = m_mArrays.begin(); it != m_mArrays.end(); ++it) {
        if (it->second != NULL) {
            delete it->second;
        }
    }
}


//-----------------------------------------------------------------------------
// addArray
//  register an array for sharing:
//  adds an arraystruct (size, void*) to the named list
//
int ArrayShare::shareArray(const std::string sName, int iSize, void *pdArr) {
    return shareArray(sName, iSize, "", pdArr);
}

//-----------------------------------------------------------------------------
// addArray
//  register an array for sharing:
//  adds an arraystruct (size, void*) to the named list
//
int ArrayShare::shareArray(const std::string sName, int iSize, std::string sType, void *pdArr) {
    int iResult = -1;
    arraymap::iterator it = m_mArrays.find(sName);
    if (it == m_mArrays.end()) {
        m_mArrays[sName] = new arraystruct(iSize, pdArr, sType);
        iResult = 0;
    } else {
        printf("[ArrayShare::shareArray] name [%s] already in list\n", sName.c_str());
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getSize
//   get size of array with name
//
int ArrayShare::getSize(const std::string sName) {
    int iSize = -1;
    arraymap::iterator it = m_mArrays.find(sName);
    if (it != m_mArrays.end()) {
        iSize = it->second->m_iSize;
    } else {
        printf("[ArrayShare::getSize] no array with name [%s]\n", sName.c_str());
        iSize = -1;
    }
    return iSize;   
}


//-----------------------------------------------------------------------------
// getArray
//   get pointer to the array with name
//
void *ArrayShare::getArray(const std::string sName) {
    void *pdArr = NULL;
    arraymap::iterator it = m_mArrays.find(sName);
    if (it != m_mArrays.end()) {
        pdArr = it->second->m_pdArr;
    } else {
        //        printf("[ArrayShare::getArray] no array with name [%s]\n", sName.c_str());
    }
    return pdArr;
}


//-----------------------------------------------------------------------------
// getArrayStruct
//   get the entire arraystruct with name
//
const arraystruct *ArrayShare::getArrayStruct(const std::string sName) {
    arraystruct *pas = NULL;
    arraymap::iterator it = m_mArrays.find(sName);
    if (it != m_mArrays.end()) {
        pas = it->second;
    } else {
        //        printf("[ArrayShare::getArrayStruct] no array with name [%s]\n", sName.c_str());
    }
    return pas;
}


//-----------------------------------------------------------------------------
// removeArray
//   unregister array with name:
//   remove arrstruct for given name
//
int ArrayShare::removeArray(const std::string sName) {
    int iResult = -1;

    arraymap::iterator it = m_mArrays.find(sName);
    if (it != m_mArrays.end()) {
        arraystruct *pas = it->second;
        m_mArrays.erase(it);
        delete pas;
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getNamesLike
//   get all names matching the simple regex pNamePattern
//
const stringlist &ArrayShare::getNamesLike(const std::string sNamePattern) {
    m_vNameMatches.clear();
    arraymap::iterator it;

    for (it = m_mArrays.begin(); it != m_mArrays.end(); ++it) {
        if (std::regex_match(it->first, std::regex(sNamePattern))) {
            m_vNameMatches.push_back(it->first);
        }
    }
    return m_vNameMatches;
}



//-----------------------------------------------------------------------------
// display
//
void ArrayShare::display() {
    arraymap::iterator it ;
    for (it = m_mArrays.begin(); it != m_mArrays.end(); ++it) {
        printf("  [%s] -> %p (%d)\n", it->first.c_str(), it->second->m_pdArr, it->second->m_iSize);
    }
}    


