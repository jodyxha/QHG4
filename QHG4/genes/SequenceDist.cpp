#include <cstring>

#include "xha_strutilsT.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArrayT.h"
//#include "QDFArray.cpp"

#include "LineReader.h"

#include "SequenceDist.h"


//----------------------------------------------------------------------------
// constructor
//   
template<typename T>
SequenceDist<T>::SequenceDist(int iNumSequences,
                              int iSequenceSize,
                              id_node      &mIDNodes, 
                              id_sequences &mIDSeq,
                              tnamed_ids   &mvIDs,
                              id_locs      &mIdLocs, 
                              SequenceDist<T>::calcdist_t fCalcDist) 
    : m_iSequenceSize(iSequenceSize),
      m_iNumSequences(iNumSequences),
      m_mIDNodes(mIDNodes),
      m_mvIDs(mvIDs),
      m_mIDLocs(mIdLocs),
      m_mIDSeq(mIDSeq),
      m_fcalcdist(fCalcDist) {

}

//----------------------------------------------------------------------------
// destructor
//   
template<typename T>
SequenceDist<T>::~SequenceDist() {

}


//----------------------------------------------------------------------------
// prepareNodes
//   
template<typename T>
int SequenceDist<T>::prepareNodes(const std::string sGridQDF, const std::string sMoveStatQDF, const std::string sSpeciesName) {
    int iResult = 0;
    std::vector<int> vNodeIDs;
    
    id_node::const_iterator it1;
    for (it1 = m_mIDNodes.begin(); it1 != m_mIDNodes.end(); it1++) {
        vNodeIDs.push_back(it1->second);
    }
                        
                        
    m_mNodeIndexes.clear();
    iResult = nodeIdsToIndexes(sGridQDF, vNodeIDs, m_mNodeIndexes);
    if (iResult == 0) {

        m_mNodeDistances.clear();
        iResult = nodeIdsToDistances(sMoveStatQDF, sSpeciesName, m_mNodeIndexes, m_mNodeDistances);
        if (iResult == 0) {

            m_mNodeTimes.clear();
            iResult = nodeIdsToTravelTimes(sMoveStatQDF, sSpeciesName, m_mNodeIndexes, m_mNodeTimes);
            if (iResult == 0) {
                
                
                m_mTravelDists.clear();
                m_mTravelTimes.clear();
                tnamed_ids::const_iterator it;
                for (it = m_mvIDs.begin(); it != m_mvIDs.end(); ++it) {
                    for (uint k = 0; k < it->second.size(); k++) {
                        m_mTravelDists[it->second[k]] = m_mNodeDistances[m_mNodeIndexes[m_mIDNodes[it->second[k]]]];
                        m_mTravelTimes[it->second[k]] = m_mNodeTimes[m_mNodeIndexes[m_mIDNodes[it->second[k]]]];
                    }
                }
                
                
            } else {
                xha_fprintf(stderr, "couldn't extract travel times from stat file\n");
            }

        } else {
            xha_fprintf(stderr, "couldn't extract distances from stat file\n");
        }
        
    } else {
        xha_fprintf(stderr, "reading grid qdf [%s] failed\n", sGridQDF);
    }
    return iResult;
}



