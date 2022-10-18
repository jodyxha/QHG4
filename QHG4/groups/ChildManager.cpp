
#include <vector>
#include <map>
#include <algorithm>

#include "types.h"
#include "clsutils.h"
#include "MessLoggerT.h"
#include "QDFUtils.h"
#include "GroupInterface.h"
#include "ChildManager.h"


//----------------------------------------------------------------------------
// constructor
//
template<typename T>
ChildManager<T>::ChildManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL)
    : m_pGPop(dynamic_cast<GroupInterface *>(pPop)),
      m_fAdultAge(0) {
}


//----------------------------------------------------------------------------
// destructor
//
template<typename T>
ChildManager<T>::~ChildManager() {
}


//----------------------------------------------------------------------------
// getChildren
//
template<typename T>
std::vector<int> ChildManager<T>::getChildren(int iParent) {
    std::vector<int> vResult;
    std::map<int, std::vector<int>>::const_iterator it = m_mvChildren.find(iParent);
    if (it != m_mvChildren.end()) {
        vResult = it->second;
    } 
    return vResult;
}
 

//----------------------------------------------------------------------------
// checkAdult
//
template<typename T>
int ChildManager<T>::checkAdult() {
    int iResult = 0;

    return iResult;
}


//----------------------------------------------------------------------------
// addChild
//
template<typename T>
int ChildManager<T>::addChild(int iParent, int iChild) {
    int iResult = 0;
    m_mvChildren[iParent].push_back(iChild); // if parent-key not yet there, the vector will be empty
    m_pGPop->setParent(iChild, iParent);
    return iResult;
}


//----------------------------------------------------------------------------
// removeChild
//
template<typename T>
int ChildManager<T>::removeChild(int iParent, int iChild) {
    int iResult = -1;

    std::map<int, std::vector<int>>::iterator itp = m_mvChildren.find(iParent);
    if (itp != m_mvChildren.end()) {
        std::vector<int>::const_iterator ita = std::find(itp->second.begin(),itp->second.end(), iChild);
        if (ita != itp->second.end()) {
            itp->second.erase(ita);
            if (itp->second.size() == 0) {
                m_mvChildren.erase(itp);
            }
            iResult = 0;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// extractAttributesQDF
//
template<typename T>
int ChildManager<T>::extractAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_CHILDMAN_ADULT_AGE_NAME,  1, &m_fAdultAge);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_CHILDMAN_ADULT_AGE_NAME);
        }
    }

    return iResult;
}



//----------------------------------------------------------------------------
// writeAttributesQDF
//
template<typename T>
int ChildManager<T>::writeAttributesQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_CHILDMAN_ADULT_AGE_NAME,  1,  &m_fAdultAge);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
template<typename T>
int ChildManager<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    int iFullSize = 1; // number of parents
    std::map<int, std::vector<int>>::iterator itp;
    for (itp = m_mvChildren.begin(); itp != m_mvChildren.end(); ++itp) {
        iFullSize += (2 +itp->second.size()); // numchildren, children
    }
    int *pChildrenData = new int[iFullSize];
    int *pCur = pChildrenData;
    *pCur++ = m_mvChildren.size();
    for (itp = m_mvChildren.begin(); itp != m_mvChildren.end(); ++itp) {
        *pCur++ = itp->first; // parent ID
        *pCur++ = itp->second.size(); // numchildren
        for (uint k = 0; k < itp->second.size(); ++k) {
            *pCur++ = itp->second[k];
        }
    }

    iResult = qdf_writeArray(hSpeciesGroup, DS_CHILDMAN_DATA, iFullSize, pChildrenData, H5T_NATIVE_INT);
    if (iResult != 0) {
        LOG_ERROR("[ChildManager<T>::writeAdditionalDataQDF] couldn't write ChildManager data");
    }
    delete[] pChildrenData;

    return iResult;
}



//----------------------------------------------------------------------------
// readAdditionalDataQDF
//
template<typename T>
int ChildManager<T>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    std::vector<hsize_t> vSizes;
    iResult = qdf_getDataExtents(hSpeciesGroup, DS_CHILDMAN_DATA, vSizes);
    if (iResult == 0) {
        size_t iFullSize = vSizes[0];
        int *pChildrenData = new int[iFullSize];
    

        iResult = qdf_readArray(hSpeciesGroup, DS_CHILDMAN_DATA, iFullSize, pChildrenData);
        if (iResult == 0) {
            int *pCur = pChildrenData;
            uint iNumParents = *pCur++;
            for (uint i = 0; i < iNumParents; i++) {
                uint iParentID = *pCur++;
                uint iNumChildren = *pCur++;
                std::vector<int> vCur;
                for (uint k = 0; k < iNumChildren; k++) {
                    m_mParents[*pCur] = iParentID;
                    vCur.push_back(*pCur++);
                }
                m_mvChildren[iParentID] = vCur;
            }
        } else  {
            LOG_ERROR("[ChildManager<T>::readAdditionalDataQDF] couldn't read ChildManager data");
        }
    } else {
        LOG_ERROR("[ChildManager<T>::readAdditionalDataQDF] couldn't determine data extents for [%s]", DS_CHILDMAN_DATA);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// tryGetParams
//
template<typename T>
int ChildManager<T>::tryGetParams(const ModuleComplex *pMC) {

    int iResult = -1;
    std::string sTemp; 

    const stringmap &mParams = pMC->getAttributes();
    if (this->checkAttributes(mParams) == 0) {
        iResult = 0;
        iResult += getAttributeVal(mParams, ATTR_CHILDMAN_ADULT_AGE_NAME,    &m_fAdultAge);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// isEqual
//
template<typename T>
bool ChildManager<T>::isEqual(Action<T> *pAction, bool bStrict) {
    bool bEqual = false;
    ChildManager<T>* pA = static_cast<ChildManager<T>*>(pAction);
    if (m_fAdultAge == pA->m_fAdultAge) {
        bEqual = true;
    } 
    return bEqual;
}
