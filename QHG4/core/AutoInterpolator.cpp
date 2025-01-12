#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include <map>
#include <string>

#include <hdf5.h>

#include "LineReader.h"
#include "strutils.h"
#include "xha_strutilsT.h"
#include "types.h"
#include "EventConsts.h"
#include "SCellGrid.h"
#include "QDFUtils.h"
#include "AutoInterpolator.h"


//----------------------------------------------------------------------------
// createInstance
//   pCG:             cell grid of simulation
//   pEnvEventFile:   file listine environment load events
//   dStartTime:      start time of simulation (e.g. -85000)
//   vTargets:        list of targets of the form <group>/<array>>
//
AutoInterpolator *AutoInterpolator::createInstance(SCellGrid *pCG, const std::string sEnvEventFile, double dStartTime, target_list &vTargets) {
    AutoInterpolator *pAI = new AutoInterpolator(pCG, vTargets);
    int iResult = pAI->init(sEnvEventFile, dStartTime);
    if (iResult != 0) {
        delete pAI;
        pAI = NULL;
    }
    return pAI;
}


//----------------------------------------------------------------------------
// destructor
//
AutoInterpolator::~AutoInterpolator() {
    
    // set m_Input[iWhich][pName]=lengtharray(size, array);
    for (int i = 0; i < 2; i++) {
        for (auto nn : m_mInput[i]) {
            delete[] nn.second.second;
        }
    }
    for (auto nn : m_mDiff) {
        delete[] nn.second.second;
    }
}


//----------------------------------------------------------------------------
// constructor
//
AutoInterpolator::AutoInterpolator(SCellGrid *pCG, target_list &vTargets)
    : m_pCG(pCG),
      m_vTargets(vTargets),
      m_iCur(0) {
}


