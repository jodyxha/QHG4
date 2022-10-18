#ifndef __POPLOOPER_H__
#define __POPLOOPER_H__

#include <string>
#include <vector>
#include <map>
#include "types.h"
#include "PopFinder.h"

class PopBase;

static const int DEF_CHUNK=16;

typedef std::map<int, PopBase *> popmap;
typedef std::vector<PopBase *>   popvec;

class PopLooper : public PopFinder {
public:
    PopLooper();

    virtual ~PopLooper();

    // unused    int checkMerge(PopBase *pPop);
    int tryMerge();
    int addPop(PopBase *pPop);
    PopBase *removePopByName(const std::string sSpeciesName, bool bAddToExtinct);
    PopBase *removePopByIndex(int iIndex, bool bAddToExtinct);

    virtual int  preLoop();
    virtual int  postLoop();
    virtual int  preWrite(float fTime);

    int doStep(float fStep);

    size_t  getNumPops() { return m_mP.size();};
    //    const popvec  &getPops() { return m_vP;};
    //    const popmap  &getMap() { return m_mP;};
    const popvec  &getDead() { return m_vExtinctPops;};


    idtype  getMaxID() { return m_iMaxID;};

    popmap::iterator begin() {return m_mP.begin();};
    popmap::iterator end()   {return m_mP.end();};

    double dTimeActions;
    double dTimeFinalize;
    

    // PopFinder implementation
    virtual PopBase *getPopByName(const std::string sSpeciesName);

protected:

    std::set<uint> m_vPrioLevels;
    int m_iChunkSize;
    popmap m_mP;

    popvec m_vExtinctPops;
    idtype m_iMaxID;

    int m_iCurIndex;
 
};
#endif
