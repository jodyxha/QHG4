
#include <cstdio>
#include <string>
#include <algorithm>

#include "hdf5.h"
#include "QDFUtils.h"

#include "Agent3DataExtractor.h"


//----------------------------------------------------------------------------
// createInstance
//
Agent3DataExtractor *Agent3DataExtractor::createInstance(const std::string sFileName, std::string sDataSetPath) {
    Agent3DataExtractor *pADE = new Agent3DataExtractor();
    int iResult = pADE->init(sFileName, sDataSetPath);
    if (iResult != 0) {
        delete pADE;
        pADE =  NULL;
    }
    return pADE;
} 

//----------------------------------------------------------------------------
// destructor
//
Agent3DataExtractor::~Agent3DataExtractor() {

    qdf_closeDataSet(m_hDataSet);
    qdf_closeFile(m_hFile);
}


//----------------------------------------------------------------------------
// listDataType
//
void Agent3DataExtractor::listDataType() { 
   
    int iNum = H5Tget_nmembers(m_hDSType);
    printf("dataset has %d members\n", iNum);
    for (int i = 0; i < iNum; i++) {
        hid_t hMembType =   H5Tget_member_type(m_hDSType, i);
        printf("  %s (%lx)\n",  H5Tget_member_name(m_hDSType, i),  H5Tget_size(hMembType));
        
    }
    
}


//----------------------------------------------------------------------------
// constructor
//
Agent3DataExtractor::Agent3DataExtractor() 
    : m_hFile(H5P_DEFAULT),
      m_hDataSet(H5P_DEFAULT),
      m_hDSType(H5P_DEFAULT),
      m_iNumItems(0) {

}

//----------------------------------------------------------------------------
// init
//
int Agent3DataExtractor::init(const std::string sFileName, std::string sDataSetPath) {
    int iResult = -1;

    m_sFileName = sFileName;
    m_hFile = qdf_openFile(m_sFileName, "r");
    if (m_hFile != H5P_DEFAULT) {
        printf("opened HDF file [%s]\n", m_sFileName.c_str());

        m_hDataSet = qdf_openDataSet(m_hFile, sDataSetPath, true);
        if (m_hDataSet != H5P_DEFAULT) {
            printf("opened ds [%s]\n", sDataSetPath.c_str());
            m_hDSType = H5Dget_type(m_hDataSet);
            H5T_class_t hc = H5Tget_class(m_hDSType);
            printf("DS has class (%d) %s  \n", hc, asClasses[hc+1].c_str()); 
            if (hc == H5T_COMPOUND) {
                m_sDataSetPath = sDataSetPath;
                m_hDataSpace = H5Dget_space(m_hDataSet);
                if (m_hDataSpace != H5P_DEFAULT) {
                    int iNumDims = H5Sget_simple_extent_ndims(m_hDataSpace);
                    if (iNumDims == 1) {
                        
                        iNumDims = H5Sget_simple_extent_dims(m_hDataSpace, &m_iNumItems, NULL);
                        printf("we have a 1-dimensioal array with %lld elments\n", m_iNumItems);
                        iResult = 0;

                    } else {
                        printf("Data space has bad number of dimensions: %d\n", iNumDims);
                    }
                } else {
                    printf("Couldn't get data space for dataset [%s]\n", sDataSetPath.c_str());
                }
             }  else {
                printf("Datatype of [%s] is not 'COMPOUND'\n", sDataSetPath.c_str());
            }
         } else {
            printf("Couldn't open dataset [%s]\n", sDataSetPath.c_str());
        }
    } else {
        printf("Couldn't open [%s] as HDF file\n", m_sFileName.c_str());
    }

    return iResult;
}


