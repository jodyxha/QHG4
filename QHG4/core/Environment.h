#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__


class Environment {
public:

    bool m_bUpdated;
    uint m_iNumCells;

    virtual void resetUpdated()=0;
};


#endif

