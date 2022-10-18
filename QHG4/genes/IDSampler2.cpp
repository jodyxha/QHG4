#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <omp.h>

#include "types.h"
#include "stdstrutilsT.h"
#include "utils.h"
#include "colors.h"
#include "geomutils.h"
#include "WELL512.h"
#include "QDFUtils.h" 
#include "QDFArray.h"
#include "QDFArrayT.h" 
#include "AnalysisUtils.h"
#include "IDSample.h"
#include "IDSampler2.h"

#define RADIUS 6371.3
#define SPECIAL_CHAR "~"

//----------------------------------------------------------------------------
// constructor
//
IDSampler2::IDSampler2(WELL512 *pWELL) 
    : m_iNumAgents(0),
      m_pIDs(NULL), 
      m_pCellIDs(NULL),
      m_pGenders(NULL),
      m_pCurSample(NULL),
      m_pRefSample(NULL),
      m_dScale(1.0), 
      m_fCalcDist(NULL),
      m_pWELL(pWELL) {
}


//----------------------------------------------------------------------------
// destructor
//
IDSampler2::~IDSampler2() {

    deleteArrays();
    
}


//----------------------------------------------------------------------------
// createInstance
//
IDSampler2 *IDSampler2::createInstance(const std::string sQDFGrid, WELL512 *pWELL, bool bCartesian) {
    IDSampler2 *pIS = new IDSampler2(pWELL);
    int iResult = pIS->init(sQDFGrid, bCartesian);
    if (iResult != 0) {
        delete pIS;
        pIS = NULL;
    }

    return pIS;
}


//----------------------------------------------------------------------------
// deleteArrays
//
void IDSampler2::deleteArrays() {
    if (m_pIDs != NULL) {
        delete[]  m_pIDs;
    }
    m_pIDs = NULL;

    if (m_pCellIDs != NULL) {
        delete[]  m_pCellIDs;
    }
    m_pCellIDs = NULL;

    if (m_pGenders != NULL) {
        delete[]  m_pGenders;
    }
    m_pGenders = NULL;
}


