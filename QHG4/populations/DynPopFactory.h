#ifndef __DYNPOPFACTORY_H__
#define __DYNPOPFACTORY_H__

class SCellGrid;
class IDGen;
class PopFinder;
class PopBase;
class ParamProvider2;

#include <string>
#include "types.h"
#include "PopulationFactory.h"

class DynPopFactory : public PopulationFactory {
public: 
    static DynPopFactory *createInstance(stringvec vSODirs, SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    
    
    virtual PopBase *createPopulationByName(const std::string sName);
    virtual PopBase *readPopulation(ParamProvider2 *pPP);
    virtual ~DynPopFactory();

protected:
    DynPopFactory(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds) :
        m_pCG(pCG),
        m_pPopFinder(pPopFinder),
        m_iLayerSize(iLayerSize),
        m_apIDG(apIDG),
        m_aulState(aulState),
        m_aiSeeds(aiSeeds) {};

    int init(stringvec vSODirs);
    int collectPlugins();
    int collectPluginsInDir(std::string sPath);


    SCellGrid *m_pCG;
    PopFinder *m_pPopFinder;
    int        m_iLayerSize;
    IDGen    **m_apIDG;
    uint32_t  *m_aulState;
    uint      *m_aiSeeds;

    stringvec m_vSODirs;
    stringmap m_mNameFiles; 
    std::vector<void *> m_vLibHandles;
};

#endif
