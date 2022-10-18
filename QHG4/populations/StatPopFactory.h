#ifndef __STATPOPFACTORY_H__
#define __STATPOPFACTORY_H__

#include <cstdint>

class SCellGrid;
class IDGen;
class PopFinder;
class PopBase;
class ParamProvider2;
class LineReader;

#include "PopulationFactory.h" 

class StatPopFactory : public PopulationFactory {
public:
    StatPopFactory(SCellGrid *pCG, PopFinder *pPopFInder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);

    virtual PopBase *createPopulationByName(const std::string sClassName);
    virtual PopBase *readPopulation(ParamProvider2 *pPP);

protected:

    SCellGrid *m_pCG;
    int        m_iLayerSize;
    PopFinder *m_pPopFinder;
    IDGen    **m_apIDG;
    uint32_t  *m_aulState;
    uint      *m_aiSeeds;

};

#endif
