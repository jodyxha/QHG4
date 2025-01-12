#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <regex>

#include <algorithm>

#include "xha_strutilsT.h"
#include "DepScanner.h"


//-----------------------------------------------------------------------------
// split_list
//   split list along separator, turn into stringvec
//
int split_list(const std::string sList, stringvec &vSuffs) {
    int iResult = 0;
    vSuffs.clear();

    if (!sList.empty()) {
        stringvec vParts;
        uint iNum = splitString(sList, vParts, "+ ");

        for (uint i = 0; (iResult == 0) && (i < iNum); ++i)  {
            if ((vParts[i] == "cpp") ||
                (vParts[i] == "h") ||
                (vParts[i] == "py") ||
                (vParts[i] == "sh")) {
                vSuffs.push_back(vParts[i]);
            } else {
                xha_printf("invalid suffix: [%s]\n", vParts[i]);
                iResult = -1;
            }

        }

        std::sort(vSuffs.begin(), vSuffs.end());
    } else {
        iResult = -1;
    }
    return iResult; 
}


//----------------------------------------------------------------------------
// createInstance
//
DepScanner *DepScanner::createInstance(const std::string sCalleeDir, const std::string sCalleeSuffs,
                                       const std::string sCallerDir, const std::string sCallerSuffs) {
    DepScanner *pDS = new DepScanner();
    int iResult = pDS->init(sCalleeDir, sCalleeSuffs, sCallerDir, sCallerSuffs);
    if (iResult != 0) {
        delete pDS;
        pDS = NULL;
    }
    return pDS;
}


//----------------------------------------------------------------------------
// find_all_deps
//
int DepScanner::find_all_deps() {
    int iResult = 0;
    m_mAllDeps.clear();
    for (uint iCalleeSuff = 0; (iResult == 0) && (iCalleeSuff < m_vCalleeSuffs.size()); iCalleeSuff++) {
        std::string &sSuffCallee = m_vCalleeSuffs[iCalleeSuff];

        for (uint iCallerSuff = 0; (iResult == 0) && (iCallerSuff < m_vCallerSuffs.size()); iCallerSuff++) {
            std::string &sSuffCaller = m_vCallerSuffs[iCallerSuff];

            mapsuffilevec mCalls;
            xha_printf("++++ callee suff %s, caller suff %s\n", sSuffCallee, sSuffCaller);
            find_deps(sSuffCallee, sSuffCaller, mCalls);
            if (mCalls.size() > 0) {
                m_mAllDeps[stringpair(sSuffCallee, sSuffCaller)] = mCalls;
            }
        }
    }
    return iResult;
}



//----------------------------------------------------------------------------
// ~DepScanner
//
DepScanner::~DepScanner() {
}


//----------------------------------------------------------------------------
// show_deps
//
void DepScanner::show_deps() {

    mfulldeps::const_iterator it;
    for (it = m_mAllDeps.begin(); it != m_mAllDeps.end(); ++it) {
        xha_printf("%s used by %s:\n", it->first.first, it->first.second);
        mapsuffilevec::const_iterator it2;
        for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            xha_printf("  %s:\n", it2->first);
            for (uint i = 0; i < it2->second.size(); i++)  {
                xha_printf("    %s\n", it2->second[i]);
            }
        }
    }


}


//----------------------------------------------------------------------------
// 
//
DepScanner::DepScanner() {

}


