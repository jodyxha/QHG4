#include <regex>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <glob.h>

#include "types.h"
#include "ParamReader.h"
#include "LineReader.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "CSVXMLChecker.h"

//----------------------------------------------------------------------------
// usage
//
void usage(const std::string sApp) {
    stdprintf("%s - check xml files created from a csv file\n", sApp);
    stdprintf("usage:\n");
    stdprintf("  %s --csv=<csv-file> --xml=\"<xml-file-pat>\"\n", sApp);
    stdprintf("               [--csv-ignore-keys=<ignore-list>]\n");
    stdprintf("               [--xml-ignore-keys=<ignore-list>]\n");
    stdprintf("where\n");
    stdprintf("  csv-file       CSV file from which the XML fies were created\n");
    stdprintf("  xml-file-pat   pattern for XML files created from CSV file\n");
    stdprintf("  ignore-list    ':'-separated list of keys to ignore\n");
    stdprintf("\n");
}
 


//----------------------------------------------------------------------------
// showsmcontents
//
void showsmcontents(const stringmap &smContents, std::string sIndent) {
    stringmap::const_iterator it;
    for (it = smContents.begin(); it != smContents.end(); ++it) {
        stdprintf("%s%40s: %20s\n", sIndent, it->first, it->second);
    }
}


//----------------------------------------------------------------------------
// showsmcontentsL
//   write the values on the left end of the line
//
void showvcontentsL(const stringvec &vKeys, const stringmap &smContents, stringvec &vIgnore,  std::string sIndent) {
    for (uint i = 0; i < vKeys.size(); i++) {
        bool bOK = true;
        for (uint j = 0; bOK && (j < vIgnore.size()); j++) {
            if (vKeys[i] == vIgnore[j]) {
                bOK = false;
            }
        }

        if (bOK && (smContents.find(vKeys[i]) != smContents.end())) {     
            stdprintf("%s%40s:%20s\n", sIndent, vKeys[i], smContents.at(vKeys[i]));
        }
    }
}


//----------------------------------------------------------------------------
// showvcontentsR
//   write the values on the right end of the line
//
void showvcontentsR(const stringvec &vKeys, const stringmap &smContents, stringvec &vIgnore, std::string sIndent) {
    for (uint i = 0; i < vKeys.size(); i++) {
        bool bOK = true;
        for (uint j = 0; bOK && (j < vIgnore.size()); j++) {
            if (vKeys[i] == vIgnore[j]) {
                bOK = false;
            }
        }
        if (bOK && (smContents.find(vKeys[i]) != smContents.end())) {
            stdprintf("%s%40s:%40s\n", sIndent, vKeys[i], smContents.at(vKeys[i]));
        }
    }
}


//----------------------------------------------------------------------------
// showsmcontentsLR
//   write the values on both ends of the line
//
void showvcontentsLR(const stringvec &vKeys, const stringmap &smContents1, const stringmap &smContents2, std::string sIndent) {
    for (uint i = 0; i < vKeys.size(); i++) {
        stdprintf("%s%40s:%20s%20s\n", sIndent, vKeys[i], smContents1.at(vKeys[i]), smContents2.at(vKeys[i]));
    }
}


//----------------------------------------------------------------------------
// showtablenames
//
void showtablenames(int iMatch, std::string sIndent) {
    std::string sCSV = stdsprintf("CSV[%d]", iMatch);
    stdprintf("%s%40s %20s%20s\n", sIndent, "Parameter Name", sCSV, "XML");
    std::string sBreak(81, '-');
    stdprintf("%s%s\n", sIndent, sBreak);
}


//----------------------------------------------------------------------------
// showsmvcontents
//
void showsmvcontents(const stringmapvec &smvContents) {
    for (uint i = 0; i < smvContents.size(); i++) {
        std::printf("line %d\n", i+1);
        showsmcontents(smvContents[i], "  ");
    }
    
}


//----------------------------------------------------------------------------
// collectFiles
//  use glob to get all files matching the pattern and put into vFiles
//
int collectFiles(std::string sPat, stringvec &vFiles) {
    int iResult = -1;

    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));
    
    iResult = glob(sPat.c_str(), GLOB_TILDE, NULL, &glob_result);
    if (iResult == 0) {
        for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
            vFiles.push_back(std::string(glob_result.gl_pathv[i]));
        }
    }
    globfree(&glob_result);

    return iResult;
}