//----------------------------------------------------------------------------
// init
//
int IDSampler2::init(const std::string sQDFGrid, bool bCartesian) {
    int iResult = -1;

    iResult = fillCoordMap(sQDFGrid);

    if (bCartesian) {
        m_fCalcDist = &cartdist;
    } else { 
        m_fCalcDist = &spherdistDeg;
        m_dScale = RADIUS;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// makeSelectionList
//
int IDSampler2::makeSelectionList(int iNumTotal, int iNumSelect, intset &sSelectedIndexes) {
    int *aTemp = new int[iNumTotal];
    for (int i = 0; i< iNumTotal; i++) {
        aTemp[i] = i;
    }

    if (iNumSelect > iNumTotal) {
        iNumSelect = iNumTotal;
    }
    
    // repeated swapping of array elements will result in
    // iNumSelect randomly selected elements at the end of the array
    for (int i = iNumTotal-1; i > iNumTotal-iNumSelect-1; i--) {
        // swap the current element with a random lower
        //        int iIndex = (int)(((double)(i+1)*rand())/RAND_MAX);
        int iIndex = m_pWELL->wrandi(0, i+1);
        int t = aTemp[i];
        aTemp[i] = aTemp[iIndex];
        aTemp[iIndex] = t;
    }

    // copy to intset
    sSelectedIndexes.insert(aTemp+iNumTotal-iNumSelect, aTemp+iNumTotal);

    delete[] aTemp;
    return 0;
}


//----------------------------------------------------------------------------
// getCandidatesOld
//
int IDSampler2::getCandidatesOld(loc_data  &mLocData, loc_varrpos &mvCandidates) {
    int iResult = 0;
    stdprintf("old version\n");
    uint iAssigned = 0;
    uint iDismissed = 0;
    for (int i = 0; i < m_iNumAgents; i++) {
        double dLon0 = m_mCoords[m_pCellIDs[i]].first;
        double dLat0 = m_mCoords[m_pCellIDs[i]].second;
            
        // each agent should only belong to one region
        bool bSearching = true;
        loc_data::const_iterator it;
        for (it = mLocData.begin(); (iResult == 0) && bSearching && (it != mLocData.end()); ++it) {
            double d = 0;
            
            // the coordinates are in degrees! make them radians
            // normal spherical distance
            d = m_fCalcDist(dLon0, dLat0, it->second.dLon, it->second.dLat, m_dScale);
                
            if (d < it->second.dDist) {
                mvCandidates[it->first].push_back(i);
                bSearching = false;
                iAssigned++;
            } else {
                
            }
        }
        if (bSearching) {
            iDismissed++;
        }
    }
    loc_varrpos::const_iterator it3;
    int iTot = 0;
    for (it3 = mvCandidates.begin(); it3 != mvCandidates.end(); ++it3) {
        //           stdprintf("For [%s]: %zd elements«\n", it3->first.c_str(), it3->second.size());
        iTot +=  it3->second.size();
    }
    intset s;
    for (it3 = mvCandidates.begin(); it3 != mvCandidates.end(); ++it3) {
        s.insert(it3->second.begin(), it3->second.end());
    }
    stdprintf("Old: Summed number of candidate: %d, Cumulated number of candidates %zd\n", iTot, s.size()); 
    stdprintf("Old: Assigned: %d; dismissed: %d; unhandled: %d\n", iAssigned, iDismissed, m_iNumAgents-iAssigned-iDismissed);
    return iResult;
}


//----------------------------------------------------------------------------
// getCandidatesNew1
//
int IDSampler2::getCandidatesNew1(loc_data  &mLocData, loc_varrpos &mvCandidates) {
    int iResult = 0;

    stdprintf("new version 1: loc-parag\n");
    uint *aiAssignedPar  = new uint[omp_get_max_threads()];
    uint *aiDismissedPar = new uint[omp_get_max_threads()];
    memset(aiAssignedPar,  0, omp_get_max_threads()*sizeof(uint));
    memset(aiDismissedPar, 0, omp_get_max_threads()*sizeof(uint));
    // each agent should only belong to one region
    loc_data::const_iterator it;
    loc_varrpos *amvCandidatesPar = new loc_varrpos[omp_get_max_threads()];

    for (it = mLocData.begin(); (iResult == 0)  && (it != mLocData.end()); ++it) {
        double dLon0 = it->second.dLon;
        double dLat0 = it->second.dLat;
        
#pragma omp parallel for
        for (int i = 0; i < m_iNumAgents; i++) {
            double dLon1 = m_mCoords[m_pCellIDs[i]].first;
            double dLat1 = m_mCoords[m_pCellIDs[i]].second;
        
            
            //            double d = spherdist(dLon0*M_PI/180, dLat0*M_PI/180, dLon1*M_PI/180, dLat1*M_PI/180, RADIUS);
            double d = m_fCalcDist(dLon0, dLat0, dLon1, dLat1, m_dScale);
            

            if (d < it->second.dDist) {
                amvCandidatesPar[omp_get_thread_num()][it->first].push_back(i);
                aiAssignedPar[omp_get_thread_num()]++;
            } else {
                aiDismissedPar[omp_get_thread_num()]++;
            }
        }
    }
    
    uint iAssignedPar  = 0;
    uint iDismissedPar = 0;
    for (int i = 0; i < omp_get_max_threads(); i++) {
        loc_varrpos::const_iterator it2;
        for (it2 = amvCandidatesPar[i].begin(); it2 != amvCandidatesPar[i].end(); ++it2) {
            mvCandidates[it2->first].insert(mvCandidates[it2->first].end(), it2->second.begin(), it2->second.end());
            
            iAssignedPar  += aiAssignedPar[i];
            iDismissedPar += aiDismissedPar[i];
        }
    }
    loc_varrpos::iterator it3Par;
    int iTotPar = 0;
    for (it3Par = mvCandidates.begin(); it3Par != mvCandidates.end(); ++it3Par) {
        //       stdprintf("For [%s]: %zd elements«\n", it3Par->first.c_str(), it3Par->second.size());
        iTotPar +=  it3Par->second.size();
    }

    // stdprintf("Par: Assigned: %u; dismissed: %u; unhandled: %u\n", iAssignedPar, iDismissedPar, m_iNumAgents-iAssignedPar-iDismissedPar);
    // stdprintf("iTotPar: %d (mvCandidates:%zd)\n", iTotPar, mvCandidates.size());
    delete[] amvCandidatesPar;
    delete[] aiAssignedPar;
    delete[] aiDismissedPar;
    return iResult;
}


//----------------------------------------------------------------------------
// getCandidatesNew2
//
int IDSampler2::getCandidatesNew2(loc_data  &mLocData, loc_varrpos &mvCandidates) {
    int iResult = 0;
    stdprintf("new version 2: parag-loc\n");
    uint *aiAssignedPar  = new uint[omp_get_max_threads()];
    uint *aiDismissedPar = new uint[omp_get_max_threads()];
    memset(aiAssignedPar,  0, omp_get_max_threads()*sizeof(uint));
    memset(aiDismissedPar, 0, omp_get_max_threads()*sizeof(uint));
    // each agent should only belong to one region
    loc_data::const_iterator it;
    loc_varrpos *amvCandidatesPar = new loc_varrpos[omp_get_max_threads()];

#pragma omp parallel for
    for (int i = 0; i < m_iNumAgents; i++) {
        double dLon1 = m_mCoords[m_pCellIDs[i]].first;
        double dLat1 = m_mCoords[m_pCellIDs[i]].second;
        bool bSearching = true;
        
        for (it = mLocData.begin(); (iResult == 0) && bSearching && (it != mLocData.end()); ++it) {
            double dLon0 = it->second.dLon;
            double dLat0 = it->second.dLat;
           
            //            double d = spherdist(dLon0*M_PI/180, dLat0*M_PI/180, dLon1*M_PI/180, dLat1*M_PI/180, RADIUS);
            double d = m_fCalcDist(dLon0, dLat0, dLon1, dLat1, m_dScale);

            if (d < it->second.dDist) {
                amvCandidatesPar[omp_get_thread_num()][it->first].push_back(i);
                bSearching = false;
                aiAssignedPar[omp_get_thread_num()]++;
            } else {
                
            }
        }
        if (bSearching) {
            aiDismissedPar[omp_get_thread_num()]++;
        }
    }
    
    uint iAssignedPar  = 0;
    uint iDismissedPar = 0;
    for (int i = 0; i < omp_get_max_threads(); i++) {
        loc_varrpos::const_iterator it2;
        for (it2 = amvCandidatesPar[i].begin(); it2 != amvCandidatesPar[i].end(); ++it2) {
            mvCandidates[it2->first].insert(mvCandidates[it2->first].end(), it2->second.begin(), it2->second.end());

            iAssignedPar  += aiAssignedPar[i];
            iDismissedPar += aiDismissedPar[i];
        }
    }
    delete[] aiAssignedPar;
    delete[] aiDismissedPar;

    loc_varrpos::iterator it3Par;
    int iTotPar = 0;
    for (it3Par = mvCandidates.begin(); it3Par != mvCandidates.end(); ++it3Par) {
        //            stdprintf("For [%s]: %zd elements«\n", it3->first.c_str(), it3->second.size());
        iTotPar +=  it3Par->second.size();
    }
    //    stdprintf("Par: Assigned: %u; dismissed: %u; unhandled: %u\n", iAssignedPar, iDismissedPar, m_iNumAgents-iAssignedPar-iDismissedPar);
    //    stdprintf("iTotPar: %d\n", iTotPar);
    delete[] amvCandidatesPar;
    return iResult;
}


//----------------------------------------------------------------------------
// getSamplesCore
//
IDSample *IDSampler2::getSamplesCore(loc_data  &mLocData, const std::string sRefLocName) {
    int iResult = 0;
    if (iResult == 0) {
        //double dT0 = omp_get_wtime();

        loc_varrpos mvCandidates;
        getCandidatesNew1(mLocData, mvCandidates);

        //double dT1 = omp_get_wtime();
        // create selection indexes for the indexes in mvCandidates 
        std::map<std::string, intset> msSelectedIndexes;
        loc_varrpos::const_iterator it2;
        for (it2 = mvCandidates.begin(); it2 != mvCandidates.end(); ++it2) {
            intset sSelectedIndexes;
            makeSelectionList(it2->second.size(), mLocData[it2->first].iNum, sSelectedIndexes);
            msSelectedIndexes[it2->first] = sSelectedIndexes;
        }
        //double dT2 = omp_get_wtime();

        // now get details for selected candidates
        std::map<std::string, intset>::const_iterator itm;

        for (itm = msSelectedIndexes.begin(); (iResult == 0) && (itm != msSelectedIndexes.end()); ++itm) {
            const intset &sSelIdx = itm->second;
            const std::vector<int> &vCandidates = mvCandidates[itm->first];

            std::vector<agdata*> vAGD;
            intset_cit its;
            for (its = sSelIdx.begin(); its != sSelIdx.end(); ++its) {
                agdata *pAD = new agdata;
                int iCand = vCandidates[*its];
                pAD->iID = m_pIDs[iCand];
                pAD->iMomID = -1;
                pAD->iDadID = -1;
                pAD->iGender = m_pGenders[iCand];
                pAD->iCellID = m_pCellIDs[iCand];
                pAD->dLon = m_mCoords[pAD->iCellID].first;
                pAD->dLat = m_mCoords[pAD->iCellID].second;
                pAD->iArrayPos = iCand;
                vAGD.push_back(pAD);

            }
            if ((!sRefLocName.empty()) && (itm->first == sRefLocName)) {
                m_pRefSample->addAgentDataVec(itm->first, m_fTimeStamp, vAGD); 
            } else {
                m_pCurSample->addAgentDataVec(itm->first, m_fTimeStamp, vAGD); 
            }
        }
        /*
        double dT3 = omp_get_wtime();
        stdprintf("IDSampler: time to select IDs:      %f\n", dT1 - dT0);
        stdprintf("IDSampler: time to get indexes:     %f\n", dT2 - dT1);
        stdprintf("IDSampler: time to fill attributes: %f\n", dT3 - dT2);
        */
    } else {
        // mlocdata fail
    }
    idset s;
    m_pCurSample->getFullIDSet(s);
    
    if (iResult != 0) {
        delete m_pCurSample;
        m_pCurSample = NULL;
    }

    return m_pCurSample;
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(const std::string sQDFTime, const std::string sPopName, const locspec *pLocSpec, loc_data  &mLocData, const locspec *pRefLocSpec) {
    
    IDSample *pNew = NULL;
    int iResult = 0;
    std::string sRefLocName = "";

    iResult = readArrays(sQDFTime, sPopName);
    if (iResult == 0) {
        m_pCurSample = new IDSample();
        m_pRefSample = new IDSample();
    
        if (iResult == 0) {
            iResult = fillLocData(pLocSpec, mLocData);
            if (iResult == 0) {

                stringvec vNames;
                iResult = fillLocData(pRefLocSpec, mLocData, &vNames);
                if (iResult == 0) {
                    if (vNames.size() > 0) {
                        m_sRefLocName = vNames[0];
                        sRefLocName = m_sRefLocName;
                    } else {
                        stdfprintf(stderr, "No Reflocname returned\n");
                        iResult = -1;
                    }
                }

            }
        }
    }
    if (iResult == 0) {
        pNew = getSamplesCore(mLocData, sRefLocName);
    }
    return pNew;
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(stringvec &vQDFPops, const std::string sPopName, const locspec *pLocSpec, loc_data &mLocData, const locspec *pRefLocSpec) {

    int iResult = 0;

    m_pCurSample = new IDSample();
    m_pRefSample = new IDSample();
    std::string sRefLocName = "";

    iResult = fillLocData(pLocSpec, mLocData);
    if (iResult == 0) {
        if (!pRefLocSpec->sLocFile.empty()) {
            // but we do want the ref location name
            stringvec vNames;
            iResult = fillLocData(pRefLocSpec, mLocData, &vNames);
            if (iResult == 0) {
                if (vNames.size() > 0) {
                    m_sRefLocName = vNames[0];
                    sRefLocName = m_sRefLocName;
                } else {
                    stdfprintf(stderr, "No Reflocname returned\n");
                    iResult = -1;
                }
            }
        }
        if (iResult == 0) {
            IDSample *pNew = m_pCurSample;
            for (uint i = 0; (pNew != NULL) && (i < vQDFPops.size()); ++i) {
                stdprintf("Getting samples from [%s]\n", vQDFPops[i]);fflush(stdout);
                iResult = readArrays(vQDFPops[i], sPopName);
                if (iResult == 0) {
                    pNew = getSamplesCore(mLocData, sRefLocName);
                } else {
                    pNew = NULL;
                }
            }
        }
    }
    
    return m_pCurSample;
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(const std::string sQDFTime, const std::string sPopName, const std::string sLocFile, loc_data &mLocData, const std::string sRefLocFile) {
    locspec ls(sLocFile);
    locspec rs(sRefLocFile);
    return getSamples(sQDFTime, sPopName, &ls, mLocData, &rs);
}


//----------------------------------------------------------------------------
// getSamples
//
IDSample *IDSampler2::getSamples(stringvec &vQDFPops, const std::string sPopName, const std::string sLocFile, loc_data &mLocData, const std::string sRefLocFile) {
    locspec ls(sLocFile);
    locspec rs(sRefLocFile);
    return getSamples(vQDFPops, sPopName, &ls, mLocData, &rs);
}


//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributesCore(loc_data  &mLocData, idset &sSelected) {
    int iResult = 0;

    if (iResult == 0) {
        // transform CellIDs to coords
        // we need the indexes to find gender etc
        for (int i = 0; (i < m_iNumAgents) && (iResult == 0); i++) {
            idset_cit it0 = sSelected.find(m_pIDs[i]);
            if (it0 != sSelected.end()) {
                // the coordinates are in degrees! make them radians
                double dLon0 = m_mCoords[m_pCellIDs[i]].first*M_PI/180;
                double dLat0 = m_mCoords[m_pCellIDs[i]].second*M_PI/180;
                bool bSearching = true;
                loc_data::const_iterator it;
                for (it = mLocData.begin(); (iResult == 0) && bSearching && (it != mLocData.end()); ++it) {
                    //                    double d = spherdist(dLon0, dLat0, it->second.dLon*M_PI/180, it->second.dLat*M_PI/180, RADIUS);
                    double d = m_fCalcDist(dLon0, dLat0, it->second.dLon, it->second.dLat, m_dScale);
                    if (d < it->second.dDist) {

                        agdata *pAD = new agdata;
                        pAD->iID     = m_pIDs[i];
                        pAD->iMomID  = -1;
                        pAD->iDadID  = -1;
                        pAD->iGender = m_pGenders[i];
                        pAD->iCellID = m_pCellIDs[i];
                        pAD->dLon = dLon0;
                        pAD->dLat = dLat0;
                        pAD->iArrayPos = i;
                        m_pCurSample->addAgentData(it->first, m_fTimeStamp, pAD);
                        
                        bSearching = false;

                    }
                }
                if (bSearching) {
                    stdfprintf(stderr, "%selement %ld not in range of any location - maybe different location list was used%s\n", colors::RED, m_pIDs[i], colors::OFF);
                    iResult = -1;
                }
            }
        }
        
        if (iResult == 0) {
            //            idset sSelected;
            m_pCurSample->getFullIDSet(sSelected);
            for (uint i = 0; i < sSelected.size(); i++) {
                //                rand();
                m_pWELL->wrand();
            }
        }
    } else {
        stdfprintf(stderr, "%sCouldn't fill loc data%s\n", colors::RED, colors::OFF);
    }
    
    if (iResult != 0) {
        delete m_pCurSample;
        m_pCurSample = NULL;
    }

    return m_pCurSample;
}


//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(const std::string sQDFTime, const std::string sPopName, const locspec *pLocSpec, 
                                    loc_data  &mLocData, idset &sSelected) {
    
    IDSample *pNew = NULL;
    int iResult = 0;


    m_pCurSample = new IDSample();
    
    iResult = fillLocData(pLocSpec, mLocData);
        
    if (iResult == 0) {
        pNew = getAttributesCore(mLocData, sSelected);
    }
    return pNew;
}


//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(stringvec &vQDFPops, const std::string sPopName, const locspec *pLocSpec, loc_data  &mLocData, idset &sSelected) {
    m_pCurSample = new IDSample();

    IDSample *pNew = m_pCurSample;

    for (uint i = 0; (pNew != NULL) && (i < vQDFPops.size()); ++i) {
        int iResult = readArrays(vQDFPops[i], sPopName);
        if (iResult == 0) {
            pNew = getAttributesCore(mLocData, sSelected);
        } else {
            pNew = NULL;
        }
    }    

    return m_pCurSample;
}

//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(const std::string sQDFTime, const std::string sPopName, const std::string sLocFile, loc_data  &mLocData, idset &sSelected) {
    locspec ls(sLocFile);
    return getAttributes(sQDFTime, sPopName, &ls, mLocData, sSelected);
}


//----------------------------------------------------------------------------
// getAttributes
//   get attributes (mAgentData) from pQDFTime for selected IDs
// 
IDSample *IDSampler2::getAttributes(stringvec &vpQDFPops, const std::string sPopName, const std::string sLocFile, loc_data  &mLocData, idset &sSelected) {
    locspec ls(sLocFile);
    return getAttributes(vpQDFPops, sPopName, &ls, mLocData, sSelected);
}


//----------------------------------------------------------------------------
// getFullIDSet
//
void IDSampler2::getFullIDSet(idset &sSelected) {
    m_pCurSample->getFullIDSet(sSelected);
}


//----------------------------------------------------------------------------
// getFullIndexIDMap
//
void IDSampler2::getFullIndexIDMap(arrpos_ids &mSelected) {
    m_pCurSample->getFullIndexIDMap(mSelected);
}


//----------------------------------------------------------------------------
// getSelectedIDSet
//
void IDSampler2::getLocationIDSet(loc_ids &msSelected) {
    m_pCurSample->getLocationIDSet(msSelected);
}


//----------------------------------------------------------------------------
// getRefIDSet
//
void IDSampler2::getRefIDSet(idset &sSelected) {
    m_pRefSample->getFullIDSet(sSelected);
}


//----------------------------------------------------------------------------
// getRefIndexIDMap
//
void IDSampler2::getRefIndexIDMap(arrpos_ids &mSelected) {
    m_pRefSample->getFullIndexIDMap(mSelected);
}


//----------------------------------------------------------------------------
// getRefSelectedIDSet
//
void IDSampler2::getRefLocationIDSet(loc_ids &msSelected) {
    m_pRefSample->getLocationIDSet(msSelected);
}


//----------------------------------------------------------------------------
// readArrays
//
int IDSampler2::readArrays(const std::string sQDFTime, const std::string sPopName) {
    int iResult = -1;

    deleteArrays();

    m_pIDs        = NULL;
    m_pCellIDs    = NULL;
    m_pGenders    = NULL;

    m_iNumAgents  = 0;
    m_fTimeStamp  = -1;

    QDFArray *pQA = QDFArray::create(sQDFTime);
    if (pQA != NULL) {
        m_fTimeStamp = pQA->getTimeStep();
        iResult = pQA->openArray(POPGROUP_NAME, sPopName, AGENT_DATASET_NAME);
        if (iResult == 0) {
            m_iNumAgents = pQA->getSize();

            m_pIDs      = new idtype[m_iNumAgents];
            m_pCellIDs  = new int[m_iNumAgents];
            m_pGenders  = new int[m_iNumAgents];
            
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pIDs, m_iNumAgents, "AgentID");
                if (iCount != m_iNumAgents) {
                    stdfprintf(stderr, "%sGot %d agent IDs instead of %d%s\n", colors::RED, iCount, m_iNumAgents, colors::OFF);
                    iResult = -1;
                }
            }
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pCellIDs, m_iNumAgents, "CellID");
                if (iCount != m_iNumAgents) {
                    stdfprintf(stderr, "%sGot %d cell IDs instead of %d%s\n", colors::RED, iCount, m_iNumAgents, colors::OFF);
                    iResult = -1;
                }
            }
            if (iResult == 0) {
                int iCount = pQA->getFirstSlab(m_pGenders, m_iNumAgents, "Gender");
                if (iCount != m_iNumAgents) {
                    stdfprintf(stderr, "%sGot %d genders instead of %d%s\n", colors::RED, iCount, m_iNumAgents, colors::OFF);
                    iResult = -1;
                }
            }
                
            if (iResult == 0) {
                stdprintf("  read agent data: %d items\n", m_iNumAgents);
            }
        } else {
            stdfprintf(stderr, "%sCouldn't open dataset [%s/%s%s]%s\n", colors::RED, POPGROUP_NAME, sPopName, AGENT_DATASET_NAME, colors::OFF);
        }
        pQA->closeArray();

        delete pQA;
    } else {
        iResult = -1;
        stdfprintf(stderr, "%sCouldn't open file [%s]%s\n", colors::RED, sQDFTime, colors::OFF);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// fillCoordMap
//
int IDSampler2::fillCoordMap(const std::string sQDFGeoGrid) {
    int iResult = -1;
    uint iNumCells = 0;
    int *pCellIDs = NULL;
    double *pdLon = NULL;
    double *pdLat = NULL;
    
    QDFArray *pQA = QDFArray::create(sQDFGeoGrid);
    if (pQA != NULL) {
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            iNumCells = pQA->getSize();
            pCellIDs = new int[iNumCells];
            uint iCount = pQA->getFirstSlab(pCellIDs, iNumCells, GRID_DS_CELL_ID);
            if (iCount == iNumCells) {
                //               stdprintf("Read %d CellIDs\n", iCount);
                iResult = 0;
            } else {
                stdfprintf(stderr, "%sRead bad number of grid IDs from [%s:%s/%s/%s]: %d (instead of %d)%s\n", colors::RED, sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME,GRID_DS_CELL_ID, iCount, iNumCells, colors::OFF);
                iResult = -1;
            }
            pQA->closeArray();
        } else {
            iResult = -1;
            stdfprintf(stderr, "%sCouldn't open QDF array for [%s:%s/%s]%s\n", colors::RED, sQDFGeoGrid, GRIDGROUP_NAME, CELL_DATASET_NAME, colors::OFF);
        }

        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LONGITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLon = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLon, iNumCells);
                    if (iCount == iNumCells) {
                        stdprintf("Read %d Longitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "%sRead bad number of read longitudes from [%s:%s/%s]: %d instead of %d%s\n", colors::RED, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, iCount, iNumCells, colors::OFF);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "%sNumber of longitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LONGITUDE, colors::OFF);
                }
                pQA->closeArray();
            }
        }

        if (iResult == 0) {
            iResult = pQA->openArray(GEOGROUP_NAME, GEO_DS_LATITUDE);
            if (iResult == 0) {
                uint iNumCellsL = pQA->getSize();
                if (iNumCellsL == iNumCells) {
                    pdLat = new double[iNumCells];
                    uint iCount = pQA->getFirstSlab(pdLat, iNumCells);
                    if (iCount == iNumCells) {
                        stdprintf("Read %d Latitudes\n", iCount);
                        iResult = 0;
                    } else {
                        iResult = -1;
                        stdfprintf(stderr, "%sCouldn't read latitudes from [%s:%s/%s]: %d instead of %d%s\n", colors::RED, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, iNumCellsL,iNumCells, colors::OFF);
                    }
                } else {
                    iResult = -1;
                    stdfprintf(stderr, "%sNumber of latitudes (%d) differs from number of cellIDs (%d) in [%s:%s/%s]%s\n", colors::RED, iNumCellsL, iNumCells, sQDFGeoGrid, GEOGROUP_NAME, GEO_DS_LATITUDE, colors::OFF);
                }

                pQA->closeArray();
            }
        }
    
        delete pQA;
    } else {
        stdfprintf(stderr, "%sCouldn't create QDFArray%s\n", colors::RED, colors::OFF);
    }
     
    if (iResult == 0) {
        // save coordinate values
        for (uint i = 0; i < iNumCells; i++) {
            m_mCoords[pCellIDs[i]] = std::pair<double, double>(pdLon[i], pdLat[i]);
        }
    }

    if (pCellIDs != NULL) {
        delete[] pCellIDs;
    }
    if (pdLon != NULL) {
        delete[] pdLon;
    }
    if (pdLat != NULL) {
        delete[] pdLat;
    }

    return iResult;
}


