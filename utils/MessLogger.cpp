#include <fstream>
#include <iostream>

#include "types.h"
#include "colors.h"
#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "MessLogger.h"


const static std::string HEADER_STATUS  = "S:";
const static std::string HEADER_WARNING = "W:";
const static std::string HEADER_ERROR   = "E:";
const static std::string HEADER_DISP    = "D:";


const static std::string COL_STATUS  = colors::GREEN;
const static std::string COL_WARNING = colors::BLUE;
const static std::string COL_ERROR   = colors::RED;
const static std::string COL_DISP    = colors::PURPLE;
const static std::string COL_STOP    = colors::OFF;


std::string  _STDOUT = "(stdout)";



MessLogger  *MessLogger::s_pML=NULL;
std::string  MessLogger::s_sLogName;
stringvec    MessLogger::s_vFirstMessages;


std::string MessLogger::s_sCurHeaderStatus  = HEADER_STATUS;
std::string MessLogger::s_sCurHeaderWarning = HEADER_WARNING;
std::string MessLogger::s_sCurHeaderError   = HEADER_ERROR;
std::string MessLogger::s_sCurHeaderDisp    = HEADER_DISP;
std::string MessLogger::s_sCurColorStatus   = COL_STATUS;
std::string MessLogger::s_sCurColorWarning  = COL_WARNING;
std::string MessLogger::s_sCurColorError    = COL_ERROR;
std::string MessLogger::s_sCurColorDisp     = COL_DISP;
std::string MessLogger::s_sCurColorStop     = COL_STOP;



MessLogger::MessLogger(const std::string sName) 
    : m_fOut(sName, std::ofstream::out),
      m_iNumStatus(0),
      m_iNumWarnings(0),
      m_iNumErrors(0),
      m_iNumDisp(0) {
 
}

MessLogger::~MessLogger() {
    if (m_fOut.is_open()){
        m_fOut.close();
    }
}


//----------------------------------------------------------------------------
// create
//   creates a MessLogger object writing to the specified file.
//   if NULL is passed for pName, output goes to stdout
//   
//   should be called somewhere at the start of the program
//
MessLogger *MessLogger::create(const std::string sName) {
    if (s_pML == NULL) {
        std::ofstream fOut(sName, std::ofstream::out);
        
        if (fOut.good()) {
            fOut.close();
            s_sLogName = sName;
            stdprintf("Log name: [%s]\n", s_sLogName);
            s_pML = new MessLogger(sName);
        } else {
            stdprintf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ couldn't open [%s]\n", (sName.empty())?"stdout":sName);
        }
    }
    return s_pML;
}

//----------------------------------------------------------------------------
// free
//  deletes the MessLogger
//
//  should be called somewhere at the end of the program
//
void MessLogger::free() {
    if (s_pML != NULL) {
        delete s_pML;
    }
    s_pML = NULL;
}


//----------------------------------------------------------------------------
// write
//  writes the line to file
//
void MessLogger::write(const std::string sLine0, const std::string sPre, const std::string sPost) {


    std::string sLine = sLine0;
    if (endsWith(sLine, "\n")) {
        sLine.pop_back();
    }
    sLine = stdsprintf("%s%s%s", sPre, sLine, sPost);

    if (!endsWith(sLine, "\n")) {
            sLine += "\n";
    }

    // colors for stdout
    //    if (m_fOut == stdout) {
    //    sLine += sPost;
    //}
    

    
    if (s_vFirstMessages.size() < NUM_MESS) {
        std::string sTemp = sLine0;
        strReplace(sTemp, "\n", "");
        s_vFirstMessages.push_back(sTemp);
   }

    //fputs(sLine.c_str(), m_fOut);
    //fflush(m_fOut);
    m_fOut << sLine;
    m_fOut.flush();
}

//----------------------------------------------------------------------------
// showLog
//  displays a colored log file
//  use a or-ed combination of SHOW_STATUS, SHOW_WARNING, and SHOW_ERROR,
//  or use SHOW_ALL to see all messages
//
void MessLogger::showLog(int iWhat) {
    showLog(s_sLogName, iWhat);
}

