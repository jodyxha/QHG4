#include <stdint.h>
#include <cstdio>
#include <cstdlibo>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <algorithm>

#include "LineReader.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

typedef std::vector <std::string> stringvec;
typedef std::map<std::string, stringvec> stringstringvecmap;
typedef std::map<std::string, std::string> stringstringmap;

stringstringvecmap attr_spc_names={{"NPersZHybBirthDeathRel", {"HybBirthDeathRel_hybminprob"}},
                

                                   {"LocEnv",                 {"NPPCap_K_max_", 
                                                               "NPPCap_K_min_", 
                                                               "NPPCap_NPP_max_", 
                                                               "NPPCap_NPP_min_", 
                                                               "NPPCap_coastal_factor_", 
                                                               "NPPCap_coastal_max_latitude_", 
                                                               "NPPCap_coastal_min_latitude_", 
                                                               "NPPCap_water_factor_"}},
                                   
                                   {"PrivParamMix",           {"NPersHybBirthDeathRel_b0_" ,
                                                               "NPersHybBirthDeathRel_d0_" ,
                                                               "NPersHybBirthDeathRel_theta_" ,
                                                               "NPersHybBirthDeathRel_other_" ,
                                                               "NPersHybBirthDeathRel_this_" ,
                                                               "Fertility_min_age_" ,
                                                               "Fertility_max_age_" ,
                                                               "Fertility_interbirth_" ,
                                                               "NPersWeightedMove_" ,
                                                               "OAD_max_age_" ,
                                                               "OAD_uncertainty_" ,
                                                               "PrivParamMix_mode"}},
                                   
                                   {"Navigate",               {"Navigate_decay",
                                                               "Navigate_dist0",
                                                               "Navigate_min_dens",
                                                               "Navigate_bridge_prob",
                                                               "Navigate_prob0"}}};

stringstringmap dat_new_entries = {{"NPersWeightedMove",          "0.0885986328125"},
                                   {"OAD_max_age",                "69.404296875"},     
                                   {"OAD_uncertainty",            "0.1"},                  
                                   {"Fertility_min_age",          "13.983398"},            
                                   {"Fertility_max_age",          "44.94629"},           
                                   {"Fertility_interbirth",       "2.2236328"},      
                                   {"NPersHybBirthDeathRel_b0",   "0.2585693359375"},
                                   {"NPersHybBirthDeathRel_d0",   "0.020107421875"}};

stringstringvecmap dat_hyb_entries = {{"ali",   {"NeaFrac", "NeaSDev"}},
                                      {"zolli", {"GenHybM", "GenHybF"}}};

stringvec species_names = {"sapiens", "neander"};

std::map<std::string, std::map<std::string, stringvec>> hyb_vals = {{"ali",   {{"neander", {"1", "0.02"}}, {"sapiens", {"0", "0.02"}}}},
                                                                    {"zolli", {{"neander", {"1", "1"}},    {"sapiens", {"0", "0"}}}}};


stringvec dat_min_entries = {"Longitude",
                             "Latitude",
                             "LifeState",
                             "AgentID",
                             "BirthTime",
                             "Gender",
                             "Age",
                             "LastBirth",
                             "Species"};



const int SPECIES_INDEX = 8; //dat_min_entries.index("Species")

const std::string SEP = ";";
const int         DEF_WIDTH = 5;
const std::string DEF_WHAT  = "xml+dat";




//----------------------------------------------------------------------------
// collect_vals
//   from the head line of the csv and a given line, create a dictionary
//   associating attribute names with attribute values
// 
int collect_vals(stringvec csv_headers, std::string csv_line, stringstringmap &namvals) {
    int iResult = -1;
    stringvec vals;
    uint iNum = splitString(csv_line, vals, SEP);
    if (iNum == csv_headers.size()) {
        for (uint i = 0; i < iNum; ++i) {
            namvals[trim(csv_headers[i])] = trim(vals[i]);
        }
        iResult = 0;

    } else {
        stdprintf("%zd names, but %uvalues", csv_headers.size(),iNum);
        for (uint i = 0; i < iNum; ++i) {
            stdprintf("  %s\n", vals[i]);
        }
    }
    return iResult;
}; 