//----------------------------------------------------------------------------
// buildStructArray1
//
struct_manager *Agent3DataExtractor::buildStructArray1(std::string sFieldName1, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex  = H5Tget_member_index(m_hDSType, sFieldName1.c_str());
    if (iIndex >= 0) {
        hsize_t iStructSize = 0;
        hsize_t iItem1Offset   = 0;
        
        /*printf("Item [%s] of DS has index %d\n", sFieldName1.c_str(), iIndex);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            //printf("item [%s} is numeric (%d)\n", sFieldName1.c_str(), hc2);
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                printf("FLOAT\n");
                val_manager1<float> *pVM = new val_manager1<float>();
                pVM->m_pVals = new val_struct1<float>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<float>);
                iItem1Offset =  HOFFSET(val_struct1<float>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                printf("DOUBLE\n");
                val_manager1<double> *pVM  = new val_manager1<double>();
                pVM->m_pVals = new val_struct1<double>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<double>);
                iItem1Offset =  HOFFSET(val_struct1<double>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                printf("LDOUBLE\n");
                val_manager1<long double> *pVM  = new val_manager1<long double>();
                pVM->m_pVals = new val_struct1<long double>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<long double>);
                iItem1Offset =  HOFFSET(val_struct1<long double>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                printf("CHAR\n");
                val_manager1<char> *pVM  = new val_manager1<char>();
                pVM->m_pVals = new val_struct1<char>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<char>);
                iItem1Offset =  HOFFSET(val_struct1<char>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                printf("UCHAR\n");
                val_manager1<uchar> *pVM  = new val_manager1<uchar>();
                pVM->m_pVals = new val_struct1<uchar>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<uchar>);
                iItem1Offset =  HOFFSET(val_struct1<uchar>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                printf("SHORT\n");
                val_manager1<short int> *pVM  = new val_manager1<short int>();
                pVM->m_pVals = new val_struct1<short int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<short int>);
                iItem1Offset =  HOFFSET(val_struct1<short int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                printf("USHORT\n");
                val_manager1<unsigned short int> *pVM  = new val_manager1<unsigned short int>();
                pVM->m_pVals = new val_struct1<unsigned short int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<unsigned short int>);
                iItem1Offset =  HOFFSET(val_struct1<unsigned short int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                printf("INT32\n");
                val_manager1<int> *pVM  = new val_manager1<int>();
                pVM->m_pVals = new val_struct1<int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<int>);
                iItem1Offset =  HOFFSET(val_struct1<int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                printf("UINT32\n");
                val_manager1<uint> *pVM  = new val_manager1<uint>();
                pVM->m_pVals = new val_struct1<uint>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<uint>);
                iItem1Offset =  HOFFSET(val_struct1<uint>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                printf("LONG\n");
                val_manager1<long int> *pVM  = new val_manager1<long int>();
                pVM->m_pVals = new val_struct1<long int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<long int>);
                iItem1Offset =  HOFFSET(val_struct1<long int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                printf("ULONG\n");
                val_manager1<unsigned long int> *pVM  = new val_manager1<unsigned long int>();
                pVM->m_pVals = new val_struct1<unsigned long int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<unsigned long int>);
                iItem1Offset =  HOFFSET(val_struct1<unsigned long int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                printf("LLONG\n");
                val_manager1<long long int> *pVM  = new val_manager1<long long int>();
                pVM->m_pVals = new val_struct1<long long int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<long long int>);
                iItem1Offset =  HOFFSET(val_struct1<long long int>,  m_tVal);
            } else {
                printf("Something elsen");
            }
            if (pSM != NULL) {
                printf("size: %lld, [%s] offset %lld\n", iStructSize, sFieldName1.c_str(), iItem1Offset);
                m_vFullInfo.m_iStructSize = iStructSize;
                m_vFullInfo.m_vInfos.push_back({sFieldName1, iItem1Offset});

            }
            
        } else {
            printf("Data element is not numericaln");
        }
    } else {
        printf("DS does not have an item named [%s]\n", sFieldName1.c_str());
    }
    return pSM;
}


//----------------------------------------------------------------------------
// setSecondField2
//
template<typename T>
struct_manager *Agent3DataExtractor::setSecondField2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex2  = H5Tget_member_index(m_hDSType, sFieldName2.c_str());
    if (iIndex2 >= 0) {
        hsize_t iStructSize = 0;
        hsize_t iItem1Offset   = 0;
        hsize_t iItem2Offset   = 0;
        
        /*printf("Item [%s] of DS has index %d\n", sFieldName1.c_str(), iIndex2);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex2); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            //printf("item [%s} is numeric (%d)\n", sFieldName1.c_str(), hc2);
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex2);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                printf("FLOAT\n");
                val_manager2<T, float> *pVM = new val_manager2<T, float>();
                using tCur = val_struct2<T, float>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);


            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                printf("DOUBLE\n");
                val_manager2<T, double> *pVM  = new val_manager2<T, double>();
                using tCur = val_struct2<T, double>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                printf("LDOUBLE\n");
                val_manager2<T, long double> *pVM  = new val_manager2<T, long double>();
                using tCur = val_struct2<T, long double>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                printf("CHAR\n");
                val_manager2<T, char> *pVM  = new val_manager2<T, char>();
                using tCur = val_struct2<T, char>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                printf("UCHAR\n");
                val_manager2<T, uchar> *pVM  = new val_manager2<T, uchar>();
                using tCur = val_struct2<T, uchar>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                printf("SHORT\n");
                val_manager2<T, short int> *pVM  = new val_manager2<T, short int>();
                using tCur = val_struct2<T, short int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                printf("USHORT\n");
                val_manager2<T, unsigned short int> *pVM  = new val_manager2<T, unsigned short int>();
                using tCur = val_struct2<T, unsigned short int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                printf("INT32\n");
                val_manager2<T, int> *pVM  = new val_manager2<T, int>();
                using tCur = val_struct2<T, int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                printf("UINT32\n");
                val_manager2<T, uint> *pVM  = new val_manager2<T, uint>();
                using tCur = val_struct2<T, uint>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                printf("LONG\n");
                val_manager2<T, long int> *pVM  = new val_manager2<T, long int>();
                using tCur = val_struct2<T, long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                printf("ULONG\n");
                val_manager2<T, unsigned long int> *pVM  = new val_manager2<T, unsigned long int>();
                using tCur = val_struct2<T, unsigned long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                printf("LLONG\n");
                val_manager2<T, long long int> *pVM  = new val_manager2<T, long long int>();
                using tCur = val_struct2<T, long long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else {
                printf("Something elsen");
            }
            if (pSM != NULL) {
                printf("size: %lld, [%s] offset %lld, [%s] offset %lld\n", iStructSize, sFieldName1.c_str(), iItem1Offset, sFieldName2.c_str(), iItem2Offset);
                m_vFullInfo.m_iStructSize = iStructSize;
                m_vFullInfo.m_vInfos.push_back({ sFieldName1, iItem1Offset});
                m_vFullInfo.m_vInfos.push_back({ sFieldName2, iItem2Offset});
     
            }
            
        } else {
            printf("Data element is not numericaln");
        }
    } else {
        printf("DS does not have an item named [%s]\n", sFieldName1.c_str());
    }
    return pSM;
}

//----------------------------------------------------------------------------
// setSecondField3
//
template<typename T>
struct_manager *Agent3DataExtractor::setSecondField3(std::string sFieldName1, std::string sFieldName2, std::string sFieldName3, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex2  = H5Tget_member_index(m_hDSType, sFieldName2.c_str());
    if (iIndex2 >= 0) {
       
        //printf("Item [%s] of DS has index %d\n", sFieldName2.c_str(), iIndex2);
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex2); 
        
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            //printf("item [%s] is numeric (%d)\n", sFieldName2.c_str(), hc2);
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex2);
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                printf("FLOAT\n");
            
                pSM = setThirdField3<T,float>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            
            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                printf("DOUBLE\n");
   
                pSM = setThirdField3<T, double>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
       
            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                printf("LDOUBLE\n");
                pSM = setThirdField3<T, long double>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
                
            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                printf("CHAR\n");
                pSM = setThirdField3<T, char>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
              
            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                printf("UCHAR\n");
                pSM = setThirdField3<T, uchar>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
             
            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                printf("SHORT\n");
                pSM = setThirdField3<T, short int>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
               
            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                printf("USHORT\n");
                pSM = setThirdField3<T, unsigned short int>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
                
            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                printf("INT32\n");
                pSM = setThirdField3<T, int>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
               
            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                printf("UINT32\n");
                pSM = setThirdField3<T, uint>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
                
            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                printf("LONG\n");
                pSM = setThirdField3<T, long>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
              
            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                printf("ULONG\n");
                pSM = setThirdField3<T, unsigned long>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
                
            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                printf("LLONG\n");
                pSM = setThirdField3<T, long long>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
               

            } else {
                printf("Something elsen");
            }
           
            
        } else {
            printf("Data element is not numeric\n");
        }
    } else {
        printf("DS does not have an item named [%s]\n", sFieldName2.c_str());
    }
    return pSM;
}

//----------------------------------------------------------------------------
// setThirdField3
//
template<typename T, typename U>
struct_manager *Agent3DataExtractor::setThirdField3(std::string sFieldName1, std::string sFieldName2, std::string sFieldName3, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex3  = H5Tget_member_index(m_hDSType, sFieldName3.c_str());
    if (iIndex3 >= 0) {
        hsize_t iStructSize   = 0;
        hsize_t iItem1Offset  = 0;
        hsize_t iItem2Offset  = 0;
        hsize_t iItem3Offset  = 0;
       
        /*printf("Item [%s] of DS has index %d\n", sFieldName3.c_str(), iIndex3);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex3); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            /*printf("item [%s} is numeric (%d)\n", sFieldName3.c_str(), hc2);*/
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex3);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                printf("FLOAT\n");
                val_manager3<T,U, float> *pVM = new val_manager3<T,U,float>();
                using tCur = val_struct3<T,U,float>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                printf("DOUBLE\n");
                val_manager3<T,U,double> *pVM  = new val_manager3<T,U,double>();
                using tCur = val_struct3<T,U,double>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                printf("LDOUBLE\n");
                val_manager3<T,U,long double> *pVM  = new val_manager3<T,U,long double>();
                using tCur = val_struct3<T,U,long double>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                printf("CHAR\n");
                val_manager3<T,U,char> *pVM  = new val_manager3<T,U,char>();
                using tCur = val_struct3<T,U,char>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                printf("UCHAR\n");
                val_manager3<T,U,uchar> *pVM  = new val_manager3<T,U,uchar>();
                using tCur = val_struct3<T,U,uchar>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                printf("SHORT\n");
                val_manager3<T,U,short int> *pVM  = new val_manager3<T,U,short int>();
                using tCur = val_struct3<T,U,short int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                printf("USHORT\n");
                val_manager3<T,U,unsigned short int> *pVM  = new val_manager3<T,U,unsigned short int>();
                using tCur = val_struct3<T,U,unsigned short int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                printf("INT32\n");
                val_manager3<T,U,int> *pVM  = new val_manager3<T,U,int>();
                using tCur = val_struct3<T,U,int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                printf("UINT32\n");
                val_manager3<T,U,uint> *pVM  = new val_manager3<T,U,uint>();
                using tCur = val_struct3<T,U,uint>; 
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                printf("LONG\n");
                val_manager3<T,U,long int> *pVM  = new val_manager3<T,U,long int>();
                using tCur = val_struct3<T,U,long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                printf("ULONG\n");
                val_manager3<T,U,unsigned long int> *pVM  = new val_manager3<T,U,unsigned long int>();
                using tCur = val_struct3<T,U,unsigned long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                printf("LLONG\n");
                val_manager3<T,U,long long int> *pVM  = new val_manager3<T,U,long long int>();
                using tCur = val_struct3<T,U,long long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize  = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);
                iItem3Offset =  HOFFSET(tCur,  m_vVal);

            } else {
                printf("Something elsen");
            }
            if (pSM != NULL) {
                printf("size: %lld, [%s] offset %lld, [%s] offset %lld, [%s] offset %lld\n", iStructSize, sFieldName1.c_str(), iItem1Offset, sFieldName2.c_str(), iItem2Offset, sFieldName3.c_str(), iItem3Offset);
                m_vFullInfo.m_iStructSize = iStructSize;
                m_vFullInfo.m_vInfos.push_back({ sFieldName1, iItem1Offset});
                m_vFullInfo.m_vInfos.push_back({ sFieldName2, iItem2Offset});
                m_vFullInfo.m_vInfos.push_back({ sFieldName3, iItem3Offset});
           
            }
            
        } else {
            printf("Data element is not numeric\n");
        }
    } else {
        printf("DS does not have an item named [%s]\n", sFieldName2.c_str());
    }
    return pSM;
}


