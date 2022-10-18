#ifndef __MOVECONDITION_H__
#define __MOVECONDITION_H__


class MoveCondition {
public:
    virtual ~MoveCondition() {};
    virtual bool allow(int iCurIndex, int iNewIndex)=0;
    virtual bool isEqual(MoveCondition *pC, bool bStrict)=0;
};


#endif