//----------------------------------------------------------------------------
// create_xml
// 
int create_xml(std::string xml_template,  stringstringmap &namvals, FILE *fout_xml) {
    int iResult = 0;
    LineReader *pLR = LineReader_std::createInstance(xml_template, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM);
        while ((iResult == 0) && (pLine != NULL))  {
            bool bModified = false;
            stringstringvecmap::iterator itssv;
            for (itssv = attr_spc_names.begin(); itssv !=  attr_spc_names.end(); ++itssv) {
                stringvec::iterator itsv;
                for (itsv = itssv->second.begin(); itsv !=  itssv->second.end(); ++itsv) {
                    if (itsv->back() == '_') {
                        bool bFound = false;
                        for (uint i = 0; (not bFound) && (i < 2); i++) {
                            std::string fullname = *itsv + species_names[i];
                            stringstringmap::const_iterator itnv = namvals.find(fullname);
                            if (itnv != namvals.end()) {
                                // do the match; set bModified = true if match
                                std::string sPat = stdsprintf("(.*)(%s\" *value=\")(.*)(\" * />)", fullname);
                                std::cmatch cm;
                                if (std::regex_match(pLine, cm, std::regex(sPat))) { 
                                    /*
                                    std::string base1 = cm[1].str();
                                    std::string base2 = cm[2].str();
                                    stdprintf("b1 %s, b2 %s\n", base1, base2);
                                    */
                                    stdfprintf(fout_xml, "%s%s%s%s\n", cm[1].first, cm[2].first, namvals[fullname], cm[4].first);
                                    bModified = true;

                                }
                            }

                        }
                    } else {
                        //species independent attributes
                        //simple match
                        stringstringmap::const_iterator itnv = namvals.find(*itsv);
                        if (itnv != namvals.end()) {
                            std::string sPat = stdsprintf("(.*)(%s\" *value=\")(.*)(\" * />)", *itsv);
                            std::cmatch cm;
                            if (std::regex_match(pLine, cm, std::regex(sPat))) { 
                                stdfprintf(fout_xml, "%s%s%s%s\n", cm[1].first, cm[2].first, namvals[*itsv], cm[4].first);
                                bModified = true;
                            }
                        }
                        
                    }
                }
            }

            // if we have not done any modification, return original line
            if (!bModified) {
                stdfprintf(fout_xml, "%s\n", pLine);
            }

            pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM);
        }

        delete pLR;
    } else {
        iResult = -1;
        stdprintf("Couldn't open [%s]\n", xml_template);
    }
   
    return iResult;
}




