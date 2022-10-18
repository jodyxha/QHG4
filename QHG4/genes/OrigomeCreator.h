/*============================================================================
| OrigomeCreator
| 
|  Creates a number of initial origome sequences
|
|  GenomeCreator's template is usuallay GeneUtils or BitGeneUtils
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __ORIGOMECREATOR_H__
#define __ORIGOMECREATOR_H__

#include <string>
#include "types.h"
#include "LayerArrBuf.h"
#include "WELL512.h"

template<class U>
class OrigomeCreator {
public:
    OrigomeCreator(uint iNumParents);
    
    int determineInitData(std::string sLine);
    const std::string &getInitString() { return m_sInitString;};
    int createInitialOrigomes(int iGenomeSize, int iNumGenomes, LayerArrBuf<ulong> &aOrigome);

private:
    void buildInitString();

    uint   m_iNumParents;
    int    m_iInitialType;
    int    m_iOrigValue;
    std::string m_sInitString;
};


#endif
