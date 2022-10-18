#ifndef __GRIDSCRAMBLER_H__
#define __GRIDSCRAMBLER_H__

class WELL512;
class Permutator;
class SCellGrid;

class GridScrambler {
public:
    static GridScrambler *createInstance(SCellGrid *pCG, int iSeed);
    ~GridScrambler();
    int scrambleConnections();

protected:
    GridScrambler(SCellGrid *pCG);
    int init(int iSeed);

    SCellGrid   *m_pCG;
    WELL512    **m_apWELLs;
    Permutator  *m_pPerm;
};


#endif

