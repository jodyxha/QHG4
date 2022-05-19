#ifndef __FERTILITY_H__
#define __FERTILITY_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_FERTILITY_NAME            "Fertility"
#define ATTR_FERTILITY_MIN_AGE_NAME    "Fertility_min_age"
#define ATTR_FERTILITY_MAX_AGE_NAME    "Fertility_max_age"
#define ATTR_FERTILITY_INTERBIRTH_NAME "Fertility_interbirth"


template<typename T>
class Fertility : public Action<T> {
    
public:
    Fertility(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    ~Fertility();
    int operator()(int iA, float fT);

    int extractParamsQDF(hid_t hSpeciesGroup);
    int writeParamsQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetParams(const ModuleComplex *pMC);
   
    float getMinAge() {return m_fFertilityMinAge;};
    float getMaxAge() {return m_fFertilityMaxAge;};
    float getInterBirth() { return m_fInterbirth;};

    bool isEqual(Action<T> *pAction, bool bStrict);
    
  
protected:
    float m_fFertilityMinAge;
    float m_fFertilityMaxAge;
    float m_fInterbirth;

    static const char *asNames[];
};

#endif
