/*============================================================================
| strutils
| 
|  Some string utilities. Amongst others,
|  - strToNum(): string to number conversion (for all number types)
|  - putMem(): copying any data to uchar buffer (e.g. writing binary files)
|  - getMem(): copying from buffer to variable (e.g. reading binary files)
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cctype>
#include <cstring>
#include <cstdlib>

// for searchFile
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
//#include <unistd>


#include "strutils.h"

//-----------------------------------------------------------------------------
// trim
//   left & right trim
//
char *trim(char *pString) {
    char *p = pString;

    while (isspace(*p) && (*p != '\0')) {
        ++p;
    }

    char *q = p+strlen(p)-1;
    while (isspace(*q) && (q > p)) {
        *q = '\0';
        --q;
    }
    return p; 
}

//-----------------------------------------------------------------------------
// readHex
//   read a hex number from a string
//
bool readHex(char *pString, int *pNum) {
    *pNum = 0;
    if (strstr(pString, "0x") == pString) {
        pString += 2;
    }
    while ((*pString != '\0') && isxdigit(*pString)) {
        *pNum *= 16;
        if (isdigit(*pString)) {
            *pNum += *pString - '0';
        } else if (*pString < 'a') {
            *pNum += 10+*pString -'A';
        } else {
            *pNum += 10+*pString -'a';
        }
        ++pString;
    }
    
    return (*pString != '\0') ||  !isxdigit(*pString);

}

//-----------------------------------------------------------------------------
// niceDir
//
char *niceDir(const char *pDir, char *pNice) {
    strcpy(pNice, pDir);
    if (pNice[strlen(pNice)-1] != '/') {
        strcat(pNice, "/");
    }
    return pNice;
}

//-----------------------------------------------------------------------------
// searchFile
//   if file exists in local dir return this name
//   else if exists in DataDir return DATADIR/file
//        else return ""      
//
char *searchFile(const char *pFile, const char *pDataDir, char *pFinal) {
    struct stat buf; 
    int iResult = stat(pFile, &buf);
    if (iResult == 0) {
        strcpy(pFinal, pFile);
    } else {
        if (pDataDir != NULL) {
            char sDir[256];
            sprintf(pFinal, "%s%s", niceDir(pDataDir, sDir), pFile);
            iResult = stat(pFinal, &buf);
        }
        if (iResult != 0) {
            strcpy(pFinal, pFile);
        }
    }
    return pFinal;
}

//-----------------------------------------------------------------------------
// fillStringDouble
//
char *fillStringDouble(const char *pName, double dVal, char *pBuffer) {
    uint k = (uint)strlen(pName)+1;
    memcpy(pBuffer, (char *) &k, sizeof(uint));
    pBuffer+=sizeof(uint);
    strcpy(pBuffer, pName);
    pBuffer+= k;

    memcpy(pBuffer, (char *) &dVal, sizeof(double));
    pBuffer+=sizeof(double);
    return pBuffer;
}

//-----------------------------------------------------------------------------
// splitSizeString
//
bool splitSizeString(char *pSize, int *piW, int *piH) {
    bool bOK = false;
    char *pSep = strchr(pSize, 'x');
    if (pSep != NULL) {
        *pSep = '\0';
        pSep++;
        bOK = true;
        char *pEnd;
        *piW = (int)strtol(pSize, &pEnd, 10);
        if ((*pEnd != '\0') || (*pSize == '\0')) {
            *piW = -1;
            bOK = false;
        }
        *piH = (int)strtol(pSep, &pEnd, 10);
        if ((*pEnd != '\0') || (*pSep == '\0')) {
            *piH = -1;
            bOK = false;
        }
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// splitSizeString
//
bool splitSizeString(const char *pSize, int *piW, int *piH) {
    bool bOK = false;
    char *sSize = new char[strlen(pSize)+1];
    strcpy(sSize, pSize);
    bOK = splitSizeString(sSize, piW, piH);
    delete[] sSize;
    return bOK;
}

//-----------------------------------------------------------------------------
// splitRangeString
//
bool splitRangeString(char *pRange, int *piMin, int *piMax) {
    bool bOK = false;

    char *pSep = strchr(pRange, ':');
    if (pSep != NULL) {
        *pSep = '\0';
        pSep++;

        if (strToNum(pRange, piMin)) {
            if (strToNum(pSep, piMax)) {
                bOK = true;
            }
        }
    }
    return bOK;
}


//-----------------------------------------------------------------------------
// splitRangeString
//
bool splitRangeString(char *pRange, double *pdMin, double *pdMax) {
    bool bOK = false;

    char *pSep = strchr(pRange, ':');
    if (pSep != NULL) {
        *pSep = '\0';
        pSep++;
        if (strToNum(pRange, pdMin)) {
            if (strToNum(pSep, pdMax)) {
                bOK = true;
            }
        }
    }
    return bOK;
}


//-----------------------------------------------------------------------------
// splitRangeString
//
bool splitRangeString(char *pRange, float *pfMin, float *pfMax) {
    bool bOK = false;

    double dMin;
    double dMax;
    bOK = splitRangeString(pRange, &dMin, &dMax);
    if (bOK) {
        *pfMin = (float) dMin;
        *pfMax = (float) dMax;
    }
    return bOK;
}

void safeStrCpy(char *pDest, const char *pSource, int iLen) {
    strncpy(pDest, pSource, iLen);
    if ((int)strlen(pSource) >= iLen) {
        pDest[iLen-1] = '\0';
    }
}

bool strToNum(const char *pData, char *c) {
    char *pEnd;
    *c = (char) strtol(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, short int *s) {
    char *pEnd;
    *s = (short int) strtol(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, uint *u) {
    char *pEnd;
    *u = (uint) strtol(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, int *i) {
    char *pEnd;
    *i = (int) strtol(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, long *l) {
    char *pEnd;
    *l = (long) strtol(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, ulong *l) {
    char *pEnd;
    *l = (ulong) strtol(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, long long *l) {
    char *pEnd;
    *l = (long long) strtoll(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, float *f) {
    char *pEnd;
    *f = (float) strtod(pData, &pEnd);
    return (*pEnd == '\0');
}

bool strToNum(const char *pData, double *d) {
    char *pEnd;
    *d = (double) strtod(pData, &pEnd);
    return (*pEnd == '\0');
}
bool strToNum(const char *pData, ssize *s){
    char *pEnd;
    *s = (ssize) strtol(pData, &pEnd, 10);
    return (*pEnd == '\0');
}

//-----------------------------------------------------------------------------
// show
//
void  show(uchar *p, int iSize, const char *pCaption) {
    int i = 0;
    printf("%s\n", pCaption);
    while (i < iSize) {
        for (int col = 0; (i < iSize) && (col < 2); col++) {
            if (col == 0) {
                printf("  %08x: ", i);
            }
            for (int j = 0; (i < iSize) && (j < 8); j++) {
                printf("%02x ", p[i++]);
            }
            if ((i < iSize) && (col == 0)) {
                printf("- ");
            }
        }
        if (i < iSize) {
            printf("\n");
        }
    }
    
    printf("\n");
}

//-----------------------------------------------------------------------------
// putMem
//
uchar *putMem(uchar *p, const void *pData, ulong iSize) {
    memcpy(p, pData, iSize);
    return p+iSize;
}

//-----------------------------------------------------------------------------
// getMem
//
uchar *getMem(void *pData, const uchar *p, ulong iSize) {
    memcpy(pData, p, iSize);
    return (uchar *)(p+iSize);
}

//-----------------------------------------------------------------------------
// putMem
//
char *putMem(char *p, const void *pData, ulong iSize) {
    memcpy(p, pData, iSize);
    return p+iSize;
}

//-----------------------------------------------------------------------------
// getMem
//
char *getMem(void *pData, const char *p, ulong iSize) {
    memcpy(pData, p, iSize);
    return (char *) (p+iSize);
}


//-----------------------------------------------------------------------------
// readKeyString
//   gets the string value from a line of form
//   <Key> <Sep> <Val>
char *readKeyString(char *pLine, const char *pKey, const char *pSep) {
    char *pVal = NULL;
    pLine = trim(pLine);
  
    if (strstr(pLine, pKey) == pLine) {
        if ((pSep != NULL) && (*pSep != '\0')) {
            pVal = strstr(pLine, pSep);
            if (pVal != NULL) {
                pVal = trim(pVal+strlen(pSep));
            } else {
                printf("Expected '%s' between key and value [%s]\n", pSep, pLine);
                pVal = NULL;
            }
        }
    }

    return pVal;
}


//-----------------------------------------------------------------------------
// nextWord
//   returns the next word delimited by blanks, "," or ";"
//   sets pString to the character following the returned word
// (i.e. if pString becomes empty, we have reached the end of the string)
//
char *nextWord(char **pString) {
    return nextWord(pString, ",;");
}


//-----------------------------------------------------------------------------
// nextWord
//   returns the next word delimited by blanks, or any of the chars in pSep
//   sets pString to the character following the returned word
// (i.e. if pString becomes empty, we have reached the end of the string)
//
char *nextWord(char **pString, const char *pSep) {
    char *p =*pString;
    while ((*p != '\0') && (isspace(*p) || (strchr(pSep, *p) != NULL))) {
        p++;
    }
    char *p0 = p;

    while ((*p != '\0') && !(isspace(*p) ||  (strchr(pSep, *p) != NULL))) {
        p++;
    }
    *p = 0;
    p++;
    *pString = p;
    return p0;
}



//----------------------------------------------------------------------------
// defaultIfNULL
//   creates a string containing the given String, or, if it is NULL, the default
//   The caller must delete[] returned string
//
char *defaultIfNULL(const char *pGiven, const char *pDefault) {
    char *pNew = NULL;
    if (pGiven != NULL) {
        pNew =  new char[1+strlen(pGiven)];
        strcpy(pNew, pGiven);
    } else {
        pNew =  new char[1+strlen(pDefault)];
        strcpy(pNew, pDefault);
    }
    return pNew;
}


//----------------------------------------------------------------------------
// centerPad
//
char *centerPad(const char *pString, char cPad, int iSize, char *pOut) {
    memset(pOut, cPad, iSize);
    pOut[iSize+1] = '\0';
    int iOff = (iSize-strlen(pString))/2;
    memcpy(pOut+iOff, pString, strlen(pString));
    return pOut;
}
