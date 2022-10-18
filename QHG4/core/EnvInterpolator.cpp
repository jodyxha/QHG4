#include <cstdio>
#include <cstring>

#include <omp.h>
#include <hdf5.h>

#include "stdstrutilsT.h"
#include "SCellGrid.h"
#include "Geography.h"
#include "Climate.h"
#include "Vegetation.h"

#include "EventConsts.h"
#include "QDFUtils.h"
#include "EnvInterpolator.h"

#define ATTR_TARG "Targets"


//----------------------------------------------------------------------------
// constructor
//
EnvInterpolator::EnvInterpolator(SCellGrid *pCG)
    : m_pCG(pCG),
      m_bActive(true) {

}


//----------------------------------------------------------------------------
// destructor
//
EnvInterpolator::~EnvInterpolator() {
    cleanUp();
}


//----------------------------------------------------------------------------
// hasInterpolations
//
bool EnvInterpolator::hasInterpolations() {
    return m_bActive && (m_mArrays.size() > 0) ;
}


//----------------------------------------------------------------------------
// setActive
//
void EnvInterpolator::setActive(bool bState) {
    m_bActive = bState;
}


//----------------------------------------------------------------------------
// cleanUp
//
void EnvInterpolator::cleanUp() {
    stdprintf("EnvInterpolator::cleanUp\n");
    m_vNames.clear();
    m_mTargets.clear();
    for (auto it : m_mArrays) {
        delete[] it.second.second;
    }
    m_mArrays.clear();
}


//----------------------------------------------------------------------------
// readInterpolationData
//
int EnvInterpolator::readInterpolationData(const std::string sQDFInter) {
    int iResult = -1;
    // clean up previous stuff
    cleanUp();

    stdprintf("[EnvInterpolator::readInterpolationData] reading [%s]\n", sQDFInter);
    // open file 
    hid_t hInterp = qdf_openFile(sQDFInter);
    if (hInterp > 0) {
        iResult = extractTargets(hInterp);
        if (iResult == 0) {
            iResult = readArrays(hInterp);
            if (iResult == 0) {
                iResult = findTargetArrays();
            }
        }
        qdf_closeFile(hInterp);
    } else {
        stdprintf("Couldn't open [%s] as QDF file\n", sQDFInter);
    }
    return iResult;
}
 

//----------------------------------------------------------------------------
// extractTargets
//
int EnvInterpolator::extractTargets(hid_t hInterp) {
    int iResult = -1;
    hid_t hRoot=qdf_opencreateGroup(hInterp, "/", false);
    
    stdprintf("[EnvInterpolator::extractTargets] checking root groups\n");
    // check for top-levele attribute "targets"
    if (H5Aexists(hRoot, ATTR_TARG)) {
        // read it
        hid_t hTarget = H5Aopen_by_name(hRoot, ".", ATTR_TARG,H5P_DEFAULT,H5P_DEFAULT);
        hid_t atype = H5Aget_type(hTarget);
        hsize_t n = H5Tget_size(atype);
        char *pTargets = new char[n+1]; 
        memset(pTargets, 0, n+1);
        stdprintf("[EnvInterpolator::extractTargets]  attribute size %llu\n", n);

        H5Aread(hTarget, atype, pTargets);
        qdf_closeAttribute(hTarget);

        stdprintf("[EnvInterpolator::extractTargets] we have attribute[%s]\n", pTargets);

        iResult = 0;
        char *p = strtok(pTargets, "#");
        while ((iResult == 0) && (p != NULL)) {
            char *p1 = strchr(p, '/');
            if (p1 != NULL) {
                m_vNames.push_back(p);
                p = strtok(NULL, "#");
            } else {
                stdprintf("[EnvInterpolator::extractTargets] ERROR: attribute must have form 'GroupName'/'ArrayName' [%s]\n", p);
                iResult = -1;
            }
        }
        delete[] pTargets;
        showNames();
    } else {
        stdprintf("[EnvInterpolator::extractTargets] ERROR:Couldn't find attribute [%s] in file\n", ATTR_TARG);
    }


    return iResult;
}



//----------------------------------------------------------------------------
// readArrays
//
int EnvInterpolator::readArrays(hid_t hInterp) {
    int iResult = 0;
    stdprintf("[EnvInterpolator::readArrays] reading arrays\n");

    for (uint i = 0; (iResult == 0) && (i < m_vNames.size()); i++) {
        stringvec vParts;
        uint iNum = splitString(m_vNames[i], vParts, "/");
        if (iNum == 2) {
            std::string sGroup = vParts[0];
            std::string sArray = vParts[1];


            hid_t hGroup = qdf_openGroup(hInterp, sGroup);
            
            if (hGroup > 0) {
                hid_t hDataSet = qdf_openDataSet(hGroup, sArray);
                hid_t hDataSpace = H5Dget_space(hDataSet);
 
                hsize_t dim;
                H5Sget_simple_extent_dims(hDataSpace, &dim, NULL);

                double *pArr = new double[dim];
                herr_t status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pArr);
                if (status >= 0) {
                    m_mArrays[m_vNames[i]] = length_array(dim, pArr);
                } else {
                    iResult = -1;
                    stdprintf("[EnvInterpolator::readArrays] Error reading array [%s]\n", m_vNames[i]);
                }

            } else {
                iResult = -1;
                stdprintf("[EnvInterpolator::readArrays]  ERROR: Interpolation file specifies %s but no group [%s] exists\n", m_vNames[i], sGroup);
            }
        } else {
            // shouldn't happen - we checked this before
            iResult = 1;
            stdprintf("[EnvInterpolator::readArrays]  ERROR:no '/' in array name\n");
        }
          
    }

    return iResult;
}


