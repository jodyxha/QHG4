
#include <cstdio>
#include <string>
#include <algorithm>

#include "hdf5.h"
#include "QDFUtils.h"
#include "strutils.h"
#include "xha_strutilsT.h"

#include "Agent2DataExtractor.h"


//----------------------------------------------------------------------------
// createInstance
//
Agent2DataExtractor *Agent2DataExtractor::createInstance(const std::string sFileName, std::string sDataSetPath) {
    Agent2DataExtractor *pADE = new Agent2DataExtractor();
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
Agent2DataExtractor::~Agent2DataExtractor() {

    qdf_closeDataSet(m_hDataSet);
    qdf_closeFile(m_hFile);
}


//----------------------------------------------------------------------------
// listDataType
//
void Agent2DataExtractor::listDataType() { 
   
    int iNum = H5Tget_nmembers(m_hDSType);
    xha_printf("dataset has %d members\n", iNum);
    for (int i = 0; i < iNum; i++) {
        hid_t hMembType =   H5Tget_member_type(m_hDSType, i);
        xha_printf("  %s (%lx)\n",  H5Tget_member_name(m_hDSType, i),  H5Tget_size(hMembType));
        
    }
    
}


//----------------------------------------------------------------------------
// constructor
//
Agent2DataExtractor::Agent2DataExtractor() 
    : m_hFile(H5P_DEFAULT),
      m_hDataSet(H5P_DEFAULT),
      m_hDSType(H5P_DEFAULT),
      m_iNumItems(0),
      m_bVerbose(false) {

}


//----------------------------------------------------------------------------
// init
//
int Agent2DataExtractor::init(const std::string sFileName, std::string sDataSetPath) {
    int iResult = -1;

    m_sFileName = sFileName;
    m_hFile = qdf_openFile(m_sFileName, "r");
    if (m_hFile != H5P_DEFAULT) {
        if (m_bVerbose) xha_printf("opened HDF file [%s]\n", m_sFileName);

        m_hDataSet = qdf_openDataSet(m_hFile, sDataSetPath, true);
        if (m_hDataSet != H5P_DEFAULT) {
            if (m_bVerbose) xha_printf("opened ds [%s]\n", sDataSetPath);
            m_hDSType = H5Dget_type(m_hDataSet);
            H5T_class_t hc = H5Tget_class(m_hDSType);
            if (m_bVerbose) xha_printf("DS has class (%d) %s  \n", hc, asClasses[hc+1]); 
            if (hc == H5T_COMPOUND) {
                m_sDataSetPath = sDataSetPath;
                m_hDataSpace = H5Dget_space(m_hDataSet);
                if (m_hDataSpace != H5P_DEFAULT) {
                    int iNumDims = H5Sget_simple_extent_ndims(m_hDataSpace);
                    if (iNumDims == 1) {
                        
                        iNumDims = H5Sget_simple_extent_dims(m_hDataSpace, &m_iNumItems, NULL);
                        if (m_bVerbose) xha_printf("we have a 1-dimensioal array with %lld elments\n", m_iNumItems);
                        iResult = 0;

                    } else {
                        xha_printf("Data space has bad number of dimensions: %d\n", iNumDims);
                    }
                } else {
                    xha_printf("Couldn't get data space for dataset [%s]\n", sDataSetPath);
                }
             }  else {
                xha_printf("Datatype of [%s] is not 'COMPOUND'\n", sDataSetPath);
            }
         } else {
            xha_printf("Couldn't open dataset [%s]\n", sDataSetPath);
        }
    } else {
        xha_printf("Couldn't open [%s] as HDF file\n", m_sFileName);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// buildStructArray1
//
struct_manager *Agent2DataExtractor::buildStructArray1(std::string sFieldName1, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex  = H5Tget_member_index(m_hDSType, sFieldName1.c_str());
    if (iIndex >= 0) {
        hsize_t iStructSize = 0;
        hsize_t iItem1Offset   = 0;
        
        /*xha_printf("Item [%s] of DS has index %d\n", sFieldName1, iIndex);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            //xha_printf("item [%s} is numeric (%d)\n", sFieldName1, hc2);
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                if (m_bVerbose) xha_printf("FLOAT\n");
                val_manager1<float> *pVM = new val_manager1<float>();
                pVM->m_pVals = new val_struct1<float>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<float>);
                iItem1Offset =  HOFFSET(val_struct1<float>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                if (m_bVerbose) xha_printf("DOUBLE\n");
                val_manager1<double> *pVM  = new val_manager1<double>();
                pVM->m_pVals = new val_struct1<double>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<double>);
                iItem1Offset =  HOFFSET(val_struct1<double>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                if (m_bVerbose) xha_printf("LDOUBLE\n");
                val_manager1<long double> *pVM  = new val_manager1<long double>();
                pVM->m_pVals = new val_struct1<long double>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<long double>);
                iItem1Offset =  HOFFSET(val_struct1<long double>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                if (m_bVerbose) xha_printf("CHAR\n");
                val_manager1<char> *pVM  = new val_manager1<char>();
                pVM->m_pVals = new val_struct1<char>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<char>);
                iItem1Offset =  HOFFSET(val_struct1<char>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                if (m_bVerbose) xha_printf("UCHAR\n");
                val_manager1<uchar> *pVM  = new val_manager1<uchar>();
                pVM->m_pVals = new val_struct1<uchar>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<uchar>);
                iItem1Offset =  HOFFSET(val_struct1<uchar>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                if (m_bVerbose) xha_printf("SHORT\n");
                val_manager1<short int> *pVM  = new val_manager1<short int>();
                pVM->m_pVals = new val_struct1<short int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<short int>);
                iItem1Offset =  HOFFSET(val_struct1<short int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                if (m_bVerbose) xha_printf("USHORT\n");
                val_manager1<unsigned short int> *pVM  = new val_manager1<unsigned short int>();
                pVM->m_pVals = new val_struct1<unsigned short int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<unsigned short int>);
                iItem1Offset =  HOFFSET(val_struct1<unsigned short int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                if (m_bVerbose) xha_printf("INT32\n");
                val_manager1<int> *pVM  = new val_manager1<int>();
                pVM->m_pVals = new val_struct1<int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<int>);
                iItem1Offset =  HOFFSET(val_struct1<int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                if (m_bVerbose) xha_printf("UINT32\n");
                val_manager1<uint> *pVM  = new val_manager1<uint>();
                pVM->m_pVals = new val_struct1<uint>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<uint>);
                iItem1Offset =  HOFFSET(val_struct1<uint>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                if (m_bVerbose) xha_printf("LONG\n");
                val_manager1<long int> *pVM  = new val_manager1<long int>();
                pVM->m_pVals = new val_struct1<long int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<long int>);
                iItem1Offset =  HOFFSET(val_struct1<long int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                if (m_bVerbose) xha_printf("ULONG\n");
                val_manager1<unsigned long int> *pVM  = new val_manager1<unsigned long int>();
                pVM->m_pVals = new val_struct1<unsigned long int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<unsigned long int>);
                iItem1Offset =  HOFFSET(val_struct1<unsigned long int>,  m_tVal);
            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                if (m_bVerbose) xha_printf("LLONG\n");
                val_manager1<long long int> *pVM  = new val_manager1<long long int>();
                pVM->m_pVals = new val_struct1<long long int>[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(val_struct1<long long int>);
                iItem1Offset =  HOFFSET(val_struct1<long long int>,  m_tVal);
            } else {
                xha_printf("Unsupported type");
            }
            if (pSM != NULL) {
                if (m_bVerbose) xha_printf("size: %lld, [%s] offset %lld\n", iStructSize, sFieldName1, iItem1Offset);
                m_vFullInfo.m_iStructSize = iStructSize;
                m_vFullInfo.m_vInfos.push_back({sFieldName1, iItem1Offset});

            }
            
        } else {
            xha_printf("Data element is not numericaln");
        }
    } else {
        xha_printf("DS does not have an item named [%s]\n", sFieldName1);
    }
    return pSM;
}


//----------------------------------------------------------------------------
// setSecondField2
//
template<typename T>
struct_manager *Agent2DataExtractor::setSecondField2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex2  = H5Tget_member_index(m_hDSType, sFieldName2.c_str());
    if (iIndex2 >= 0) {
        hsize_t iStructSize = 0;
        hsize_t iItem1Offset   = 0;
        hsize_t iItem2Offset   = 0;
        
        /*xha_printf("Item [%s] of DS has index %d\n", sFieldName1, iIndex2);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex2); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            //xha_printf("item [%s} is numeric (%d)\n", sFieldName1, hc2);
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex2);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                if (m_bVerbose) xha_printf("FLOAT\n");
                val_manager2<T, float> *pVM = new val_manager2<T, float>();
                using tCur = val_struct2<T, float>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);


            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                if (m_bVerbose) xha_printf("DOUBLE\n");
                val_manager2<T, double> *pVM  = new val_manager2<T, double>();
                using tCur = val_struct2<T, double>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                if (m_bVerbose) xha_printf("LDOUBLE\n");
                val_manager2<T, long double> *pVM  = new val_manager2<T, long double>();
                using tCur = val_struct2<T, long double>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                if (m_bVerbose) xha_printf("CHAR\n");
                val_manager2<T, char> *pVM  = new val_manager2<T, char>();
                using tCur = val_struct2<T, char>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                if (m_bVerbose) xha_printf("UCHAR\n");
                val_manager2<T, uchar> *pVM  = new val_manager2<T, uchar>();
                using tCur = val_struct2<T, uchar>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                if (m_bVerbose) xha_printf("SHORT\n");
                val_manager2<T, short int> *pVM  = new val_manager2<T, short int>();
                using tCur = val_struct2<T, short int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                if (m_bVerbose) xha_printf("USHORT\n");
                val_manager2<T, unsigned short int> *pVM  = new val_manager2<T, unsigned short int>();
                using tCur = val_struct2<T, unsigned short int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                if (m_bVerbose) xha_printf("INT32\n");
                val_manager2<T, int> *pVM  = new val_manager2<T, int>();
                using tCur = val_struct2<T, int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                if (m_bVerbose) xha_printf("UINT32\n");
                val_manager2<T, uint> *pVM  = new val_manager2<T, uint>();
                using tCur = val_struct2<T, uint>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                if (m_bVerbose) xha_printf("LONG\n");
                val_manager2<T, long int> *pVM  = new val_manager2<T, long int>();
                using tCur = val_struct2<T, long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                if (m_bVerbose) xha_printf("ULONG\n");
                val_manager2<T, unsigned long int> *pVM  = new val_manager2<T, unsigned long int>();
                using tCur = val_struct2<T, unsigned long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                if (m_bVerbose) xha_printf("LLONG\n");
                val_manager2<T, long long int> *pVM  = new val_manager2<T, long long int>();
                using tCur = val_struct2<T, long long int>;
                pVM->m_pVals = new tCur[iNumItems];
                pSM = pVM;
                iStructSize = sizeof(tCur);
                iItem1Offset =  HOFFSET(tCur,  m_tVal);
                iItem2Offset =  HOFFSET(tCur,  m_uVal);

            } else {
                xha_printf("Unsupported type");
            }
            if (pSM != NULL) {
                if (m_bVerbose) xha_printf("size: %lld, [%s] offset %lld, [%s] offset %lld\n", iStructSize, sFieldName1, iItem1Offset, sFieldName2, iItem2Offset);
                m_vFullInfo.m_iStructSize = iStructSize;
                m_vFullInfo.m_vInfos.push_back({ sFieldName1, iItem1Offset});
                m_vFullInfo.m_vInfos.push_back({ sFieldName2, iItem2Offset});
     
            }
            
        } else {
            xha_printf("Data element is not numericaln");
        }
    } else {
        xha_printf("DS does not have an item named [%s]\n", sFieldName2);
    }
    return pSM;
}


//----------------------------------------------------------------------------
// buildStructArray2
//
struct_manager *Agent2DataExtractor::buildStructArray2(std::string sFieldName1, std::string sFieldName2, hsize_t iNumItems) {

    struct_manager *pSM = NULL;

    int iIndex1  = H5Tget_member_index(m_hDSType, sFieldName1.c_str());
    if (iIndex1 >= 0) {
        
        /*xha_printf("Item [%s] of DS has index %d\n", sFieldName1, iIndex1);*/
        H5T_class_t hc2 = H5Tget_member_class(m_hDSType, iIndex1); 
        if ((hc2 == H5T_FLOAT) || (hc2 == H5T_INTEGER)) {
            /*xha_printf("item [%s} is numeric (%d)\n", sFieldName1, hc2);*/
            hid_t htype = H5Tget_member_type(m_hDSType, iIndex1);
            
            if (H5Tequal(htype, H5T_NATIVE_FLOAT)) {
                if (m_bVerbose) xha_printf("FLOAT\n");
               
                pSM = setSecondField2<float>(sFieldName1, sFieldName2, iNumItems);
                
            } else if (H5Tequal(htype, H5T_NATIVE_DOUBLE)) {
                if (m_bVerbose) xha_printf("DOUBLE\n");
   
                pSM = setSecondField2<double>(sFieldName1, sFieldName2, iNumItems);
              
            } else if (H5Tequal(htype, H5T_NATIVE_LDOUBLE)) {
                if (m_bVerbose) xha_printf("LDOUBLE\n");
                pSM = setSecondField2<long double>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_CHAR)) {
                if (m_bVerbose) xha_printf("CHAR\n");
                pSM = setSecondField2<char>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_UCHAR)) {
                if (m_bVerbose) xha_printf("UCHAR\n");
                pSM = setSecondField2<uchar>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_SHORT)) {
                if (m_bVerbose) xha_printf("SHORT\n");
                pSM = setSecondField2<short int>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_USHORT)) {
                if (m_bVerbose) xha_printf("USHORT\n");
                pSM = setSecondField2<unsigned short int>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_INT32)) {
                if (m_bVerbose) xha_printf("INT32\n");
                pSM = setSecondField2<int>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_UINT32)) {
                if (m_bVerbose) xha_printf("UINT32\n");
                pSM = setSecondField2<uint>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_LONG)) {
                if (m_bVerbose) xha_printf("LONG\n");
                pSM = setSecondField2<long>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_ULONG)) {
                if (m_bVerbose) xha_printf("ULONG\n");
                pSM = setSecondField2<unsigned long>(sFieldName1, sFieldName2, iNumItems);
            } else if (H5Tequal(htype, H5T_NATIVE_LLONG)) {
                if (m_bVerbose) xha_printf("LLONG\n");
                pSM = setSecondField2<long long>(sFieldName1, sFieldName2, iNumItems);
            } else {
                xha_printf("Unsupported type\n");
            }
           
        } else {
            xha_printf("Data element is not numeric\n");
        }
    } else {
        xha_printf("DS does not have an item named [%s]\n", sFieldName1);
    }
    return pSM;
}


