/*============================================================================
| MessLogger
| 
|  A singleton for writing messages of different types to file.
|
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __MESSLOGGER_H__
#define __MESSLOGGER_H__


//#include <cstdio>
//#include <cstdarg>

#include <fstream>
#include <string>
#include "types.h"

#define LOG_STATUS   MessLogger::logStatus
#define LOG_WARNING  MessLogger::logWarning
#define LOG_DISP     MessLogger::logDisp
#define LOG_ERROR    MessLogger::logError
#define LOG_STATUS2  MessLogger::logStatus2
#define LOG_WARNING2 MessLogger::logWarning2
#define LOG_DISP2    MessLogger::logDisp2
#define LOG_ERROR2   MessLogger::logError2

const int NUM_MESS     = 128;
 

class MessLogger {
public:
    
    static MessLogger *create(const std::string sName);
    static void free();

    template<typename... Args>
    static void logStatus(const std::string sFormat, Args... args);

    template<typename... Args>
    static void logWarning(const std::string sFormat, Args... args);

    template<typename... Args>
    static void logError(const std::string sFormat, Args... args);

    template<typename... Args>
    static void logDisp(const std::string sFormat, Args... args);
    /*
    template<typename... Args>
    static void logIgnore(const std::string sFormat, Args... args);
    
    template<typename... Args>
    static void mprintf(const std::string sFormat, Args... args);
    */
    template<typename... Args>
    static void logStatus2(const std::string sFormat, Args... args);

    template<typename... Args>
    static void logWarning2(const std::string sFormat, Args... args);

    template<typename... Args>
    static void logError2(const std::string sFormat, Args... args);

    template<typename... Args>
    static void logDisp2(const std::string sFormat, Args... args);

    static void showLog(const std::string sLogFile, int iWhat);
    static void showLog(int iWhat);
    
    static int getNumStatus()  { return s_pML->m_iNumStatus;};
    static int getNumWarning() { return s_pML->m_iNumWarnings;};
    static int getNumError()   { return s_pML->m_iNumErrors;};
    static int getNumDisp()    { return s_pML->m_iNumDisp;};

    static void useColors(bool bUseColors);
    static void usePrefix(bool bUsePrefix);


    //    static FILE *getOut() { return s_pML->m_fOut;};
    static const std::string  getFile() { return s_sLogName;};

    static  const int SHOW_NONE    = 0;
    static const int SHOW_STATUS  = 1;
    static const int SHOW_WARNING = 2;
    static const int SHOW_ERROR   = 4;
    static const int SHOW_DISP    = 8;
    static const int SHOW_ALL     = (SHOW_STATUS | SHOW_WARNING | SHOW_ERROR | SHOW_DISP);

protected:
    MessLogger(const std::string);
    ~MessLogger();
    void write(const std::string sLine0, const std::string sPre, const std::string sPost);
    static void coloredLine(std::string sLine, int iWhat);

    template<typename... Args>
    static void logSpecial(const std::string sHeader, const std::string sColor, const std::string sFormat, Args... args);

    static void incNumStatus()  { s_pML->m_iNumStatus++;};
    static void incNumWarning() { s_pML->m_iNumWarnings++;};
    static void incNumError()   { s_pML->m_iNumErrors++;};
    static void incNumDisp()    { s_pML->m_iNumDisp++;};

    static std::string s_sLogName;

    std::ofstream m_fOut;
    //    FILE *m_fOut;
    static MessLogger *s_pML;

    int m_iNumStatus;
    int m_iNumWarnings;
    int m_iNumErrors;
    int m_iNumDisp;

    static std::string s_sCurHeaderStatus;
    static std::string s_sCurHeaderWarning;
    static std::string s_sCurHeaderError;
    static std::string s_sCurHeaderDisp;

    static std::string s_sCurColorStatus;
    static std::string s_sCurColorWarning;
    static std::string s_sCurColorError;
    static std::string s_sCurColorDisp;
    static std::string s_sCurColorStop;

    
    static stringvec s_vFirstMessages;
};


#endif

