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
//
void Sampling::makeRefs() {
    grouplist::const_iterator it;
    for (it = m_mGroups.begin(); it != m_mGroups.end(); ++it) {
        for (unsigned int i = 0; i < it->second.size(); ++i) {
	    m_mRefs[it->second[i]] = it->first;

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
