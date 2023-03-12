#ifndef __MOVESTATS_H__
#define __MOVESTATS_H__


#include "Action.h"
#include "ParamProvider2.h"
#include "Geography.h"

#define ATTR_MOVESTATS_NAME          "MoveStats"
#define ATTR_MOVESTATS_MODE_NAME     "MoveStats_Mode"


static const std::string MOVESTAT_DS_HOPS = "HopsNew";
static const std::string MOVESTAT_DS_DIST = "DistNew";
static const std::string MOVESTAT_DS_TIME = "TimeNew";


template<typename T>
class MoveStats : public Action<T> {
    
 public:
    MoveStats(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    virtual ~MoveStats();

    virtual int preLoop();

    // merge movelists into map cellT
    virtual int finalize(float fTime);

    virtual int extractAttributesQDF(hid_t hSpeciesActionGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesActionGroup);

    virtual int writeAdditionalDataQDF(hid_t hSpeciesActionGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesActionGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);

    static const int MODE_STAT_NONE  = -1;
    static const int MODE_STAT_FIRST = 0;
    static const int MODE_STAT_MIN   = 1;
    static const int MODE_STAT_LAST  = 2;
    
protected:
    uint       m_iNumCells;
    int        m_iMode;

    int    *m_aiHops;      // the definitive values
    double *m_adDist;
    double *m_adTime;

    int    **m_aiHopsTemp; // one array per thread
    double **m_adDistTemp; // one array per thread
    double **m_adTimeTemp; // one array per thread
    intset *m_asChanged;   // one set per thread

    std::vector<int>** m_vMoveList;  // one list per thread   


    double (*m_fCalcDist)(double dX1, double  dY1, double dX2, double dY2, double dScale);
    double m_dDistScale;
    int initializeOccupied();
 
    static const char *asNames[];

    Geography *m_pGeography;
};
#endif