//----------------------------------------------------------------------------
// buildStructArray2
//
struct_manager *Agent3DataExtractor::buildStructArray2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex1  = H5Tget_member_index(m_hDSType, sFieldName1.c_str());
    if (iIndex1 >= 0) {
        
        /*printf("Item [%s] of DS has index %d\n", sFieldName1.c_str(), iIndex1);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex1); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            /*printf("item [%s} is numeric (%d)\n", sFieldName1.c_str(), hc2);*/
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex1);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                printf("FLOAT\n");
               
                pSM = setSecondField2<float>(sFieldName1, sFieldName2, iNumItems);
                
            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                printf("DOUBLE\n");
   
                pSM = setSecondField2<double>(sFieldName1, sFieldName2, iNumItems);
              
            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                printf("LDOUBLE\n");
                pSM = setSecondField2<long double>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                printf("CHAR\n");
                pSM = setSecondField2<char>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                printf("UCHAR\n");
                pSM = setSecondField2<uchar>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                printf("SHORT\n");
                pSM = setSecondField2<short int>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                printf("USHORT\n");
                pSM = setSecondField2<unsigned short int>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                printf("INT32\n");
                pSM = setSecondField2<int>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                printf("UINT32\n");
                pSM = setSecondField2<uint>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                printf("LONG\n");
                pSM = setSecondField2<long>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                printf("ULONG\n");
                pSM = setSecondField2<unsigned long>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                printf("LLONG\n");
                pSM = setSecondField2<long long>(sFieldName1, sFieldName2, iNumItems);
            } else {
                printf("Something else\n");
            }
           
        } else {
            printf("Data element is not numeric\n");
        }
    } else {
        printf("DS does not have an item named [%s]\n", sFieldName1.c_str());
    }
    return pSM;
}