//----------------------------------------------------------------------------
// create_dat
// 
//   we expect the template line to have the values
//   Longitude Latitude LifeState AgentID BirthTime Gender Age LastBirth Species
//
int create_dat(std::string dat_template,   stringstringmap &namvals, std::string approach, FILE *fout_dat) {
    int iResult =0;
    
    LineReader *pLR = LineReader_std::createInstance(dat_template, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM);
        while ((iResult == 0) && (pLine != NULL))  {
            std::string p = trim(pLine);

            if (p[0] == '#') {
                // header line
                stringvec vParts;
                uint iNum = splitString(p, vParts, "# "+SEP);

                bool bEqual = true;
                if (iNum == dat_min_entries.size()) {
                    for (uint i = 0; bEqual &&(i < iNum); i++)  {
                        if (dat_min_entries[i] != vParts[i]) {
                            bEqual = false;
                        }
                    }

                } else {
                    bEqual = false;
                }
                if (bEqual) {

                } else {
                    stdprintf("Header inconsistent!");
                    stdprintf("Expected: [",dat_min_entries);
                    for (uint i = 0; i < iNum; ++i) {
                        stdprintf("%s,", dat_min_entries[i]);
                    }
                    stdprintf("]\n");

                    stdprintf("Got:      [");
                    for (uint i = 0; i < iNum; ++i) {
                        stdprintf("%s,", vParts[i]);
                    }
                    stdprintf("]\n");
                    iResult = -1;
                }
            } else {
                stringvec vals0;
                // grab the values already there
                /*uint iNum = */splitString(p, vals0, SEP);
                int iIndex = 0;
                
                if (strToNum(vals0[SPECIES_INDEX], &iIndex)) {
                    std::string species = species_names[iIndex];
                    vals0.erase(vals0.begin()+iIndex);
                    
                    // add the ali/zolli-specific vars
                    vals0.insert(vals0.end(), hyb_vals[approach][species].begin(), hyb_vals[approach][species].end());
                    
                    // now the values for the hyb data
                    stringstringmap::iterator it; 
                    for (it = dat_new_entries.begin(); it != dat_new_entries.end(); ++it) {
                        std::string n1 = it->first + "_"+ species;
                        stringstringmap::const_iterator itv = namvals.find(n1);
                        if (itv != namvals.end()) {
                            vals0.push_back(namvals[n1]);
                        } else {
                            vals0.push_back(dat_new_entries[it->first]);
                        }
                    }
                    
                    stdfprintf(fout_dat, "%s\n", join(vals0, SEP));  

                } else {
                    //shouldn't happen
                    iResult = -1;

                } 
                                   
            }
            

            pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM);
        }
        delete pLR;
    } else {
        iResult = -1;
        stdprintf("Couldn't open [%s]\n", dat_template);
    }

    return iResult;

}


static const char *const str =
    "1) John Driverhacker;\n2) John Doe;\n3) John Foo;\n";
static const char *const re = "John.*o";

