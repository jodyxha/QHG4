#ifndef __PIEWRITER_H__
#define __PIEWRITER_H__

#include <string>
#include "hdf5.h"

#include "types.h"
#include "Sampling.h"

// shuould be equal to the definition given in Sampling.h
template<typename T>
using groupedvals = std::map<int, std::vector<T>>;


typedef std::map<int, int *>                           maphistos;

typedef struct {
    double m_vPos[3];
    double m_vNorm[3];
} pointnorm;

class PieWriter {
public:
    //    static PieWriter *createInstance(const std::string sPieName, groupedvals<double> &mGroups);
    static PieWriter *createInstance(const std::string sPieName, maphistos &mHistos, uint iNumBins, uint iNumDims);
    virtual ~PieWriter();

    int setValueNames(const stringvec &svValueNames);

    int prepareData(int iNumCoords, double *dLons, double *dLats, bool bSpherical, double dRadius=1);
    int prepareData(std::map<int, pointnorm>);

    int writeToQDF(const std::string sQDFFile);

    int setVerbosity(bool bVerbose) {bool bOldV = m_bVerbose; m_bVerbose = bVerbose; return bOldV;};
protected:
    PieWriter(const std::string sPieName, uint iNumBins, uint iNumDims);
    //    int init(groupedvals<double> &mGroups);
    int init(maphistos &mHistos);
    int prepareDataReal();

    //    groupedvals<double> m_mGroups;
    maphistos m_mHistos;

    uint m_iNumPies;
    uint m_iNumVals;
    uint m_iNumDims;
    std::string m_sPieName;
    std::string m_sValNames;

    double *m_pAllData;

    std::map<int, pointnorm> m_mPointsNorms;
    bool m_bSpherical;
    bool m_bPointsOK;

    bool m_bVerbose;
};

#endif
