#ifndef __SAMPLING_H__
#define __SAMPLING_H__

#include <vector>
#include <map>

typedef std::vector<int>      intvec;
typedef std::map<int, intvec> grouplist; // cells belonging to a refcell
typedef std::map<int, int>    reflist;   // refcell for each cell

template<typename T>
struct refval {
    refval():m_iRefCell(-1){};
    refval(int iRefCell, T tValue):m_iRefCell(iRefCell),m_tValue(tValue){};
    int m_iRefCell;
    T   m_tValue;
};



template<typename T>
using indexedvals = std::vector<refval<T>>;  // (refcell, value)

template<typename T>
using groupedvals = std::map<int, std::vector<T>>;   // refcell->values

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
    virtual int setRangeDescription(void *pDescr) { return 0;}
protected:
    void makeRefs();
    grouplist m_mGroups;
    reflist   m_mRefs;

    bool m_bVerbose;
}; 


#endif
