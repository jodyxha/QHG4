#include <cstdio>
#include <cstring>

#include <map>
#include <vector>
#include <string>

#include "strutils.h"
#include "LineReader.h"

std::map<std::string,std::string> trans ={
    {"NPPCap",   "NPPCapacity"},
    {"OAD",      "OldAgeDeath"},
    {"Multi",    "MultiEvaluator"},
    {"MM",       "MassManager"},
    {"RandMove", "RandomMove"},
};

typedef std::map<std::string, std::string>             namevals;
typedef std::pair<std::string, std::string>            stringpair;
typedef std::vector<stringpair>                        stringpairs;
typedef std::map<std::string, stringpairs>             attribs; 

//----------------------------------------------------------------------------
// collectAttribs
//
int collectAttribs(char *pInput, namevals &cattr, namevals &nv, namevals &prios) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(pInput, "r");
    if (pLR != NULL) {
        iResult = 0;
        char *pLine = pLR->getNextLine();
        while ((pLine != NULL) && (!pLR->isEoF()) && (iResult == 0)) {
            if (strstr(pLine, "CLASS") == pLine) {
                char *p = trim(strchr(pLine, ' '));
                cattr["ClassName"] = p;
            } else if  (strstr(pLine, "SPECIES") == pLine) {
                char *p = strtok(pLine, " ");
                if (p != NULL) {
                    p=strtok(NULL, " ");
                    if (p != NULL) {
                        cattr["SpeciesID"] = p;
                        p=strtok(NULL, " ");
                        if (p != NULL) {
                            cattr["SpeciesName"] = p;
                        } else {
                            iResult = -1;
                            printf("Bad format for SPECIES line: [%s]\n", pLine);
                        }
                    } else {
                        iResult = -1;
                        printf("Bad format for SPECIES line: [%s]\n", pLine);
                    }
                } else {
                    iResult = -1;
                        printf("Bad format for SPECIES line: [%s]\n", pLine);
                }
            } else if (strstr(pLine, "PRIO") == pLine) {
                char *p = strtok(pLine, " ");
                if (p != NULL) {
                    p=strtok(NULL, " ");
                    if (p != NULL) {

                        char *p2=strtok(NULL, " ");
                        if (p2 != NULL) {
                            prios[p] = p2;
                        } else {
                            iResult = -1;
                            printf("Bad format for PRIO line: [%s]\n", pLine);
                        }
                    } else {
                        iResult = -1;
                        printf("Bad format for PRIO line: [%s]\n", pLine);
                    }
                } else {
                    iResult = -1;
                        printf("Bad format for PRIO line: [%s]\n", pLine);
                }    
            } else {
                char *p = strchr(pLine, '=');
                if (p != 0) {
                    
                    *p++ = '\0';
                    p = trim(p);
                    pLine = trim(pLine);
                    nv[pLine] = p;
                }
            }
            pLine = pLR->getNextLine();
        }

        printf("Have %zd nam-val pairs\n", nv.size());
        // now try to group them
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pInput);
    }   
    return iResult;
}


//----------------------------------------------------------------------------
// bundleAttribs
//
int bundleAttribs(namevals &nv, attribs &al, stringpairs &badlines) {

   int iResult = 0;


   namevals::const_iterator it;
   for (it = nv.begin(); it != nv.end(); ++it) {
       char sName[256];
       strcpy(sName, it->first.c_str());
       char *p = strchr(sName, '_');
       if (p != NULL) {
           *p++ = '\0';
                
                
           std::map<std::string,std::string>::const_iterator itn = trans.find(sName);
           if (itn != trans.end()) {
               strcpy(sName, itn->second.c_str());
           }
                       
           al[sName].push_back(stringpair(it->first.c_str(), it->second.c_str()));
       } else {
           if (strstr(sName, "Pref") == ((sName + strlen(sName) - 4))) {
               al["SingleEvaluator"].push_back(stringpair(it->first.c_str(), it->second.c_str()));
           } else {
               badlines.push_back(*it);
           }
           iResult = 0;
       }
   }
        
    return iResult;
}


