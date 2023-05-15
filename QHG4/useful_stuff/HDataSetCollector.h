#ifndef __HDATASETCOLLECTOR_H__
#define __HDATASETCOLLECTOR_H__

#include <hdf5.h>
#include <string>
#include <map>

#include "types.h"

typedef std::map<int, std::string> patmap;

typedef std::map<std::string, hid_t> groupinfo;

class HDataSetCollector {
public:
    static HDataSetCollector *createInstance(const std::string sQDF);
    
    virtual ~HDataSetCollector();

    const stringvec &getPaths() { return m_vPathNames;};
    const stringvec &getPseudos(patmap &PseudoPats);
protected:
    HDataSetCollector();
    int init(const std::string sQDF);


    stringvec m_vPathNames;
};



#endif
