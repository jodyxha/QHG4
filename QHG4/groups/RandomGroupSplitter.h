#ifndef __RANDOMGROUPSPLITTER_H__
#define __RANDOMGROUPSPLITTER_H__

#include <vector>
#include "types.h"

#include "WELL512.h"
#include "GroupSplitter.h"

class RandomGroupSplitter : public GroupSplitter  {
public:
    RandomGroupSplitter(WELL512 *pWELL, uint iMinSize, uint iMaxSize);
    virtual ~RandomGroupSplitter(){};
  
    virtual int split(intvec &vOriginal, intvec &vSplitOff);
protected:
    WELL512 *m_pWELL;
    uint m_iMinSize;
    uint m_iMaxSize;

};

#endif
