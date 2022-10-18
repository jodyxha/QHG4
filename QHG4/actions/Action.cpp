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
// showAttributes 
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
bool Action<T>::hasAttribute(std::string sAtt) {
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
int Action<T>::checkAttributes(const stringmap &mParams) {
    int iResult = 0;
    stringvec vRequired;
    stringvec vUnknown;

    if (checkAttributesV(mParams, m_vNames, vRequired, vUnknown) != 0) {
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
