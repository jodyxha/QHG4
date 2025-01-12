/*============================================================================
| MessLogger
| 
|  A singleton for writing messages of different types to file.
|
|  
|  Author: Jody Weissmann
\===========================================================================*/ 
#ifndef __MESSLOGGERT_H__
#define __MESSLOGGERT_H__

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <sys/types.h>
#include <unistd.h>

#include "types.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "MessLogger.h"

/*
//----------------------------------------------------------------------------
// mprintf
//  writes message in printf style
//
template<typename... Args>
void MessLogger::mprintf(const std::string sFormat, Args... args) {
#ifndef MUTE

    std::string sLine = xha_sprintf(sFormat.c_str(), args...);

    if (s_pML != NULL) {
        s_pML->write(sLine, "", "");
    }
#endif
}*/

//----------------------------------------------------------------------------
// logStatus
//  writes status message in printf style
//
template<typename... Args>
void MessLogger::logStatus(const  std::string sFormat, Args... args) {
#ifndef MUTE
    MessLogger::logSpecial(MessLogger::s_sCurHeaderStatus, MessLogger::s_sCurColorStatus, sFormat, args...);
#endif
    MessLogger::incNumStatus();
}

//----------------------------------------------------------------------------
// logWarning
//  writes warning message in printf style
//
template<typename... Args>
void MessLogger::logWarning(const  std::string sFormat, Args... args) {
#ifndef MUTE
    MessLogger::logSpecial(MessLogger::s_sCurHeaderWarning, MessLogger::s_sCurColorWarning, sFormat, args...);
#endif
    MessLogger::incNumWarning();
}


//----------------------------------------------------------------------------
// logError
//  writes error message in printf style
//
template<typename... Args>
void MessLogger::logError(const  std::string sFormat, Args... args) {
#ifndef MUTE
    MessLogger::logSpecial(MessLogger::s_sCurHeaderError, MessLogger::s_sCurColorError, sFormat, args...);
#endif
    MessLogger::incNumError();
}

//----------------------------------------------------------------------------
// logDisp
//  writes specially colored liner in printf style
//
template<typename... Args>
void MessLogger::logDisp(const  std::string sFormat, Args... args) {
#ifndef MUTE
    MessLogger::logSpecial(MessLogger::s_sCurHeaderDisp, MessLogger::s_sCurColorDisp, sFormat, args...);
#endif
    MessLogger::incNumDisp();
}

//----------------------------------------------------------------------------
// logSpecial
//  
//
template<typename... Args>
void MessLogger::logSpecial(const std::string sHeader, const std::string sColor, const std::string sFormat, Args... args) {
#ifndef MUTE
  
    std::string sLine = sHeader;

    sLine += xha_sprintf(sFormat.c_str(), args...);

    if (s_pML != NULL) {
        s_pML->write(sLine, sColor, MessLogger::s_sCurColorStop);
    }
#endif  
}

/*
//----------------------------------------------------------------------------
// logIgnore
//  does nothing
//
template<typename... Args>
void MessLogger::logIgnore(const std::string sFormat, Args... args) {
    xha_printf("ignore\n");
}
*/

//----------------------------------------------------------------------------
// logStatus2
//  write status message to file and to stdout
//
template<typename... Args>
void MessLogger::logStatus2(const std::string sFormat, Args... args) {
    xha_printf(sFormat, args...);
    logStatus(sFormat, args...);
}

//----------------------------------------------------------------------------
// logWarning2
//  write warning message to file and to stdout
//
template<typename... Args>
void MessLogger::logWarning2(const std::string sFormat, Args... args) {
    xha_printf(sFormat, args...);
    logWarning(sFormat, args...);
}

//----------------------------------------------------------------------------
// logError2
//  write error message to file and to stdout
//
template<typename... Args>
void MessLogger::logError2(const std::string sFormat, Args... args) {
    xha_printf(sFormat, args...);
    logError(sFormat, args...);
}

//----------------------------------------------------------------------------
// logDisp2
//  write display message to file and to stdout
//
template<typename... Args>
void MessLogger::logDisp2(const std::string sFormat, Args... args) {
    xha_printf(sFormat, args...);
    logDisp(sFormat, args...);
}


#endif
