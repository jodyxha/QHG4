#ifndef __CELL_SAMPLER_H__
#define __CELL_SAMPLER_H__

#include "AnalysisUtils.h"

typedef std::map<std::string, std::vector<int>> loc_cells;
 
class CellSampler {
public:
    static CellSampler *createInstance(const char *pQDFGrid, const char *pLocFile, double dDistance=0);
    ~CellSampler();
    int init(const char *pQDFGrid, const char *pLocFile, double dDistance);
    const loc_cells &getSelected() { return m_mvSelected; };
    int getNumCells() { return m_mCoords.size();};
    stringvec &getNames() { return m_vNames;};

    void showSelected(FILE *fOut, uint iLim);
private:
    CellSampler();
    int fillCoordMap(const char *pQDFGeoGrid);
    void deleteArrays();
    int selectCells(loc_cells &mvCandidates);

    arrpos_coords m_mCoords;
    loc_data      m_mLocData;
    stringvec     m_vNames;
    loc_cells     m_mvSelected;

};



#endif
