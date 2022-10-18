#ifndef __GROUPINTERFACE_H__
#define __GROUPINTERFACE_H__

class GroupInterface {
public:
    virtual int setGroupID(int iGroupID, idtype iAgentIdx) = 0;

    virtual int setParent(int iChildIdx, int iParentIdx) = 0;
};

#endif
