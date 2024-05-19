#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <set>
#include <algorithm>

#include "stdstrutils.h"
#include "stdstrutilsT.h"

#include "LineReader.h"
#include "SubSpace.h"


static void display_slice_data(std::string sCaption, uintuintvec vSliceData) {
    stdprintf("%s (%zd)\n", sCaption, vSliceData.size());
    for (uint i = 0; i < vSliceData.size(); i++) {
        stdprintf("    %d: %d %d\n", i, vSliceData[i].first, vSliceData[i].second);
    }
}


//----------------------------------------------------------------------------
// cartesian_product
//   create all possible combinations of elements taking one from each
//   vector in vSets.
//   Actually caretesian product of all the sets
//   Start of a recursion.
//
template<typename T>
int SubSpace<T>::cartesian_product(uintvecvec vSets, uintvecvec &vvProducts) {
    int iResult = 0;
    uintvec vTemp;
    iResult = cartesian_product_rec(vSets, {}, vvProducts);
    return iResult;
}


//----------------------------------------------------------------------------
// cartesian_product_rec
//   create all possible combinations of elements taking one from each
//   vector in vSets. 
//   Recursion step
//
template<typename T>
int SubSpace<T>::cartesian_product_rec(uintvecvec vSets, uintvec vTemp, uintvecvec &vvProducts) {
    if (vSets.size() > 0) {
        uintvec vCur = vSets.back();
        vSets.pop_back();
        for (uint i = 0; i < vCur.size(); i++) {
        
            vTemp.push_back(vCur[i]);
            cartesian_product_rec(vSets, vTemp, vvProducts);
            vTemp.pop_back();
        }
        
    } else {
        uintvec vTR;
        vTR.resize(vTemp.size());
        std::reverse_copy(vTemp.begin(), vTemp.end(), vTR.begin());
        vvProducts.push_back(vTR);
    }

    return 0;
}


