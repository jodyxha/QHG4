#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>     
#include <cstdarg>
#include <cstring>
#include "types.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "qhg_consts.h"
#include "ParamReader.h"

/*****************************************************************************\
 * OptionInfo
\*****************************************************************************/
class OptionInfo {
public:
    OptionInfo(std::string sOptionDef, void *pVariable);
    ~OptionInfo();
    const std::string getName() { return m_sName;};
    bool requiresArgument() { return m_bRequiresArgument;};
    bool isMandatory() { return m_bMandatory;};
    bool isCorrect() { return m_bCorrect;};
    bool isSet() { return m_bSet;};
    bool isLong() { return m_bLongOption;};
    char getType() { return m_cType;};
    bool setVal(const std::string sNewVal);
    const std::string getVal() { return m_sValue;};
private:
    std::string m_sName;
    std::string m_sValue;
    void       *m_pVariable;
    char        m_cType;
    bool        m_bCorrect;
    bool        m_bMandatory;
    bool        m_bRequiresArgument;
    bool        m_bSet;
    bool        m_bLongOption;
    char       *m_pTemp;
};

//-----------------------------------------------------------------------------
// constructor
//
OptionInfo::OptionInfo(std::string sOptionDef, void *pVariable)
    :   m_pVariable(pVariable),
        m_cType('\0'),
        m_bCorrect(false),
        m_bMandatory(false),
        m_bRequiresArgument(true),
        m_bSet(false),
        m_bLongOption(false),
        m_pTemp(NULL) {
    
    std::string::size_type pos = sOptionDef.find(":",0);
    if (pos != std::string::npos) {
        m_sName = sOptionDef.substr(0, pos);
        std::string sRest = sOptionDef.substr(pos+1);
        switch (sRest[0]) {
        case 'c':
        case 's':
        case 'S':
        case 'h':
        case 'i':
        case 'l':
        case 'f':
        case 'd':
        case 'b':
            m_cType = sRest[0];
            m_bCorrect = true;
            m_bRequiresArgument = true;
            break;
        case '0':
            m_cType = 'b';
            m_bCorrect = true;
            m_bRequiresArgument = false;
            break;
        default:
            m_bCorrect = false;
            break;
        }

        pos = m_sName.find("--", 0);
        if (pos != std::string::npos) {
            m_bLongOption = true;
        }

        pos = sRest.find("!");
        if (pos != std::string::npos) {
            m_bMandatory = true;
        }

        if (m_bCorrect && (m_cType != 's') && (m_cType != 'S') && (m_cType != 'b')) {
            // numeric argument name with numeric value type is a bad idea
            double dDummy;
            if (strToNum(m_sName, &dDummy)) {
                m_bCorrect = false;
            }         
        }
    } else {
        m_bCorrect = false;
    }
}

//-----------------------------------------------------------------------------
// destructor
//
OptionInfo::~OptionInfo() {
    if (m_pTemp != NULL) {
        delete[] m_pTemp;
    }
}

