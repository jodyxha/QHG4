#ifndef __NODEINDEX_H__
#define __NODEINDEX_H__

#include <map>
#include <string>
#include "types.h"

class IcoNode;

#include "types.h" // for stringmap

class NodeIndex {
public:
    NodeIndex(){};
    ~NodeIndex(){};
    
    std::map<gridtype, IcoNode*> m_mNodes;
    uint getNumNodes() { return m_mNodes.size();};
protected:
   
};

#endif