//----------------------------------------------------------------------------
// merge_slice_data
//   mergeSliceData tries clump together sliceData items with consecutive
//   positions, to reduce the number of copy operations
//   
template<typename T>
int SubSpace<T>::merge_slice_data(uintuintvec vSliceDataIn, uintuintvec &vSliceDataOut, bool bVerbose){
    int iResult = 0;
    
    vSliceDataOut.clear();
    uint i = 0;
    while (i < vSliceDataIn.size()) {

        uint s =vSliceDataIn[i].first;
        uint n =vSliceDataIn[i].second;
        uint j = i+1;
       
        while ((j < vSliceDataIn.size()) && ((s + n) == vSliceDataIn[j].first)) {
            n += vSliceDataIn[j].second;
            j++;
        }
        vSliceDataOut.push_back({s,n});
        if (j < vSliceDataIn.size()) {
            i = j;
        } else {
            i = j+1;
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// show_uintvecvec
//   display the contents of  uintvecvec
//
static void show_uintvecvec(const uintvecvec s){
    for (uint i = 0; i < s.size(); i++) {
        stdprintf("    %d: {", i);
        
        for (uint j = 0; j < s[i].size(); j++) {
            if (j > 0) {
                stdprintf(",");
            }
            stdprintf("%d", s[i][j]);
        }
        stdprintf("}\n");
    }
}


//----------------------------------------------------------------------------
// dims2Sizes
//  creates a uint vector of sizes from the sizes in the dimensions string
//
template<typename T>
int SubSpace<T>::dims2Sizes(std::string sDims, uintvec &vSizes, const std::string sSep) {
    int iResult = 0;
    stringvec vsSizes;
            
    uint iNumDims = splitString(sDims, vsSizes, sSep);
            
    for (uint i = 0; (iResult == 0) && (i < iNumDims); i++) {
        int k = 0;
        if (strToNum(vsSizes[i], &k)) {
            vSizes.push_back(k);
        } else {
            throw SubSpaceException(stdsprintf("invalid dimension size [%s]", vsSizes[i]));
        }
    }
    return iResult; 
}

//----------------------------------------------------------------------------
// create_instance_from_sizes
//   creates an instance SubSpace given an vector of dimension sizes
//
template<typename T>
SubSpace<T> *SubSpace<T>::create_instance_from_sizes(const uintvec &vSizes, bool bVerbosity) {
    SubSpace *pSS = new SubSpace(bVerbosity);
    int iResult = pSS->init(vSizes);
    if (iResult != 0) {
        delete pSS;
        pSS = NULL;
    }
    return pSS;
}


//----------------------------------------------------------------------------
// create_instance_from_dims
//   creates an instance SubSpace given a string with ":"-separated dimension sizes
//
template<typename T>
SubSpace<T> *SubSpace<T>::create_instance_from_dims(const std::string sDim, bool bVerbosity) {
    SubSpace *pSS = new SubSpace(bVerbosity);
    uintvec vSizes;
    int iResult = dims2Sizes(sDim, vSizes, ":");
    if (iResult == 0) {
        iResult = pSS->init(vSizes);
    }

    if (iResult != 0) {
        delete pSS;
        pSS = NULL;
    }
    return pSS;
}


//----------------------------------------------------------------------------
// constructor
//
template<typename T>
SubSpace<T>::SubSpace(bool bVerbosity)
    : m_iNumDims (0),
      m_adData(NULL),
      m_iNumVals(0),
      m_bVerbose(bVerbosity) {

} 


//----------------------------------------------------------------------------
// destructor
//
template<typename T>
SubSpace<T>::~SubSpace() {
    if (m_adData != NULL) {
        delete[] m_adData;
    }
}


//----------------------------------------------------------------------------
// init
//   creation od auxiliary vectors and allocation of data array
//
template<typename T>
int SubSpace<T>::init(const uintvec &vSizes) {
    int iResult = -1;

    // these lines work for scalars, too
    m_iNumVals = 1;
    m_iNumDims = vSizes.size();
    // both m_vSizesX and m_vSubVolumes have a 1 in first place to ease some calculations
    m_vSizesX.push_back(1);
    m_vSubVolumes.push_back(1);

    // this is only for higher dimensional objects
    if (vSizes.size() > 0) {
        
        m_vSizesX.insert(m_vSizesX.end(), vSizes.begin(), vSizes.end());
        m_vSizes.insert(m_vSizes.end(), vSizes.begin(), vSizes.end());
        
        for (uint i = 0; i < m_iNumDims; i++) {
            m_iNumVals *= m_vSizes[i];
        }
        for (uint i = 1; i < m_iNumDims + 1; i++) {
            m_vSubVolumes.push_back(m_vSizesX[i]*m_vSubVolumes.back());
            
        }

        for (uint iDim = 0; iDim < m_iNumDims; iDim++) {
            m_vDimNames.push_back(stdsprintf("%d", iDim));
            stringvec vCoordNames;
            for (uint c = 0; c < m_vSizes[iDim]; c++) {
                vCoordNames.push_back(stdsprintf("%d", c));
            }
            m_vvCoordNames.push_back(vCoordNames);
        }
        
    } 
    // allocate data (scalars have just a single element)
    m_adData = new T[m_iNumVals];
    memset(m_adData, 0, m_iNumVals*sizeof(T));
    iResult = 0;
    

    return iResult;
}

//----------------------------------------------------------------------------
// copyFull
//   create a deep copy of this
//
template<typename T>
SubSpace<T> *SubSpace<T>::copyFull() {
    SubSpace *pSS = create_instance_from_sizes(m_vSizes, false);
    pSS->set_data(m_adData, 0, m_iNumVals);
    return pSS;
}
 

//----------------------------------------------------------------------------
// pos_to_coord
//
template<typename T>
int SubSpace<T>::pos_to_coord(uint iPos, uintvec &vCoord) {
    int iResult = 0;
    uint v = m_iNumVals/m_vSizesX.back();

    if (iPos < m_iNumVals) {
        vCoord.resize(m_iNumDims, 0);
        for (int i = m_iNumDims - 1; i >= 0; i--) {
            div_t qr = div(iPos, v);
            vCoord[i] = qr.quot;
            iPos   = qr.rem;
            v /= m_vSizesX[i];
        }
        iResult = 0;
    } else {
        throw SubSpaceException(stdsprintf("iPos (%d) should be less than %d", iPos, m_iNumVals));
    }
    return iResult;
}


//----------------------------------------------------------------------------
// coord_to_pos
//
template<typename T>
int SubSpace<T>::coord_to_pos(uintvec vCoord) {
    int iPos = -1;

    if (vCoord.size() == m_iNumDims) {
        iPos = 0;
        for (int i = m_iNumDims - 1; (iPos >= 0) && (i >= 0); i--) {
            if (vCoord[i] <  m_vSizesX[i+1]) {
                iPos = m_vSizesX[i]*(vCoord[i] + iPos);
            } else {
                throw SubSpaceException(stdsprintf("coord[%d]:%d should be less than %d", i, vCoord[i], m_vSizesX[i+1]));
            }
        }
    } else {
        throw SubSpaceException(stdsprintf("coords should have size %u, not %u", m_iNumDims, vCoord.size()));
    }

    return iPos;
}



//----------------------------------------------------------------------------
// set_data
//  copy iNumElements elements from pNewData into m_adData at offset iOffset
//
template<typename T>
int SubSpace<T>::set_data(T *pNewData, uint iOffset, uint iNumElements) {
    int iResult = 0;

    if (iOffset + iNumElements <= m_iNumVals) {
        memcpy(m_adData+iOffset, pNewData, iNumElements*sizeof(T));
    } else {
        throw SubSpaceException(stdsprintf("offset (%d) + numelements (%d) > numvals (%d)", iOffset, iNumElements, m_iNumVals));
    }
    return iResult;
}


//----------------------------------------------------------------------------
// set_dim_names
//  copy the passed vector to m_vDimNames if the sizes match
//
template<typename T>
int SubSpace<T>::set_dim_names(stringvec vDimNames) {
    int iResult = 0;

    if (vDimNames.size() == m_iNumDims) {
        m_vDimNames.clear();
        m_vDimNames.insert(m_vDimNames.end(), vDimNames.begin(), vDimNames.end());
        iResult = 0;
    } else {
        throw SubSpaceException(stdsprintf("The number of dimension names (%zd) should be equal to the number of dimensions (%zd)", vDimNames.size(), m_iNumDims));
    }

    return iResult;
}


//----------------------------------------------------------------------------
// set_coord_names
//  copy the passed vectors to m_vvCoordNames if all sizes match
//
template<typename T>
int SubSpace<T>::set_coord_names(stringvecvec vvCoordNames) {
    int iResult = 0;

    if (vvCoordNames.size() == m_iNumDims) {
        for (uint i = 0; (iResult == 0) && (i < m_iNumDims); i++) {
            if ((vvCoordNames[i].size() > 0) && (vvCoordNames[i].size() != m_vSizes[i])) {
                throw SubSpaceException(stdsprintf("The sizes of coordinate name vector[%d] (%zd) should be equal to the size of dimension %d (%zd)", i, vvCoordNames[i].size(), i, m_vSizes[i]));
            }
        }
        if (iResult == 0) {
            // now we overwrite the vectrings in vvCoordNames
            // for which we have replacementd
            for (uint i = 0; (iResult == 0) && (i < m_iNumDims); i++) {
                if (vvCoordNames[i].size() > 0) {
                    m_vvCoordNames[i] = vvCoordNames[i];
                } else {
                    // empty vector means: use default values
                }
            }
        }

    } else {
        throw SubSpaceException(stdsprintf("The number of coordinate name vectors (%zd) should be equal to the number of dimensions (%zd)", vvCoordNames.size(), m_iNumDims));
    }

    return iResult;
}


//----------------------------------------------------------------------------
// normalize_slices
//  sort and reove double entries 
//
template<typename T>
void SubSpace<T>::normalize_slices(uintuintvec &vSliceData) {
    // sort (normally not necessary)
    std::sort(vSliceData.begin(), vSliceData.end());

    // remove equal entries (normally not necessary)
    std::set<std::pair<int,int>> sSet(vSliceData.begin(), vSliceData.end());

    vSliceData.clear();
    vSliceData.insert(vSliceData.begin(), sSet.begin(), sSet.end());

}


//----------------------------------------------------------------------------
// calculate_slice_description
//  vvIndexes holds the indexes and coord values to be used for each dimension
//  vvIndex[i] index-value pairs for dimension i
//  vvIndex[i][j] j-th index-value pair for dimension i
//
// create a slice description: a vector of start position and lengths to extract 
// data for a slice. dpending on the slice's orienation this can ve a short 
// vector containing descriptions with big lengths, or a large vector with many
// description wit short lengths.
//
template<typename T>
int SubSpace<T>::calculate_slice_description(uintvecvec vvIndexes) {
    int iResult = 0;
    
    if (m_bVerbose) {
        stdfprintf(stderr, "got %zd indexes\n", vvIndexes.size());
        show_uintvecvec(vvIndexes);
    }
    

    m_vSliceDimensions.clear();
    int    iNumSliceVals = 1;
    for (uint i = 0; i < vvIndexes.size(); i++) {
        m_vSliceDimensions.push_back(vvIndexes[i].size());
        iNumSliceVals *= vvIndexes[i].size();
    }
    if (m_bVerbose) {
        stdfprintf(stderr, "slice sizes calc (%d): ", iNumSliceVals);
        for (uint i = 0; i < m_vSliceDimensions.size(); i++) {
            stdfprintf(stderr, " %d", m_vSliceDimensions[i]);
        }
        stdfprintf(stderr, "\n");
    }
    
    if (check_indexes(vvIndexes)) {

        uintvecvec vvProduct; 
        cartesian_product(vvIndexes, vvProduct);
            
        if (m_bVerbose) {
            stdfprintf(stderr, "got %zd combinations\n", vvProduct.size());
            show_uintvecvec(vvProduct);
        }
            
        uintuintvec vSliceData;
        // o the slice for each index combination
        for (uint i = 0; i < vvProduct.size(); i++) {
            single_slice(vvProduct[i], vSliceData);
        }
            
        // all slices are done; here clump combinations
        if (m_bVerbose) {
            display_slice_data("slicedata", vSliceData);
        }
        /*            
        // make sure the entries are sorted and contain no duplicates
        normalize_slices(vSliceData);
        if (m_bVerbose) {
            display_slice_data("slicedata after cleanSlices", vSliceData);
        }
        */  
        // clump together entries with consecutive positions
        m_vSliceDescription.clear();
        merge_slice_data(vSliceData, m_vSliceDescription, m_bVerbose);
            
        if (m_bVerbose) {
            display_slice_data("slice data merged", m_vSliceDescription);
        }
    } else {
        m_vSliceDescription.clear();
        throw SubSpaceException("Bad index values");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// single_slice
//  vIndexes holds the indexes (one for each coord value)
//
template<typename T>
int SubSpace<T>::single_slice(uintvec vIndexes, uintuintvec &vStartNums) {
    int iResult = 0;
    if (vIndexes.size() == m_iNumDims) {
        
        int s = 0;
        int v = m_vSubVolumes.back();
        
        for (uint i = 0; i < m_iNumDims; i++) {
            v /= m_vSizes[i];
            s += vIndexes[i]*m_vSubVolumes[i]; 
        }
        vStartNums.push_back(std::pair<int,int>{s,v});

    } else {
        throw SubSpaceException(stdsprintf("there must be %d slice vectors provided", m_iNumDims));
    }
    return iResult;
}


//----------------------------------------------------------------------------
// check_indexes
//   checks that the indexes do not exceed the dimension sizes
//
template<typename T>
bool SubSpace<T>::check_indexes(uintvecvec &vvIndexes) {
    bool bOK = true;

    for (uint i = 0; i < vvIndexes.size(); i++) {
        uintvec &vIndexes = vvIndexes[i];
        for (uint j = 0; j < vIndexes.size(); j++) {
            if ((vIndexes[j] < 0) || (vIndexes[j] >= m_vSizes[i])) {
                stdfprintf(stderr, "bad index in dimension %d (max %d): %d\n", i, m_vSizes[i]-1, vIndexes[j]);
                bOK = false;
            }
        }
    }
    return bOK;
}


//----------------------------------------------------------------------------
// create_slice
//  create a new SubSpace containing the data addressed by the current slice
//
template<typename T>
SubSpace<T> *SubSpace<T>::create_slice(uintvecvec vAllIndexes) {
    int iResult = 0;
    SubSpace<T> *pSS = NULL;

    iResult = calculate_slice_description(vAllIndexes);
    if ((m_vSliceDescription.size() > 0) && (m_vSliceDimensions.size() > 0)) {

        pSS = SubSpace<T>::create_instance_from_sizes(m_vSliceDimensions, m_bVerbose);
        uint iNumVals = 1;
        for (uint i = 0; i < m_vSliceDimensions.size(); i++) {
            iNumVals *= m_vSliceDimensions[i];
        }
        if (pSS != NULL) {
            uint iDestOff = 0;
            for (uint i = 0; (iResult == 0) && (i < m_vSliceDescription.size()); i++) {
                uint iStart = m_vSliceDescription[i].first;
                uint iCount = m_vSliceDescription[i].second;

                if (iDestOff+iCount <= iNumVals) { 
                    pSS->set_data(m_adData+iStart, iDestOff, iCount);
                    iDestOff += iCount;
                } else {
                    stdfprintf(stderr, "description item %u: cant copy %u items\n", i, iCount);
                    iResult = 0;
                }
            }
            if (iResult == 0) {
                // maybe new coords?
  
                const stringvecvec vvTemp = get_coord_names();
                stringvecvec vvNewCoordNames;
                for (uint i = 0; i < vAllIndexes.size(); i++) {
                    stringvec vTemp;
                    for (uint k = 0; k < vAllIndexes[i].size(); k++) {
                        vTemp.push_back(vvTemp[i][ vAllIndexes[i][k]]);
                    }
                    vvNewCoordNames.push_back(vTemp);
                }       
                pSS->set_coord_names(vvNewCoordNames);
            } else {
                delete pSS;
                pSS = NULL;
            }
        } else {
            throw SubSpaceException("slice creation failed");
        }

    } else {
        throw SubSpaceException("no SliceDescription or SliceDimensions found. run calculate_slice_description() before create_slice()");
    }
    return pSS;
}



//----------------------------------------------------------------------------
// calculate_sum_description
//    this function finds the locatons of the summands for the summation
//    along dimension iDim.
//
template<typename T>
int SubSpace<T>::calculate_sum_description(int iDim, uintvec &vBases, uintvec &vBaseOffsets, uintvec &vSumOffsets) {
    int iResult = 0;
    uint b1 = 1;
    // "covolume"
    for (uint i = iDim+1; i < m_vSizes.size(); i++) {
        b1 *= m_vSizes[i];
    }

    
    for (uint i = 0; i < m_vSubVolumes[iDim]; i++) {
        vBaseOffsets.push_back(i);
    }

    for (uint i = 0; i < b1; i++) {
        vBases.push_back(i*m_vSubVolumes[iDim+1]);
    }
    // we use each vBase[i]+vBaseOffsets[j] as summation start


    for (uint i = 0; i < m_vSizes[iDim]; i++) {
        vSumOffsets.push_back(i*m_vSubVolumes[iDim]);
    }
    // from the summation start we will sum up all items at offsets vSumOffset[i] after 

 
    return iResult;
}



//----------------------------------------------------------------------------
// create_red
//
template<typename T>
SubSpace<T> *SubSpace<T>::create_reductions(uintuintmap &mRedDims) {
    SubSpace<T> *pSS = NULL;

    stringvec vRedNames{"sum", "avg"};
    uintvec vRedDims;
    for (auto const& imap: mRedDims) {
        vRedDims.push_back(imap.first);
    }

    std::sort(vRedDims.begin(), vRedDims.end());

    //    show_intvec("create_sum for", vSumDims);
    
    if (vRedDims.size() > 0) {
        uint iCurDim = vRedDims.back();
        uint iRedType = mRedDims[iCurDim];

        vRedDims.pop_back();;
        uintuintmap::iterator it = mRedDims.find(iCurDim);
        if (it != mRedDims.end()) {
            mRedDims.erase(it);
        } else {
            printf("ERERERERERERER\n");
        }
    
        uintvec vBases;
        uintvec vBaseOffsets;
        uintvec vAvgOffsets;
        
        calculate_sum_description(iCurDim, vBases, vBaseOffsets, vAvgOffsets);

        uintvec vSubSizes{m_vSizes};
        vSubSizes[iCurDim] = 1;

        SubSpace<T> *pNewSS = SubSpace<T>::create_instance_from_sizes(vSubSizes, m_bVerbose);

        for (uint iBase = 0; iBase < vBases.size(); iBase++) {
            for (uint iBaseOffset = 0; iBaseOffset < vBaseOffsets.size(); iBaseOffset++) {
                uint iPos = vBases[iBase] + vBaseOffsets[iBaseOffset];
                T dAvg = 0;
                for (uint iAvgOff = 0; iAvgOff < vAvgOffsets.size(); iAvgOff++) {
                    dAvg += m_adData[iPos+vAvgOffsets[iAvgOff]];
                }
                if (iRedType == 1) {
                    dAvg /=  vAvgOffsets.size();
                }
                uintvec vCoords;
                /*int iResult = */pos_to_coord(iPos, vCoords);
                int iPos1 = pNewSS->coord_to_pos(vCoords);

                pNewSS->set_data(&dAvg, iPos1, 1);
          
                
                const stringvecvec vvTemp = get_coord_names();
                stringvecvec vvNewCoordNames=vvTemp;
                vvNewCoordNames[iCurDim] = {vRedNames[iRedType]};
                pNewSS->set_coord_names(vvNewCoordNames);


            }
        }
        // pNewSS->showDataNice(DISP_INT);
        pSS = pNewSS->create_reductions(mRedDims);
        if (pSS == NULL) {
            pSS = pNewSS;
        } else {
            delete pNewSS;
        }
        
    }

    return pSS;
}

//----------------------------------------------------------------------------
// squeeze
//   remove all dimensions of size 1
template<typename T>
SubSpace<T> *SubSpace<T>::squeeze() {

    uintvec vNewSizes;
    for (uint i = 0; i < m_vSizes.size(); i++) {
        if (m_vSizes[i] > 1) {
            vNewSizes.push_back(m_vSizes[i]);
        }
    }

    SubSpace *pSS = create_instance_from_sizes(vNewSizes, false);
    pSS->set_data(m_adData, 0, m_iNumVals);
    return pSS;
};



//----------------------------------------------------------------------------
// show_sizes
//
template<typename T>
void SubSpace<T>::show_sizes() {
    stdprintf("sizes:      ");
    for (uint i = 0; i < m_vSizes.size(); i++) {
        stdprintf(" %d", m_vSizes[i]);
    }
    stdprintf("\n");
    stdprintf("subvolumes: ");
    for (uint i = 1; i < m_vSubVolumes.size(); i++) {
        stdprintf(" %d", m_vSubVolumes[i]);
    }
    stdprintf("\n");

}


//----------------------------------------------------------------------------
// show_data
//
template<typename T>
void SubSpace<T>::show_data() {
    char sMask[16];
    sprintf(sMask, "%%0%zdd\n", m_iNumDims); // we don'count the 1 at the first position

    for (uint i = 0; i < m_iNumVals; i++) {
        printf(sMask, (int)(m_adData[i]));
    }
}


//----------------------------------------------------------------------------
// show_data_nice
//
template<typename T>
void SubSpace<T>::show_data_nice(int iType, FILE *fOut, stringvec vSeparators, bool bFrame) {
    char sMask[16];
    uint iRowLen = 0;

    switch (iType) {
    case DISP_INT:
        sprintf(sMask, "%%%dd ", 6);
        iRowLen = m_vSizes[0]*7;
        break;
    case DISP_FLOAT:
        {
        // find largest value
        T tMax = -1000000;
        T tMin =  1000000;
        for (uint i = 0; i < m_iNumVals; i++) {
            if (m_adData[i] > tMax) {
                tMax = m_adData[i];
            }
            if (m_adData[i] < tMin) {
                tMin = m_adData[i];
            }
        }
        T tMax1 = (tMax > fabs(tMin))?tMax:fabs(tMin);
        // get number of digits: (max>0)->log(max)+1, (max==0)->1
        uint iNumDigits = (fabs(tMax1) > 0)?(uint)(log(fabs(tMax1))/log(10)+1):1;
        if (tMin < 0) {
            iNumDigits++;
        }
        sprintf(sMask, "%%%d.4f ", iNumDigits+5); //+5: decimal point and 4 digits after it
        iRowLen = m_vSizes[0]*(iNumDigits+5+1)-1;
        
        }
        break;
    case DISP_FLOAT_01:
        strcpy(sMask, "%08.6f "); 
        iRowLen = m_vSizes[0]*9;
        break;
    default:
        strcpy(sMask, "%09.4f "); 
        iRowLen = m_vSizes[0]*10;
    }
    
    // create a frame row 2 chars longer than the data lines
    std::string sFrame("");
    if (bFrame) {
        uint iRowLenF = iRowLen + 2;
        for (uint j = 0;j < iRowLenF/2; j++) {
            sFrame += "-+";
        }
        if ((iRowLenF%2) == 1) {
            sFrame += "-";
        }
    }

    // create separators consisting of iRowLen identical chars 
    for (uint i = 0; i < vStandardSepChars.size(); i++) {
        if (vStandardSepChars[i] != '\0') {
            vStandardSeps[i] = std::string(iRowLen, vStandardSepChars[i])+"\n";
        }
    }
 
    if (bFrame) {stdprintf("%s\n ", sFrame);}

    // print sll numbers sequentially, but insert separators if necessary
    uint i = 0;
    while (i < m_iNumVals) {
        if (i > 0) {
            std::string sSep = "";

            for (uint j = 1; j < m_vSubVolumes.size(); j++) {
                //printf("((  %d, s%d ))", i, m_vSubVolumes[j]);
                if ((i%m_vSubVolumes[j]) == 0) {
                    uint j0 = j;
                    if (j >= vStandardSeps.size()) {
                        j0 = vStandardSeps.size()-1;
                    }

                    if (j0 == 1) {
                        fprintf(fOut, "\n ");
                    } else {
                        sSep = vStandardSeps[j0]+" ";
                    }
                }
            }
            if (!sSep.empty()) {
                fprintf(fOut, "%s", sSep.c_str());
            }

        }
        fprintf(fOut, sMask, m_adData[i]);
        
        i++;
    }
    if (bFrame) {stdprintf("\n%s\n", sFrame);}



}

//----------------------------------------------------------------------------
// show_data_csv
//
template<typename T>
void SubSpace<T>::show_data_csv() {

    uint i = 0;
    uintvec vNameIndexes(m_iNumDims, 0);
    bool bLineStart = true;

    while (i < m_iNumVals) {
        if (i > 0) {
            // break the line after the fastest dimension has beeen written
            if ((i%m_vSubVolumes[1]) == 0) {
                
                printf("\n");
                bLineStart = true;
                bool bShift = true;
                for (uint k = 1; k < m_iNumDims; k++) {
                    //  stdprintf("k:%d sh:%s vni[%d]:%d vcns[%d]:%d \n", k, bShift?"T":"F", k, vNameIndexes[k], k, m_vvCoordNames[k].size());
                    if (vNameIndexes[k] < m_vvCoordNames[k].size()-1) {
                        if (bShift) {
                            vNameIndexes[k]++;
                            bShift = false;
                            // stdprintf("increasing\n");
                        } else {
                            // stdprintf("nop\n");
                        }
                    } else {
                        //                        stdprintf("K:%d - idx %d A= #coordnames %d\n", k, vNameIndexes[k] , m_vvCoordNames[k].size()-1);
                        if (bShift) {
                            vNameIndexes[k] = 0;
                            // stdprintf("Resetting\n");
                        }
                        bShift = true;
                    }
                }

            }
        }
        if (bLineStart) {
            std::string sHead = "";
            for (uint k = m_iNumDims-1; k > 0; k--) {
                sHead += m_vvCoordNames[k][vNameIndexes[k]]+";";
            }
            bLineStart = false;
            stdprintf("%s", sHead);
            
        }
        stdprintf(std::to_string(m_adData[i])+(bLineStart?"":";"));
        i++;
    }
    stdprintf("\n");
}


//----------------------------------------------------------------------------
// show_names_nice
//
template<typename T>
void SubSpace<T>::show_names() {

    stdprintf("Coord names:\n");
    for (uint i = 0; i < m_vDimNames.size(); i++) {
        stdprintf("  dimension %s\n", m_vDimNames[i]);
        for (uint j = 0; j < m_vvCoordNames[i].size(); j++) {
            stdprintf("    %s\n", m_vvCoordNames[i][j]);
        }
    }
}