//-----------------------------------------------------------------------------
// setVal
//
bool OptionInfo::setVal(const std::string sNewVal) { 
    m_bSet = true;
    char *pEnd;
    if (!sNewVal.empty()) {
        m_sValue = sNewVal;
    }
    const char *pNewVal = sNewVal.c_str();
    switch (m_cType) {
    case 'c' : {
        char *pChar = (char *) m_pVariable;
        *pChar = *pNewVal;
        break;
    }
    case 's' : {
        /*
        char *pString = (char *) m_pVariable;
        strcpy(pString, pNewVal);
        break;
        */
        std::string *pStr = (std::string *) m_pVariable;
        *pStr = sNewVal;
        break;
    }
    case 'S' : {
        char **ppString = (char **) m_pVariable;
        m_pTemp = new char[strlen(pNewVal)+1];
        strcpy(m_pTemp, pNewVal);
        *ppString = m_pTemp;
        break;
    }
    case 'h' : {
        short int *pSInt = (short int *) m_pVariable;
        short int iDummy = (short int) strtol(pNewVal, &pEnd, 10);
        if (*pEnd =='\0') {
            *pSInt = iDummy;
        } else {
            m_bSet = false;
        }
        break;
    }
    case 'i' : {
        int *pInt = (int *) m_pVariable;
        int iDummy = (int) strtol(pNewVal, &pEnd, 10);
        if (*pEnd =='\0') {
            *pInt = iDummy;
        } else {
            m_bSet = false;
        }

        break;
    }
    case 'l' : {
        long *pLong = (long *) m_pVariable;
        long lDummy = strtol(pNewVal, &pEnd, 10);
        if (*pEnd =='\0') {
            *pLong = lDummy;
        } else {
            m_bSet = false;
        }
        break;
    }
    case 'd' : {
        double *pDouble = (double *) m_pVariable;
        double dDummy = strtod(pNewVal, &pEnd);
        if (*pEnd =='\0') {
            *pDouble = dDummy;
        } else {
            m_bSet = false;
        }
        break;
    }  
    case 'f' : {
        float *pFloat = (float *) m_pVariable;
        float fDummy = (float) strtod(pNewVal, &pEnd);
        if (*pEnd =='\0') {
            *pFloat = fDummy;
        } else {
            m_bSet = false;
        }
        *pFloat = (float) atof(pNewVal);
        break;
    }  
    case 'b' : {
        bool *pBool = (bool *) m_pVariable;
        if (m_bRequiresArgument) {
            *pBool =  ((strcasecmp(pNewVal, "yes")  == 0) ||
                       (strcasecmp(pNewVal, "y")    == 0) ||
                       (strcasecmp(pNewVal, "on")   == 0) ||
                       (strcasecmp(pNewVal, "true") == 0) ||
                       (strcasecmp(pNewVal, "t")    == 0) ||
                       (strcasecmp(pNewVal, "1")    == 0));
        } else {
            *pBool = true;
        } 
      break;
    }  
    default:
        m_bSet = false;
        break; 
    }
    return m_bSet;
}

typedef std::map<std::string, OptionInfo *> OPTIONLIST;

/*****************************************************************************\
 * OptionList
\*****************************************************************************/
class OptionList {
public:
    OptionList();
   ~OptionList();
   
    bool addOption(std::string sDef, void *pVar);
    OptionInfo *getOptionInfo(std::string sOption); 
    bool allMandatorySet(stringvec &vsMissing);
    bool optionOK(std::string &sOption, bool bSetCur);
    bool optionSet();
    bool requiresArgument();
    bool setValue(const std::string sValue);
    bool writeSetOptions(FILE *fOut, bool bLines, const std::string sOmit);
    void collectOptions(stringvec &vsOptions);
    uint numOptions() { return (uint)m_MapOptions.size();};
    void getMandatoryList(stringvec &vMand);
    void getUnknownList(stringvec &vUnknown);

private:
    OPTIONLIST m_MapOptions;
    OptionInfo *m_pCurOption;  
};

//-----------------------------------------------------------------------------
// constructor
//
OptionList::OptionList()
    :   m_pCurOption(NULL) {
}

//-----------------------------------------------------------------------------
// destructor
//
OptionList::~OptionList() {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {

        delete iter->second;
    }
}

//-----------------------------------------------------------------------------
// addOption
//
bool OptionList::addOption(std::string sDef, void *pVar) {
    bool bOK = false;
    OptionInfo *pOpt = new OptionInfo(sDef, pVar);
    if (pOpt->isCorrect()) {
        m_MapOptions[pOpt->getName()] = pOpt;
        bOK = true;
    }

    return bOK;
}

//-----------------------------------------------------------------------------
// getOptionInfo
//
OptionInfo *OptionList::getOptionInfo(std::string sOption) {
    return m_MapOptions[sOption];
}