//----------------------------------------------------------------------------
// init
//
int AutoInterpolator::init(const std::string sEnvEventFile, double dStartTime) {
    int iResult = 0;

    iResult = readFileList(sEnvEventFile, dStartTime);
    if (iResult == 0) {
        iResult = checkFiles();
        if (iResult == 0) {
            allocateArrays();

            m_iCurStep = m_mFileList.begin()->first;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readFileList
//   we expect lines of the form
//   "env|"<groups>":"<file>"@"<time>
//   groups = <group>["+"<group>]
//   time   = ["S"|"T"]"["<number>"]"
//   group  = "grid" | "geo" | "climate" | "veg" | "nav"
//
//   additionally we expect at least one of the groups "geo" and "veg"
// 
int AutoInterpolator::readFileList(const std::string sEnvEventFile, double dStartTime) {
    int iResult = 0;
    char sFullLine[1024];
    xha_printf("[AutoInterpolator::readFileList]Looking at event file [%s]\n", sEnvEventFile);
    LineReader *pLR = LineReader_std::createInstance(sEnvEventFile, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine();
        while ((iResult == 0) && (pLine != NULL)) {
            strcpy(sFullLine, pLine);
            //xha_printf("Looking at line [%s]\n", pLine);
            char *pType = strtok(pLine, "|");
            //xha_printf("Have type [%s]\n", pType);

            if ((pType != NULL) && (strcmp(pType, "env") == 0)) {
                char *pGroup = strtok(NULL, ":");
                if (pGroup != NULL) {
                    //xha_printf("Have groups [%s]\n", pGroup);
                    std::set<std::string> vGroups;
                    char *p1 = pGroup;
                    char *p2 = strchr(p1, '+'); 
                    while (p2 != NULL) {
                        *p2 = '\0';
                        vGroups.insert(p1);
                        p1 = p2+1;
                        p2 = strchr(p1, '+');
                    }
                    char *pFile = strtok(NULL, "@");
                    if (pFile != NULL) {
                        //xha_printf("Have File [%s]\n", pFile);
                        char *pTime =  strtok(NULL, "@");
                        if (pTime != NULL) {
                            //xha_printf("Have TimeExpr [%s]\n", pTime);
                            pTime = strchr(pTime, '[');
                            if (pTime != NULL) {
                                pTime++;
                                char *pEnd = strchr(pTime, ']');
                                if (pEnd != NULL) {
                                    *pEnd = '\0';
                                    double dTime;
                                    if (strToNum(pTime, &dTime)) {
                                        int iStep = (int)(dTime - dStartTime);
                                        m_mFileList[iStep].vGroups = vGroups;
                                        m_mFileList[iStep].sFile = pFile;
                                    } else {
                                        iResult = -1;
                                        xha_printf("Line\n  %s\nNot a number [%s]\n", sFullLine, pTime);
                                    }
                                } else {
                                    iResult = -1;
                                    xha_printf("Line\n  %s\nExpected closing ']' in time expression\n", sFullLine);
                                }
                            } else {
                                iResult = -1;
                                xha_printf("Line\n  %s\nExpected opening '[' in time expression\n", sFullLine);
                            }
                        } else {
                            iResult = 0;
                            xha_printf("Line\n  %s\nExpected time expression after '@'\n", sFullLine);
                        }
                    } else {
                        iResult = -1;
                        xha_printf("Line\n  %s\nExpected filename after ':'\n", sFullLine);
                    }
                } else {
                    iResult = -1;
                    xha_printf("Line\n  %s\nExpected groups after '|'\n", sFullLine);
                }
            } else {
                iResult = -1;
                xha_printf("Line\n  %s\nExpected line to begin with \"env\"\n", sFullLine);
            }

            pLine = pLR->getNextLine();
        }
        delete pLR;
    } else {
        iResult = -1;
        xha_printf("Couldn't open [%s]\n", sEnvEventFile);
    }
    xha_printf("[AutoInterpolator::readFileList] have %zd files\n", m_mFileList.size());
    return iResult;	
}


//----------------------------------------------------------------------------
// prepareTargets
//   check if target data (group + array) is correct
//   if ok, save target data and required event ids
//   (as a side effect, short group names ('geo', 'veg') are set to 
//   corrrect group names (Geography, Vegetation)
//
int AutoInterpolator::prepareTargets() {
    int iResult = 0;

    m_vEvents.clear();
    // we use a set to avoid multiple entries
    std::set<int> sTempEvents;

    // now save pointers to the arrays which will be affected
    for (uint i = 0; (iResult == 0) && (i < m_vTargets.size()); i++) {
        iResult = -1;
        
        std::string  sGroupFinal;

        const std::string &sFullName = m_vTargets[i].sFullName;//.first;
        const std::string &sGroup    = m_vTargets[i].sGroup;//.second.first;
        const std::string &sArray    = m_vTargets[i].sArray;//.second.second;
        
        if ((sGroup == GEOGROUP_NAME) || (sGroup == "geo")){
            if (sArray == GEO_DS_ALTITUDE)  {
                // make sure group name is not 'shortcut'
                m_vTargets[i].sGroup = GEOGROUP_NAME;
                // set event
                sTempEvents.insert(EVENT_ID_GEO);
                // save pointer to target array
                m_mTargets[sFullName] = length_array(m_pCG->m_iNumCells, m_pCG->m_pGeography->m_adAltitude);
                iResult = 0;
            } else {
                xha_printf("Array [%s] in group [%s] not supported\n", sArray, sGroup);
            }
        } else if ((sGroup == VEGGROUP_NAME) || (sGroup == "veg")) {
            if (sArray == VEG_DS_NPP)  {
                // make sure group name is not 'shortcut'
                m_vTargets[i].sGroup = VEGGROUP_NAME;
                // set event
                sTempEvents.insert(EVENT_ID_VEG);
                // save pointer to target array
                m_mTargets[sFullName] = length_array(m_pCG->m_iNumCells, m_pCG->m_pVegetation->m_adBaseANPP);
                iResult = 0;
            } else {
                xha_printf("Array [%s] in group [%s] not supported\n", sArray, sGroup);
            }
        } else {
            // maybe add Climate/Rain, Climate/Temp
            xha_printf("No arrays in group [%s] are supported\n", sGroup);
        }
    }
    m_vEvents.insert(m_vEvents.begin(), sTempEvents.begin(), sTempEvents.end());
    return iResult;
}


//----------------------------------------------------------------------------
// checkArraySizes
//
int AutoInterpolator::checkArraySizes(hid_t hFile) {
    int iResult = 0;

    for (uint i = 0; (iResult == 0) && (i < m_vTargets.size()); i++) {

        const std::string &sFullName = m_vTargets[i].sFullName;//.first;
        const std::string &sGroup    = m_vTargets[i].sGroup;//.second.first;
        const std::string &sArray    = m_vTargets[i].sArray;//.second.second;

        //xha_printf("[AutoInterpolator::checkFiles] opening group [%s]\n", pGroup);fflush(stdout);
        hid_t hGroup = qdf_openGroup(hFile, sGroup);
        if (hGroup != H5P_DEFAULT) {
            if (qdf_link_exists(hGroup, sArray)) {
                hsize_t dims;

                hid_t hDataSet = qdf_openDataSet(hGroup, sArray);
                hid_t hDataSpace = H5Dget_space(hDataSet);
                //                                char sFullName[1024];
                //                                sprintf(sFullName, "%s/%s", sGroup, pArray);
                herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
                if (status >= 0) {
                    if (m_mArrSizes.find(sFullName) != m_mArrSizes.end()) {
                        if (m_mArrSizes[sFullName] == dims) {
                            //xha_printf("[AutoInterpolator::checkFiles] completed check of [%s] (following)\n", sFullName);
                            iResult = 0;
                        } else {
                            xha_printf("array size mismatch %u != %llu\n", m_mArrSizes[sFullName], dims);
                            iResult = -1;
                        }
                    } else {
                        if (m_pCG->m_iNumCells == dims) {
                            m_mArrSizes[sFullName] = dims;
                            //xha_printf("[AutoInterpolator::checkFiles] completed check of [%s] (first)\n", sFullName);
                            iResult = 0;
                        } else {
                            xha_printf("array size %llu does not match number of cells %d\n", dims, m_pCG->m_iNumCells);
                            iResult = -1;
                        }
                    }
                }
 
                qdf_closeDataSpace(hDataSpace);
                qdf_closeDataSet(hDataSet);


            } else {
                xha_printf("Didn't find array [%s] in group [%s]\n", sArray, sGroup);
            }
            qdf_closeGroup(hGroup);
        } else {
            xha_printf("Couldn't open group [%s]\n", sGroup);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// checkFiles
//   - exists?
//   - is QDF ?
//   - contains groups/arrays?
//   if ok, save target data and required event ids
//
int AutoInterpolator::checkFiles() {
    int iResult = 0;
    struct stat statbuf;
    xha_printf("[AutoInterpolator::checkFiles] starting file check\n");
    m_mTargets.clear();
    
    // we use a set to avoid multiple entries
    std::set<int> sTempEvents;
 
    timed_files::const_iterator it;
    for (it = m_mFileList.begin(); (iResult == 0) && (it != m_mFileList.end()); ++it){
        const std::string &sFile = it->second.sFile;
        iResult = stat(sFile.c_str(), &statbuf);
        if (iResult == 0) {
  	    //printf("[AutoInterpolator::checkFiles] looking at file %d:[%s]\n", it->first, pFile);fflush(stdout);
            hid_t hFile = qdf_openFile(sFile);
            if (hFile != H5P_DEFAULT) {
                iResult = prepareTargets();
                if (iResult == 0) {
                    iResult = checkArraySizes(hFile);
                }
                
                qdf_closeFile(hFile);
            } else {
                xha_printf("[%s] is not a qdf file\n", sFile);
                iResult = -1;
            }
        } else {
            xha_printf("[%s] does not exist\n", sFile);
            iResult = -1;
        }
    }
    m_vEvents.insert(m_vEvents.begin(), sTempEvents.begin(), sTempEvents.end());
    
    return iResult;
}


//----------------------------------------------------------------------------
// allocateArrays
//
void AutoInterpolator::allocateArrays() {
    for (uint k = 0; k < m_vTargets.size(); k++) {

        const std::string &sFullName =  m_vTargets[k].sFullName;//.first;
        for (int i = 0; i < 2; i++) {

            double *pData = new double[m_mArrSizes[sFullName]];
            m_mInput[i][sFullName]=length_array(m_mArrSizes[sFullName], pData);
        }
        double *pData = new double[m_mArrSizes[sFullName]];
        m_mDiff[sFullName]=length_array(m_mArrSizes[sFullName], pData);

    }
}


//----------------------------------------------------------------------------
// loadArray
//
int AutoInterpolator::loadArray(hid_t hFile, int iWhich, target_info &sTarget) {
    int iResult = -1;

    // get new array size
    hsize_t dims;

    const std::string &sFullName  = sTarget.sFullName;//first;
    const std::string &sGroupName = sTarget.sGroup;//second.first;
    const std::string &sArrayName = sTarget.sArray;//second.second;
    
    hid_t hGroup   = qdf_openGroup(hFile, sGroupName);
    hid_t hDataSet = qdf_openDataSet(hGroup, sArrayName);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    if (status >= 0) {
        
        double *pData = m_mInput[iWhich][sFullName].second;

        iResult = qdf_readArray(hGroup, sArrayName, dims, pData);
        if (iResult == 0) {
            m_mInput[iWhich][sFullName] = length_array(dims, pData);
        } else {
            xha_printf("Couldn't read array [%s/%s]\n", sGroupName, sArrayName);
        }
    } else {
        xha_printf("Couldn't get array size for [%s/%s]\n", sGroupName, sArrayName);
    }
    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
    qdf_closeGroup(hGroup);
    
    return iResult;
}


//----------------------------------------------------------------------------
// loadArrayForTime
//
int AutoInterpolator::loadArrayForTime(int iTime, int iWhich, target_info &sTarget) {
    int iResult = -1;
    
    timed_files::const_iterator it =  m_mFileList.find(iTime);
    if (it != m_mFileList.end()) {
        hid_t hFile = qdf_openFile(it->second.sFile);
        iResult = loadArray(hFile, iWhich, sTarget);
        qdf_closeFile(hFile);
    } else {
        xha_printf("Time [%d] not found in list\n", iTime);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// loadArrayFromGrid
//    Load array currently used by the grid.
//    For the time being, only alt and npp can be loaded.
//
int AutoInterpolator::loadArrayFromGrid(target_info &sTarget) {
    int iResult = 0;

    xha_printf("[AutoInterpolator::loadArrayFromGrid] loading data from initial grid\n");
    const std::string &sFullName   = sTarget.sFullName;//.first;
    const std::string &sGroupName  = sTarget.sGroup;//.second.first;
    const std::string &sArrayName  = sTarget.sArray;//.second.second;
    int iNumCells = m_pCG->m_iNumCells;
    if (sGroupName == GEOGROUP_NAME) {
        if (sArrayName == GEO_DS_ALTITUDE) {
            double *pData = m_mInput[m_iCur][sFullName].second;
            memcpy(pData, m_pCG->m_pGeography->m_adAltitude, iNumCells*sizeof(double));
            m_mInput[m_iCur][sFullName] = length_array(iNumCells, pData);
        } else {
            xha_printf("[AutoInterpolator::startInterpolations]unknown array: [%s]\n", sArrayName);
            iResult = -1;
        }
    } else if (sGroupName == VEGGROUP_NAME) {
        if (sArrayName == VEG_DS_NPP) {
            double *pData = m_mInput[m_iCur][sFullName].second;
            memcpy(pData, m_pCG->m_pVegetation->m_adTotalANPP, m_pCG->m_iNumCells*sizeof(double));
            m_mInput[m_iCur][sFullName] = length_array(iNumCells, pData);
        } else {
            xha_printf("[AutoInterpolator::startInterpolations]unknown array: [%s]\n", sArrayName);
            iResult = -1;
        }
    } else {
        xha_printf("[AutoInterpolator::startInterpolations]unknown group: [%s]\n", sGroupName);
        iResult = -1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// interpolate
//   increases the values in m_mTargets by the corresponding m_mDiff
//
int AutoInterpolator::interpolate(int iSteps) {
    xha_printf("EnvInterpolator::interpolate\n");
    if (iSteps == 1) {
        for (target_info sc : m_vTargets) {
            double *pSource = m_mDiff[sc.sFullName].second;
            double *pTarget = m_mTargets[sc.sFullName].second;
#pragma omp parallel for
            for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                pTarget[i] += pSource[i];
            }
        }
    } else {
        for (target_info sc : m_vTargets) {
            double *pSource = m_mDiff[sc.sFullName].second;
            double *pTarget = m_mTargets[sc.sFullName].second;
#pragma omp parallel for
            for (uint i = 0; i < m_pCG->m_iNumCells; i++) {
                pTarget[i] += iSteps*pSource[i];
            }
        }
    }
    return 0;
}


//----------------------------------------------------------------------------
// checkNewInterpolation
//   0: no new diffs
//   1: new diffs
//  -1: error
//
int AutoInterpolator::checkNewInterpolation(int iCurStep) {
    int iResult = 0;
    if (iCurStep >= m_iCurStep) {
        iResult = calcNextDiff();
        if (iResult == 0) {
            iResult = 1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// startInterpolations
//
int AutoInterpolator::startInterpolations(int iFirstStep) {
    int iResult = 0;
    xha_printf("AutoInterpolator::startInterpolations]\n");
    m_iCur = 0;
    if (iFirstStep <=  m_mFileList.begin()->first) {
        
        m_iCurStep = 0;
        m_itCur = m_mFileList.begin();

        for (uint i = 0; (iResult == 0) && (i < m_vTargets.size()); i++) {
            iResult = loadArrayFromGrid(m_vTargets[i]);
        }

    } else {
        // need to find last entry less than iFirstStep
        timed_files::const_iterator itC = m_mFileList.begin();
        bool bSearch = true;
        while (bSearch && (itC !=  m_mFileList.end())) {
            if (itC->first > iFirstStep) {
                bSearch = false;
            } else {
                ++itC;
            }
        }

        if (itC != m_mFileList.end()) {

            xha_printf("[AutoInterpolator::startInterpolations] found file with t=%d\n", itC->first); fflush(stdout);
            m_itCur =  itC;
            m_iCurStep = m_itCur->first;
            xha_printf("starting with : [%s] (%d)\n", m_itCur->second.sFile, m_iCurStep); fflush(stdout);

            for (uint i = 0; (iResult == 0) && (i < m_vTargets.size()); i++) {
                xha_printf("loading array [%s]\n", m_vTargets[i].sFullName); fflush(stdout);
                iResult = loadArrayForTime(m_iCurStep, m_iCur, m_vTargets[i]);
            }

        } else {
            xha_printf("[AutoInterpolator::startInterpolations] found nothing\n"); fflush(stdout);
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// calcNextDiff
//
int AutoInterpolator::calcNextDiff() {
    int iResult = 0;
    xha_printf("[AutoInterpolator::calcNextDiff] cur is %d\n", m_itCur->first); 
    m_itCur++;
    if (m_itCur != m_mFileList.end()) {
        int iNextStep = m_itCur->first;
        xha_printf("[AutoInterpolator::calcNextDiff] %d -> %d\n", m_iCurStep, iNextStep); 
        for (uint i = 0; (iResult == 0) && (i < m_vTargets.size()); i++) {
            iResult = loadArrayForTime(iNextStep, 1-m_iCur, m_vTargets[i]);
            std::string &sFullName = m_vTargets[i].sFullName;//.first;
            double *pCur   = m_mInput[m_iCur][sFullName].second;
            double *pNext  = m_mInput[1-m_iCur][sFullName].second;
            double *pDiff  = m_mDiff[sFullName].second;
            uint iLen =  m_mDiff[sFullName].first;
            double fDelta = (iNextStep - m_iCurStep);

#pragma omp parallel for
            for (uint j = 0; j < iLen; j++) {
                pDiff[j] = (pNext[j] - pCur[j])/fDelta;
            }
        }

        m_iCurStep = iNextStep;
        m_iCur = 1-m_iCur;

    } else {
        xha_printf("calcNextDiff: All zero\n");
        //set all differences to 0
        for (uint i = 0; (iResult == 0) && (i < m_vTargets.size()); i++) {
            const length_array &la = m_mDiff[m_vTargets[i].sFullName];
            memset(la.second, 0,la.first*sizeof(double));

        }
        m_iCur = 1-m_iCur;
        
        if (m_iCurStep != 0x7fffffff) {
            iResult = 1;
            m_iCurStep = 0x7fffffff;
        } else {
            iResult = 0;
        }
        //iResult = -1;
    }

    return iResult;
}




/*****************************************************************************/
/* obsolete / testing                                                        */
/*****************************************************************************/

//----------------------------------------------------------------------------
// displayArrays
//
void AutoInterpolator::displayArrays() {
    named_arrays::const_iterator it;
    for (int j = 0; j < 2; j++) {
        for (it = m_mInput[j].begin(); it != m_mInput[j].end(); ++it) {
            xha_printf("%s[%d][%s](%u):\n", (j==m_iCur)?"*":" ", j, it->first, it->second.first );
            // length  it->second.first
            for (uint i = 0; i < 50; i++) {
                printf(" %f", it->second.second[i]);
            }
            printf("\n");
        }
    }

    for (it = m_mDiff.begin(); it != m_mDiff.end(); ++it) {
        xha_printf(" Diff [%s](%u):\n", it->first, it->second.first );
        // length  it->second.first
        for (uint i = 0; i < 20; i++) {
            xha_printf(" %f", it->second.second[i]);
        }
        xha_printf("\n");
    }
}

        
//----------------------------------------------------------------------------
// displayFiles
//
void AutoInterpolator::displayFiles() {
    for (auto tf : m_mFileList) {
        xha_printf("[%d] : {", tf.first);
        for (auto vf : tf.second.vGroups) {
            xha_printf(" %s", vf);
        }
        xha_printf(" } [%s]\n", tf.second.sFile);
    }

}


//----------------------------------------------------------------------------
// loadtest
//
int AutoInterpolator::loadtest() {
    int iResult = 0;
    timed_files::const_iterator it;
    for (it =  m_mFileList.begin(); (iResult == 0) && (it != m_mFileList.end()); ++it) {
        const std::string &sFile = it->second.sFile;
        hid_t hFile = qdf_openFile(sFile);
        if (hFile != H5P_DEFAULT) {
            for (uint i = 0; (iResult == 0) && (i< m_vTargets.size()); i++) {
                iResult = loadArray(hFile, 0, m_vTargets[i]);
            }
            qdf_closeFile(hFile);
        }
    }
    return iResult;
}

/*
//----------------------------------------------------------------------------
// calcDiffs
//
int AutoInterpolator::calcDiffs(int iTime1, int iTime2) {
    int iResult = -1;
    xha_printf("[AutoInterpolator::calcDiffs] calculating diffs for %d -> %d\n", iTime1, iTime2);
    for (auto tt : m_vTargets) {
       
        const std::string &sFullName =  tt.sFullName;
        timed_files::const_iterator it = m_mFileList.find(iTime1);
        if (it != m_mFileList.end()) {
            hid_t hFile1 = qdf_openFile(it->second.sFile);
            iResult = loadArray(hFile1, 0, tt);
            
            it = m_mFileList.find(iTime2);
            if ((iResult == 0) && (it != m_mFileList.end())) {
                hid_t hFile2 = qdf_openFile(it->second.sFile);
                iResult = loadArray(hFile2, 1, tt);
                
                uint iLen = m_mInput[0][sFullName].first;

                double *pDiff = m_mDiff[sFullName].second;
                
                double *pD1 =  m_mInput[0][sFullName].second;
                double *pD2 =  m_mInput[1][sFullName].second;
#pragma omp parallel for
                for (uint i = 0; i < iLen; i++) {
                    pDiff[i] = (pD2[i] - pD1[i])/(iTime2 -iTime1);
                }
                m_mDiff[sFullName]=length_array(iLen, pDiff);

            } else {
                xha_printf("didn't find file with timestamp %d\n", iTime2);
            }
            
        } else {
            xha_printf("didn't find file with timestamp %d\n", iTime1);
        }
    }
    return iResult;
}
*/
