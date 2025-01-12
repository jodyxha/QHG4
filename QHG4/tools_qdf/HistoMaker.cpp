#include <cstring>
#include <cmath>

#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "Sampling.h"
#include "HistoMaker.h"

// expect file with format
//  file      ::= <dsname><cs><numpies><cr><numvals><cr><dataline>*
//  dsname    ::= "DATASET="<name_of_dataset>
//  numpies   ::= "NUQ_PIES="<number_of_pies>
//  numvals   ::= "NUM_VALS="<number of_values>
//  valnames  ::= "VAL_NAMES="<name_list>
//  name_list ::= <name:[" "<name>}*
//  dataline  ::= <pos><norm><value>*
//  pos       ::= <pos_x>  <pos_y>  <pos_z>
//  norm      ::= <norm_x> <norm_y> <norm_z>

const std::string PIEGROUP_NAME      = "PiePlots";
const std::string PIE_ATTR_NUQ_PIES  = "NumPies";
const std::string PIE_ATTR_NUM_VALS  = "NumVals";
const std::string PIE_DATASET_NAME   = "PieDataSet";
const std::string PIE_ATTR_VAL_NAMES = "ValNames";

//----------------------------------------------------------------------------
//  constructor
//
HistoMaker::HistoMaker(groupedvals<double> vGroupedVals) 
    : m_vGroupedVals(vGroupedVals),
      m_iNumItems(vGroupedVals.size()),
      m_pCurHistos(NULL),
      m_dMin(0),
      m_dMax(0),
      m_iNumBins(0),
      m_bStrict(false),
      m_iNumOutsideRange(0),
      m_bVerbose(false) {

    m_pCurHistos = new int *[m_iNumItems];
}


//----------------------------------------------------------------------------
//  destructor
//
HistoMaker::~HistoMaker() {
    if (m_pCurHistos != NULL) {
        /*
        for (uint i = 0; i < m_iNumItems; i++) {
            delete[] m_pCurHistos[i];
        }
        */
        delete[] m_pCurHistos;
    }
}

//----------------------------------------------------------------------------
//  createHistos
//
int **HistoMaker::createHistos(double dMin, double dMax, int iNumBins, bool bStrict) {
    
    m_dMin     = dMin; 
    m_dMax     = dMax;
    m_iNumBins = iNumBins;
    m_bStrict  = bStrict;

    for (uint i = 0; i < m_iNumItems; i++) {
        m_pCurHistos[i] = new int[m_iNumBins];
        memset(m_pCurHistos[i], 0, m_iNumBins*sizeof(int));
    }

    uint iCurIndex = 0;
    typename groupedvals<double>::const_iterator it;
    for (it = m_vGroupedVals.begin(); it != m_vGroupedVals.end(); ++it) {
        printf("[HistoMaker::createHistos] histo #%d for %d...\n", iCurIndex, it->first);
        createSingleHisto(iCurIndex, it->second);
        iCurIndex++;
    }

    return m_pCurHistos;
}



//----------------------------------------------------------------------------
//  createSingleHisto
//
int HistoMaker::createSingleHisto(uint iIndex, const std::vector<double> &vVals) {
    int iResult = 0;

    printf("creating histo for  %u (%zd elements)\n", iIndex, vVals.size());
    for (uint k = 0; k < vVals.size(); k++) {
        bool bInRange = true;
        double r = m_iNumBins *(vVals[k]-m_dMin)/(m_dMax - m_dMin);
        if (r < 0) {
            bInRange = false;
            r = 0;
            m_vBelow.push_back(vVals[k]);
        } else if (r >= m_iNumBins) {
            bInRange = false;
            r = m_iNumBins-1;
            m_vAbove.push_back(vVals[k]);
        }
        
        if (bInRange || !m_bStrict) {
            m_pCurHistos[iIndex][(int)(floor(r))]++;
        } else {
            m_iNumOutsideRange++;
        }
    }
    

    return iResult;
}


//----------------------------------------------------------------------------
// showHistos
//
void HistoMaker::showHistos(maphistos &mH) {
    maphistos::const_iterator it;
    for (it = mH.begin(); it != mH.end(); ++it) {
        xha_printf("(%d): ", it->first);
	for (int k = 0; k < m_iNumBins; ++k) {
             xha_printf(" %d", it->second[k]);
	}
	xha_printf("\n");
    }
    if (m_bStrict) {
	xha_printf("Omitted %d values out of range\n", m_iNumOutsideRange);
        xha_printf("Below: %zd (down to ", m_vBelow.size());
        double dMin = m_dMin;
        for (uint i = 0; i < m_vBelow.size(); i++) {
            if (m_vBelow[i] < dMin) {
                dMin = m_vBelow[i];
            }
        }
        xha_printf("%f)\n", dMin);
    }

    xha_printf("Above: %zd (up to ", m_vAbove.size());
    double dMax = m_dMax;
    for (uint i = 0; i < m_vAbove.size(); i++) {
        if (m_vAbove[i] < dMax) {
            dMax = m_vAbove[i];
        }
    }
    xha_printf("%f)\n", dMax);
}