//----------------------------------------------------------------------------
// buildStructArray3
//
struct_manager *Agent3DataExtractor::buildStructArray3(std::string sFieldName1, std::string sFieldName2, std::string sFieldName3, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex1  = H5Tget_member_index(m_hDSType, sFieldName1.c_str());
    if (iIndex1 >= 0) {
       
        /*printf("Item [%s] of DS has index %d\n", sFieldName1.c_str(), iIndex1);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex1); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            /*printf("item [%s} is numeric (%d)\n", sFieldName1.c_str(), hc2);*/
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex1);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                printf("FLOAT\n");
               
                pSM = setSecondField3<float>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
                
            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                printf("DOUBLE\n");
   
                pSM = setSecondField3<double>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
              
            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                printf("LDOUBLE\n");
                pSM = setSecondField3<long double>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                printf("CHAR\n");
                pSM = setSecondField3<char>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                printf("UCHAR\n");
                pSM = setSecondField3<uchar>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                printf("SHORT\n");
                pSM = setSecondField3<short int>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                printf("USHORT\n");
                pSM = setSecondField3<unsigned short int>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                printf("INT32\n");
                pSM = setSecondField3<int>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                printf("UINT32\n");
                pSM = setSecondField3<uint>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                printf("LONG\n");
                pSM = setSecondField3<long>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                printf("ULONG\n");
                pSM = setSecondField3<unsigned long>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                printf("LLONG\n");
                pSM = setSecondField3<long long>(sFieldName1, sFieldName2, sFieldName3, iNumItems);
            } else {
                printf("Something elsen");
            }
           
        } else {
            printf("Data element is not numericaln");
        }
    } else {
        printf("DS does not have an item named [%s]\n", sFieldName1.c_str());
    }
    return pSM;
}

