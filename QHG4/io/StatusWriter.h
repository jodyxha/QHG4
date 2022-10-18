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

enum output_flags {
    WR_NONE =   0,
    WR_GRID =   1,
    WR_GEO  =   2,
    WR_CLI  =   4,
    WR_VEG  =   8,
    WR_NAV  =  16,
    WR_ALL  =  31,
    WR_MOV  =  32,
    WR_OCC  =  64,
    WR_POP  = 128, // must be the highest value because we add these
};

inline output_flags& operator+=(output_flags &a, output_flags b) {
    a = static_cast<output_flags>(static_cast<int>(a) + static_cast<int>(b));
    return a;
}

inline output_flags operator|(output_flags a, output_flags b) {
    return static_cast<output_flags>(static_cast<int>(a) | static_cast<int>(b));
}

inline output_flags& operator|=(output_flags &a, output_flags b) {
    a = a | b;
    //    a = static_cast<output_flags>(static_cast<int>(a) | static_cast<int>(b));
    return a;
}


class StatusWriter {
public:
    static StatusWriter *createInstance(SCellGrid *pCG, std::vector<PopBase *> vPops);
    virtual ~StatusWriter();

    int write(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, output_flags iWhat, std::vector<std::pair<std::string, popwrite_flags>> &vSub, int iDumpMode=-1);
    int write(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString, output_flags iWhat, int iDumpMode=-1);
    
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
