#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <sys/types.h>

#include "dbgprint.h"

//----------------------------------------------------------------------------
// dbgprintf
//  writes message in printf style
//
void dbgprintf(int iLogLevel, int iThisLevel, const char * pFormat, ...) {
    va_list vl;

    if (iLogLevel > iThisLevel) {
        va_start(vl, pFormat);
        
        vprintf(pFormat, vl);
        va_end(vl);
    }
}
