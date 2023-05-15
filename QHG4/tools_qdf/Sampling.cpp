#include <cstdio>
#include "Sampling.h"

const intvec empty; 

//----------------------------------------------------------------------------
//  constructor
//
Sampling::Sampling() : m_bVerbose(false) {
}

//----------------------------------------------------------------------------
//  destructor
//
Sampling::~Sampling() {
}


//----------------------------------------------------------------------------
//  getRefIndex
//    find the reference cell to which iCellID belongs 
//    or -1 if it doesn't belong to any reference cell
//
int Sampling::getRefIndex(int iCellID) {
    int iRef = -1;

    reflist::const_iterator it = m_mRefs.find(iCellID);
    if (it != m_mRefs.end()) {
        iRef = it->second;
    }
    return iRef;
}


//----------------------------------------------------------------------------
//  getGroup
//    return the group of reference cell iRefCellID
//    or an empty vector if RefCellID is not the id of a reference cell
//
const intvec &Sampling::getGroup(int iRefCellID) {

    grouplist::const_iterator it = m_mGroups.find(iRefCellID);
    if (it != m_mGroups.end()) {
        return it->second;
    } else {
        return empty;
    }
}


//----------------------------------------------------------------------------
//  makeRefs
//    build the map 
//      cell id -> refcell id
//    from m_mGroups (refcell -> set of all id of cells belonging to refcell)
//
void Sampling::makeRefs() {
    printf("[Sampling::makeRefs] got %zd groups\n", m_mGroups.size());
    grouplist::const_iterator it;
    for (it = m_mGroups.begin(); it != m_mGroups.end(); ++it) {
        for (unsigned int i = 0; i < it->second.size(); ++i) {
            m_mRefs[it->second[i]] = it->first;
        }
    }
    
    printf("[Sampling::makeRefs] got %zd refs\n", m_mRefs.size());
    if (m_bVerbose) {
        reflist::const_iterator itr;
        for (itr = m_mRefs.begin(); itr != m_mRefs.end(); ++itr) {
            printf("ref %d: %d\n", itr->first, itr->second);
        }
    }
}

//----------------------------------------------------------------------------
//  showSamples
//
void Sampling::showSamples() {
    grouplist::const_iterator it;
    for (it = m_mGroups.begin(); it != m_mGroups.end(); ++it) {
        printf("%03d: ", it->first);
        for (unsigned int i = 0; i < it->second.size(); ++i) {
            printf(" %d", it->second[i]);
	}
	printf("\n");
    }
}



/*
template<typename T>
int Sampling::groupValues(indexedvals<T> &vIndexedVals, groupedvals<T> &mGroupedVals) {

    int iResult = 0;

    typename indexedvals<T>::const_iterator it;
    for (it = vIndexedVals.begin(); it != vIndexedVals.end(); ++it) {
        int r = getRefIndex(it->first);
        if (r >= 0) {
            mGroupedVals[r].push_back(it->second);
	}
    }
    return iResult;
}
*/
