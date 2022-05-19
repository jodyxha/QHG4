#ifndef __LINKTOOLS_H__
#define __LINKTOOLS_H__


#include "SCellGrid.h"
#include "EQsahedron.h"

class WELL512;

class LinkTools {
public:

    static LinkTools *createInstance(const char *pQDFGeo);
    virtual ~LinkTools();

    int symmetrize();
    int scramble(WELL512 **pWELL);
    int findNeighborCoords(int iCell);
    int checkAntipodeSymmetry();
    int save();
protected:
    LinkTools();
    int init(const char *pQDFGeo);
    int readGeo();
    SCellGrid *readGrid();
    int createAntipodes();

    hid_t m_hFile;
    int m_iNumCells;
    int m_iSubDivs;
    SCellGrid *m_pCG;
    int       *m_pAntipodes;
    EQsahedron *m_pEQ;

};

#endif