//-----------------------------------------------------------------------------
// setValue
//
bool OptionList::setValue(const std::string sValue) {
    bool bOK = false;
    if (m_pCurOption != NULL) {
        bOK = m_pCurOption->setVal(sValue);
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// optionOK
//  sets m_pCurOption if sOption is valid
//
bool OptionList::optionOK(std::string &sOption, bool bSetCur) {
    OptionInfo *pDummy = m_MapOptions[sOption];
    if (bSetCur) {
        m_pCurOption = pDummy;
    }
    return pDummy != NULL;
}

//-----------------------------------------------------------------------------
// optionSet
//
bool OptionList::optionSet() {
    bool bSet = false;
    if (m_pCurOption != NULL) {
        bSet = m_pCurOption->isSet();
    }
    return bSet;
}

//-----------------------------------------------------------------------------
// requiresArgument
//
bool OptionList::requiresArgument() {
    bool bRequires = false;
    if (m_pCurOption != NULL) {
        bRequires = m_pCurOption->requiresArgument();
    }
    return bRequires;
}

//-----------------------------------------------------------------------------
// allMandatorySet
//
bool OptionList::allMandatorySet(stringvec &vsMissing) {
    bool bAllSet = true;
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
         
        if (iter->second != NULL) {
            if (iter->second->isMandatory() && !iter->second->isSet()) {
                vsMissing.push_back(iter->first);
                bAllSet = false;
            }
        }
    }
    return bAllSet;
}

//-----------------------------------------------------------------------------
// getMandatoryList
//
void OptionList::getMandatoryList(stringvec &vMand) {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
        if (iter->second != NULL) {
            if (iter->second->isMandatory()) {
                vMand.push_back(iter->second->getName());
            }
        }
    }
}

//-----------------------------------------------------------------------------
// getUnknownList
//
void OptionList::getUnknownList(stringvec &vUnknown) {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
        if (iter->second == NULL) {
            vUnknown.push_back(iter->first);
        }
    }
}

//-----------------------------------------------------------------------------
// writeSetOptions
//   bLines: write each option on a single line
//   pOmit:  comma-separated list of options to omit
//   If bLines is true, omitted options are written as comments
// 
bool OptionList::writeSetOptions(FILE *fOut, bool bLines, std::string sOmit) {
    bool bOK = true;
    
    // create a map with options to omit
    std::map<std::string, bool> mOmit;
    stringvec vParts;
    uint iNum = splitString(sOmit, vParts, " ,:;");
    if (iNum > 0) {
        for (uint i = 0; i < vParts.size(); ++i) {
            mOmit[vParts[i]] = true;
        }
    }

    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
 
        
        if (!mOmit[iter->first] || bLines) {
            if (iter->second != NULL) {
                if (iter->second->isSet()) {
                    // if omitted in lines mode: comment
                    if (mOmit[iter->first]) {
                        xha_fprintf(fOut, "#");
                    }
                    if (bLines) {
                        xha_fprintf(fOut, "  ");
                    }

                    std::string sValue = iter->second->getVal();  

                    char sSep[2];
                    if (iter->second->isLong()) {
                        strcpy(sSep, "=");
                    } else {
                        strcpy(sSep, " ");
                    }

                    char sQuote[3];
                    if ((sValue.find('|') != std::string::npos) ||
                        (sValue.find(',') != std::string::npos) ||
                        (sValue.find('#') != std::string::npos) ||
                        (sValue.find('"') != std::string::npos) ||
                        (sValue.find(':') != std::string::npos)) {
                        strcpy(sQuote, "\'");
                    } else if (sValue.find('\'') != std::string::npos) {
                        strcpy(sQuote, "\"");
                    } else {
                        sQuote[0]='\0';
                    }

                    xha_fprintf(fOut, "%s", iter->first);

                    //                    if ((iter->second->getType() != 'b') ||
                    //                        (iter->second->requiresArgument())) {
                    if (iter->second->requiresArgument()) {
                        xha_fprintf(fOut, "%s%s%s%s ", sSep, sQuote, sValue, sQuote); fflush(fOut);
                    } else if (!iter->second->requiresArgument()) {
                        xha_fprintf(fOut, " ");
                    }
                    if (bLines) {
                        xha_fprintf(fOut, "\n");
                    }
                }
            }
        }        
    }   
    //xha_fprintf(fOut, "\n"); fflush(fOut);  
    return bOK;
}

