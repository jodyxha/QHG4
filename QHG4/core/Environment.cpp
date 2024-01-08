
#include "SCellGrid.h"
#include "Environment.h"


Environment::Environment(SCellGrid *pCG) 
    : m_bUpdated(false),
      m_iNumCells(pCG->m_iNumCells),
      m_pCG(pCG)  { 
};
