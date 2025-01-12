#ifndef __SUBSPACEINDEXPARSER_H__
#define __SUBSPACEINDEXPARSER_H__

#include "types.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "SubSpace.h"  // for types and constants




class SubSpaceIndexParser {
public:
    static SubSpaceIndexParser *create_instance(uintvec vSizes, bool bVerbose=false);
    int parseSliceDesc(std::string sSliceDesc, bool bSqueezeAll);

    int init(uintvec vSizes);

    const uintuintmap &get_reduction_dims() { return m_mReductionDims;};
    const uintuintmap &get_squeeze_dims() { return m_mSqueezeDims;};
    const uintvecvec &get_needed_dims()  { return m_vNeededDims;};

    void setVerbosity(bool bVerbose) { m_bVerbose = bVerbose;};
    bool getVerbosity() { return m_bVerbose;};
protected:
    SubSpaceIndexParser();
    int parseReduction(std::string sCurDesc,  uint iCurDim, uintvec &vCurIndexes);
    int parseNumeric(std::string sCurDesc, uint iCurDim,  uintvec &vCurIndexes);
    bool validIndex(std::string sVal, uint iCurDim,  uint *piVal);

    uint m_iNumDims;
    bool m_bVerbose;
 
   uintvec m_vSizes;
    
    uintuintmap m_mReductionDims;
    uintuintmap m_mSqueezeDims;

    uintvecvec  m_vNeededDims;
    
};


#endif