//----------------------------------------------------------------------------
// findTargetArrays
//
int EnvInterpolator::findTargetArrays() {
    int iResult = 0;

    m_vEvents.clear();
    stdprintf("[EnvInterpolator::findTargetArrays] finding targets for %zd arrays\n", m_vNames.size());
    name_list::iterator it;
    for (it = m_vNames.begin(); (iResult == 0) && (it != m_vNames.end()); ++it) {
        //stdprintf("[EnvInterpolator::findTargetArrays] finding targets for array [%s]\n", *it);
  
        
        stringvec vParts;
        uint iNum = splitString(*it, vParts, "/");
        if (iNum == 2) {
            std::string sGroup = vParts[0];
            std::string sArray = vParts[2];
            
            if (sGroup == "Geography") {
                if (sArray == GEO_DS_ALTITUDE) {
                    m_mTargets[*it] = length_array(m_pCG->m_iNumCells, m_pCG->m_pGeography->m_adAltitude);
                    m_vEvents.push_back(EVENT_ID_GEO);
                    iResult = 0;
                } else {
                    stdprintf("[EnvInterpolator::findTargetArrays] ERROR: No interpolation supported for array [%s] in group [%s]\n", sArray, sGroup);
                    iResult = -1;
                }
            } else if (sGroup == "Vegetation") {
                if (sArray == VEG_DS_BASE_NPP) {
                    m_mTargets[*it] = length_array(m_pCG->m_iNumCells, m_pCG->m_pVegetation->m_adBaseANPP);
                    m_vEvents.push_back(EVENT_ID_VEG);
                    iResult = 0;
                } else {
                    stdprintf("[EnvInterpolator::findTargetArrays] ERROR: No interpolation supported for array [%s] in group [%s]\n", sArray, sGroup);
                    iResult = -1;
                }
            } else if (sGroup == "Climate") {
                if (sArray == CLI_DS_ACTUAL_TEMPS) {
                    m_mTargets[*it] = length_array(m_pCG->m_iNumCells, m_pCG->m_pClimate->m_adActualTemps);
                    m_vEvents.push_back(EVENT_ID_CLIMATE);
                    iResult = 0;
                } else if (sArray == CLI_DS_ACTUAL_RAINS) {
                    m_mTargets[*it] = length_array(m_pCG->m_iNumCells, m_pCG->m_pClimate->m_adActualRains);
                    m_vEvents.push_back(EVENT_ID_CLIMATE);
                    iResult = 0;
                } else {
                    stdprintf("[EnvInterpolator::findTargetArrays] ERROR: No interpolation supported for array [%s] in group [%s]\n", sArray, sGroup);
                    iResult = -1;
                }
            } else {
                // append new interpolation events here;
                stdprintf("[EnvInterpolator::findTargetArrays] ERROR: No interpolation supported for group [%s]\n", *it);
                iResult = -1;
            }

            if (iResult == 0) {
                //printf("[EnvInterpolator::findTargetArrays] Target for [%s]: %p\n", *it, m_mTargets[*it]);
            } else {
                stdprintf("[EnvInterpolator::findTargetArrays] ERROR: No interpolation supported for array [%s] in group [%s]\n", sArray, sGroup);
            }
        } else {
            // shouldn't happen - we checked this before
            iResult = -1;
            stdprintf("[EnvInterpolator::findTargetArrays] ERROR: no '/' in array name [%s]\n", *it);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// interpolate
//
int EnvInterpolator::interpolate(int iSteps) {
    stdprintf("EnvInterpolator::interpolate\n");
    if (iSteps == 1) {
        for (std::string name : m_vNames) {
            double *pSource = m_mArrays[name].second;
            double *pTarget = m_mTargets[name].second;
#pragma omp parallel for
            for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                pTarget[i] += pSource[i];
            }
        }
    } else {
        for (std::string name : m_vNames) {
            double *pSource = m_mArrays[name].second;
            double *pTarget = m_mTargets[name].second;
#pragma omp parallel for
            for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                pTarget[i] += iSteps*pSource[i];
            }
        }
    }
    return 0;
}


//----------------------------------------------------------------------------
// showNames
//
void EnvInterpolator::showNames() {
    stdprintf("Names\n");

    for (std::string name : m_vNames) {
        stdprintf("  %s\n", name);
    }
}


//----------------------------------------------------------------------------
// showArrays
//
void EnvInterpolator::showArrays() {
    stdprintf("Arrays\n");
    for (auto la : m_mArrays) {
        stdprintf("  %s (%d)\n", la.first, la.second.first);
    }
}



