#ifndef __FERTILITY_H__
#define __FERTILITY_H__

#include "Action.h"
#include "ParamProvider2.h"

const static std::string ATTR_FERTILITY_NAME              = "Fertility";
const static std::string ATTR_FERTILITY_MIN_AGE_NAME      = "Fertility_min_age";
const static std::string ATTR_FERTILITY_MAX_AGE_NAME      = "Fertility_max_age";
const static std::string ATTR_FERTILITY_INTERBIRTH_NAME   = "Fertility_interbirth";


template<typename T>
class Fertility : public Action<T> {
    
public:
    Fertility(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID);
    virtual ~Fertility();
    virtual int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
 
    virtual int tryGetAttributes(const ModuleComplex *pMC);
   
    float getMinAge() {return m_fFertilityMinAge;};
    float getMaxAge() {return m_fFertilityMaxAge;};
    float getInterBirth() { return m_fInterbirth;};

    virtual bool isEqual(Action<T> *pAction, bool bStrict);
    
  
protected:
    float m_fFertilityMinAge;
    float m_fFertilityMaxAge;
    float m_fInterbirth;
public:
    static const std::string asNames[];
};

#endif
