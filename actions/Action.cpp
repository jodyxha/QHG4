#include "Action.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "clsutils.cpp"


//----------------------------------------------------------------------------
// constructor
//
template<typename T>
Action<T>::Action(SPopulation<T> *pPop, SCellGrid *pCG, const std::string sActionName, const std::string sID) 
    : m_pPop(pPop),
      m_pCG(pCG),
      m_sActionName(sActionName) {
    if (!sID.empty()) {
        m_sActionName += '[' + sID + ']';
    }
}


//----------------------------------------------------------------------------
// readPopKeyVal
//
template<typename T>
template<typename T2>
int Action<T>::readPopKeyVal(std::string sLine, const std::string sKey, T2* pParam) {

    T2 pDummy = 0;
    int iResult = 0; 
    
    iResult += readKeyVal(sLine, sKey, "=", &pDummy);
    if (iResult == 0) {
        *pParam = pDummy;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// readPopKeyArr
//
template<typename T>
template<typename T2>
int Action<T>::readPopKeyArr(std::string sLine, const std::string sKey, int iNum, T2* pParam) {

    int iResult = 0;
    
    iResult += readKeyArr(sLine, sKey, "=", iNum, pParam);

    return iResult;
}


//----------------------------------------------------------------------------
// readPopKeyString
//
template<typename T>
int Action<T>::readPopKeyString(std::string sLine, const std::string sKey, int iSize, std::string &sValue) {
    int iResult = 0;
    
    iResult += readKeyStr(sLine, sKey, "=", iSize, sValue);

    return iResult;
}


//----------------------------------------------------------------------------
// 
//
template<typename T>
void Action<T>::showAttributes() {
    if (this->m_vNames.size() > 0) {
        for (uint i = 0; i < this->m_vNames.size(); i++) {
            stdprintf("  %s\n", this->m_vNames[i]);
        }
    } else {
        printf("  (none)\n");
    }
}


//----------------------------------------------------------------------------
// hasParam
//
template<typename T>
bool Action<T>::hasParam(std::string sAtt) {
    bool bSearching = true;
    for (uint i = 0; bSearching && (i < this->m_vNames.size()); i++) {
        bSearching = (this->m_vNames[i].compare(sAtt) == 0);
    }
    return !bSearching;
}
    

//----------------------------------------------------------------------------
// checkParams
//
template<typename T>
int Action<T>::checkParams(const stringmap &mParams) {
    int iResult = 0;
    stringvec vRequired;
    stringvec vUnknown;

    if (checkParamsV(mParams, m_vNames, vRequired, vUnknown) != 0) {
        iResult = -1;
        if (vRequired.size() > 0) {
            stdprintf("[Action %s] Missing param%s:\n", m_sActionName, (vRequired.size()>0)?"s":"");
            for (uint i = 0; i < vRequired.size(); i++) {
                stdprintf("[Action %s]     %s\n", m_sActionName,vRequired[i]);
            }
        }
        if (vUnknown.size() > 0) {
            stdprintf("[Action %s] Unknown param%s:\n", m_sActionName, (vUnknown.size()>0)?"s":"");
            for (uint i = 0; i < vUnknown.size(); i++) {
                stdprintf("[Action %s]     %s\n", m_sActionName, vUnknown[i]);
            }
        }
    }
    return iResult;
}
