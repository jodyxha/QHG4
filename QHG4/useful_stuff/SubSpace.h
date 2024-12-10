#ifndef __SUBSPACE_H__
#define __SUBSPACE_H__

#include <exception> 

#include "types.h"

typedef std::vector<char> charvec;
typedef std::vector<intvec> intvecvec;
typedef std::vector<uintvec> uintvecvec;
typedef std::vector<std::pair<int,int>> intintvec;
typedef std::vector<std::pair<uint,uint>> uintuintvec;
typedef std::map<uint,uint> uintuintmap;
typedef std::vector<intintvec> intintvecvec;
typedef std::vector<stringvec> stringvecvec;

class SubSpaceException : public std::exception { 
private: 
    std::string message; 
  
public: 
    // Constructor accepts a const char* that is used to set 
    // the exception message 
    SubSpaceException(const std::string msg) 
        : message(msg) 
    { 
    } 
  
    // Override the what() method to return our message 
    const char* what() const throw() 
    { 
        return message.c_str(); 
    } 
}; 
  
const int DISP_INT      = 0;
const int DISP_FLOAT_01 = 1;
const int DISP_FLOAT    = 2;
 
static const uint RED_TYPE_ALL = 0;
static const uint RED_TYPE_SUM = 1;
static const uint RED_TYPE_AVG = 2;


static stringvec vStandardSeps = {
    "\n",
    "\n",
    "-------------\n",
    "+++++++++++++++++\n",
    "*********************\n",
    "=========================\n",
    "#############################\n",
    ":::::::::::::::::::::::::::::::::\n",
    };

const static charvec vStandardSepChars = {
    '\0',
    '\0',
    '-',
    '+',
    '*', 
    '=', 
    '#',
    ':',
    };

const static stringvec vFileSeps = {
    "\n",
    "\n",
    "\n",
    "\n\n",
    "\n\n\n",
    "\n\n\n\n",
    "\n\n\n\n\n",
    };

template<typename T>
class SubSpace {
public:
    static SubSpace *create_instance_from_sizes(const uintvec &vSizes, bool bVerbosity);
    static SubSpace *create_instance_from_exts(const std::string sDim, bool bVerbosity);

    virtual ~SubSpace();

    int  set_data(T *pData, uint iOffset, uint iNumElements);
    int  set_dim_names(stringvec vDimNames);
    int  set_coord_names(stringvecvec vvCoordNames);

    int pos_to_coord(uint iPos, uintvec &vCoord);
    int coord_to_pos(uintvec vCoord);

    SubSpace *copyFull();

    void show_sizes();
    void show_names();
    void show_data();
    void show_data_nice(int iDispType, FILE *fOut, stringvec vSeparators, bool bFrame);
    void show_data_csv();

    uint getNumDims() {return m_iNumDims;};
    uint getNumVals() { return m_iNumVals;};
    const uintvec &get_sizes() { return m_vSizes;}; 
    uintvec &get_sizes_copy() { return m_vSizes;}; 
    const T* getBuffer() {return m_adData;};

    SubSpace *create_slice(uintvecvec vvIndexes);
    SubSpace *create_reductions(uintuintmap mRedDims);
    SubSpace *squeeze(const uintuintmap &mSqueezeDims);
    SubSpace *squeeze();

    const stringvecvec &get_coord_names(){ return m_vvCoordNames;};

    static int dims2Sizes(std::string sDims, uintvec &vSizes, const std::string sSep);

protected:
    SubSpace(bool bVerbosity);
    int    init(const uintvec &vSizes);
    void   normalize_slices(uintuintvec &vSliceData);
    int    single_slice(uintvec vIndexes, uintuintvec &vSliceData);
    bool   check_indexes(uintvecvec &vvIndexes);
    int    calculate_slice_description(uintvecvec vvIndexes);
    int    calculate_sum_description(int iDim, uintvec &vBases, uintvec &vBaseOffsets, uintvec &vSumOffsets);

    static int cartesian_product(uintvecvec vSets, uintvecvec &vvCombinations);
    static int cartesian_product_rec(uintvecvec vSets, uintvec vTemp, uintvecvec &vvCombinations);
    static int merge_slice_data(uintuintvec vSliceDataIn, uintuintvec &vSliceDataOut, bool bVerbose);

    uint     m_iNumDims;
    uintvec  m_vSizes;
    uintvec  m_vSizesX;
    T       *m_adData;
    uint     m_iNumVals;
    uintvec  m_vSubVolumes;
    //uintvec  m_vSubVolumesR;
    bool     m_bVerbose;

    uintuintvec m_vSliceDescription;
    uintvec     m_vSliceDimensions;

    stringvec    m_vDimNames;
    stringvecvec m_vvCoordNames;
};


#endif
