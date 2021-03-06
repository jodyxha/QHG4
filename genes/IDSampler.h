#ifndef __IDSAMPLER_H__
#define __IDSAMPLER_H__

#include <set>
#include <map>
#include <vector>
#include <string>

#include "types.h"

typedef struct locitem {
    double dLon;
    double dLat;
    double dDist;
    int iNum;
} locitem;

typedef struct agdata {
    idtype iID;
    idtype iMomID;
    idtype iDadID;
    int iGender;
    int iCellID;
    double dLon;
    double dLat;
} agdata;


// map: location name => id list
typedef std::map<std::string, std::vector<idtype> >  stringvidmap;
// map: location name => locitem
typedef std::map<std::string, locitem>               locdata;
// map: cell id => coord pair
typedef std::map<int, std::pair<double, double> >    coordmap;
// vector of string
typedef std::vector<std::string>                     stringvec; 
// map: idtype => agdata
typedef std::map<idtype, agdata*>                    idagdatamap;
// map: location name => (map: time => id list)
typedef std::map<std::string, std::map<float, std::vector<idtype> > > loctimeids;

class IDSampler {

public:
    static IDSampler *createInstance(const std::string sQDFGrid);
    ~IDSampler();
    int init(const std::string sQDFGrid);

    int getSamples(const std::string sQDFTime, const std::string sPopName, const std::string sLocFile, locdata &mLocData, stringvidmap &mvIDs, idagdatamap &mAgentData);
    int getSamplesP(const std::string sQDFTime, const std::string sPopName, const std::string sLocFile, locdata &mLocData, stringvidmap &mvIDs, idagdatamap &mAgentData);
    int getAttributes(const std::string sQDFTime, const std::string sPopName, const std::string sLocFile, locdata  &mLocData, stringvidmap &mvIDs, idagdatamap &mAgentData, idset &sSelected);

    int getSamples(stringvec &vQDFPops, const std::string sPopName, const std::string sLocFile, locdata &mLocData, loctimeids &mmvIDs, idagdatamap &mAgentData, bool bParallel);
    int getAttributes(stringvec &vpQDFPops, const std::string sPopName, const std::string sLocFile, locdata  &mLocData, loctimeids &mmvIDs, idagdatamap &mAgentData, idset &sSelected);

    int getNumAgents() { return m_iNumAgents;};
    idtype *getIDs() { return m_pIDs;};

private:
    IDSampler();
    int fillCoordMap(const std::string sQDFGeoGrid);

    int readArrays(const std::string sQDFTime, const std::string sPopName);
    int fillLocData(const std::string sLocFile, stringvec &vNames, locdata &mLocData);

    void deleteArrays();


    coordmap m_mCoords;
    int m_iNumAgents;
    idtype *m_pIDs;
    int    *m_pCellIDs;
    int    *m_pGenders;
    float   m_fTimeStep;

    bool *m_pHits;

};

#endif
