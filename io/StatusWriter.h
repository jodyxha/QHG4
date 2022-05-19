#ifndef __STATUSWRITER_H__
#define __STATUSWRITER_H__

#include <hdf5.h>
#include <vector>
#include <string>


class SCellGrid;
class PopBase;
class PopWriter;
class GridWriter;
class GeoWriter;
class ClimateWriter;
class VegWriter;
class NavWriter;
class OccWriter;

static const unsigned int WR_NONE =   0;
static const unsigned int WR_GRID =   1;
static const unsigned int WR_GEO  =   2;
static const unsigned int WR_CLI  =   4;
static const unsigned int WR_VEG  =   8;
static const unsigned int WR_NAV  =  16;
static const unsigned int WR_ALL  =  31;
static const unsigned int WR_MOV  =  32;
static const unsigned int WR_OCC  =  64;
static const unsigned int WR_POP  = 128; // must be thehighest value because we add these


class StatusWriter {
public:
    static StatusWriter *createInstance(SCellGrid *pCG, std::vector<PopBase *> vPops);
    virtual ~StatusWriter();

    int write(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, int iWhat, std::vector<std::pair<std::string, int>> &vSubt, int iDumpMode=-1);
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
