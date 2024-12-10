

#include "SubSpaceIndexParser.h"

//----------------------------------------------------------------------------
// create_instance
//
SubSpaceIndexParser *SubSpaceIndexParser::create_instance(uintvec vSizes, bool bVerbose) {
    SubSpaceIndexParser *pSSIP = new SubSpaceIndexParser();
    pSSIP->setVerbosity(bVerbose);
    int iResult = pSSIP->init(vSizes);
    
    if (iResult != 0) {
        delete pSSIP;
        pSSIP = NULL;
    }
    return pSSIP;
}


//----------------------------------------------------------------------------
// parseReduction
//
int SubSpaceIndexParser::parseReduction(std::string sCurDesc, uint iCurDim,  uintvec &vCurIndexes) {
    int iResult = 0;
    if (m_bVerbose) {stdprintf("dim %d, found '%s' for %d: range(%d)\n", iCurDim, sCurDesc, iCurDim, m_vSizes[iCurDim]);}
                
    // for * and reductions we keep all indexes
    for (uint k = 0; k < m_vSizes[iCurDim]; ++k) {
        vCurIndexes.push_back(k);
    }

    if (m_bVerbose) {stdprintf("vX (%zd): %bv\n", vCurIndexes.size(), vCurIndexes);}
   
    if (sCurDesc[0] == '#') {
        m_mReductionDims[iCurDim] = RED_TYPE_SUM;
    } else if (sCurDesc[0] == '%') {
        m_mReductionDims[iCurDim] = RED_TYPE_AVG;
    }
                
    // if the operator is followed by a '.', this dimension should be squeezed 
    if (sCurDesc.size() == 2) {
        if (sCurDesc[1] == '.') {
            m_mSqueezeDims[iCurDim] = 1;
        } else {
            stdprintf("Only a '!' may follow a reduction operator, not [%c]\n", sCurDesc[1]);
            iResult = -1;
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
// parseNumeric
//
bool SubSpaceIndexParser::validIndex(std::string sVal, uint iCurDim, uint *piVal) {
    bool bRes = false;
    *piVal = 0;
    if (strToNum(sVal, piVal)) {
        if (*piVal < m_vSizes[iCurDim]) {
            bRes = true;
        }
    }
    return bRes;
}

//----------------------------------------------------------------------------
// parseNumeric
//
int SubSpaceIndexParser::parseNumeric(std::string sCurDesc, uint iCurDim,  uintvec &vCurIndexes) {
    int iResult = 0;
    
    std::set<uint> sRanges;
    stringvec vsSummands;
    uint iNumSummands = splitString(sCurDesc, vsSummands, "+");
                
        
    for (uint j= 0; (iResult == 0) && (j < iNumSummands); j++) {
        stringvec vsSubSums;
        uint iNumSubSums = splitString(vsSummands[j], vsSubSums, "-");
        if (iNumSubSums == 1) {
            uint k = 0;
            if (validIndex(vsSubSums[0], iCurDim, &k)) {
                stdfprintf(stderr, "inserting value [%u]\n", k);
                sRanges.insert(k);
            } else {
                stdfprintf(stderr, "slice indexes [%d] should be a number not exceeding the extents [%d]\n", k, m_vSizes[iCurDim]);
                iResult = -1;
            }
        
        } else if (iNumSubSums == 2) {
            uint k0 = 0;
            uint k1 = 0;
            if (validIndex(vsSubSums[0], iCurDim, &k0)) {
                if (validIndex(vsSubSums[1], iCurDim, &k1)) {
                    if (k0 <= k1) {
                        for (uint k = k0; k <= k1; k++) {
                            // stdfprintf(stderr, "inserting value [%u]\n", k);
                            sRanges.insert(k);
                        }
                    } else {
                        stdprintf("Lower range should be less or equal to upper range [%d:%u]\n", k0, k1);
                        iResult = -1;
                    }
                } else {
                    stdfprintf(stderr, "upper range [%d] should be a number not exceeding the extents [%d\n", k1, m_vSizes[iCurDim]);
                    iResult = -1;
                }
            } else {
                stdfprintf(stderr, "lower range [%d] should be a number not exceeding the extents [%d]\n", k0, m_vSizes[iCurDim]);
                iResult = -1;
            }
            
        } else {
            stdfprintf(stderr, "invalid index configuration [%s]\n", vsSummands[j]);
            iResult = -1;
        }
    } // for j

    // now add the set contents to the required iindexed for this dimension
    if (iResult == 0)  {
        vCurIndexes.insert(vCurIndexes.end(), sRanges.begin(), sRanges.end());
    }
    return iResult;
}

//----------------------------------------------------------------------------
// parseIndexDesc
//  parse slice descriptions
//  slice-desc ::= <single-slice> ( ":" <single-slice>){numdim - 1}
//  single-slice ::= <numeric> | <reduction>
//  numeric      ::= <index-desc> ("+" <index-desc>)*
//  index-desc   ::= <number> [ "-" <number>]
//  reduction    ::= "*" |"#" | "%" 
//
int SubSpaceIndexParser::parseSliceDesc(std::string sSliceDesc, bool bSqueezeAll) {

    int iResult = -1;
    //setlocale(LC_ALL, "");
    m_vNeededDims.clear();
    m_mReductionDims.clear();
    m_mSqueezeDims.clear();

    stringvec vsDescriptors;
    uintvec vCurIndexes;

    uint iDims2 = splitString(sSliceDesc, vsDescriptors, ":");
    
    if (iDims2 == m_iNumDims) {
        iResult = 0;
        
        // each part has the form "<int>[+<int>]" or "#" or "*" or "%"
        for (uint i = 0; (iResult == 0) && (i < m_iNumDims); i++) {
            // stdfprintf(stderr, "looking at part [%s]\n", vsDescriptors[i]);
            stringvec vsCurIndexes;
            uintvec   vCurIndexes;
            if ((vsDescriptors[i][0] == '*') || 
                (vsDescriptors[i][0] == '#') || 
                (vsDescriptors[i][0] == '%')) {
                
                iResult = parseReduction(vsDescriptors[i], i, vCurIndexes);
                
            } else {

                if (m_bVerbose) {stdprintf("vsDescriptors (%zd): %bv\n", vsDescriptors.size(), vsDescriptors);}
               
                iResult = parseNumeric(vsDescriptors[i], i, vCurIndexes);
               
            } 
            
            // save the array of required indexes for this dimension
            if (iResult == 0)  {
                m_vNeededDims.push_back(vCurIndexes);
            }

            // handle global squeeze
            if (bSqueezeAll) {
                for (uint i = 0; i < m_iNumDims; i++) {
                    m_mSqueezeDims[i] = 1;
                }
            }

        } // for i
       
        
    } else {
        stdfprintf(stderr, "there should be %u index combinations but only found %u in [%s]\n", m_iNumDims, iDims2, sSliceDesc); 
    }
    return iResult;
}



//----------------------------------------------------------------------------
// init
//
int SubSpaceIndexParser::init(uintvec vSizes) {
    int iResult = 0;
    if (vSizes.size() > 0) {
        m_vSizes = vSizes;
        m_iNumDims = m_vSizes.size();
    } else {
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// constructor
//
SubSpaceIndexParser::SubSpaceIndexParser()
    : m_iNumDims(0), m_bVerbose(false) {
}

