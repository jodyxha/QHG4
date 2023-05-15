#ifndef __HISTOMAKER_MULTI_H__
#define __HISTOMAKER_MULTI_H__

#include "types.h"
#include "Sampling.h"

//typedef std::pair<double, double> doublepair;
typedef std::map<int, int *>  maphistos;
typedef std::map<std::string, maphistos> multimaphistos;


class HistoMaker_multi {

public:
    HistoMaker_multi(groupedvals<double> vGroupedVals);
    virtual ~HistoMaker_multi();

    int **createHistos(double dMin, double dMax, int iNumBins, bool bStrict = false);


    void showHistos(maphistos &mH);

    int setVerbosity(bool bVerbose) {bool bOldV = m_bVerbose; m_bVerbose = bVerbose; return bOldV;};

protected:

    int createSingleHisto(uint iIndex, const std::vector<double> &vVals);
    
    groupedvals<double> m_vGroupedVals;

    uint m_iNumItems;

    int **m_pCurHistos;

    double m_dMin;
    double m_dMax;
    int m_iNumBins;
    bool m_bStrict;

    int m_iNumOutsideRange;
    doublevec m_vBelow;
    doublevec m_vAbove;
    
    bool m_bVerbose;
};



#endif
 