//-----------------------------------------------------------------------------
// collectOptions
//
void OptionList::collectOptions(stringvec &vsOptions) {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
 
        if (iter->second != NULL) {
            if (iter->second->isSet()) {
                std::string sLine=iter->first;
                std::string sValue = iter->second->getVal();  

                std::string sSep;
                if (iter->second->isLong()) {
                    sSep = "=";
                } else {
                    sSep = " ";
                }

                std::string sQuote;
                if ((sValue.find('|') != std::string::npos) ||
                    (sValue.find(',') != std::string::npos) ||
                    (sValue.find('#') != std::string::npos) ||
                    (sValue.find('"') != std::string::npos) ||
                    (sValue.find(' ') != std::string::npos) ||
                    (sValue.find(':') != std::string::npos)) {
                    sQuote = "\'";
                } else if (sValue.find('\'') != std::string::npos) {
                    sQuote = "\"";
                } else {
                    sQuote ="";
                }

                if ((iter->second->getType() != 'b') ||
                    (iter->second->requiresArgument())) {
                    sLine += sSep + sQuote + sValue + sQuote;
                }

                vsOptions.push_back(sLine);
            }
        }        
    }   
}



//-----------------------------------------------------------------------------
// constructor
//
ParamReader::ParamReader() 
    :   m_pOptionList(NULL),
        m_bVerbose(false),
        m_sBadArg(""),
        m_sBadVal("") {
}

//-----------------------------------------------------------------------------
// destructor
//
ParamReader::~ParamReader() {
    if (m_pOptionList != NULL) {
        delete m_pOptionList;
    }
}

//-----------------------------------------------------------------------------
// setOptions
//
bool ParamReader::setOptions(int iNumOptions, ...) {
    bool bOK = false;
    std::string sOptionString;
    void *pOptionVar;
    va_list vl;
  
    va_start(vl, iNumOptions);

    if (m_pOptionList != NULL) {
        delete m_pOptionList;
    }
    m_pOptionList = new OptionList();
    if (m_pOptionList != NULL) {
        bOK = true;
        for (int i = 0; bOK && (i < iNumOptions); i++) {
            sOptionString = va_arg(vl, char *);
            pOptionVar    = va_arg(vl, void *);
            if (m_pOptionList->addOption(sOptionString, pOptionVar)) {
            } else {
                bOK = false;
                xha_printf("%p : Error at NewOption(%s, pOptionVar)\n", this,  sOptionString);
                
                break;
            }
        }
    }
    va_end(vl);
    return bOK;
}


