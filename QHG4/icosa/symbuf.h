#ifndef __SYMBUF_H__
#define __SYMBUF_H__

class BufReader;

// symbols are NUM, OBR, DOT, CBR

const int SYM_ERR = 0;
const int SYM_NUL = 1;
const int SYM_NUM = 2;
const int SYM_OBR = 3;
const int SYM_DOT = 4;
const int SYM_CBR = 5;
const int SYM_LEV = 6;



#include "BufReader.h"

class symbuf {
public:
    symbuf(BufReader *pBR);
    int getNextSym();
    int getCurSym() { return m_iCurSym;};
    int getCurNum() { return m_iCurNum;};
    const char *getSymName(int iSym);
protected:
    BufReader *m_pBR;
    int   m_iCurSym;
    int   m_iCurNum;
};

#endif
