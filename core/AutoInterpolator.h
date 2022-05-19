#ifndef __AUTOINTERPOLATOR_H__
#define __AUTOINTERPOLATOR_H__

#include <map>
#include <string>
#include <set>
#include <vector>
#include <hdf5.h>

class SCellGrid;

typedef struct group_file {
    std::set<std::string> vGroups;
    std::string           sFile;
} group_file;


typedef std::pair<uint, double*>            length_array;
typedef std::map<std::string, length_array> named_arrays;
typedef std::map<int, group_file>           timed_files;

typedef std::pair<std::string,std::string>  string_pair;
typedef std::pair<std::string, string_pair> string_combi;
typedef std::vector<string_combi>           target_list;
typedef std::map<std::string, hid_t>        hid_list;


typedef std::map<std::string, uint>         array_sizes;

typedef std::vector<int>                    event_list;


class AutoInterpolator {
public:
    static AutoInterpolator *createInstance(SCellGrid *pCG,const std::string sEnvEventFile, double fStartTime, target_list &vTargets);

    virtual ~AutoInterpolator();
    void displayFiles();
    void displayArrays();

    int interpolate(int iSteps);

    int startInterpolations(int iFirstStep);
    int calcNextDiff();
    int getCurStep() {return m_iCurStep;};
    const event_list &getEvents() {return m_vEvents;};
    int checkNewInterpolation(int iCurStep);

    // only used to test
    int loadtest();

protected:
    AutoInterpolator(SCellGrid *pCG, target_list &vTargets);
    int init(const std::string sEnvEventFile, double dStartTime);

    int readFileList(const std::string sEnvEventFile, double dStartTime);
    int checkFiles();
    int prepareTargets(); 
    int checkArraySizes(hid_t hFile);
    void allocateArrays();

    int loadArray(hid_t hFile, int iWhich, string_combi &sTarget);
    int loadArrayForTime(int iTime, int iWhich, string_combi &sTarget);
    int loadArrayFromGrid(string_combi &sTarget);

    int calcDiffs(int iTime1, int iTime2);

    SCellGrid   *m_pCG;
    timed_files  m_mFileList;
    named_arrays m_mInput[2];
    double       m_adTimes[2];
    target_list  m_vTargets;
    event_list   m_vEvents;

    named_arrays m_mDiff;
    named_arrays m_mTargets;
    array_sizes  m_mArrSizes;

    int          m_iCur;
    int          m_iCurStep;
    timed_files::const_iterator m_itCur;
};

#endif
