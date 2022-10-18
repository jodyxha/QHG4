/*============================================================================
| ArrayShare
|
|  A singleton managing a collection of named (void *)-arrays.
|  It is used to share arrays between different SPopulations.
|   
|  Author: Jody Weissmann
\===========================================================================*/

#ifndef __ARRAYSHARE_H__
#define __ARRAYSHARE_H__

#include <string>
#include <map>
#include <vector>


typedef struct arraystruct {
    int m_iSize;
    void *m_pdArr;
    std::string m_sType;
    // constructor
    arraystruct(int iSize, void *pdArr) : m_iSize(iSize), m_pdArr(pdArr), m_sType("") {};
    arraystruct(int iSize, void *pdArr, std::string sType) : m_iSize(iSize), m_pdArr(pdArr), m_sType(sType) {};
} arraystruct;


typedef std::map<std::string, arraystruct *> arraymap;

typedef std::vector<std::string> stringlist;

class ArrayShare {
public:
    ArrayShare();
    virtual ~ArrayShare();

    // register an array for sharing
    int shareArray(const std::string sName, int iSize, void *pdArr);
    int shareArray(const std::string sName, int iSize, std::string sType, void *pdArr);

    // get size of array with name
    int getSize(const std::string sName);

    // get type of array with name
    std::string getType(const std::string sName);

    // get pointer to the array with name
    void *getArray(const std::string sName);

    // get the entire arraystruct with name
    const arraystruct *getArrayStruct(const std::string sName);
    
    // unregister array with name
    int removeArray(const std::string sName);

    // find registerd names matching the pattern
    const stringlist &getNamesLike(const std::string sNamePattern);
    
    void display();    

protected:
    arraymap   m_mArrays;
    stringlist m_vNameMatches;

public:
    static ArrayShare *getInstance();
    static void freeInstance();
protected:
    static ArrayShare *s_pAS;

};

#endif
