#ifndef __AGENT2DATAEXTRACTOR_H__
#define __AGENT2DATAEXTRACTOR_H__


#include <cstdio>
#include <string>
#include <iostream>

#include "hdf5.h"
#include "Sampling.h"


//----------------------------------------------------------------------------
// val_struct1
//   structure for one variable
//
template<typename T>
class val_struct1 {
public:
    val_struct1():m_tVal(0) {};
    val_struct1(T tVal):m_tVal(tVal) {};

    T m_tVal;
};

//----------------------------------------------------------------------------
// val_struct2
//   structure for two variables
//
template<typename T, typename  U>
class val_struct2 {
public:
    val_struct2(): m_tVal(0) ,m_uVal(0){};
    val_struct2(T tVal, U uVal):m_tVal(tVal), m_uVal(uVal) {};

    T m_tVal;
    U m_uVal;
};



//----------------------------------------------------------------------------
// struct_manager
//   abstract base class for templated versions
//
class struct_manager {
public:
    struct_manager() {};
    virtual ~struct_manager() {};
    virtual void display(int iNumItems) = 0;
    virtual void *convert(int iNum) = 0;
    virtual void *getArray() = 0;
    virtual void makeIndexedVals(int iNum, indexedvals<double> &vIndexedVals) =0;

};

//----------------------------------------------------------------------------
// val_manager1
//   value manager for one-variable structs
//
template<typename T>
class val_manager1 : public struct_manager {
public:
    val_manager1(): m_pVals(NULL){};

    // array of one-variable structs
    val_struct1<T> *m_pVals;

    // display
    virtual void display(int iNumItems) {
        for (int i = 0; i < iNumItems; i++) {
            std::cout <<  m_pVals[i].m_tVal << std::endl;
        }
    };

    // convert
    virtual void *convert(int iNum) {
         val_struct1<double> *pDVals = new val_struct1<double>[iNum];
        for (int i = 0; i < iNum; ++i) {
            pDVals[i].m_tVal    = (double)m_pVals[i].m_tVal;
        }
        val_manager1<double> *pVSD = new val_manager1<double>();
     
        pVSD->m_pVals = pDVals;
        return pVSD;
    };

    // getArray
    virtual void *getArray() {return m_pVals;};

    virtual void makeIndexedVals(int iNum, indexedvals<double> &vIndexedVals) {};

};
    
//----------------------------------------------------------------------------
// val_manager2
//   value manager for two-variable structs
//
template<typename T, typename U>
class val_manager2 : public struct_manager {
public:
    val_struct2<T,U> *m_pVals;


    val_manager2(): m_pVals(NULL){};

    ~val_manager2() {if (m_pVals != NULL) { delete[] m_pVals;}};

    // display
    virtual void display(int iNumItems) {
        for (int i = 0; i < iNumItems; i++) {
            std::cout << m_pVals[i].m_tVal << ", " << m_pVals[i].m_uVal << std::endl;
        }
    };

    // convert
    virtual void *convert(int iNum) {
        val_struct2<double, double> *pDVals = new val_struct2<double,double>[iNum];
        for (int i = 0; i < iNum; ++i) {
            pDVals[i].m_tVal    = (double)m_pVals[i].m_tVal;
            pDVals[i].m_uVal    = (double)m_pVals[i].m_uVal;
        }
        val_manager2<double,double> *pVSD = new val_manager2<double,double>();
     
        pVSD->m_pVals = pDVals;
        return pVSD;
    };

    // getArray
    virtual void *getArray() {return m_pVals;};

    virtual void makeIndexedVals(int iNum, indexedvals<double> &vIndexedVals) {
        if (typeid(T) == typeid(int)) {
            for (int i = 0; i < iNum; i++) {
                int iIndex = static_cast<int>(m_pVals[i].m_tVal);
                double dVal = (double)m_pVals[i].m_uVal;
                vIndexedVals.push_back(refval(iIndex, dVal));
            }
        } else {
            // error
        }
    }
};