//----------------------------------------------------------------------------
// init
//
int DepScanner::init(const std::string sCalleeDir, const std::string sCalleeSuffs,
                     const std::string sCallerDir, const std::string sCallerSuffs) {
    int iResult = 0;

    m_sCallerDir = sCallerDir;
    m_sCalleeDir = sCalleeDir;
    xha_printf("dirs caller [%s], callee [%s]\n", m_sCallerDir, m_sCalleeDir);
    if (iResult == 0) {
        iResult = split_list(sCalleeSuffs, m_vCalleeSuffs);
        if (iResult != 0) {
            m_vCalleeSuffs.assign(m_vAllSuffices.begin(), m_vAllSuffices.end());
            iResult = 0;
        }
    }

    if (iResult == 0) {
        iResult = split_list(sCallerSuffs, m_vCallerSuffs);
        if (iResult != 0) {
            m_vCallerSuffs.assign(m_vAllSuffices.begin(), m_vAllSuffices.end());
            iResult = 0;
        }
    }

    
    if (iResult == 0) {
        iResult = collect_files();
        if (iResult == 0) {
            iResult = prepare_sources();
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// collect_files
//
int DepScanner::collect_files() {
    int iResult = 0;
    // do the globbing
    glob_t glob_data;

    m_mCallerFiles.clear();
    uint iCount = 0;
    for (uint i = 0; (iResult == 0) && (i < m_vCallerSuffs.size()); i++) {
        const std::string sCallerSuff = m_vCallerSuffs[i];
        std::string sPattern = xha_sprintf("%s/*.%s", m_sCallerDir, sCallerSuff);
        xha_printf("for caller dir [%s] andsudd [%s]: pat [%s]\n", m_sCallerDir, sCallerSuff, sPattern);
        iResult = glob(sPattern.c_str(), 0, NULL, &glob_data);
        if ((iResult == 0) || (iResult = GLOB_NOMATCH)) {
            iResult = 0;
            for (uint j = 0; j < glob_data.gl_pathc; j++) {
                m_mCallerFiles[sCallerSuff].push_back(glob_data.gl_pathv[j]);
                iCount++;
            }
        } else {
            xha_printf("Error for pattern [%s]\n", sPattern);
        }
        globfree(&glob_data);
    }
    xha_printf("Collected %u caller files from [%s]\n", iCount, m_sCallerDir);
    
    m_mCalleeFiles.clear();
    iCount = 0;
    for (uint i = 0; i < m_vCalleeSuffs.size(); i++) {
        const std::string sCalleeSuff = m_vCalleeSuffs[i];
        std::string sPattern = xha_sprintf("%s/*.%s", m_sCalleeDir, sCalleeSuff);
        iResult = glob(sPattern.c_str(), 0, NULL, &glob_data);
        if ((iResult == 0) || (iResult = GLOB_NOMATCH)) {
            iResult = 0;
            for (uint j = 0; j < glob_data.gl_pathc; j++) {
                m_mCalleeFiles[sCalleeSuff].push_back(glob_data.gl_pathv[j]);
                iCount++;
            }
        } else {
            xha_printf("Error for pattern [%s]\n", sPattern);
        }
        globfree(&glob_data);
    }
    xha_printf("Collected %u callee files from [%s]\n", iCount, m_sCallerDir);

    return iResult;
}


//----------------------------------------------------------------------------
// prepare_sources
//
int DepScanner::prepare_sources() {
    int iResult = 0;
    //m_mCallerBodies
    mapsuffilevec::const_iterator it;

    m_mCallerBodies.clear();
    for (it = m_mCallerFiles.begin(); it !=  m_mCallerFiles.end(); ++it) {
        for (uint i = 0; i < it->second.size(); i++) {
            std::string sFile = it->second[i];
            uint iPosStart = sFile.find_last_of("/")+1;
            uint iPosEnd   = sFile.find_last_of(".");
            m_mCallerBodies[it->first].push_back(sFile.substr(iPosStart, iPosEnd-iPosStart)); 
        }
    }

    m_mCalleeBodies.clear();
    for (it = m_mCalleeFiles.begin(); it !=  m_mCalleeFiles.end(); ++it) {
        for (uint i = 0; i < it->second.size(); i++) {
            std::string sFile = it->second[i];
            uint iPosStart = sFile.find_last_of("/")+1;
            uint iPosEnd   = sFile.find_last_of(".");
            m_mCalleeBodies[it->first].push_back(sFile.substr(iPosStart, iPosEnd-iPosStart)); 
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// find_deps
//
int DepScanner::find_deps(const std::string sCalleeSuff, const std::string sCallerSuff, mapsuffilevec &mCalls) {
    int iResult = 0;
    std::string sPat = mInclusionPattern[stringpair(sCalleeSuff, sCallerSuff)];

    //stringvec &vCallers = m_mCallerBodies[sCallerSuff];
    stringvec &vCallers = m_mCallerFiles[sCallerSuff];
    stringvec &vCallees = m_mCalleeBodies[sCalleeSuff];


    for (uint iCallee = 0; iCallee < vCallees.size(); iCallee++) {
        std::string sCallee = vCallees[iCallee]+"."+sCalleeSuff;
        //xha_printf("callee bod [%s]\n", vCallees[iCallee]);
        std::string sCurPat0 = xha_sprintf(sPat,  vCallees[iCallee]);
        std::string sCurPat = sCurPat0;
        xha_printf("--- [%s] users of [%s]\n", sCallerSuff, sCallee);
            
        for (uint iCaller = 0; iCaller < vCallers.size(); iCaller++) {
            //std::string sCaller = vCallers[iCaller]+"."+pCallerSuff;
            std::string sCaller = vCallers[iCaller];
            //xha_printf("   [%s]:[%s]\n",  sCallee, sCaller);

            std::ifstream fIn(sCaller);
            if (fIn.good()) {
                std::string sLine; 
                while (std::getline(fIn, sLine)) {
                    sLine = trim(sLine);
                    if (std::regex_match(sLine, std::regex(sCurPat))) {
                        stringvec::const_iterator itf = std::find(mCalls[sCallee].begin(),mCalls[sCallee].end(), sCaller);
                        
                        if (itf == mCalls[sCallee].end()) {
                            xha_printf("  have match for [%s]\n", sCaller);
                            mCalls[sCallee].push_back(sCaller);
                    }
                    }
                } 
                fIn.close();
            } else {
                xha_printf("Couldn't open [%s]\n", sCaller);
            }
        }
        
    }
    return iResult;
};

