#include <regex>
#include <string>
#include <map>
#include <vector>
#include <algorithm>


#include "types.h"
#include "ParamReader.h"
#include "LineReader.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "CSVXMLChecker.h"

//----------------------------------------------------------------------------
// createInstance
//
CSVXMLChecker *CSVXMLChecker::createInstance(std::string sCSVFile, stringvec vCSVIgnoreKeys) {
    CSVXMLChecker *pCXC = new CSVXMLChecker();
    int iResult = pCXC->init(sCSVFile,vCSVIgnoreKeys);
    if (iResult != 0) {
        delete pCXC;
        pCXC = NULL;
    }
    return pCXC;
}


//----------------------------------------------------------------------------
// constructor
//
CSVXMLChecker::CSVXMLChecker() {
}


//----------------------------------------------------------------------------
// destructor
//
CSVXMLChecker::~CSVXMLChecker() {
}


//----------------------------------------------------------------------------
// init
//
int CSVXMLChecker::init(std::string sCSVFile, stringvec vCSVIgnoreKeys) {
    m_sCSVFile = sCSVFile;
    int iResult =  extractCSVContents(sCSVFile, vCSVIgnoreKeys);
    
    return iResult;
}

//----------------------------------------------------------------------------
// setCSVIgnores
//
int CSVXMLChecker::setCSVIgnores(stringvec vCSVIgnoreKeys) {
    int iResult = -1;
    m_vCSVIgnoreKeys = vCSVIgnoreKeys;
    if (m_sCSVFile != "") {
        iResult =  extractCSVContents(m_sCSVFile, m_vCSVIgnoreKeys);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// setXML
//
int CSVXMLChecker::setXML(std::string sXMLFile, stringvec vXMLIgnoreKeys) {
    m_sXMLFile = sXMLFile;
    int iResult =  extractXMLContents(sXMLFile, vXMLIgnoreKeys);
    
    return iResult;
}


//----------------------------------------------------------------------------
// setXMLIgnores
//
int CSVXMLChecker::setXMLIgnores(stringvec vXMLIgnoreKeys) {

    int iResult = -1;
    m_vXMLIgnoreKeys = vXMLIgnoreKeys;
    if (m_sXMLFile != "") {
        iResult =  extractXMLContents(m_sXMLFile, m_vXMLIgnoreKeys);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getCSVContents
//
int CSVXMLChecker::extractCSVContents(std::string sCSVFile, stringvec vCSVIgnoreKeys) {
    int iResult = -1;

    m_smvCSVContents.clear();
    m_vCSVHeaders.clear();
    m_vCSVIgnored.clear();
    LineReader *pLR = LineReader_std::createInstance(sCSVFile, "rt");
    if (pLR != NULL) {
        // we assume the first line to be the header
        char *pHeaderLine = pLR->getNextLine();

        m_vCSVHeadersReduced.clear();
        uint iNumHeads = splitString(pHeaderLine, m_vCSVHeaders, ",;:");
        stdfprintf(stderr, "headers (%u): %bv\n", iNumHeads, m_vCSVHeaders);
        iResult = 0;
        int iLine = 1;
        intvec vIgnoreIndexes;
        for (uint u = 0; u < m_vCSVHeaders.size(); ++u) {
            bool bSearching = true;
            for (uint v = 0; bSearching && (v < vCSVIgnoreKeys.size()); ++v) {
                if (m_vCSVHeaders[u] == vCSVIgnoreKeys[v]) {
                    vIgnoreIndexes.push_back(u);
                    m_vCSVIgnored.push_back(vCSVIgnoreKeys[v]);
                    bSearching = false;
                }
            }
        }


        while ((iResult == 0) && !pLR->isEoF()) {
            
            char *pCurLine = pLR->getNextLine();
            if (pCurLine != NULL) {
                iLine++;
                stringvec vCurValues;
                uint iNumVals = splitString(pCurLine, vCurValues, ",;:");
                if (iNumHeads == iNumVals) {
                    stringmap smCur;
                    for (uint i = 0; i < iNumHeads; i++) {
                        stringmap::const_iterator it = smCur.find(m_vCSVHeaders[i]);
                    
                        if (it == smCur.end()) {
                            intvec::const_iterator itv = std::find(vIgnoreIndexes.begin(), vIgnoreIndexes.end(), i);

                            if (itv == vIgnoreIndexes.end()) {
                                smCur[m_vCSVHeaders[i]] = vCurValues[i];
                                // only add to reduced headers once 
                                if (iLine == 2) {
                                    m_vCSVHeadersReduced.push_back(m_vCSVHeaders[i]);
                                }
                            }
                        } else {
                            stdprintf("Key [%s] already in smCur\n", m_vCSVHeaders[i]);
                        }
                    }
                    if (smCur.size() == iNumHeads - vIgnoreIndexes.size()) {
                        m_smvCSVContents.push_back(smCur);
                    } else {
                        stdprintf("There might be a duplicate header key (sm size %zd, h size %u)\n", smCur.size(), iNumHeads);
                        iResult = -1;
                    }
                } else {
                    stdprintf("Number of values (%u) does not match number of header items(%u) on line %d\n", iNumVals, iNumHeads, iLine);
                    iResult = -1;
                }
            }
        }

        delete pLR;
    } else {
        stdprintf("Couldn't open [%s] for reading\n", sCSVFile);
    }
    
    return iResult; 
}


//----------------------------------------------------------------------------
// getXMLContents
//
int CSVXMLChecker::extractXMLContents(std::string sXMLFile, stringvec vXMLIgnoreKeys) {
    int iResult = -1;
    m_smXMLContents.clear();
    m_vXMLKeys.clear();
    m_vXMLKeysReduced.clear();
    const std::string sXMLPat = "^ *<param *name=\"([-A-Za-z0-9_]*)\" *value=\"([-A-Za-z0-9_.]*)\" */>$";
    //const std::string sXMLPat = "<param *name=\"(.*)\" *value=\"(.*)\" */>";
    std::regex rPat{sXMLPat};
    LineReader *pLR = LineReader_std::createInstance(sXMLFile, "rt");
    if (pLR != NULL) {
        iResult = 0;
        while ((iResult == 0) && !pLR->isEoF()) {
            char *pCur = pLR->getNextLine();
            //stdprintf("cur is [%s]\n", pCur);
            if (pCur != NULL) {
                std::cmatch cm;
                if (std::regex_match(pCur, cm, rPat,std::regex_constants::match_default)) {
                    if (cm.size() == 3) {
                        std::string sKey =  cm.format("$1");
                        std::string sVal =  cm.format("$2");
                        stringmap::const_iterator it = m_smXMLContents.find(sKey);
                        if (it == m_smXMLContents.end()) {
                        
                            bool bOK = true;
                            for(uint u = 0; bOK && (u < vXMLIgnoreKeys.size()); ++u) {
                                if (sKey == vXMLIgnoreKeys[u]) {
                                    m_vXMLIgnored.push_back(vXMLIgnoreKeys[u]);
                                    bOK = false;
                                }
                            }
                            if (bOK) {
                                m_smXMLContents[sKey] = sVal;
                                m_vXMLKeysReduced.push_back(sKey);
                            }
                            m_vXMLKeys.push_back(sKey);
                        } else {
                            printf("it->first=[%s], it->second=[%s]\n", it->first.c_str(), it->second.c_str());
                            if (it->second == sVal) {
                                stdprintf("Have duplicate key [%s] with same value\n", sKey);
                            } else {
                                stdprintf("Have duplicate key [%s] with different values [%s] != [%s]\n", sKey, it->second, sVal);
                                iResult = -1;
                            }
                        }
                        
                    } else {
                        stdprintf("No complete match for name and value (cm:%d)\n", cm.size());
                        iResult = -1;
                    }
                } else {
                    //stdprintf("%s] does not match [%s]\n", pCur, sXMLPat);
                }
            }
            
        }
        delete pLR;
    } else {
        stdprintf("Couldn't open [%s] for reading\n", sXMLFile);
    } 
    
    return iResult;
}


//----------------------------------------------------------------------------
// getDifferences
//
void CSVXMLChecker::getDifferences() {
    std::sort(m_vCSVHeadersReduced.begin(), m_vCSVHeadersReduced.end());
    std::sort(m_vXMLKeysReduced.begin(),    m_vXMLKeysReduced.end());

    m_vCSVnotXML.clear();
    m_vXMLnotCSV.clear();
    m_vCSVandXML.clear();
    m_vCSVnotXML.resize(std::max(m_vCSVHeadersReduced.size(), m_vXMLKeysReduced.size()));
    m_vXMLnotCSV.resize(std::max(m_vCSVHeadersReduced.size(), m_vXMLKeysReduced.size()));
    m_vCSVandXML.resize(std::min(m_vCSVHeadersReduced.size(), m_vXMLKeysReduced.size()));
    stringvec::iterator it;    
    
    it = std::set_difference(m_vCSVHeadersReduced.begin(), m_vCSVHeadersReduced.end(), m_vXMLKeysReduced.begin(), m_vXMLKeysReduced.end(), m_vCSVnotXML.begin());
    m_vCSVnotXML.resize(it-m_vCSVnotXML.begin()); 

    it = std::set_difference(m_vXMLKeysReduced.begin(), m_vXMLKeysReduced.end(), m_vCSVHeadersReduced.begin(), m_vCSVHeadersReduced.end(), m_vXMLnotCSV.begin());
    m_vXMLnotCSV.resize(it-m_vXMLnotCSV.begin()); 

    it = std::set_intersection(m_vCSVHeadersReduced.begin(), m_vCSVHeadersReduced.end(), m_vXMLKeysReduced.begin(), m_vXMLKeysReduced.end(), m_vCSVandXML.begin());
    m_vCSVandXML.resize(it-m_vCSVandXML.begin()); 
}


//----------------------------------------------------------------------------
// compareValues
//
int CSVXMLChecker::compareValues(stringvec vCommonKeys) {
    int iResult = -1;
    m_vMatches.clear();
    for (uint i = 0; i < m_smvCSVContents.size(); ++i) {
        //stdfprintf(stderr, "checking for %u\n", i); 
        stringmap &smCSVContents = m_smvCSVContents[i];
        bool bSame = true;
        for (uint k = 0; k < vCommonKeys.size(); ++k) { 
            if (smCSVContents[vCommonKeys[k]] != m_smXMLContents[vCommonKeys[k]]) {
                //stdfprintf(stderr, "Fail for[%s]: csv[%s] xml[%s]\n", vCommonKeys[k], smCSVContents[vCommonKeys[k]],  m_smXMLContents[vCommonKeys[k]]);
                bSame = false;
            }
        }
        if (bSame) {
            //stdfprintf(stderr, "match with %d\n", i);
            m_vMatches.push_back(i);
        }
    }
    
    if (m_vMatches.size() > 0) {
        stdfprintf(stderr, "found matches for CSV value line%s %bv\n", (m_vMatches.size()==1)?"":"s", m_vMatches);
        iResult = 0;
    } else {
        stdfprintf(stderr, "no match found\n");
        iResult = -1;
    }
    
    return iResult;
}
