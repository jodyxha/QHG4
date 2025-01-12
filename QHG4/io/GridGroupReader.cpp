#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <string>

#include "xha_strutilsT.h"
#include "types.h"
#include "SCellGrid.h"
#include "SCellGrid.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "GridGroupReader.h"

//----------------------------------------------------------------------------
// constructor
//
GridGroupReader::GridGroupReader() {
}


//----------------------------------------------------------------------------
// createGridReader
//
GridGroupReader *GridGroupReader::createGridGroupReader(const std::string sFileName) {
    GridGroupReader *pGR = new GridGroupReader();
    int iResult = pGR->init(sFileName, GRIDGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}

//----------------------------------------------------------------------------
// createGridGroupReader
//
GridGroupReader *GridGroupReader::createGridGroupReader(hid_t hFile) {
    GridGroupReader *pGR = new GridGroupReader();
    int iResult = pGR->init(hFile, GRIDGROUP_NAME);
    if (iResult != 0) {
        delete pGR;
        pGR = NULL;
    }
    return pGR;
}


//-----------------------------------------------------------------------------
// attr_info
//  callback for H5Aiterate2
//
herr_t attr_info(hid_t loc_id, const char *name, const H5A_info_t*pInfo, void *opdata) {
    stringmap *psm = (stringmap *)(opdata);

    //    hid_t attr = H5Aopen_name(loc_id, name);
    hid_t attr = H5Aopen_by_name(loc_id, ".", name, H5P_DEFAULT, H5P_DEFAULT);
    hid_t atype = H5Aget_type(attr);
    hid_t type_class = H5Tget_class(H5Aget_type(attr)); 
    if (type_class == H5T_STRING) {
        char sDum[256];
        H5Aread(attr, atype, sDum);
        (*psm)[name] = sDum;
    }
    H5Aclose(attr);
    return 0;
}

//----------------------------------------------------------------------------
// tryReadAttributes
//
int GridGroupReader::tryReadAttributes(GridAttributes *pAttributes) {
    int iResult = GroupReader<SCellGrid, GridAttributes>::tryReadAttributes(pAttributes);


    if (iResult == 0) {
        herr_t status = H5Aiterate2(m_hGroup, H5_INDEX_NAME, H5_ITER_INC, NULL, attr_info, &(pAttributes->smData));
        iResult = (status == 0)?0:-1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// createCellDataType
//
hid_t createCellDataType(int iMaxNeighbors) {
    hid_t hCellDataType = H5Tcreate (H5T_COMPOUND, sizeof(SCell));
    H5Tinsert(hCellDataType, GRID_DS_CELL_ID.c_str(),    HOFFSET(SCell, m_iGlobalID),      H5T_NATIVE_INT);
    H5Tinsert(hCellDataType, GRID_DS_NUM_NEIGH.c_str(),  HOFFSET(SCell, m_iNumNeighbors),  H5T_NATIVE_UCHAR);
    hsize_t dims = iMaxNeighbors;
    hid_t hAttrArr = H5Tarray_create2(H5T_NATIVE_INT, 1, &dims);
    H5Tinsert(hCellDataType, GRID_DS_NEIGHBORS.c_str(),  HOFFSET(SCell, m_aNeighbors), hAttrArr);


    return hCellDataType;
}


//-----------------------------------------------------------------------------
// readArray
//
int GridGroupReader::readArray(SCellGrid *pCG, const std::string sArrayName) {
    return 0;
}


//-----------------------------------------------------------------------------
// readData
//
int GridGroupReader::readData(SCellGrid *pCG) {
    int iResult = -1;
    // before loading the array, make sure that the essential variables correspond
    if ((m_pAttributes == NULL) ||
        (m_pAttributes->m_iNumCells == pCG->m_iNumCells)) {
        iResult = 0;
    } else {
        iResult = -1;
        if (m_pAttributes != NULL) {
            xha_printf("Number of cells or max neighbors do not correspond:\n");
            xha_printf("  VegGroupReader::m_iNumCells: %d; SCellGrid::m_iNumCells: %d\n", m_pAttributes->m_iNumCells,  pCG->m_iNumCells);
        }
    }

    if (iResult == 0) {
        hid_t hCellType = createCellDataType(pCG->m_iMaxNeighbors);
        hid_t hDataSet   = H5Dopen(m_hGroup, CELL_DATASET_NAME.c_str(), H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        hsize_t dimsm = pCG->m_iNumCells;
        hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
    
        herr_t status = H5Dread(hDataSet, hCellType, hMemSpace, hDataSpace, H5P_DEFAULT, pCG->m_aCells);
    
        for (uint i = 0; i < pCG->m_iNumCells; i++) {
            pCG->m_mIDIndexes[pCG->m_aCells[i].m_iGlobalID]=i;
        }
        iResult = (status >= 0)?0:-1;
    }
    return iResult;
}
