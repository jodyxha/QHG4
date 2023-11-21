#ifndef __ICOGRIDNODES_H__
#define __ICOGRIDNODES_H__

#include <map>
#include <string>
#include "icoutil.h"
#include "surfconsts.h"


class IcoNode;

#include "types.h" // for stringmap

class IcoGridNodes {
public:
    IcoGridNodes();
    ~IcoGridNodes();
    

    int write(const std::string sOutput, int iMaxLinks, bool bAddTilingInfo, stringmap &mAdditionalHeaderLines);
    int read(const std::string sInput);
    int readHeader(FILE *fIn, int  *piMaxLinks, bool *pAddTilingInfo, stringmap *pAdditionalHeader, bool *pbVer);

    std::map<gridtype, IcoNode*> m_mNodes;
    int setTiledIDs( std::map<gridtype,gridtype> &mID2T);

    void setData(stringmap &smNew) { m_smData = smNew;};
    stringmap getData() { return m_smData; }; // no reference: want a copy
protected:
    int blockWrite(FILE *fOut, int iMaxLinks);
    int sequentialWrite(FILE *fOut, bool bAddTilingInfo);

    int blockRead(FILE *fIn, int iMaxLinks, bool bVer2);
    int sequentialRead(FILE *fIn, bool bAddTilingInfo, bool bVer2);

    stringmap m_smData;
};

#endif

