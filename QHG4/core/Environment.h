#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

class SCellGrid;

class Environment {
public:

    Environment(SCellGrid *pCG) : m_pCG(pCG) {};

    void setCellGrid(SCellGrid *pCG) { m_pCG = pCG;};

    bool m_bUpdated;
    uint m_iNumCells;

    SCellGrid *m_pCG;

    virtual void resetUpdated()=0;
};


#endif

