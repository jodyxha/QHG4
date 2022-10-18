#ifndef __NAVGROUPREADER_H__
#define __NAVGROUPREADER_H__

#include <hdf5.h>

#include "Navigation.h"

struct NavAttributes : Attributes {
    uint     m_iNumPorts;
    uint     m_iNumDests;
    uint     m_iNumDists;
    double   m_dSampleDist;
    uint     m_iNumBridges;
};


class NavGroupReader : public GroupReader<Navigation, NavAttributes> {

public:
    static NavGroupReader *createNavGroupReader(const std::string sFileName);
    static NavGroupReader *createNavGroupReader(hid_t hFile);

    int tryReadAttributes(NavAttributes *pAttributes);
    int readArray(Navigation *pGroup, const std::string sArrayName);
    int readData(Navigation *pGroup);
    int readBridges(Navigation *pGroup);
    
protected:
    NavGroupReader();

};




#endif
