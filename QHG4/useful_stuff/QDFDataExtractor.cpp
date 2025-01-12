#include <cstdio>
#include <cstring>
#include <hdf5.h>
#include <omp.h>

#include <map>
#include <string>
#include <regex>

#include "strutils.h"
#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFUtilsT.h"
#include "EQsahedron.h"
#include "AgentCounter.h"
#include "PseudoPopManager.h"
#include "QDFDataExtractor.h"


#define PSEUDO_COUNT 0
#define PSEUDO_HYBR  1
 
patmap QDFDataExtractor::mPseudoArrPatterns = {
    {PSEUDO_COUNT, "(/?Populations/)([0-9A-Za-z_]+)/AgentCount"},
    {PSEUDO_HYBR,  "(/?Populations/)([0-9A-Za-z_]+)/Hybridization"},
 };

//----------------------------------------------------------------------------
// createInstance
//
QDFDataExtractor *QDFDataExtractor::createInstance(const std::string sQDFGrid, 
                                                   const std::string sQDFAdditional, 
                                                   const std::string sArrayPath, 
                                                   int          iWidth, 
                                                   int          iHeight) {
    QDFDataExtractor *pQDE = new QDFDataExtractor();
    int iResult = pQDE->init(sQDFGrid, sQDFAdditional, sArrayPath, iWidth/360.0, iHeight/180.0);

    if (iResult != 0) {
        delete pQDE;
        pQDE = NULL;
    }
    return pQDE;
}


//----------------------------------------------------------------------------
// createInstance
//
QDFDataExtractor *QDFDataExtractor::createInstance(const std::string sQDFGrid, 
                                                   const std::string sQDFAdditional, 
                                                   const std::string sArrayPath, 
                                                   float        fScale) {

    QDFDataExtractor *pQDE = new QDFDataExtractor();
    int iResult = pQDE->init(sQDFGrid, sQDFAdditional, sArrayPath, fScale, fScale);

    if (iResult != 0) {
        delete pQDE;
        pQDE = NULL;
    }
    return pQDE;
}