//----------------------------------------------------------------------------
// writeXML
//
int writeXML(attribs &al, namevals &cattr, namevals &prios, stringpairs &badlines, FILE *fOut) {
    int iResult = 0;

    fprintf(fOut, "<class name=\"%s\" species_name=\"%s\" species_id=\"%s\">\n", cattr["ClassName"].c_str(), cattr["SpeciesName"].c_str(), cattr["SpeciesID"].c_str());
        
    attribs::const_iterator ita;
    for (ita = al.begin(); ita != al.end(); ++ita) {
        fprintf(fOut, "  <module name=\"%s\">\n", ita->first.c_str());
        
        std::vector<stringpair>::const_iterator itv;
        for (itv = ita->second.begin(); itv != ita->second.end(); ++itv) {
            fprintf(fOut, "    <attribute  name=\"%s\" value=\"%s\" />\n", itv->first.c_str(), itv->second.c_str());
        }
        fprintf(fOut, "  </module>\n");
        
    }
    
    
    namevals::const_iterator itp;
    fprintf(fOut, "  <priorities>\n");
    for (itp = prios.begin(); itp != prios.end(); ++itp) {
       fprintf(fOut, "    <prio  name=\"%s\" value=\"%s\" />\n", itp->first.c_str(), itp->second.c_str());
    }
    fprintf(fOut, "  </priorities>\n");
        
    fprintf(fOut, "</class>\n");    

    if (badlines.size() > 0) {
        stringpairs::const_iterator its;
        fprintf(fOut, "  <badlines>\n");
        for (its = badlines.begin(); its != badlines.end(); ++its) {
            fprintf(fOut, "    <badline  name=\"%s\" value=\"%s\" />\n", its->first.c_str(), its->second.c_str());
        }
        fprintf(fOut, "  </badlines>\n");
    }

    return iResult;
} 

//----------------------------------------------------------------------------
// simpleShow
//
void simpleShow(attribs &al, namevals &cattr, namevals &prios, stringpairs &badlines) {
           
    printf("class [%s]\n", cattr["ClassName"].c_str());
    printf("species [%s] (%s)\n", cattr["SpeciesName"].c_str(), cattr["SpeciesID"].c_str());
    attribs::const_iterator ita;
    for (ita = al.begin(); ita != al.end(); ++ita) {
        printf("%s:\n", ita->first.c_str());
        std::vector<stringpair>::const_iterator itv;
        for (itv = ita->second.begin(); itv != ita->second.end(); ++itv) {
            printf("  %-30s : %s\n", itv->first.c_str(), itv->second.c_str());
        }
        
    }
    namevals::const_iterator itnv;
    for (itnv = prios.begin(); itnv != prios.end(); ++itnv) {
        printf("PRIO %s %s\n", itnv->first.c_str(), itnv->second.c_str());
    }

    
    stringpairs::const_iterator its;
    for (its = badlines.begin(); its != badlines.end(); ++its) {
        printf("BAD %s %s\n", its->first.c_str(), its->second.c_str());
    }

}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char sInput[256];
    char sOutput[256];

    if (iArgC > 1) {
        strcpy(sInput, apArgV[1]);
        if (iArgC > 2) {
            strcpy(sOutput, apArgV[2]);
        } else {
            *sOutput = '\0';
        }

        attribs al;
        namevals cattr;
        namevals prios;
        stringpairs badlines;
        namevals nv;
        iResult = collectAttribs(sInput, cattr, nv, prios);
        
        if (iResult == 0) {
            iResult = bundleAttribs(nv, al, badlines);
        }
        if (iResult == 0) {
            
            FILE *fOut;
            if (*sOutput == '\0') {
                fOut = stdout;
            } else {
                fOut = fopen(sOutput, "wt");
            } 
            if (fOut != NULL) {
                writeXML(al, cattr, prios, badlines, fOut);
            }

            if (*sOutput != '\0') {
                fclose(fOut);
            }
            if (badlines.size() > 0) {
                printf("ATTENTION bad lines found\n");
                printf("They are collected under the tag <badlines> in the XML output\n");
            }
            //simpleShow(al, cattr, prios, badlines);
        
        }
    } 
    return iResult;
}
