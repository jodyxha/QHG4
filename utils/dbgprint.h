#ifndef __DBGPRINT_H__
#define __DBGPRINT_H__

#include <cstdio>
#include <cstdarg>

const int LL_NONE   = 0;
const int LL_TOP    = 1;
const int LL_INFO   = 2;
const int LL_DETAIL = 3;

void dbgprintf(int iLogLevel, int iThisLevel, const char * pFormat, ...);



#endif