//-----------------------------------------------------------------------------
// getParams
//
int ParamReader::getParams(int argc, char *argv[], bool bOverwrite) {
    int iResult = PARAMREADER_OK;

    for (int i = 1; (iResult >= 0) && (i < argc); i++) {

        bool bValPresent = false;
        std::string sVal;
        std::string sArg = argv[i];
        
        if (sArg[0] == '-') {
            std::string::size_type pos = sArg.find("=", 0);
            if (pos != std::string::npos) {
                sVal = sArg.substr(pos+1);
                sArg = sArg.substr(0, pos);
                bValPresent = true;
            }

            if (m_pOptionList->optionOK(sArg, true)) {
                if (!m_pOptionList->optionSet() || bOverwrite) {
                    bool bVarSet = false;
                    if (m_pOptionList->requiresArgument()) {
                        if (sArg[1] == '-') {
                            if (!bValPresent) {
                                if (m_bVerbose) {
                                    xha_printf("Missing parameter for option %s\n", sArg);
                                }
                                iResult = PARAMREADER_ERR_MISSING_PARAM;
                                m_sBadArg = sArg;
                                m_sBadVal = "";
                            }
                        } else {
                            if ((i+1) >= argc) {
                                if (m_bVerbose) {
                                    xha_printf("Missing parameter for option %s\n", sArg);
                                }
                                iResult = PARAMREADER_ERR_MISSING_PARAM;
                                m_sBadArg = sArg;
                                m_sBadVal = "";
                            }  else if (argv[i+1][0] == '-') {
                                std::string sTemp = argv[i+1];
                                if (m_pOptionList->optionOK(sTemp, false)) {
                                    if (m_bVerbose) {
                                        xha_printf("Missing parameter for option %s\n", sArg);
                                    }
                                    iResult = PARAMREADER_ERR_MISSING_PARAM;
                                    m_sBadArg = sArg;
                                    m_sBadVal = "";
                                } else {
                                    // we have a value starting with '-' (negative number?)
                                    sVal = argv[++i];
                                }
                            } else {
                                sVal = argv[++i];
                            }
                        }
                        bVarSet = m_pOptionList->setValue(sVal);
                    } else  {
                        bVarSet = m_pOptionList->setValue("");
                    }
                    if (!bVarSet) {
                        iResult = PARAMREADER_ERR_OPTION_SET;
                        m_sBadArg = sArg;
                        m_sBadVal = sVal;
                        if (m_bVerbose) {
                            xha_printf("Error setting option %s to %s\n", sArg, sVal);
                        }
                    }
                }
            } else {
                // unknown option
                if (m_bVerbose) {
                    xha_printf("Unknown Option %s\n", sArg);
                }
                iResult |= PARAMREADER_ERR_UNKNOWN_OPTION;
                m_sBadArg = sArg;
                m_sBadVal = sVal;
            }

        } else {
            if (m_bVerbose) {
                xha_printf("expected '-' (instead of %s)\n", sArg);
            }
            m_vFreeParams.push_back(sArg);
            iResult |= PARAMREADER_ERR_FREE_PARAMS;
        }
    }
  

    if (iResult >= 0) {
        if (!m_pOptionList->allMandatorySet(m_vMissingManadatory)) {
            if (m_bVerbose) {
                xha_printf("Not all mandatory arguments set\n");
            }
            iResult = PARAMREADER_ERR_MANDATORY_MISSING;
        }
    }
  
  
    return iResult;
}
/*
//-----------------------------------------------------------------------------
// getParams
//
int ParamReader::getParams(char *pConfigFile) {
    int iResult = 0;
    char sLine[MAX_LINE];
    char sLine1[MAX_LINE];
 
    FILE *fIn = fopen(pConfigFile, "rt");
    if (fIn != NULL) {
        // count words in file
        int iNum = 0;
        char *p1 = fgets(sLine1, MAX_LINE, fIn);
        strcpy(sLine, sLine1);
 
        char *p11 = strtok(p1, " \t");
        while (p11 != NULL) {
            ++iNum;
            p11 = strtok(NULL, " \t");
        }

        char **ppArgs = new CHARP[iNum+1];
        if (ppArgs != NULL) {
            int iCount = 0;
            ppArgs[iCount++] = NULL;
            char *p = sLine;
            if (p != NULL) {
                char *pBuf;
                char *p0 = strtok_r(sLine, " \t", &pBuf);
                while (p0 != NULL) {
                    ppArgs[iCount++] = p0;
                    p0 = strtok_r(NULL, " \t", &pBuf);
                }
                iResult = getParams(iCount, ppArgs);
            } else {
                iResult = -1;
            }
            fclose(fIn);
        }
        delete[] ppArgs;
    } else {
        iResult = -1;
    }
    
    return iResult;
}
*/

//-----------------------------------------------------------------------------
// countWords
//
int ParamReader::countWords(const std::string sConfigFile) {
    int iNumWords = 0;
    char sLine[MAX_LINE];
 
    FILE *fIn = fopen(sConfigFile.c_str(), "rt");
    if (fIn != NULL) {
        while (!feof(fIn)) {
            // count words in file
            char *p = fgets(sLine, MAX_LINE, fIn);
            if (p != NULL) {
                if (*p != '#') {
                    char *p11 = strtok(p, " \t\n");
                    while (p11 != NULL) {
                        ++iNumWords;
                        p11 = strtok(NULL, " \t\n");
                    }
                }
            }
        }
        fclose(fIn);
    }
    return iNumWords;
}

