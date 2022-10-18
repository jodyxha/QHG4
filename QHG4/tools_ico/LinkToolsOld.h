#ifndef __LINKTOOLS_H__
#define __LINKTOOLS_H__


#include "Vec3D.h"
#include "Quat.h"
#include "SCellGrid.h"
#include "EQsahedron.h"

class WELL512;

class LinkTools {
public:

    static LinkTools *createInstance(const char *pQDFGrid);
    virtual ~LinkTools();

    int scramble(WELL512 **pWELL);
    int findNeighborCoords(int iCell);

    int save();
protected:
    LinkTools();
    int init(const char *pQDFGrid);
    int readGeo();

    hid_t m_hFile;
    int m_iNumCells;
    SCellGrid *m_pCG;

};

#endif
