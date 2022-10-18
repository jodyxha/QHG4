#ifndef __OCCGROUPREADER_H__
#define __OCCGROUPREADER_H__

#include <string>
#include <hdf5.h>

class OccHistory;

struct OccAttributes : Attributes {
    stringvec m_vPopNames;
};


class OccGroupReader : public GroupReader<OccHistory, OccAttributes> {

public:
    static OccGroupReader *createOccGroupReader(const std::string sFileName);
    static OccGroupReader *createOccGroupReader(hid_t hFile);

    int tryReadAttributes(OccAttributes *pAttributes);
    int readArray(OccHistory *pGroup, const std::string sArrayName);
    int readData(OccHistory *pGroup);
    
protected:
    OccGroupReader();

};




#endif
