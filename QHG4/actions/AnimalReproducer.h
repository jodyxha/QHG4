#ifndef __ANIMALREPRODUCER_H__
#define __ANIMALREPRODUCER_H__


#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_ANIMALREP_NAME              = "AnimalReproducer";
const static std::string ATTR_ANIMALREP_MASS_FERT_NAME    = "MassFert";
const static std::string ATTR_ANIMALREP_MASS_BABY_NAME    = "MassBaby";
const static std::string ATTR_ANIMALREP_BIRTH_PROB_NAME   = "BirthProb";

template<typename T>
class AnimalReproducer : public Action<T> {
    
 public:
    AnimalReproducer(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    virtual ~AnimalReproducer();

    virtual int execute(int iA, float fT);
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
 
protected:
    WELL512 **m_apWELL;
    double m_dMassFert;
    double m_dMassBaby;
    double m_dBirthProb;
public:
    static const std::string asNames[];

};


#endif
