#ifndef __GRAPHDESC_H__
#define __GRAPHDESC_H__

#include <string>
#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "LineReader.h"

const std::string HEADER_LINE     = "QHG4GraphDesc";
const std::string NUMNODE_PREFIX  = "NUM_NODES=";
const std::string MAXLINKS_PREFIX = "MAX_LINKS=";
const std::string NODES_BEGIN     = "NODES_BEGIN";
const std::string NODES_END       = "NODES_END";
const std::string LINKS_BEGIN     = "LINKS_BEGIN";
const std::string LINKS_END       = "LINKS_END";

typedef std::map<uint, std::vector<uint>> intvecmap;

class GraphDesc {
public:
    static GraphDesc *createInstance(std::string sFileName);

    ~GraphDesc();
    int write(std::string sFileName);
    const intvecmap &getLinks() { return m_mNodeLinks;};
    const stringmap &getData() {return m_sData;};

protected:
    GraphDesc();
    int init(std::string sFileName);
    int readNodes(LineReader *pLR);
    int readLinks(LineReader *pLR);

    uint        m_iNumNodes;
    uint        m_iMaxLinks;
    stringmap   m_sData;

    intvecmap   m_mNodeLinks;
};

#endif
