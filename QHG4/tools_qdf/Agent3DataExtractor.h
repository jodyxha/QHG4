#ifndef __AGENT3DATAEXTRACTOR_H__
#define __AGENT3DATAEXTRACTOR_H__


#include <cstdio>
#include <string>
#include <iostream>

#include "hdf5.h"

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


// makes the datastruct compact (no 8-byte padding
//#pragma pack(1)
//----------------------------------------------------------------------------
// val_struct3
//   structure for two variables
//
template<typename T, typename  U, typename V>
class val_struct3 {
public:
    val_struct3(): m_tVal(0) ,m_uVal(0),m_vVal(0){};
    val_struct3(T tVal, U uVal, V vVal):m_tVal(tVal), m_uVal(uVal), m_vVal(vVal) {};

    T m_tVal;
    U m_uVal;
    V m_vVal;
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
};

//----------------------------------------------------------------------------
// val_manager1
//   value manager for one-variable structs
//
template<typename T>
class val_manager1 : public struct_manager {
public:
    val_manager1(): m_pVals(NULL){};
    ~val_manager1() { if (m_pVals != NULL) delete[] m_pVals;};
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
};
    
//----------------------------------------------------------------------------
// val_manager2
//   value manager for two-variable structs
//
template<typename T, typename U>
class val_manager2 : public struct_manager {
public:
    val_manager2(): m_pVals(NULL){};
    ~val_manager2() { if (m_pVals != NULL) delete[] m_pVals;};

    val_struct2<T,U> *m_pVals;

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
};

//----------------------------------------------------------------------------
// val_manager3
//   value manager for two-variable structs
//
template<typename T, typename U, typename V>
class val_manager3 : public struct_manager {
public:
    val_manager3(): m_pVals(NULL){};
    ~val_manager3() { if (m_pVals != NULL) delete[] m_pVals;};

    val_struct3<T,U,V> *m_pVals;

    // display
    virtual void display(int iNumItems) {
        for (int i = 0; i < iNumItems; i++) {
            std::cout << m_pVals[i].m_tVal << ", " << m_pVals[i].m_uVal << ", " << m_pVals[i].m_vVal << std::endl;
        }
    };

    // convert
    virtual void *convert(int iNum) {
        val_struct3<double, double,double> *pDVals = new val_struct3<double,double,double>[iNum];
        for (int i = 0; i < iNum; ++i) {
            pDVals[i].m_tVal    = (double)m_pVals[i].m_tVal;
            pDVals[i].m_uVal    = (double)m_pVals[i].m_uVal;
            pDVals[i].m_vVal    = (double)m_pVals[i].m_vVal;
        }
        val_manager3<double,double,double> *pVSD = new val_manager3<double,double,double>();
     
        pVSD->m_pVals = pDVals;
        return pVSD;
    };

    // getArray
    virtual void *getArray() {return m_pVals;};
};

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

//----------------------------------------------------------------------------
// struct_info2
//   name and offset info for 2-variable structs
//
class struct_info3 {
public: 
    struct_info3(hsize_t iStructSize, std::string sName1, hsize_t iItem1Offset, std::string sName2, hsize_t iItem2Offset, std::string sName3, hsize_t iItem3Offset):
        m_iStructSize(iStructSize), m_sName1(sName1), m_iItem1Offset(iItem1Offset), m_sName2(sName2), m_iItem2Offset(iItem2Offset), m_sName3(sName3), m_iItem3Offset(iItem3Offset){};
    struct_info3():m_iStructSize(0), m_sName1(""), m_iItem1Offset(0), m_sName2(""), m_iItem2Offset(0), m_sName3(""), m_iItem3Offset(0){};
    hsize_t m_iStructSize;
    std::string m_sName1;
    hsize_t m_iItem1Offset;
    std::string m_sName2;
    hsize_t m_iItem2Offset;
    std::string m_sName3;
    hsize_t m_iItem3Offset;
};

class basic_info {
public:
    basic_info():m_sName(":"),m_iItemOffset(0){};
    basic_info(const std::string sName, hsize_t iItemOffset): m_sName(sName), m_iItemOffset(iItemOffset) {};

    std::string m_sName;
    hsize_t m_iItemOffset;
};

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


class var_holder {
public:
    var_holder():type_const(DS_TYPE_NONE){};
    var_holder(int it):type_const(it){};
    int type_const;
};

template<typename T>
class typed_arr : public var_holder {
public:
    typed_arr():var_holder (), m_pArr(NULL){};
    typed_arr(int iT, T *pArr):var_holder (iT), m_pArr(pArr){};
    T * m_pArr;
};

//----------------------------------------------------------------------------
// AgentDataExtractor
//

class Agent3DataExtractor {
public:
    static Agent3DataExtractor *createInstance(const std::string sFileName, std::string sDataSetPath);
    virtual ~Agent3DataExtractor();

    void listDataType();
 
    struct_manager *extractVarV(stringvec &sFieldNames);
    
    size_t getNumItems() { return m_iNumItems;};
protected:
    Agent3DataExtractor();
    int init(const std::string sFileName, std::string sDataSetPath);

    struct_manager *buildStructArray1(std::string sFieldName, hsize_t iNumItems);
    struct_manager *buildStructArray2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems);
    struct_manager *buildStructArray3(std::string sFieldName1, std::string sFieldName2, std::string sFieldName3, hsize_t iNumItems);

    hid_t createCompoundDataTypeV();

    template<typename T>
    struct_manager *setSecondField2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems);

    template<typename T>
    struct_manager *setSecondField3(std::string sFieldName1, std::string sFieldName2, std::string sFieldName3, hsize_t iNumItems);
    template<typename T, typename U>
    struct_manager *setThirdField3(std::string sFieldName1, std::string sFieldName2, std::string sFieldName3, hsize_t iNumItems);
    static int removeDoubleNames(const stringvec &vItems, stringvec &vUniqueItems);

    std::string m_sFileName;
    std::string m_sDataSetPath;
    std::string m_sFieldName;

    hid_t m_hFile;
    hid_t m_hDataSet;
    hid_t m_hDSType;
    hid_t m_hDataSpace;

    hsize_t m_iNumItems;

    full_info     m_vFullInfo;
};
#endif
