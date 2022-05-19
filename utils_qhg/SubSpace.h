#ifndef __SUBSPACE_H__
#define __SUBSPACE_H__

#include "types.h"

typedef std::vector<intvec> intvecvec;
typedef std::vector<uintvec> uintvecvec;
typedef std::vector<std::pair<int,int>> intintvec;
typedef std::vector<std::pair<uint,uint>> uintuintvec;
typedef std::vector<intintvec> intintvecvec;
typedef std::vector<stringvec> stringvecvec;

const int DISP_INT      = 0;
const int DISP_FLOAT_01 = 1;
const int DISP_FLOAT    = 2;
 
template<typename T>
class SubSpace {
public:
    static SubSpace *create_instance(uintvec &vSizes, bool bVerbosity);
    virtual ~SubSpace();

    int  set_data(T *pData, uint iOffset, uint iNumElements);
    int  set_dim_names(stringvec vDimNames);
    int  set_coord_names(stringvecvec vvCoordNames);

    int pos_to_coord(uint iPos, uintvec &vCoord);
    int coord_to_pos(uintvec vCoord);

    void show_sizes();
    void show_names();
    void show_data();
    void show_data_nice(int iDispType);
    void show_data_csv();


    SubSpace *create_slice(uintvecvec vvIndexes);
    SubSpace *create_sum(uintvec vSumDims);

    const stringvecvec &get_coord_names(){ return m_vvCoordNames;};
protected:
    SubSpace(bool bVerbosity);
    int    init(uintvec &vSizes);
    void   normalize_slices(uintuintvec &vSliceData);
    int    single_slice(uintvec vIndexes, uintuintvec &vSliceData);
    bool   check_indexes(uintvecvec &vvIndexes);
    int    calculate_slice_description(uintvecvec vvIndexes);
    int    calculate_sum_description(int iDim, uintvec &vBases, uintvec &vBaseOffsets, uintvec &vSumOffsets);

    static int cartesian_product(uintvecvec vSets, uintvecvec &vvCombinations);
    static int cartesian_product_rec(uintvecvec vSets, uintvec vTemp, uintvecvec &vvCombinations);
    static int merge_slice_data(uintuintvec vSliceDataIn, uintuintvec &vSliceDataOut, bool bVerbose);
    
    uint     m_iDim;
    uintvec  m_vSizes;
    uintvec  m_vSizesX;
    T       *m_adData;
    uint     m_iNumVals;
    uintvec  m_vSubVolumes;
    bool     m_bVerbose;

    uintuintvec m_vSliceDescription;
    uintvec     m_vSliceDimensions;

    stringvec    m_vDimNames;
    stringvecvec m_vvCoordNames;
};


#endif
