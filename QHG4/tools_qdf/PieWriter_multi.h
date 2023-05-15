#ifndef __PIEWRITER_MULTI_H__
#define __PIEWRITER_MULTI_H__

#include <string>
#include "hdf5.h"

#include "types.h"
#include "Sampling.h"

// shuould be equal to the definition given in Sampling.h
template<typename T>
using groupedvals = std::map<int, std::vector<T>>;


typedef std::map<int, int *>                           maphistos;

typedef std::map<std::string, maphistos> multimaphistos;


typedef struct {
    double m_vPos[3];
    double m_vNorm[3];
} pointnorm;

typedef std::map<int, pointnorm>                mappointnorms; //cellID-> coords & norm
typedef std::map<std::string, mappointnorms>    mapmappointnorms; // dataitem name -> {cellID -> coords & norm }

typedef std::map<std::string, double *> mapdata;

class PieWriter_multi {
public:
    //    static PieWriter *createInstance(const std::string sPieName, groupedvals<double> &mGroups);
    static PieWriter_multi *createInstance(const multimaphistos &mHistos, uint iNumBins, uint iNumDims);
    virtual ~PieWriter_multi();

    int setValueNames(const stringvec &svValueNames);

    int prepareData(int iNumCoords, double *dLons, double *dLats, bool bSpherical, double dRadius=1);
    int prepareData(const mapmappointnorms &mmPointNorms);

    int writeToQDF(const std::string sQDFFile);

    int setVerbosity(bool bVerbose) {bool bOldV = m_bVerbose; m_bVerbose = bVerbose; return bOldV;};
protected:
    PieWriter_multi(uint iNumBins, uint iNumDims);
    //    int init(groupedvals<double> &mGroups);
    int init(const multimaphistos &mHistos);
    int prepareDataReal();

    //    groupedvals<double> m_mGroups;
    multimaphistos m_mHistos;

    uint m_iNumPies;
    uint m_iNumVals;
    uint m_iNumDims;
    std::string m_sPieName;
    std::string m_sValNames;

    mapdata m_pAllData;

    mapmappointnorms m_mmPointsNorms;
    bool m_bSpherical;
    bool m_bPointsOK;

    bool m_bVerbose;
};

#endif
