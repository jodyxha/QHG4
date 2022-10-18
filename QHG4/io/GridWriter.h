#ifndef __GRIDWRITER_H__
#define __GRIDWRITER_H__

#include <hdf5.h>

class SCellGrid;

class GridWriter {
public:
    GridWriter(SCellGrid *pCG, stringmap *psm);

    int writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, bool bNew);
    int write(hid_t hFile);

protected:
    SCellGrid *m_pCG;
    stringmap *m_psm;
    hid_t m_hCellDataType;

    hid_t createCellDataType();
    int writeCellData(hid_t hDataSpace, hid_t hDataSet, hid_t hCellType);
    int writeGridAttributes(hid_t hGridGroup);
 
};



#endif
