#ifndef __SIMPLECONDITION_H__
#define __SIMPLECONDITION_H__

#include "MoveCondition.h"

const static uint ALLOW_NEVER     = 0;
const static uint_ALWAYS          = 1;
const static uint ALLOW_GREATER   = 2;
const static uint ALLOW_LESS      = 3;
const static uint ALLOW_EQUAL     = 4;
const static uint ALLOW_GREATEREQ = 5;
const static uint ALLOW_LESSEQ    = 6;
const static uint ALLOW_DIFFERENT = 7;

typedef bool (*condfunc)(double, double);

class SimpleCondition : public MoveCondition {
public:
    SimpleCondition(double *adRefValues, int iMode);
    virtual ~SimpleCondition();

    virtual bool allow(int iCurIndex, int iNewIndex);
    virtual bool isEqual(MoveCondition *pC, bool bStrict);
protected:
    double  *m_adRefValues;
    condfunc m_funcCondition;
    int      m_iMode;
};


#endif
