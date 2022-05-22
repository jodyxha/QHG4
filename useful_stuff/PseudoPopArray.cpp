
#include "PseudoPopArray.h"


PseudoPopArray::PseudoPopArray(const std::string sArrayName, const std::string sPathPat) 
    : m_sArrayName(sArrayName), 
      m_sPathPat(sPathPat),
      m_pdData(NULL),
      m_sUsedPath(""),
      m_iArrSize(-1) {
}

PseudoPopArray::~PseudoPopArray() {

};

