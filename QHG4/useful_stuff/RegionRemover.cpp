#include <cstdio>

#include <hdf5.h>
#include "qhg_consts.h"
#include "xha_strutilsT.h"
#include "geomutils.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"

#include "RegionRemover.h"

//----------------------------------------------------------------------------
// createInstance
//
RegionRemover *RegionRemover::createInstance(const std::string sQDF, stringvec &vsRegions) {
    RegionRemover *pRR = new RegionRemover();
    int iResult = pRR->init(sQDF, vsRegions);
    if (iResult != 0) {
        delete pRR;
        pRR = NULL;
    }
    return pRR;
};


//----------------------------------------------------------------------------
// getRegionNames
//
void RegionRemover::getRegionNames(stringvec &vs) {
    namedcoords::const_iterator it;
    for (it = mvRegions.begin(); it != mvRegions.end(); ++it) {
        vs.push_back(static_cast<std::string>(it->first));
    }
}


//----------------------------------------------------------------------------
// isPointInPoly
//
bool RegionRemover::isPointInPoly(double dX, double dY,coords &c) {
    bool bInside = false;
    dvec &vX =c.first;
    dvec &vY = c.second;
    int iN =vY.size();

    double fXPrev = vX[iN-1];
    double fYPrev = vY[iN-1];

    for (int i = 0; i < iN; ++i) {
        double fXCur = vX[i];
        double fYCur = vY[i];

        if (((fYCur > dY) != (fYPrev > dY)) &&
            (dX > (fXPrev - fXCur)*(dY - fYCur)/(fYPrev-fYCur)+fXCur)) {
            bInside = !bInside;
        }
        fXPrev = fXCur;
        fYPrev = fYCur;
    }

    return bInside;
}


//----------------------------------------------------------------------------
// constructor
//
RegionRemover::RegionRemover()
    : m_hFile(H5P_DEFAULT),
      m_hGeoGroup(H5P_DEFAULT),
      m_iNumCells(0),
      m_adLon(NULL),
      m_adLat(NULL),
      m_adAlt(NULL) {
}

//----------------------------------------------------------------------------
// destructor
//
RegionRemover::~RegionRemover() {
    if (m_adAlt != NULL) {
        delete[] m_adAlt;
    }
    if (m_adLat != NULL) {
        delete[] m_adLat;
    }
    if (m_adLon != NULL) {
        delete[] m_adLon;
    }

    namedboxes::iterator it;
    for (it = m_vBoxes.begin(); it != m_vBoxes.end(); ++it) {
        delete[] it->second;
    }
}


//----------------------------------------------------------------------------
// removeRegions
//
int RegionRemover::removeRegions() {
    int iResult = 0;
    int iFlips = 0;
    for (int i = 0; i < m_iNumCells; i++) {
        if (m_adAlt[i] > 0) {
            namedboxes::iterator it;
            for (it = m_vBoxes.begin(); it != m_vBoxes.end(); ++it) {
                if (isPointBox(m_adLon[i], m_adLat[i],
                               it->second[0],
                               it->second[1],
                               it->second[2],
                               it->second[3])) {
                    if (isPointInPoly(m_adLon[i], m_adLat[i], m_vPoly[it->first])) {
                        m_adAlt[i] = -m_adAlt[i];
                        iFlips++;
                    }
                }
            }
        }
    }
    xha_printf("%d flips\n", iFlips);
    if (iResult == 0) {
        herr_t status = H5Ldelete(m_hGeoGroup, GEO_DS_ALTITUDE.c_str(), H5P_DEFAULT);
        if (status >= 0) {
            iResult = qdf_writeArray(m_hGeoGroup, GEO_DS_ALTITUDE, m_iNumCells, m_adAlt);
            if (iResult != 0) {
                xha_printf("Couldn't write alt array\n");
            }
        } else {
            iResult =-1;
            xha_printf("Couldn't delete link\n");
        }
    }
    
    qdf_closeGroup(m_hGeoGroup);
    qdf_closeFile(m_hFile);
    return iResult;
}


//----------------------------------------------------------------------------
// removeRegionsInvert
//
int RegionRemover::removeRegionsInvert() {
    int iResult = 0;
    int iFlips = 0;
    for (int i = 0; i < m_iNumCells; i++) {
        if (m_adAlt[i] > 0) {
            namedboxes::iterator it;
            for (it = m_vBoxes.begin(); it != m_vBoxes.end(); ++it) {
                if (!isPointInPoly(m_adLon[i], m_adLat[i], m_vPoly[it->first])) {
                    m_adAlt[i] = -m_adAlt[i];
                    iFlips++;
                }
            }
        }
    }
    xha_printf("%d flips\n", iFlips);
    if (iResult == 0) {
        herr_t status = H5Ldelete(m_hGeoGroup, GEO_DS_ALTITUDE.c_str(), H5P_DEFAULT);
        if (status >= 0) {
            iResult = qdf_writeArray(m_hGeoGroup, GEO_DS_ALTITUDE, m_iNumCells, m_adAlt);
            if (iResult != 0) {
                xha_printf("Couldn't write alt array\n");
            }
        } else {
            iResult =-1;
            xha_printf("Couldn't delete link\n");
        }
    }
    
    qdf_closeGroup(m_hGeoGroup);
    qdf_closeFile(m_hFile);
    return iResult;
}