static const char *const re2="(.*)(%s\" *value=\")(.*)(\" */>)";

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    std::string out_prefix = "";
    int width = DEF_WIDTH;
    std::string what = DEF_WHAT;
    bool bDoXML = false;
    bool bDoDAT = false;
    
    int cur = 1;
    while ((iArgC > cur+1) && (iResult == 0) && (apArgV[cur][0] == '-')) {
        if (strcmp(apArgV[cur], "-w") == 0) {
            iResult = (strToNum(apArgV[cur+1], &width))?0:-1;
        } else if (strcmp(apArgV[cur], "-a") == 0) {
            what = apArgV[cur+1];
        }
        cur = cur + 2;
    }

   
    if (iResult == 0) {
        stringvec actions;
        uint iNum = splitString(what, actions, "+");
        if (iNum < 3) {
            bDoXML = (actions.end() != std::find(actions.begin(), actions.end(), "xml"));
            bDoDAT = (actions.end() != std::find(actions.begin(), actions.end(), "dat"));
        } else {
            stdprintf("Bad number of whats: [%s]\n", what);
            iResult = -1;
        }
    }

    if ((iResult == 0) && (iArgC > cur + 3)) {
        std::string xml_template = apArgV[cur];
        std::string dat_template = apArgV[cur+1];
        std::string csv_file     = apArgV[cur+2];
        std::string approach     = apArgV[cur+3];
        if (iArgC > cur + 4) {
            out_prefix = apArgV[cur+4];
        }
        
        FILE *fout_xml = NULL;
        FILE *fout_dat = NULL;
        std::string sname_xml = "";
        std::string sname_dat = "";

        LineReader *pLR = LineReader_std::createInstance(csv_file, "rt");
        if (pLR != NULL) {
            char *pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM);
            if (pLine[0] == '#') {
                stringvec csv_headers;
                uint iNum = splitString(pLine, csv_headers, SEP);
                if (iNum > 1) {
                // TODO: find sim_idx
                    int count = 0;
                    pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM);
                    while ((iResult == 0) && (pLine != NULL)) {
                        stdprintf("doing line %d\n", count);fflush(stdout); 
                        stringstringmap namvals;
                        iResult = collect_vals(csv_headers, pLine, namvals);


                        if (iResult == 0) {
                            std::string sBody;
                            if (out_prefix != "") {
                                
                                std::string sW = stdsprintf("%s_%%0", out_prefix);
                                std::string sW2= stdsprintf("%d", width);
                                std::string sBody = stdsprintf(sW+sW2+"d",count);
                            }
                            
                            if (bDoXML) {
                                if (out_prefix == "") {
                                    fout_xml  = stdout;
                                    sname_xml = "(stdout)";
                                } else {
                                   
                                    sname_xml = sBody+".xml";
                                    
                                    std::string sT2 = stdsprintf("%%0%dd", width);
                                    stdprintf("t2: [%s]\n", sT2);

                                    fout_xml  = fopen(sname_xml.c_str(), "wt");
                                    stdprintf("four: %p\n", fout_xml);
                                }
                                stdprintf("----- create_xml [%s] -----\n", sname_xml);fflush(stdout); 
                                // create an xml for this line (using the xml template) 
                                iResult = create_xml(xml_template, namvals, fout_xml);
                            

                                stdprintf("----- closing xml file -----\n");fflush(stdout); 
                                if (out_prefix != "") {
                                    fclose(fout_xml);
                                }
                            }

                            if (bDoDAT) {
                                if (out_prefix == "") {
                                    fout_dat  = stdout;
                                    sname_dat = "(stdout)";
                                } else {
                                    sname_dat = sBody + ".dat";
                                    //                                   sname_dat = stdsprintf("%s_%0*d.dat", out_prefix, width, count);
                                    fout_dat  = fopen(sname_dat.c_str(), "wt");
                                }
                                stdprintf("----- create_dat [%s] -----\n", sname_dat); fflush(stdout);
                                // create an dat for this line (using the dat template) 
                                iResult = create_dat(dat_template, namvals, approach, fout_dat);
                                stdprintf("----- closing dat file -----\n"); fflush(stdout);
                                if (out_prefix != "") {
                                    fclose(fout_dat);
                                }

                            }
                        }
                        stdprintf("done with #%d\n",count);fflush(stdout);
                        count = count + 1;

                        pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM);
                    }
                } else {
                    stdprintf("there seems to be no [%s] in the header\n", SEP);
                }
                
            } else {
                stdprintf("expected a #-commentred header in the first line\nn");
                iResult = -1;
            }
            
            delete pLR;
        }
        
    } else {
        stdprintf("%s - turn csv to xml+dat files\n", apArgV[0]);
        stdprintf("usage:\n");
        stdprintf("  %s [-w <width>] [-a <what>] <xml-template> <dat-template> <csv-file> <approach> [<out-body>]\n", apArgV[0]);
        stdprintf("where\n");
        stdprintf("  width:         number of digits in output name (0-padded); default:%d\n", DEF_WIDTH);
        stdprintf("  what:          do xml only, dat only or both. format: ('xml'|'dat')['+'(xml|dat)'] default:%s\n", DEF_WHAT);
        stdprintf("  xml-template:  an XML attribute file to be used as template\n");
        stdprintf("  dat-template:  an dat agent data file to be used as template (all values (including header) ';'-separated)\n");
        stdprintf("  csv-file:      a CSV file containing attribute values\n");
        stdprintf("  approach;      'ali' or 'zolli'\n");
        stdprintf("  out-body;      name template for output files\n");
    }


    return iResult;
}

    /*
    static const char *s = str;
    regex_t     regex;
    regmatch_t  pmatch[1];
    regoff_t    off, len;

    if (regcomp(&regex, re, REG_NEWLINE))
        exit(EXIT_FAILURE);

    printf("String = \"%s\"\n", str);
    printf("Matches:\n");

    for (int i = 0; ; i++) {
        if (regexec(&regex, s, ARRAY_SIZE(pmatch), pmatch, 0))
            break;

        off = pmatch[0].rm_so + (s - str);
        len = pmatch[0].rm_eo - pmatch[0].rm_so;
        printf("#%d:\n", i);
        printf("offset = %jd; length = %jd\n", (intmax_t) off,
               (intmax_t) len);
        printf("substring = \"%.*s\"\n", len, s + pmatch[0].rm_so);

        s += pmatch[0].rm_eo;
    }

    exit(EXIT_SUCCESS);
    */