//----------------------------------------------------------------------------
// extractVarv
//
struct_manager *Agent3DataExtractor::extractVarV(stringvec &vFieldNames) {

    struct_manager *pSM = NULL;
    stringvec vUniqueFieldNames;

    int iNum = removeDoubleNames(vFieldNames, vUniqueFieldNames);
    if (iNum > 0) {
        printf("Removed %d multiples\n", iNum);
    }

    m_vFullInfo.m_vInfos.clear();

    switch (vUniqueFieldNames.size()) {
    case 1:
        pSM = buildStructArray1(vUniqueFieldNames[0], m_iNumItems);
        break;
    case 2:
        pSM = buildStructArray2(vUniqueFieldNames[0], vUniqueFieldNames[1], m_iNumItems);
        break;
    case 3:
        pSM = buildStructArray3(vUniqueFieldNames[0], vUniqueFieldNames[1], vUniqueFieldNames[2], m_iNumItems);
        break;
    default:
        printf("More than 3 vars not yet supported(%zd)\n", vUniqueFieldNames.size());
    }

    if (pSM != NULL) {
        hid_t hAgentDataType = createCompoundDataTypeV();
        
        if (hAgentDataType != H5P_DEFAULT) {

            hsize_t dims;
            H5Sget_simple_extent_dims(m_hDataSpace, &dims, NULL);

            hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
            herr_t status = H5Dread(m_hDataSet, hAgentDataType, hMemSpace, m_hDataSpace, H5P_DEFAULT, pSM->getArray());
            if (status >= 0) {
                printf("Read %lld items\n", m_iNumItems);
                
            } else {
                printf("bad status for read\n");
            }
            qdf_closeDataSpace(hMemSpace);
    
 
            H5Tclose(hAgentDataType);
        } else {
            printf("Couldn't build agent data type\n");
        }
     } else {
        printf("Couldn't build struct array\n");
    }
    
    return pSM ;

}



