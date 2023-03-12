#ifndef __SAMPLING_H__
#define __SAMPLING_H__

#include <vector>
#include <map>

typedef std::vector<int>      intvec;
typedef std::map<int, intvec> grouplist;
typedef std::map<int, int>    reflist;

template<typename T>
using indexedvals = std::vector<std::pair<int, T>>;

template<typename T>
using groupedvals = std::map<int, std::vector<T>>;

class Sampling {
public:
    Sampling();
    virtual ~Sampling();

    int getRefIndex(int iCellID);
    const intvec& getGroup(int iRefCellID);

    template<typename T>
    int groupValues(indexedvals<T> &vIndexedVals, groupedvals<T> &mGroupedVals);
    
    void showSamples();
    int setVerbosity(bool bVerbose) {bool bOldV = m_bVerbose; m_bVerbose = bVerbose; return bOldV;};
protected:
    void makeRefs();
    grouplist m_mGroups;
    reflist   m_mRefs;

    bool m_bVerbose;
}; 


#endif