//----------------------------------------------------------------------------
// extractVarv
//
struct_manager *Agent2DataExtractor::extractVarV(stringvec &vFieldNames) {

    struct_manager *pSM = NULL;
    stringvec vUniqueFieldNames;

    int iNum = removeDoubleNames(vFieldNames, vUniqueFieldNames);
    if (iNum > 0) {
       if (m_bVerbose)  xha_printf("Removed %d multiples\n", iNum);
    }

    m_vFullInfo.m_vInfos.clear();

    switch (vUniqueFieldNames.size()) {
    case 1:
        pSM = buildStructArray1(vUniqueFieldNames[0], m_iNumItems);
        break;
    case 2:
        pSM = buildStructArray2(vUniqueFieldNames[0], vUniqueFieldNames[1], m_iNumItems);
        break;
    default:
        xha_printf("More than 3 vars not yet supported(%zd)\n", vUniqueFieldNames.size());
    }

    if (pSM != NULL) {
        hid_t hAgentDataType = createCompoundDataTypeV();
        
        if (hAgentDataType != H5P_DEFAULT) {

            hsize_t dims;
            H5Sget_simple_extent_dims(m_hDataSpace, &dims, NULL);

            hid_t hMemSpace = H5Screate_simple (1, &dims, NULL); 
            herr_t status = H5Dread(m_hDataSet, hAgentDataType, hMemSpace, m_hDataSpace, H5P_DEFAULT, pSM->getArray());
            if (status >= 0) {
                if (m_bVerbose) xha_printf("Read %lld items\n", m_iNumItems);
                
            } else {
                xha_printf("bad status for read\n");
            }
            qdf_closeDataSpace(hMemSpace);
    
 
            H5Tclose(hAgentDataType);
        } else {
            xha_printf("Couldn't build agent data type\n");
        }
     } else {
        xha_printf("Couldn't build struct array\n");
    }
    
    return pSM ;

}