//----------------------------------------------------------------------------
// init
//
int QDFDataExtractor::init(const std::string sQDFGrid, 
                           const std::string sQDFAdditional, 
                           const std::string sArrayPath, 
                           float        fScaleX,
                           float        fScaleY) {

    int iResult = 0;
    m_sQDFGrid       = sQDFGrid;
    m_sQDFAdditional = sQDFAdditional;
    m_pdData         = NULL;

    m_sArrayPath = sArrayPath;

    m_pPPM = PseudoPopManager::createInstance();

    m_dDeltaLon = 1.0 / fScaleX;
    m_dDeltaLat = 1.0 / fScaleY;

    m_iW = 360.0*fScaleX;
    m_iH = 180.0*fScaleY;
    iResult = getGridAttrs();
    if (iResult == 0) {
         m_pEQ = EQsahedron::createInstance(m_iSubDivs, true);
         if (m_pEQ != NULL)  {
             int iNumCells = EQsahedron::calcNumVerts(m_iSubDivs);
             printf("Have EQsahedron with %d verts\n", iNumCells);
             
             iResult = findArraySource();
         } else {
             printf("Couldn't make EQsahedron with subdiv %d\n", m_iSubDivs);
             iResult = -1;
         }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// constructor 
//
QDFDataExtractor::QDFDataExtractor() {
    printf("@@@TODO: handling of agent pseudo-attributes: number (in cells), ..\n"); 
}


//----------------------------------------------------------------------------
// destructor 
//
QDFDataExtractor::~QDFDataExtractor() {
    if (m_pdData != NULL) {
        delete[] m_pdData;
    }
    if (m_pPPM != NULL) {
        delete m_pPPM;
    }
}


//----------------------------------------------------------------------------
// getArraySource
//
int QDFDataExtractor::getGridAttrs() {
    int iResult = -1;

    hid_t hFile = qdf_openFile(m_sQDFGrid);
    if (hFile != H5P_DEFAULT) {
        if (qdf_link_exists(hFile, GRIDGROUP_NAME)) {
            hid_t hGroup = qdf_openGroup(hFile, GRIDGROUP_NAME);
            if (hGroup != H5P_DEFAULT) {
                iResult = qdf_extractAttribute(hGroup, GRID_ATTR_NUM_CELLS, 1, &m_iNumCells); 
                if (iResult == 0) {
                    std::string sSubDiv;
                    iResult = qdf_extractSAttribute(hGroup, "SUBDIV", sSubDiv);
                    if (iResult == 0) {
                        if (strToNum(sSubDiv, &m_iSubDivs)) {
                            iResult = 0;

                        } else {
                            xha_printf("Subdiv attribute in [%s] is not a number:[%s]\n", m_sQDFGrid, sSubDiv);
                        }
                    } else {
                        xha_printf("Couldn't extrat attribute [%s] from group [%s]\n", GRID_ATTR_NUM_CELLS, GRIDGROUP_NAME);
                    }
                    
                    
                } else {
                    xha_printf("Couldn't extrat attribute [%s] from group [%s]\n", GRID_ATTR_NUM_CELLS, GRIDGROUP_NAME);
                }
            } else {
                xha_printf("Couldn't open group [%s]\n", GRIDGROUP_NAME);
            }
        } else {
            xha_printf("Group [%s] does not exist in file [%s]\n", GRIDGROUP_NAME, m_sQDFGrid);
        } 
    } else {
        xha_printf("Couldn't open file [%s]\n", m_sQDFGrid);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// isCharArray
//
bool QDFDataExtractor::isCharArray() {
    bool bIsChar = false;
    size_t iPos = m_sArrayPath.find("/");
    if (iPos != std::string::npos) {
        std::string sArr = m_sArrayPath.substr(0, iPos);
        std::string sPath = m_sArrayPath.substr(iPos+1);
        if (sPath == GEOGROUP_NAME) {
            if (sArr== GEO_DS_ICE_COVER) {
                bIsChar = true;
            } else if (sArr == GEO_DS_COASTAL) {
                bIsChar = true;
            } 
        }

    } else {
        xha_printf("ARray path has no '/': [%s]\n", m_sArrayPath);
    }
  
    return bIsChar;
}
 
//----------------------------------------------------------------------------
// getArraySource
//
int QDFDataExtractor::getArraySource(const std::string sQDF) {
    int iResult = -1;
    if (qdf_checkPathExists(sQDF, m_sArrayPath) == 0) {
        
        hid_t hFile = qdf_openFile(sQDF);
        if (hFile != H5P_DEFAULT) {
            m_pdData = new double[m_iNumCells];
            if (isCharArray()) {
                char *pcData = new char[m_iNumCells];
                iResult = qdf_readArray(hFile, m_sArrayPath, m_iNumCells, pcData);
                if (iResult == 0) {
                    for (int i = 0; i < m_iNumCells; i++) {
                        m_pdData[i] = pcData[i];
                    }
                } else {
                    xha_printf("Couldn't read array [%s] \n", m_sArrayPath);
                }
            } else {
                iResult = qdf_readArray(hFile, m_sArrayPath, m_iNumCells, m_pdData);
                if (iResult != 0) {
                    xha_printf("Couldn't read array [%s] \n", m_sArrayPath);
                }
            }
           
        } else {
            xha_printf("Couldn't open file [%s] as QDF file\n", m_sQDFGrid);
        }
        qdf_closeFile(hFile);
        
    } else {
        xha_printf("The path [%s] does not exist in [%s]\n", m_sArrayPath, sQDF);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// containsPath
//
const std::string QDFDataExtractor::containsPath(const std::string sPath) {
    std::string sHit = "";
    
    int iResult = qdf_checkPathExists(m_sQDFGrid, sPath);

    if (iResult == 0) {
        sHit = m_sQDFGrid;
    } else {
        iResult = qdf_checkPathExists(m_sQDFAdditional, sPath);
        if (iResult == 0) {
            sHit = m_sQDFAdditional;
        }
    }

    return std::string(sHit);
}


//----------------------------------------------------------------------------
// handlePseudoArrays
//
int QDFDataExtractor::handlePseudoArrays() {
    int iResult = -1;
    printf("Handling pseudoARrays\n");
    m_pdData = m_pPPM->createArray(m_sArrayPath, m_sQDFAdditional, m_sQDFGrid);
    if (m_pdData != NULL) {
        iResult = 0;
    }
    return iResult;
}

/*
//----------------------------------------------------------------------------
// handlePseudoArraysOld
//
int QDFDataExtractor::handlePseudoArraysOld() {
    int iResult = -1;
    int iType = -1;
    std::string sPath;
    std::string sSpecies;
    patmap::const_iterator it;
    for (it = mPseudoArrPatterns.begin(); (iType < 0) && (it != mPseudoArrPatterns.end()); ++it) {
        std::cmatch m;
        std::regex pseudoPat(it->second);
        printf("regex_match: %s, %s\n", m_pArrayPath, it->second.c_ str());
        if(regex_match(m_pArrayPath, m, pseudoPat)) {
            iType = it->first;
            sPath    = m.format("$1/$2");
            sSpecies = m.format("$2");
        }
    }
    
    if (iType >= 0) {
        printf("checking path [%s]\n", sPath.c_ str());
        const char *pHit = containsPath(sPath.c_ str()); 
        if (pHit != NULL) {
            printf("path [%s] of pseudo array type %d found in [%s]\n", sPath.c_ str(), iType, pHit);
            if (iType == 0) {
                AgentCounter *pAC = AgentCounter::createInstance(pHit, m_pQDFGrid, NULL);
                int iTot = pAC->countAgentsInCells();
                int *pCounts = pAC->getPopCounts(sSpecies.c_ str());
                printf("species [%s]  of [%s] has %d members\n", sSpecies.c_ str(), pHit, pAC->getTotal()); 
                
                m_pdData = new double[m_iNumCells];
                
#pragma omp parallel for 
                for (int i = 0; i < m_iNumCells; i++) {
                    m_pdData[i] = pCounts [i];
                }
                iResult = 0;
                delete pAC;
            }
        }
    }
    return iResult;
}
*/

//----------------------------------------------------------------------------
// findArraySource
//
int QDFDataExtractor::findArraySource() {
    int iResult = -1;
    
    iResult = getArraySource(m_sQDFGrid);
    if (iResult != 0) {
        if (m_sQDFGrid != m_sQDFAdditional) {
            iResult = getArraySource(m_sQDFAdditional);
        }
        if (iResult != 0) {
            iResult = handlePseudoArrays();
        }
    }
    return iResult;

}


//----------------------------------------------------------------------------
// extractDataTable
//
double **QDFDataExtractor::extractDataTable() {
    double **pdResult = NULL;

    if (m_pdData != NULL) {
        printf("Creating array [%d][%d]\n", m_iH, m_iW);
        pdResult = new double*[m_iH];
        for (int iH = 0; iH < m_iH; iH++) {
            pdResult[iH] = new double[m_iW];
            memset(pdResult[iH], 0, m_iW*sizeof(double));
        }

        printf("Filling it\n");
        double dLat = 90.0;
        for (int iH = 0; iH < m_iH; iH++) {
            double dLon = -180.0;
            for (int iW = 0; iW < m_iW; iW++) {
                gridtype iNode = m_pEQ->findNode(dLon, dLat);
                //printf("  %6.2f %5.2f %d %f\n", dLon, dLat, iNode,  m_pdData[iNode]);
                pdResult[iH][iW] = m_pdData[iNode];

                dLon += m_dDeltaLon;
            }
            dLat -= m_dDeltaLat;
        }
    }
    return pdResult;
}


//----------------------------------------------------------------------------
// extractDataList
//
double **QDFDataExtractor::extractDataList() {
    double **pdResult = NULL;

    if (m_pdData != NULL) {
        printf("Creating array [%d][%d]\n", m_iH*m_iW, 3);
        pdResult = new double*[m_iH*m_iW];
        for (int i = 0; i < m_iH*m_iW; i++) {
            pdResult[i] = new double[3];
            memset(pdResult[i], 0, 3*sizeof(double));
        }

        int i = 0;
        double dLat = -90.0;
        for (int iH = 0; iH < m_iH; iH++) {
            double dLon = -180.0;
            for (int iW = 0; iW < m_iW; iW++) {
                gridtype iNode = m_pEQ->findNode(dLon, dLat);
                pdResult[i][0]   = dLon;
                pdResult[i][1]   = dLat;
                pdResult[i][2]   = m_pdData[iNode];
                i++;
                dLon += m_dDeltaLon;
            }
            dLat += m_dDeltaLat;
        }
    }
    return pdResult;
}