//----------------------------------------------------------------------------
// removeDoubleNames
//
int Agent3DataExtractor::removeDoubleNames(const stringvec &vItems, stringvec &vUniqueItems) {
    int iResult = 0;
    
    for (unsigned i = 0; i < vItems.size(); i++) {
        stringvec::const_iterator it = std::find(vUniqueItems.begin(), vUniqueItems.end(), vItems[i]);
        if (it == vUniqueItems.end()) {
            vUniqueItems.push_back(vItems[i]);
        } else {
            iResult++;
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// createCompoundDataTypev
//   create a compound datattype containing
//      cell  id
//      item 
//

hid_t Agent3DataExtractor::createCompoundDataTypeV() {

   hid_t hAgentDataType = H5P_DEFAULT;
   
   //printf("Setting struct size %lld\n",  m_vFullInfo.m_iStructSize);
   hAgentDataType = H5Tcreate(H5T_COMPOUND, m_vFullInfo.m_iStructSize);

   for (unsigned i = 0; i < m_vFullInfo.m_vInfos.size(); i++) {
       int iIndex  = H5Tget_member_index(m_hDSType, m_vFullInfo.m_vInfos[i].m_sName.c_str());
       hid_t hMembType =   H5Tget_member_type(m_hDSType, iIndex);
       
       //printf("adding entry  [%s] with offset %lld\n", m_vFullInfo.m_vInfos[i].m_sName.c_str(), m_vFullInfo.m_vInfos[i].m_iItemOffset); fflush(stdout);
       H5Tinsert(hAgentDataType, m_vFullInfo.m_vInfos[i].m_sName.c_str(), m_vFullInfo.m_vInfos[i].m_iItemOffset,  hMembType);
   }
    return hAgentDataType;
}

/*
have vFieldNames
cur_offs = 0;
loop:
  for s in vFieldNames;
    htype(s) = get hdftype 
    ctype(s) = int constant for type (cf QDF_Utils.h: DS_TYPE_XXX))
    offs(s) = cur_offs
    cur_offs += sizeof (type(s)) 
structsize = cur_offs

prepare an uchar array of numItems*structsize elements

make compound datatype from offs(s) and htype(s)

Read data

p = array
depending on type do a memcpy(var, p, sizeof(var)), p+= sizeof(var)

*/

/*
variant: extract every 1d array separately and convert to double & save in tyoed_arr of the correct type (if else if else if...)

struct var_holder {
    int type_const;
};

template<typename T>
struct typed_arr<T> : public var_holder {
    
    T * m_pArr;
}
*/