//----------------------------------------------------------------------------
// removeDoubleNames
//
int Agent2DataExtractor::removeDoubleNames(const stringvec &vItems, stringvec &vUniqueItems) {
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

hid_t Agent2DataExtractor::createCompoundDataTypeV() {

   hid_t hAgentDataType = H5P_DEFAULT;
   
   //printf("Setting struct size %lld\n",  m_vFullInfo.m_iStructSize);
   hAgentDataType = H5Tcreate(H5T_COMPOUND, m_vFullInfo.m_iStructSize);

   for (unsigned i = 0; i < m_vFullInfo.m_vInfos.size(); i++) {
       int iIndex  = H5Tget_member_index(m_hDSType, m_vFullInfo.m_vInfos[i].m_sName.c_str());
       hid_t hMembType =   H5Tget_member_type(m_hDSType, iIndex);
       
       //xha_printf("adding entry  [%s] with offset %lld\n", m_vFullInfo.m_vInfos[i].m_sName, m_vFullInfo.m_vInfos[i].m_iItemOffset); fflush(stdout);
       H5Tinsert(hAgentDataType, m_vFullInfo.m_vInfos[i].m_sName.c_str(), m_vFullInfo.m_vInfos[i].m_iItemOffset,  hMembType);
   }
    return hAgentDataType;
}

