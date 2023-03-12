#include <algorithm>
#include "Sampling.h"



//----------------------------------------------------------------------------
//  groupValues
//
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
    
    typename  groupedvals<T>::const_iterator it3; 
    for (it3 = mGroupedVals.begin(); it3 != mGroupedVals.end(); ++it3) {
        std::sort(mGroupedVals[it3->first].begin(), mGroupedVals[it3->first].end()); 
    }
    return iResult;
}

