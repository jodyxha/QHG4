#include <algorithm>
#include "Sampling.h"



//----------------------------------------------------------------------------
//  groupValues
//    fill the map mGroupedVals: 
//      for reference cell id r, create the set of all values from  cells close to r
//      each of these sets will be the data for a histogram
//
template<typename T>
int Sampling::groupValues(indexedvals<T> &vIndexedVals, groupedvals<T> &mGroupedVals) {

    int iResult = 0;

    /*
    printf("[Sampling::groupValues] got %zd refs\n", m_mRefs.size());
    if (m_bVerbose) {
        reflist::const_iterator itr;
        for (itr = m_mRefs.begin(); itr != m_mRefs.end(); ++itr) {
            printf("ref %d: %d\n", itr->first, itr->second);
        }
    }
    */
    // assign values to the appropriate vector
    typename indexedvals<T>::const_iterator it;
    for (it = vIndexedVals.begin(); it != vIndexedVals.end(); ++it) {
        int r = getRefIndex(it->m_iRefCell);
        if (r >= 0) {
                mGroupedVals[r].push_back(it->m_tValue);
        }
    }
    
    // make sure regions without any inhabited cell get added too (-> "empty" pies)
    grouplist::const_iterator itg;
    for (itg = m_mGroups.begin(); itg != m_mGroups.end(); ++itg) {
        typename  groupedvals<T>::const_iterator itf = mGroupedVals.find(itg->first);
        if (itf == mGroupedVals.end()) {
            // mGroupedVals[itg->first].push_back(0);  //works
            mGroupedVals[itg->first].clear();  // causes a crash somewhere
        }

    }
   
    // sort the keys
    typename  groupedvals<T>::const_iterator it3; 
    for (it3 = mGroupedVals.begin(); it3 != mGroupedVals.end(); ++it3) {
        std::sort(mGroupedVals[it3->first].begin(), mGroupedVals[it3->first].end()); 
    }
    return iResult;
}