//----------------------------------------------------------------------------
// showLog
//  displays a colored log file
//  use a or-ed combination of SHOW_STATUS, SHOW_WARNING, and SHOW_ERROR,
//  or use SHOW_ALL to see all messages
//
void MessLogger::showLog(const std::string sLogFile, int iWhat) {
    stdprintf("\n\n%s------------------------------------------------%s\n", MessLogger::s_sCurColorDisp, MessLogger::s_sCurColorStop);
    if (sLogFile != _STDOUT) {
        std::ifstream fIn(sLogFile, std::ifstream::in);
        if (fIn .good()) {
            stdprintf("%sShowing contents of [%s]%s\n\n", MessLogger::s_sCurColorDisp, sLogFile, MessLogger::s_sCurColorStop);
            std::string sLine;
            while (std::getline(fIn, sLine)) {        
                coloredLine(sLine, iWhat);
            }
            fIn.close();
        } else {
            stdprintf("%s[MessLogger::showLog] couldn't open  [%s] for reading%s\n", MessLogger::s_sCurColorError, sLogFile, MessLogger::s_sCurColorStop);
        }
    } else {
        stdprintf("%sShowing contents of Buffer%s\n\n", MessLogger::s_sCurColorDisp, MessLogger::s_sCurColorStop);

        for (uint i = 0; i < s_vFirstMessages.size(); i++) {
            coloredLine(s_vFirstMessages[i], iWhat);
        }
    }
    stdprintf("%s------------------------------------------------%s\n\n", MessLogger::s_sCurColorDisp, MessLogger::s_sCurColorStop);
}


//----------------------------------------------------------------------------
// coloredLine
//  print a line colored depending on the header (first 2 characters)
//
void  MessLogger::coloredLine(std::string sLine, int iWhat) {
    std::string sPre;
    std::string sPost = MessLogger::s_sCurColorStop;

    bool bShow = (iWhat == MessLogger::SHOW_ALL);
    if (!sLine.empty()) {
        if (!MessLogger::s_sCurHeaderStatus.empty() && (startsWith(sLine, MessLogger::s_sCurHeaderStatus) || startsWith(sLine.substr(7), MessLogger::s_sCurHeaderStatus)))  {
            bShow = (iWhat & MessLogger::SHOW_STATUS);
            sPre  = MessLogger::s_sCurColorStatus;
        } else if (!MessLogger::s_sCurHeaderWarning.empty() && (startsWith(sLine, MessLogger::s_sCurHeaderWarning) || startsWith(sLine.substr(7), MessLogger::s_sCurHeaderWarning))) {
            bShow = (iWhat & MessLogger::SHOW_WARNING);
            sPre  = MessLogger::s_sCurColorWarning;
        } else if (!MessLogger::s_sCurHeaderError.empty() && (startsWith(sLine, MessLogger::s_sCurHeaderError)  || startsWith(sLine.substr(7), MessLogger::s_sCurHeaderError))) {
            bShow = (iWhat & MessLogger::SHOW_ERROR);
            sPre  = MessLogger::s_sCurColorError;
        } else if (!MessLogger::s_sCurHeaderDisp.empty() && (startsWith(sLine, MessLogger::s_sCurHeaderDisp) || startsWith(sLine.substr(7), MessLogger::s_sCurHeaderDisp))) {
            bShow = (iWhat & MessLogger::SHOW_DISP);
            sPre  = MessLogger::s_sCurColorDisp;
        } else {
            bShow = true;
            //sPre  = colors::BLINK;
            sPre  = "";
        }
        if (bShow) {
            strReplace(sLine, "\n", "");
            stdprintf("%s%s%s\n", sPre, sLine, sPost);
        }
    }
}


void MessLogger::useColors(bool bUseColors) { 
    
    if (bUseColors) {
        MessLogger::s_sCurColorStatus  = COL_STATUS;
        MessLogger::s_sCurColorWarning = COL_WARNING;
        MessLogger::s_sCurColorError   = COL_ERROR;
        MessLogger::s_sCurColorDisp    = COL_DISP;
        MessLogger::s_sCurColorStop    = COL_STOP;
    } else {
        
        MessLogger::s_sCurColorStatus  = "";
        MessLogger::s_sCurColorWarning = "";
        MessLogger::s_sCurColorError   = "";
        MessLogger::s_sCurColorDisp    = "";
        MessLogger::s_sCurColorStop    = "";
    }
}

void MessLogger::usePrefix(bool bUsePrefix) { 
    if (bUsePrefix) {
        MessLogger::s_sCurHeaderStatus  = HEADER_STATUS;
        MessLogger::s_sCurHeaderWarning = HEADER_WARNING;
        MessLogger::s_sCurHeaderError   = HEADER_ERROR;
        MessLogger::s_sCurHeaderDisp    = HEADER_DISP;
    } else {
        MessLogger::s_sCurHeaderStatus  = "";
        MessLogger::s_sCurHeaderWarning = "";
        MessLogger::s_sCurHeaderError   = "";
        MessLogger::s_sCurHeaderDisp    = "";
    }
}
