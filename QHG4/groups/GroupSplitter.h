#ifndef __GROUPSPLITTER_H__
#define __GROUPSPLITTER_H__

#include <vector>
#include "types.h"

class GroupSplitter {
public:
    virtual int split(intvec &vOriginal, intvec &vSplitOff) = 0;
};

#endif
