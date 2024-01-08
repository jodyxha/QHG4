#ifndef __NAVGROUPREADER2_H__
#define __NAVGROUPREADER2_H__

#include <hdf5.h>

#include "Navigation2.h"

struct Nav2Attributes : Attributes {
    uint     m_iNumCells;
    uint     m_iNumWaterWays;
    double   m_dSampleDist;
    uint     m_iNumBridges;
};


class NavGroupReader2 : public GroupReader<Navigation2, Nav2Attributes> {

public:
    static NavGroupReader2 *createNavGroupReader2(const std::string sFileName);
    static NavGroupReader2 *createNavGroupReader2(hid_t hFile);

    int tryReadAttributes(Nav2Attributes *pAttributes);
    int readData(Navigation2 *pGroup);
    int readBridges(Navigation2 *pGroup);
    int readArray(Navigation2 *pGroup, const std::string sArrayName) { return 0;};
protected:
    NavGroupReader2();

};




#endif