//----------------------------------------------------------------------------
// nodeIdsToIndexes
//   for each node ID, find its index in the CellGrid.
//   currently, the index is equal to the ID, but in later versions 
//   with OpenMPI and spatial splitting this will not be the case anymore
//
template<typename T>
int SequenceDist<T>::nodeIdsToIndexes(const std::string sGridQDF, std::vector<int> &vNodeIDs, node_index &mNodeIndexes) {
    int iResult = -1;
    //@@    xha_printf("Reading indexes from [%s]\n", sGridQDF);
    QDFArray *pQA = QDFArray::create(sGridQDF);
    if (pQA != NULL) {
        
        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {
            
            intset sTemp;
            sTemp.insert(vNodeIDs.begin(), vNodeIDs.end());
            std::vector<int> vSorted(sTemp.size());
            std::copy(sTemp.begin(), sTemp.end(), vSorted.begin());
            int offset = 0;
            int *pBuf = new int[IDBUFSIZE];
            int iCount = pQA->getFirstSlab(pBuf, IDBUFSIZE, GRID_DS_CELL_ID);
            
            uint iCurIdx = 0;
            while ((iCurIdx < vSorted.size()) &&  (iCount > 0)) {
                int k = 0;
                while ((iCurIdx < vSorted.size()) && (k < iCount) && (pBuf[iCount-1] >= vSorted[iCurIdx])) {
                    if (pBuf[k] == vSorted[iCurIdx]) {
                        mNodeIndexes[vSorted[iCurIdx]] = offset+k;
                        iCurIdx++;
                    }
                    k++;
                }
                offset += iCount;
                iCount = pQA->getNextSlab(pBuf, IDBUFSIZE);
            }
            delete[] pBuf;

            pQA->closeArray();
        } else {
            xha_fprintf(stderr, "Couldn't open Dataset [%s/%s]\n", GRIDGROUP_NAME, CELL_DATASET_NAME);
        }
            
        delete pQA;
    } else {
        xha_fprintf(stderr, "Couldn't open QDF file [%s]\n", sGridQDF);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// nodeIdsToStat
// 
template<typename T>
int SequenceDist<T>::nodeIdsToStat(const std::string sStatQDF, const std::string sSpeciesName, node_index &mNodeIndexes, node_value &mNodeStats, const std::string sStatName) {
    int iResult = -1;
    
    //@@    xha_printf("Reading distances from [%s]\n", pStatQDF);

    QDFArray *pQA = QDFArray::create(sStatQDF);
    if (pQA != NULL) {
        iResult = pQA->openArray(POPGROUP_NAME, sSpeciesName, sStatName);
        if (iResult == 0) {
            // that's ok, then
        } else {
            xha_printf("Couldn't open array [%s] under [%s/%s]\n", sStatName, POPGROUP_NAME, sSpeciesName);
            xha_printf("Trying old-style\n");
            iResult = pQA->openArray(MSTATGROUP_NAME, sStatName);
            if (iResult == 0) {
                // that's ok, then
            } else {
                xha_printf("Couldn't open array [%s] under [%s]\n", sStatName, MSTATGROUP_NAME);
            }    
        }    
    } else {
        xha_printf("Couldn't open qdf file [%s]\n", sStatQDF);
    }

    if (iResult == 0) {

        // we must find the distance values for the nodes
        // at the specified indexes

        std::vector<int> vIndexes;
        node_index::const_iterator it;
        for (it = mNodeIndexes.begin(); it != mNodeIndexes.end(); ++it) {
            vIndexes.push_back(it->second);
        }
        std::sort(vIndexes.begin(), vIndexes.end());
        node_value mIndexStats;
        
        int offset = 0;
        double *pBuf = new double[IDBUFSIZE];
        
        // now loop through slabs of the distance values
        // to find those situated at our indexes
        int iCount = pQA->getFirstSlab(pBuf, IDBUFSIZE);
        
        uint iCurIdx = 0;
        
        while ((iCurIdx < vIndexes.size()) &&  (iCount > 0)) {
            int k = 0;
            while ((iCurIdx < vIndexes.size()) && (k < iCount) && ( offset+iCount >= vIndexes[iCurIdx])) {
                if (k+offset == vIndexes[iCurIdx]) {
                    mIndexStats[vIndexes[iCurIdx]] = pBuf[k];
                    iCurIdx++;
                }
                k++;
            }
            offset += iCount;
            iCount = pQA->getNextSlab(pBuf, IDBUFSIZE);
        }
        
        delete[] pBuf;
        //@@            xha_printf("found %zd indexdists\n", mIndexDistances.size());
        
        // now we can assign distances to node IDs 
        for (it = mNodeIndexes.begin(); it != mNodeIndexes.end(); ++it) {
            mNodeStats[it->first] = mIndexStats[it->second];
        }
        //@@            xha_printf("got %zd distances\n", mIndexDistances.size());
            
    }
    return iResult;
}


//----------------------------------------------------------------------------
// nodeIdsToDistances
// 
template<typename T>
    int SequenceDist<T>::nodeIdsToDistances(const std::string sStatQDF, const std::string sSpeciesName, node_index &mNodeIndexes, node_value &mNodeDistances) {
    
    int iResult =  nodeIdsToStat(sStatQDF, sSpeciesName, mNodeIndexes, mNodeDistances, MSTAT_DS_DIST); 
    return iResult;
}


//----------------------------------------------------------------------------
// nodeIdsToTravelTimes
// 
template<typename T>
int SequenceDist<T>::nodeIdsToTravelTimes(const std::string sStatQDF, const std::string sSpeciesName, node_index &mNodeIndexes, node_value &mNodeTimes) {
    
    int iResult =  nodeIdsToStat(sStatQDF, sSpeciesName, mNodeIndexes, mNodeTimes, MSTAT_DS_TIME); 
    return iResult;
}

    
//----------------------------------------------------------------------------
// reorderSequences
// make sure the sequences are in the order given by mvIDs
//
template<typename T>
T **SequenceDist<T>::reorderSequences(id_sequences &mIDSeq, tnamed_ids &mvIDs) {
    T **pAllSequences = new T*[mIDSeq.size()];
    uint iC = 0;
    tnamed_ids::const_iterator it;
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        for (uint k = 0; k < it->second.size(); k++) {
            if (iC < mIDSeq.size()) {
                pAllSequences[iC++] = mIDSeq[it->second[k]];
            }
        }
    }
    return pAllSequences;
}


//----------------------------------------------------------------------------
// distGeoSequences2
//   creates a file containing the following fields per individual:
//     ID
//     Longitude (or X)
//     Latitude  (or Y)
//     sampling time
//     travelled distance
//     travel time
//     location name
//     location id
//     region id (e.g. continents) 
//
//   the file name has the suffix '.tagdists'
//  
template<typename T>
int SequenceDist<T>::writeMetaData(const std::string sOutput) {
    int iResult = 0;
     
    // average the distance of all "original" indivduals to the other individuals
    int iC = 0;

    std::string sName = xha_sprintf(TEMPLATE_TABLE, sOutput);
    FILE *fOut0 = fopen(sName.c_str(), "wt");
    xha_fprintf(fOut0, "#ID Longitude Latitude Time_Step Travel_Dist Travel_Time Region_Name Location_ID Region_ID\n");

    tnamed_ids::const_iterator it;
    for (it = m_mvIDs.begin(); it != m_mvIDs.end(); ++it) {
        int i1 = it->first.first.find('[');
        std::string sLocName = it->first.first.substr(0, i1);
        int i2 = it->first.first.find(']');
        std::string sLocDefs = it->first.first.substr(i1+1, i2-i1-1);
        int iLocID = -1;
        int iRegID = -1;
        int i11 = sLocDefs.find(',');
        if (i11 >= 0) {
            std::string sLocID = sLocDefs.substr(0, i11);
            std::string sRegID = sLocDefs.substr(i11+1);
            if (strToNum(sLocID, &iLocID)) {
                if (strToNum(sRegID, &iRegID)) {
                    
                }
            }
        }


        for (uint k = 0; k < it->second.size(); k++) {
            idtype iID = it->second[k];
            //  ID Lon Lat step traveldist traveltime regionname  locationID RegionID
            xha_fprintf(fOut0, "%ld %f %f %8.1f %f %f %s %d %d \n", iID, m_mIDLocs[iID].first, m_mIDLocs[iID].second, it->first.second, m_mTravelDists[it->second[k]],  m_mTravelTimes[it->second[k]], sLocName, iLocID, iRegID);
            
            iC++;
        }

    }
    fclose(fOut0);
    xha_printf("Written data table (%dx%d)\n  [%s]\n", iC, 9, sName);
    return iResult;
}


//----------------------------------------------------------------------------
// writeFullMatrix
//   
template<typename T>
int SequenceDist<T>::writeFullMatrix(const std::string sOutput, float **pM, int iNumSequences1, int iNumSequences2) {
    int iResult = -1;
    
    FILE *fOut = NULL;

    fOut = fopen(sOutput.c_str(), "wt");

    if (fOut != NULL) {
        iResult = 0;
        
       
        // write distances with prepended location and agent ID
        for (int i = 0; i < iNumSequences1; i++) {
            for (int j = 0; j < iNumSequences2; j++) {
                xha_fprintf(fOut, "%f\t", pM[i][j]);
            }
            xha_fprintf(fOut, "\n");
        }


        fclose(fOut);
        xha_printf("Written distance matrix (%dx%d)\n  [%s]\n", iNumSequences1, iNumSequences2, sOutput);
    } else {
        xha_fprintf(stderr, "Couldn't open output file [%s]\n", sOutput);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createAndWriteDistMat
//
template<typename T>   
int SequenceDist<T>::createAndWriteDistMat(const std::string sOutput) {
    T **pSequences = NULL;
    int iResult = -1;
    pSequences = reorderSequences(m_mIDSeq, m_mvIDs);
    DistMat<T> *pDM = DistMat<T>::createDistMat(m_iSequenceSize, pSequences, m_mIDSeq.size(), m_fcalcdist);
    if (pDM != NULL) {
        float **pM = pDM->createMatrix();
        std::string sName1 = xha_sprintf(TEMPLATE_DIST_MAT, sOutput);
        iResult = writeFullMatrix(sName1, pM, m_mIDSeq.size(), m_mIDSeq.size());
        delete pDM;
    }
    
    delete[] pSequences;

    return iResult;
}


//----------------------------------------------------------------------------
// createAndWriteDistMatRef
//   
template<typename T>   
int SequenceDist<T>::createAndWriteDistMatRef(id_sequences &mIDSeqRef, tnamed_ids &mvIDsRef, 
                              const std::string sOutput) {
    float **pSequences1 = NULL;
    float **pSequences2 = NULL;
    int iResult = -1;
    pSequences1 = reorderSequences(m_mIDSeq,    m_mvIDs);
    pSequences2 = reorderSequences(mIDSeqRef,   mvIDsRef);

    //@@ to be moved to derived: DistMat<float>::calcdist_t fcalcdist = calcEuclideanDist;

    DistMat<float> *pDM = DistMat<float>::createDistMatRef(m_iSequenceSize, pSequences1, m_mIDSeq.size(), pSequences2, mIDSeqRef.size(), m_fcalcdist);
    if (pDM != NULL) {
        float **pM = pDM->createMatrix();
        
        std::string sName1 = xha_sprintf(TEMPLATE_REF_MAT, sOutput);
        iResult = writeFullMatrix(sName1, pM, m_mIDSeq.size(), mIDSeqRef.size());

        delete pDM;
    }
    delete[] pSequences1;
    delete[] pSequences2;

    return iResult;
}

//----------------------------------------------------------------------------
// extractAndWriteGeoSequenceDists
// 
template<typename T>  
int SequenceDist<T>::extractAndWriteGeoSequenceDists(const std::string sOutput) {
    int iResult = -1;

    std::string sTable  = xha_sprintf(TEMPLATE_TABLE,   sOutput);
    std::string sRefMat = xha_sprintf(TEMPLATE_REF_MAT, sOutput);
    std::string sGGOut  = xha_sprintf(TEMPLATE_GEO_SEQ, sOutput);

    LineReader *pLRTable = LineReader_std::createInstance(sTable, "rt");
    if (pLRTable != NULL) {
        LineReader *pLRRefMat = LineReader_std::createInstance(sRefMat, "rt");
        if (pLRRefMat != NULL) {
            FILE * fOut = fopen(sGGOut.c_str(), "wt");
            if (fOut != NULL) {
                iResult = 0;
                int iC = 0;
                int iW = 0;
                while ((iResult == 0) && (!pLRTable->isEoF()) && (!pLRRefMat->isEoF())) {
                    char *pLineTable = pLRTable->getNextLine();
                    char *pLineRefMat = pLRRefMat->getNextLine();
                    if ((pLineTable != NULL) && (pLineRefMat != NULL)) {
                        std::vector<std::string> vFields;
                        char *pWord = strtok(pLineTable, " \t\n");
                        while (pWord != NULL) {
                            vFields.push_back(pWord);
                            pWord = strtok(NULL, " \t\n");
                        }
                    
                        if (iW == 0) {
                            char *p = strtok(pLineRefMat, ",; \t\n");
                            while (p != NULL) {
                                iW++;
                                p = strtok(NULL, ",; \t\n");
                            }
                        }
                        if (vFields.size() > 4) {
                            xha_fprintf(fOut, "%s %s\n", vFields[4], pLineRefMat);
                            iC++;
                        } else {
                            xha_fprintf(stderr, "Not enough fields in [%s]\n", sTable);
                            iResult = -1;
                        }
                    } else {
                        if ((pLineTable != NULL) != (pLineRefMat != NULL)) {
                            xha_fprintf(stderr, "[%s] and [%s] have different number of lines\n", sTable, sRefMat);
                            iResult = -1;
                        }
                    }
                }
                fclose(fOut);
                xha_printf("Written geo-genome distance matrix (%dx%d)\n  [%s]\n", iC, iW+1, sGGOut);
            } else {
                xha_fprintf(stderr, "Couldn't open [%s] for writing\n", sGGOut);
            }
            delete pLRRefMat;
        } else {
            xha_fprintf(stderr, "Couldn't open [%s] for reading\n", sRefMat);
        }

        delete pLRTable;
    } else {
        xha_fprintf(stderr, "Couldn't open [%s] for reading\n", sTable);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// selectFromClosest
//   
template<typename T>
int SequenceDist<T>::selectFromClosest(node_value   &mNodeDistances, 
                                       node_index   &mNodeIndexes, 
                                       id_sequences &mIDSeqRef,
                                       tnamed_ids &mvIDsRef) {
    // find the population closest to the origin of the expansion
    double dMinDistance=1e99;
    std::string sMin;
    double dTime = -1;
    idtype iMinID = -1;
    tnamed_ids::const_iterator it;
    for (it = m_mvIDs.begin(); it != m_mvIDs.end(); ++it) {
        for (uint k = 0; k < it->second.size(); k++) {
            idtype iID = it->second[k];
            double dDist = m_mNodeDistances[m_mNodeIndexes[m_mIDNodes[iID]]];
            if (dDist < dMinDistance) {
                dMinDistance = dDist;
                sMin = it->first.first;
                dTime = it->first.second;

                iMinID = iID;
            }
            
        }
    }
    
    loc_time key(sMin, dTime);
    
    mIDSeqRef[iMinID] = m_mIDSeq[iMinID];
    mvIDsRef[key].push_back(iMinID);
    

    int iNumSequencesRef = mvIDsRef[key].size();
    return iNumSequencesRef;
}


