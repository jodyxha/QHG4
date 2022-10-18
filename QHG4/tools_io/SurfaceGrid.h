#ifndef __SURFACEGRID_H__
#define __SURFACEGRID_H__

#include <string>

class SCellGrid;
class Surface;

class SurfaceGrid {
public:
    SurfaceGrid();
    static SurfaceGrid *createInstance(const std::string sQDF);

    virtual ~SurfaceGrid();

    SCellGrid *getCellGrid() { return m_pCG;};
    Surface *getSurface() { return m_pSurf;};

protected:
    int init(const std::string sQDF); 


    int createCellGrid(const std::string sQDF);

    int createSurface();

    SCellGrid *m_pCG;
    Surface   *m_pSurf;
};


#endif