//----------------------------------------------------------------------------
// init
//
int RegionRemover::init(const std::string sQDF, stringvec &vsRegions) {
    int iResult = 0;
    
    iResult = checkQDF(sQDF);
    if (iResult == 0) {
        iResult = createPolys(vsRegions);
    }                            
    return iResult;
}


//----------------------------------------------------------------------------
// checkQDF
//
int RegionRemover::checkQDF(const std::string sQDF) {
    int iResult = 0;
    xha_printf("checking QDF\n");
    m_hFile = qdf_openFile(sQDF, true); // true: RW
    if (m_hFile != H5P_DEFAULT) {
        hid_t hGrid = qdf_openGroup(m_hFile, GRIDGROUP_NAME);
        if (hGrid != H5P_DEFAULT) {
            iResult = qdf_extractAttribute(hGrid, GRID_ATTR_NUM_CELLS, 1, &m_iNumCells);
            qdf_closeGroup(hGrid);
        } else {
            xha_printf("No grid group in [%s]\n", sQDF);
            iResult =-1;
        }
        printf("have %d cells\n", m_iNumCells);
        if (iResult == 0) {
            m_adLon = new double[m_iNumCells];
            m_adLat = new double[m_iNumCells];
            m_adAlt = new double[m_iNumCells];
            
            printf("Reading arrays\n");
            m_hGeoGroup =  qdf_openGroup(m_hFile, GEOGROUP_NAME);
            if (iResult == 0) {
                iResult = qdf_readArray(m_hGeoGroup, GEO_DS_LONGITUDE, m_iNumCells, m_adLon);
            }
            if (iResult == 0) {
                iResult = qdf_readArray(m_hGeoGroup, GEO_DS_LATITUDE, m_iNumCells, m_adLat);
            }
            if (iResult == 0) {
                iResult = qdf_readArray(m_hGeoGroup, GEO_DS_ALTITUDE, m_iNumCells, m_adAlt);
            }
            if (iResult == 0) {
                printf("Successfully read arrays\n");
            } else {
                printf("Reading of array failed\n");
            }
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createPolys
//
int RegionRemover::createPolys(stringvec &vsRegions) {
    int iResult = 0;
    xha_printf("checking %zd polys\n", vsRegions.size());
    for (uint i = 0; i < vsRegions.size(); i++) {
        
        namedcoords::const_iterator it;
        bool bSearching = true;
        for (it = mvRegions.begin(); bSearching && (it != mvRegions.end()); ++it) {
            if (it->first == vsRegions[i]) {
                bSearching = false;
                m_vPoly[it->first] = it->second;

                // here we hav to do the bboxes
                double *p = new double[4];
                p[0] = dPosInf;
                p[1] = dPosInf;
                p[2] = dNegInf;
                p[3] = dNegInf;
                const dvec &v = it->second.first;
                for (unsigned int k = 0; k < v.size(); ++k) {
                    if (v[k] <= p[0]) {
                        p[0] = v[k];
                    }
                    if (v[k] >= p[2]) {
                        p[2] = v[k];
                    }
                                    
                }
                const dvec &w = it->second.second;
                for (unsigned int k = 0; k < w.size(); ++k) {
                    if (w[k] <= p[1]) {
                        p[1] = w[k];
                    }
                    if (w[k] >= p[3]) {
                        p[3] = w[k];
                    }
                                    
                }
                m_vBoxes[it->first] = p;
            }
        }
        if (bSearching) {
            xha_printf("Unknown region [%s]\n",  vsRegions[i].c_str());
            iResult = -1;
        }

    }
    return iResult;
}


//----------------------------------------------------------------------------
// displaypolys
//
void RegionRemover::displayPolys() {
    namedcoords::const_iterator it;

    for (it = m_vPoly.begin(); it != m_vPoly.end(); ++it) {
        const std::string &s = it->first;
        const dvec vx = it->second.first;
        const dvec vy = it->second.second;
        xha_printf("%s (%zd)\n", s, vx.size());
        xha_printf("  x: ");
        for (unsigned int i = 0; i < vx.size(); ++i) {
            xha_printf(" %+9.3f", vx[i]);
        }
        xha_printf("\n");
        xha_printf("  y: ");
        for (unsigned int i = 0; i < vy.size(); ++i) {
            printf(" %+9.3f", vy[i]);
        }
        xha_printf("\n");
        xha_printf("\n");
    }
}

//----------------------------------------------------------------------------
// displayBoxes
//
void RegionRemover::displayBoxes() {
    namedboxes::const_iterator it;

    for (it = m_vBoxes.begin(); it != m_vBoxes.end(); ++it) {
        const std::string &s = it->first;
        double *vv = it->second;
        xha_printf("%s\n", s);
        xha_printf("  xmin: %+9.3f, xmax: %+9.3f\n", vv[0], vv[2]);
        xha_printf("  ymin: %+9.3f, ymax: %+9.3f\n", vv[1], vv[3]);
        xha_printf("\n");
    }
}
