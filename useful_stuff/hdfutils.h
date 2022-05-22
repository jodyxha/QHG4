#ifndef __HDFUTILS_H__
#define __HDFUTILS_H__

#include <hdf5.h>
typedef struct field_data {
    std::string sName;
    uint iOffset;
    hid_t hType;
} field_data;

typedef std::vector<field_data> fieldvec;

bool link_exists(hid_t hGroup, const std::string sName);
bool attr_exists(hid_t hGroup, const std::string sName);
int extract_numeric_attribute(hid_t hLoc, const std::string sName, uint iNum, void *vValue, hid_t hType);
std::string extract_string_attribute(hid_t hLoc, const std::string sName);

hid_t createCompoundDataType(uint iSize, fieldvec &vFieldDatastd);

#endif