/*
//----------------------------------------------------------------------------
// struct_info1
//   name and offset info for 1-variable structs
//
class struct_info1 {
public: 
    struct_info1(hsize_t iStructInfo, std::string sName1, hsize_t iItem1Offset):m_iStructSize(iStructInfo), m_sName1(sName1), m_iItem1Offset(iItem1Offset){};
    struct_info1():m_iStructSize(0), m_sName1(""), m_iItem1Offset(0){};
    hsize_t m_iStructSize;
    std::string m_sName1;
     hsize_t m_iItem1Offset;
};

//----------------------------------------------------------------------------
// struct_info2
//   name and offset info for 2-variable structs
//
class struct_info2 {
public: 
    struct_info2(hsize_t iStructSize, std::string sName1, hsize_t iItem1Offset, std::string sName2, hsize_t iItem2Offset):
        m_iStructSize(iStructSize), m_sName1(sName1), m_iItem1Offset(iItem1Offset), m_sName2(sName2), m_iItem2Offset(iItem2Offset){};
    struct_info2():m_iStructSize(0), m_sName1(""), m_iItem1Offset(0), m_sName2(""), m_iItem2Offset(0){};
    hsize_t m_iStructSize;
    std::string m_sName1;
    hsize_t m_iItem1Offset;
    std::string m_sName2;
    hsize_t m_iItem2Offset;
};
*/

//----------------------------------------------------------------------------
// basic_info
//   name and offset info for a variable in a struct
//
class basic_info {
public:
    basic_info():m_sName(":"),m_iItemOffset(0){};
    basic_info(const std::string sName, hsize_t iItemOffset): m_sName(sName), m_iItemOffset(iItemOffset) {};

    std::string m_sName;
    hsize_t m_iItemOffset;
};

//----------------------------------------------------------------------------
// full_info
//   name and offset infos for all variables in a struct
//
class full_info {
public:
    full_info():m_iStructSize(0){};
    full_info(hsize_t iStuctSize):m_iStructSize(iStuctSize){};

    hsize_t m_iStructSize;
    std::vector<basic_info> m_vInfos;
};

// useful for debugging (or messages)
static std::string asClasses[] = {
    "H5T_NO_CLASS",
    "H5T_INTEGER",     
    "H5T_FLOAT",       
    "H5T_TIME",        
    "H5T_STRING",      
    "H5T_BITFIELD",    
    "H5T_OPAQUE",      
    "H5T_COMPOUND",    
    "H5T_REFERENCE",   
    "H5T_ENUM ",       
    "H5T_VLEN ",       
    "H5T_ARRAY",
};


//----------------------------------------------------------------------------
// Agent2DataExtractor
//

class Agent2DataExtractor {
public:
    static Agent2DataExtractor *createInstance(const std::string sFileName, std::string sDataSetPath);
    virtual ~Agent2DataExtractor();

    void listDataType();
 
    struct_manager *extractVarV(stringvec &sFieldNames);
    
    size_t getNumItems() { return m_iNumItems;};
    int setVerbose(bool bVerbosity) {bool bOldV = m_bVerbose; m_bVerbose = bVerbosity; return bOldV;};
protected:
    Agent2DataExtractor();
    int init(const std::string sFileName, std::string sDataSetPath);

    struct_manager *buildStructArray1(std::string sFieldName, hsize_t iNumItems);
    struct_manager *buildStructArray2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems);

    hid_t createCompoundDataTypeV();

    template<typename T>
    struct_manager *setSecondField2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems);

    static int removeDoubleNames(const stringvec &vItems, stringvec &vUniqueItems);

    std::string m_sFileName;
    std::string m_sDataSetPath;
    std::string m_sFieldName;

    hid_t m_hFile;
    hid_t m_hDataSet;
    hid_t m_hDSType;
    hid_t m_hDataSpace;

    hsize_t m_iNumItems;
    hsize_t m_iNumCells;
    
    full_info     m_vFullInfo;
    bool m_bVerbose;
};
#endif
