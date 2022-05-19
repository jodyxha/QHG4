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
    StatPopFactory(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);

    virtual PopBase *createPopulationByName(const std::string sClassName);
    virtual PopBase *createPopulationByID(spcid iClassID);
    virtual PopBase *readPopulation(ParamProvider2 *pPP);

protected:
    SCellGrid *m_pCG;
    PopFinder *m_pPopFinder;
    int        m_iLayerSize;
    IDGen    **m_apIDG;
    uint32_t  *m_aulState;
    uint      *m_aiSeeds;

};

#endif
