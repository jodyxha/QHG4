#ifndef __LOCAGCOUNTER_H__
#define __LOCAGCOUNTER_H__

#include <set>
#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "utils.h"
#include "AnalysisUtils.h"

typedef std::map<std::string, int> loccounts;

class LocAgCounter {
public:
    ~LocAgCounter();
    static LocAgCounter *createInstance(const std::string sQDFGrid, const std::string sQDFTime, const std::string sLocFile, bool bCartesian);
    int doCounts();  
    void show(bool bNice, bool bSort);
 private:
    LocAgCounter();
    int init(const std::string sQDFGrid, const std::string sQDFTime, const std::string sLocFile, bool bCartesian);

    int fillCoordMap(const std::string sQDFGeoGrid);
    int readArrays(const std::string sQDFTime, const std::string sPopName);
    void deleteArrays();
    int getCandidatesNew();

    loccounts  m_mCounts;
    arrpos_coords m_mCoords;
    int m_iNumAgents;
    idtype *m_pIDs;
    int    *m_pCellIDs;
    stringvec    m_vNames;
    loc_data m_mLocData;
   
    double m_dScale;
    double (*m_fCalcDist)(double dX1, double  dY1, double dX2, double dY2, double dScale);

};

#endif