//-----------------------------------------------------------------------------
// getParams
//
int ParamReader::getParams(const std::string sConfigFile, bool bOverwrite) {
    int iResult = 0;
    char sLine[MAX_LINE];
    int iNumWords = countWords(sConfigFile);

    char **ppArgs = new char*[iNumWords+1];  // first word: appName
    if (ppArgs != NULL) {
        int iCount = 0;
        ppArgs[iCount++] = NULL;
 
        FILE *fIn = fopen(sConfigFile.c_str(), "rt");
        if (fIn != NULL) {
            while (!feof(fIn)) {
                // count words in file
                char *p = fgets(sLine, MAX_LINE, fIn);
                if (p != NULL) {
                    if (*p != '#') {
                        char *pBuf;
                        char *p0 = strtok_r(p, " \t\n", &pBuf);
                        while (p0 != NULL) {
                            ppArgs[iCount++] = strdup(p0);
                            p0 = strtok_r(NULL, " \t\n", &pBuf);
                        }
                    }
                }
            }
            fclose(fIn);
           
            iResult = getParams(iCount, ppArgs, bOverwrite);

        } else {
            iResult = PARAMREADER_ERR_BAD_CONFIG_FILE;
            m_sBadVal = sConfigFile;
        }
        // free stuff
        for (int i = 0; i < iNumWords; ++i) {
            free(ppArgs[i+1]);
        }
        delete[] ppArgs;
    } else {
        iResult = -1;
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// getMandatoryParams
//
uint ParamReader::getMandatoryParams(stringvec &vMand) {
    m_pOptionList->getMandatoryList(vMand);
    return (uint)vMand.size();
}

//-----------------------------------------------------------------------------
// getUnknownParams
//
uint ParamReader::getUnknownParams(stringvec &vUnknown) {
    m_pOptionList->getUnknownList(vUnknown);
    return (uint)vUnknown.size();
}

//-----------------------------------------------------------------------------
// getFreeParams
//
uint ParamReader::getFreeParams(stringvec &vFree) {
    vFree.clear();
    vFree.insert(vFree.end(), m_vFreeParams.begin(), m_vFreeParams.end());
    return (uint)vFree.size();
}

//-----------------------------------------------------------------------------
// writeConfigFile
//   pOmit:  comma-separated list of options to omit
//
bool ParamReader::writeConfigFile(const std::string sConfigFile, const std::string sOmit) {
    bool bOK = false;
    
    //char *pOmit2 = strdup(pOmit);
    FILE * fOut = fopen(sConfigFile.c_str(), "wt");
    if (fOut != NULL) {
        bOK = m_pOptionList->writeSetOptions(fOut, true, sOmit);
        fclose(fOut);
    }
    //  free(pOmit2);
    return bOK;
}

//-----------------------------------------------------------------------------
// collectOptions
//
void ParamReader::collectOptions(stringvec &vsOptions) {
    m_pOptionList->collectOptions(vsOptions);
}

//-----------------------------------------------------------------------------
// display
//
void ParamReader::display(const std::string sOmit) {
    m_pOptionList->writeSetOptions(stdout, true, sOmit);
}

//-----------------------------------------------------------------------------
// display
//
void ParamReader::display(FILE *fOut, bool bLines, const std::string sOmit) {
    m_pOptionList->writeSetOptions(fOut, bLines, sOmit);
}

//-----------------------------------------------------------------------------
// getErrorMessage
//
std::string ParamReader::getErrorMessage(int iResult) {
    int iNum = 0;
    std::string sNum;
    stringvec vVals;
    m_sErrorMessage = "";

    if (iResult < 0) {
        
        switch (iResult) {
        case  PARAMREADER_ERR_MANDATORY_MISSING:
            if (m_sErrorMessage != "") {
                m_sErrorMessage += "\n";
            }
            iNum = m_vMissingManadatory.size();
            sNum = xha_sprintf("[ParamReader Error] %d mandatory option%s missing: ", iNum, (iNum != 1)?"s":"");
            m_sErrorMessage += sNum;
            for (int i = 0; i < iNum; ++i) {
                m_sErrorMessage += " "+m_vMissingManadatory[i];
            }

            break;
        case PARAMREADER_ERR_MISSING_PARAM:
            m_sErrorMessage = "[ParamReader Error] option parameter missing: " +m_sBadArg;
            break;
        case PARAMREADER_ERR_OPTION_SET:
            m_sErrorMessage = "[ParamReader Error] bad option value: " + m_sBadArg +" ("+m_sBadVal +")";
            break;
        case PARAMREADER_ERR_BAD_CONFIG_FILE:
            m_sErrorMessage = "[ParamReader Error] config file doesn't exist: " + m_sBadVal;
            break;
        default:
            sNum = xha_sprintf("[ParamReader Error] Unknown error (%d)", iResult);
            m_sErrorMessage = sNum;
        }
    } else {
        if (iResult == PARAMREADER_OK) {
            m_sErrorMessage = "[ParamReader OK]";
        } else {
            if ((iResult & PARAMREADER_ERR_UNKNOWN_OPTION) != 0) {
                if (m_sErrorMessage != "") {
                    m_sErrorMessage += "\n";
                }
                iNum =  getUnknownParams(vVals);
                sNum = xha_sprintf("[ParamReader Warning] %d unknown option%s: ", iNum, (iNum != 1)?"s":"");
                m_sErrorMessage += sNum;
                for (int i = 0; i < iNum; ++i) {
                    m_sErrorMessage = m_sErrorMessage + " "+vVals[i];
                }
            }
            if ((iResult & PARAMREADER_ERR_FREE_PARAMS) != 0) {
                if (m_sErrorMessage != "") {
                    m_sErrorMessage += "\n";
                }
                iNum =  getFreeParams(vVals);
                sNum = xha_sprintf("[ParamReader Warning] %d free param%s: ", iNum, (iNum != 1)?"s":"");
                m_sErrorMessage += sNum;
                for (int i = 0; i < iNum; ++i) {
                    m_sErrorMessage += " "+vVals[i];
                }
            }
        }
        if (m_sErrorMessage == "") { 
            sNum = xha_sprintf("[ParamReader Warnubg] Unknown warning (%d)", iResult);
            m_sErrorMessage = sNum;
        }
    }
    return m_sErrorMessage;
}



 
/* 
int main_test(int iArgC,char *apArgV[]) {
    int i;
    float f;
    bool b1=false;
    bool b2=false;
    char sDada[100];
/ *    
    OptionList *pOL = new OptionList();
    pOL->addOption("-a:i", &i);
    pOL->addOption("--asdasd:s", sDada);
    pOL->addOption("--fgh:f!", &f);
    pOL->addOption("--set-on:b", &b1);
    pOL->addOption("-h:0", &b2);

    pOL->setValue("--fgh", "3.7");
    pOL->setValue("-h", "3.7");
    pOL->setValue("--set-on", "false");
    pOL->setValue("--asdasd", "zutrew");
    pOL->setValue("-a", "4");
* /
    if (GetParamsExt(iArgC, apArgV, 5, 
                                    "-a:i", &i,
                                    "--aaa:i", &i,
                                    "--asdasd:s", sDada,
                                    "--fgh:f!", &f,
                                    "--set-on:b", &b1,
                                    "-h:0", &b2) >= 0) {
                                        
                                            
        xha_printf("i : %d\n", i);
        xha_printf("f : %f\n", f);
        xha_printf("b1 : %d\n", b1);
        xha_printf("b2 : %d\n", b2);
        xha_printf("sDada : %s\n", sDada);
    } else {
        xha_printf("babababa\n");
        
    }
}
*/
