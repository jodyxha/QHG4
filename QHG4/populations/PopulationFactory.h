#ifndef __POPULATIONFACTORY_H__
#define __POPULATIONFACTORY_H__

class PopulationFactory {
public:
    virtual ~PopulationFactory() {};
    virtual PopBase *createPopulationByName(const std::string sName) = 0;
    virtual PopBase *readPopulation(ParamProvider2 *pPP) = 0;
};


#endif
