#include <cstdio>
#include <hdf5.h>

#include "QDFUtils.h"

#include "OccHistory.h"
#include "GroupReader.h"
#include "GroupReader.cpp"
#include "OccGroupReader.h"
#include "OccAnalyzer.h"




//----------------------------------------------------------------------------
// readAttributes
//
int readAttributes(hid_t hOccGroup) {
    herr_t      status = 0;
    printf("reading attribute %s\n", OCC_ATTR_POP_NAMES);fflush(stdout);
    // read attribute
    hsize_t dims[1];
    hid_t hAttr = H5Aopen_name(hOccGroup, OCC_ATTR_POP_NAMES);
    if (hAttr >= 0) {
        hid_t hDataType  = H5Aget_type(hAttr);
        hid_t hSpace = H5Aget_space(hAttr);
        int rank = H5Sget_simple_extent_ndims(hSpace);
        status = H5Sget_simple_extent_dims(hSpace, dims, NULL);
        if (status >= 0) {
            size_t size = H5Tget_size (hDataType);
            char *pBuf = new char[size*dims[0]];
            printf("rank %d, dom[0] %llu, size %lu\n", rank, dims[0], size);
            status = H5Aread(hAttr, hDataType, pBuf);
            if (status >= 0) {
                // split strings and put innto vector
                stringvec vPopNames;
                char *p = pBuf;
                for (uint i = 0; i <dims[0]; i++) {
                    vPopNames.push_back(p);
                    printf("  [%s]\n", p);
                    p+= size;
                }
            } else {
                printf("Couldn't read att\n");
            }
        } else {
            printf("Couldn't get dims\n");
        }
    } else {
        printf("attr nono\n");
    }
    return 0;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    const char *sFile = "zomboni.qdf";

    OccGroupReader *pOR = OccGroupReader::createOccGroupReader(sFile);
    if (pOR != NULL) {
        OccAttributes oAttrs;
        iResult = pOR->tryReadAttributes(&oAttrs);

        const stringvec &vPopNames = oAttrs.m_vPopNames;

        std::vector<int> vEmpty;
        OccHistory *pOH = OccHistory::createInstance(vEmpty, vPopNames.size());

        iResult = pOR->readData(pOH);
        
        OccAnalyzer *pOA = new OccAnalyzer(pOH, vPopNames);
        const intvec &vCellIDs = pOA->getCellIDs() ;
        pOA->showBitOrder();
        for (uint i = 0; i < vCellIDs.size(); i++) {
            printf("CellID %d\n", vCellIDs[i]);
            pOA->showHistory(vCellIDs[i]);
        }
        delete pOH;
        delete pOA;
        delete pOR;
    }
    return iResult;
}