//----------------------------------------------------------------------------
// diffsCompare
//
int diffsCompare(CSVXMLChecker *pCXC, std::string sXMLFile, stringvec vXMLIgnoreKeys) {
    int iResult = -1;
    
    iResult = pCXC->setXML(sXMLFile, vXMLIgnoreKeys);
    if (iResult == 0) {
        //showsmcontents(smXMLContents, "");
        // now we have both contents
        
       
        pCXC->getDifferences();
        
        iResult = pCXC->compareValues(pCXC->getCSVandXML());
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    ParamReader *pPR = new ParamReader();
    std::string sCSVFile;
    std::string sXMLFile;
    std::string sCSVIgnoreKeys;
    std::string sXMLIgnoreKeys;
    pPR->setOptions(4,
                    "--csv:s!",            &sCSVFile,
                    "--xml:s!",            &sXMLFile,
                    "--csv-ignore-keys:s", &sCSVIgnoreKeys,
                    "--xml-ignore-keys:s", &sXMLIgnoreKeys);
    iResult = pPR->getParams(iArgC, apArgV);
    if (iResult == 0) {
        stdprintf("CSV file: %s\n", sCSVFile);
        stdprintf("XML file: %s\n", sXMLFile);

        stringvec vCSVIgnore;
        if (sCSVIgnoreKeys.empty()) {
            uint iNum = splitString(sCSVIgnoreKeys, vCSVIgnore, ":", false);
            stdprintf("CSV ignore keys (%u): %bv \n", iNum, vCSVIgnore);
        } else {
            stdprintf("CSV ignore keys (0)\n");
        }

        stringvec vXMLIgnore;
        if (sXMLIgnoreKeys.empty()) {
            uint iNum = splitString(sXMLIgnoreKeys, vXMLIgnore, ":", false);
            stdprintf("XML ignore keys (%u): %bv \n", iNum, vXMLIgnore);
        } else {
            stdprintf("XML ignore keys (0)\n");
        }
        

        
        CSVXMLChecker *pCXC =CSVXMLChecker::createInstance(sCSVFile, vCSVIgnore);
        if (pCXC != NULL) {
            const stringvec &vIgnoredCSV       = pCXC->getCSVIgnoredKeys();
            const stringmapvec &smvCSVContents = pCXC->getCSVContents();
            stdfprintf(stderr, "Ignored %zd key%s from CSVFile [%s]\n", vIgnoredCSV.size(), (vIgnoredCSV.size()==1)?"":"s", sCSVFile);
            //            showsmvcontents(smvCSVContents);
            //            showsmcontents(smvCSVContents[0],"");

            stringvec vXMLFiles;
            iResult = collectFiles(sXMLFile, vXMLFiles);
            if (iResult == 0) {
                if (vXMLFiles.size() == 1) {

                    std::string sXMLFileReal{vXMLFiles[0]};
                    iResult = diffsCompare(pCXC, sXMLFileReal, vXMLIgnore);
                    if (iResult == 0) {
                        const stringvec &vIgnoredXML   = pCXC->getXMLIgnoredKeys();
                         const stringmap &smXMLContents = pCXC->getXMLContents();

                        stdfprintf(stderr,"Ignored %zd key%s from XMLFile [%s]\n", vIgnoredXML.size(), (vIgnoredXML.size()==1)?"":"s", sXMLFile);

                        const intvec &vMatches =  pCXC->getMatchIndexes();
                        if (vMatches.size() == 1) {
                            int i = vMatches[0];
                            stdprintf("\nFound match at index %d (corresponds to line %d in the CSV file)\n", i, i+2);
                            if (pCXC->getCSVnotXML().size() > 0) {
                                stdprintf("Warning: there are %zd keys in the CSV file which do not appear in the XML file (%zd ignored)\n", pCXC->getCSVnotXML().size(), vIgnoredCSV.size());
                            }
                            if (pCXC->getXMLnotCSV().size() > 0) {
                                stdprintf("Warning: there are %zd keys in the XML file which do not appear in the CSV file (%zd ignored)\n", pCXC->getXMLnotCSV().size(), vIgnoredXML.size());
                            }
                            showtablenames(i, "");
                            showvcontentsL(pCXC->getCSVnotXML(),  smvCSVContents[i], vCSVIgnore,    "");
                            showvcontentsLR(pCXC->getCSVandXML(), smvCSVContents[i], smXMLContents, "");
                            showvcontentsR(pCXC->getXMLnotCSV(),  smXMLContents,     vXMLIgnore,    "");
                                                    
                        } else {
                            stdprintf("Have %d matches - there might be duplicate lines: %bv\n", vMatches.size(), vMatches);
                        }
                    } else {
                        stdprintf("No match found\n");
                    }
                } else {
                    stdprintf("Matches for the provided XML files (%d):\n", vXMLFiles.size());
                    for (uint i = 0; i < vXMLFiles.size(); i++) {
                        std::string sXMLFileReal{vXMLFiles[i]};

                        iResult = diffsCompare(pCXC, sXMLFileReal, vXMLIgnore);
                        if (iResult == 0) {
                            const intvec &vMatches =  pCXC->getMatchIndexes();
                            if (vMatches.size() == 1) {
                                stdprintf("%30s: match %8zd\n", sXMLFileReal, vMatches[0]);
                            }
                        } else {
                            stdprintf("%30s: no match\n", sXMLFileReal);
                        }

                    }
                }
            } else {
                stdprintf("Error during glob for [%s]\n", sXMLFile);
            }

            delete pCXC;
        }
    } else {
        stdprintf("Bad Parameters\n");
        stringvec vUnknown;
        pPR->getUnknownParams(vUnknown);
        stdprintf("  Unknwon params; %bv\n", vUnknown);

        stringvec vMand;
        pPR->getMandatoryParams(vMand);
        stdprintf("  Mandatory params; %bv\n", vMand);

        usage(apArgV[0]);
    }


    return iResult;
}

