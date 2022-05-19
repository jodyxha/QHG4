#ifndef __CLIMATEGROUPREADER_H__
#define __CLIMATEGROUPREADER_H__

#include <hdf5.h>

class Climate;

struct ClimateAttributes : Attributes {
    int     m_iNumSeasons;
};


class ClimateGroupReader : public GroupReader<Climate, ClimateAttributes> {

public:
    static ClimateGroupReader *createClimateGroupReader(const std::string sFileName);
    static ClimateGroupReader *createClimateGroupReader(hid_t hFile);

    int tryReadAttributes(ClimateAttributes *pAttributes);
    int readArray(Climate *pGroup, const std::string sArrayName);
    int readData(Climate *pGroup);
    
protected:
    ClimateGroupReader();

};




#endif
