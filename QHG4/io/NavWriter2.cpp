#include <cstdio>
#include <cstring>
#include <hdf5.h>

#include "xha_strutilsT.h"
#include "Navigation2.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "NavWriter2.h"

//----------------------------------------------------------------------------
// constructor
//
NavWriter2::NavWriter2(Navigation2 *pNav) 
    : m_pNav(pNav) {

}

//----------------------------------------------------------------------------
// writeToHDF
//
int NavWriter2::writeToQDF(const std::string sFileName, int iStep, float fStartTime, const std::string sInfoString) {
    int iResult = -1;
    hid_t hFile = qdf_opencreateFile(sFileName, iStep, fStartTime, sInfoString);
    if (hFile > 0) {
        iResult = write(hFile);
        qdf_closeFile(hFile);  
    }
    return iResult;
}

class waterway {
public:
    waterway():m_iStart(0),m_iDest(0),m_dDist(0){}; 
    waterway( gridtype m_iStart, gridtype m_iDest, double m_dDist):m_iStart(m_iStart),m_iDest(m_iDest),m_dDist(m_dDist){}; 
    
    gridtype m_iStart;
    gridtype m_iDest;
    double   m_dDist;
};

//----------------------------------------------------------------------------
// write
//
int NavWriter2::write(hid_t hFile) {
    int iResult = -1;

    if (m_pNav != NULL) {
        iResult = 0;
        hid_t hNavGroup = qdf_opencreateGroup(hFile, NAV2GROUP_NAME);
        if (hNavGroup > 0) {
            iResult = writeNavAttributes(hNavGroup);
            
            if (iResult == 0) {
                //            xha_printf("Written NavAttributes\n");

            
                std::vector<waterway> vFlatWaterways;
                waterwaymap::const_iterator itWW;
                for (itWW = m_pNav->m_mWaterWays.begin(); itWW != m_pNav->m_mWaterWays.end(); ++itWW) {
                    destdistmap::const_iterator itDD;
                    for (itDD = itWW->second.begin(); itDD != itWW->second.end(); ++itDD) {
                        waterway ww(itWW->first, itDD->first, itDD->second);
                        vFlatWaterways.push_back(ww);
                    }
                }
            
                uint iNumWaterWays = vFlatWaterways.size();
                
                int *piStarts   = new gridtype[iNumWaterWays];
                int *piDests    = new gridtype[iNumWaterWays];
                double *pdDists = new double[iNumWaterWays];


                for (uint i = 0; i < iNumWaterWays; i++) {
                    waterway ww = vFlatWaterways[i];
                    piStarts[i] = ww.m_iStart;
                    piDests[i]  = ww.m_iDest;
                    pdDists[i]  = ww.m_dDist;
                }


            
                if (iResult == 0) {
                    iResult = qdf_writeArray(hNavGroup, NAV2_DS_START_NODES, iNumWaterWays, piStarts);
                }
            
                if (iResult == 0) {
                    iResult = qdf_writeArray(hNavGroup, NAV2_DS_DISTANCES,   iNumWaterWays, pdDists);
                }
            
                if (iResult == 0) {
                    //                xha_printf("[NavWriter] arrays written\n");
                } else {
                    xha_printf("[NavWriter] error writing arrays\n");
                }
            
                delete[] piStarts;
                delete[] piDests;
                delete[] pdDists;

                //@@@@ here write bridge data
                uint iNumBridges    = m_pNav->m_iNumBridges;
                if (iNumBridges > 0) {
                    double *pBridges    = new double[2*iNumBridges];
                    
                    bridgelist::const_iterator itb;
                    uint k = 0;
                    for (itb = m_pNav->m_vBridges.begin(); itb != m_pNav->m_vBridges.end(); ++itb) {
                        pBridges[k++] = itb->first;
                        pBridges[k++] = itb->second;
                    }
                    
                    iResult = qdf_writeArray(hNavGroup, NAV2_DS_BRIDGES, 2*iNumBridges, pBridges);
                    if (iResult == 0) {
                        //                xha_printf("[NavWriter] bridges written\n");
                    } else {
                        xha_printf("[NavWriter] error writing bridges\n");
                    }
                    delete[] pBridges;
                    
                }
            } else {
                xha_printf("[NavWriter] Couldn't write attributes\n");
            }
            qdf_closeGroup(hNavGroup);
        } else {
            iResult = -1;
            xha_printf("[NavWriter] Couldn't open group [%s]\n", NAV2GROUP_NAME);
            // couldn't open group
        }
    } else {
        xha_printf("[NavWriter] No Navigation found in CG\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeNavAttributes
//
int NavWriter2::writeNavAttributes(hid_t hNavGroup) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV2_ATTR_NUM_CELLS,     1, &m_pNav->m_iNumCells);
    }

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV2_ATTR_NUM_WATERWAYS, 1, &m_pNav->m_iNumWaterWays);
    }

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV2_ATTR_SAMPLE_DIST,   1, &m_pNav->m_dSampleDist);
    }

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV2_ATTR_NUM_BRIDGES,   1, &m_pNav->m_iNumBridges);
    }


    return iResult;
}
