#ifndef __DEP_SCANNER_H__
#define __DEP_SCANNER_H__

#include <map>
#include <vector>
#include <string>

typedef std::pair<std::string,std::string> stringpair;
typedef std::vector<std::string> stringvec;
typedef std::map<std::string, stringvec> mapsuffilevec;
typedef std::map<stringpair, mapsuffilevec> mfulldeps;
typedef std::map<stringpair, std::string> mpatlist;

class DepScanner {

public:
    static DepScanner *createInstance(const std::string sCalleeDir, const std::string sCalleeSuffs,
                                      const std::string sCallerDir, const std::string sCallerSuffs);
    int find_all_deps();
    ~DepScanner();

    void show_deps();

    stringvec m_vAllSuffices = {"cpp", "h", "py", "sh"};

protected:

    DepScanner();
    int collect_files();
    int prepare_sources();
    int init(const std::string sCalleeDir, const std::string sCalleeSuffs,
             const std::string sCallerDir, const std::string sCallerSuffs);
    int find_deps(const std::string sCalleeSuff, const std::string sCallerSuff, mapsuffilevec &mCalls);

    std::string m_sCallerDir;
    std::string m_sCalleeDir;

    stringvec m_vCallerSuffs;
    stringvec m_vCalleeSuffs;


    // (callee, caller)
    mpatlist mInclusionPattern  = {
        {{"cpp", "cpp"}, "#include *(<|\\\")%s.cpp(>|\\\").*"},
        {{"cpp", "h"},   "#include *(<|\\\")%s.cpp(>|\\\").*"},
        {{"cpp", "py"},  ".*%s .*"},
        {{"cpp", "sh"},  ".*%s .*"},

        {{"h", "cpp"}, "#include *(<|\\\")%s.h(>|\\\").*"},
        {{"h", "h"},   "#include *(<|\\\")%s.h(>|\\\").*"},
        {{"h", "py"},  ".*%s .*"},
        {{"h", "sh"},  ".*%s .*"},

        {{"py", "cpp"}, ".*%s.py .*"},
        {{"py", "h"},   ".*%s.py .*"},
        {{"py", "py"},  "(from [A-Za-z0-9:]+ )?import ([A-Za-z0-9_]+,? ?)*%s,?.*"},
        {{"py", "sh"},  ".*%s.py .*"},

        {{"sh", "cpp"}, ".*%s.sh .*"},
        {{"sh", "h"},   ".*%s.sh .*"},
        {{"sh", "py"},  ".*%s.sh .*"},
        {{"sh", "sh"},  ".*%s.sh .*"},
    };
    

    mapsuffilevec m_mCallerFiles;
    mapsuffilevec m_mCalleeFiles;
    mapsuffilevec m_mCallerBodies;
    mapsuffilevec m_mCalleeBodies;

    mfulldeps     m_mAllDeps;
};


#endif
