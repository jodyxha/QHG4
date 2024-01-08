#ifndef __NAVTONAV2CONVERTER_H__
#define __NAVTONAV2CONVERTER_H__

#include <string>
#include <hdf5.h>

#include "SCellGrid.h"
#include "Navigation.h"

class NavToNav2Converter {
public:
    static NavToNav2Converter *createInstance(std::string sInputFile, int iNumCells=-1);
    virtual ~NavToNav2Converter();

    int convert(std::string sOutputFile);
protected:
    NavToNav2Converter();

    int init(std::string sInputFile, uint iNumCells);
    
    Navigation *readNavigation();
    uint countConnections();
    void display();
    int createWriteNav2(std::string sOutputFile);
    int getNumCells();

    SCellGrid   *m_pCG;
    std::string  m_sInputFile;
    uint         m_iNumCells;
    hid_t        m_hInputFile;
    Navigation  *m_pNav;

};

#endif
