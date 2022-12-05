#ifndef __STATUSWRITER_H__
#define __STATUSWRITER_H__

#include <hdf5.h>
#include <vector>
#include <string>

#include "PopWriter.h"

class SCellGrid;
class PopBase;
class GridWriter;
class GeoWriter;
class ClimateWriter;
class VegWriter;
class NavWriter;
class OccWriter;



#define WR_NONE    0
#define WR_GRID   1
#define WR_GEO    2
#define WR_CLI    4
#define WR_VEG    8
#define WR_NAV   16
#define WR_ALL   31
#define WR_MOV   32
#define WR_OCC   64
#define WR_POP  128 // must be the highest value because we add these



class StatusWriter {
public:
    static StatusWriter *createInstance(SCellGrid *pCG, std::vector<PopBase *> vPops);
    virtual ~StatusWriter();

    int write(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, int iWhat, std::vector<std::pair<std::string, popwrite_flags>> &vSub, int iDumpMode=-1);
    int write(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, int iWhat, int iDumpMode=-1);
    
    std::string &getError() {return m_sError;};
protected:
    StatusWriter();
    int init(SCellGrid *pCG, std::vector<PopBase *> vPops);


    hid_t m_hFile;
    PopWriter      *m_pPopW;
    GridWriter     *m_pGridW;
    GeoWriter      *m_pGeoW;
    ClimateWriter  *m_pCliW;
    VegWriter      *m_pVegW;
    NavWriter      *m_pNavW;
    OccWriter      *m_pOccW;
    
    std::string m_sError;
};




#endif
