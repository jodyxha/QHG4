#ifndef __GRASSMANAGER_H__
#define __GRASSMANAGER_H__

#include "Action.h"
#include "ParamProvider2.h"
#include "ArrayShare.h"

const static std::string ATTR_GRASSMAN_NAME      = "GrassManager";
const static std::string ATTR_GRASSMAN_MIN_MASS_NAME      = "MinMass";
const static std::string ATTR_GRASSMAN_MAX_MASS_NAME      = "MaxMass";
const static std::string ATTR_GRASSMAN_GROWTH_RATE_NAME   = "GrowthRate";
        

template<typename T>
class GrassManager : public Action<T> {
public:
   
    GrassManager(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID,
                 const std::string sNameGrassMassAvail, const std::string sNameGrassMassConsumed);

    virtual ~GrassManager();

    virtual int preLoop();
    virtual int execute(int iAgentIndex, float fT);
    virtual int finalize(float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);

   
protected:
    ArrayShare *m_pAS;
    
    // arrays should be provided by the population object
    double *m_adGrassMassAvail;
    double *m_adGrassMassConsumed;

    int m_iNumCells;
    const std::string m_sNameGrassMassAvail;
    const std::string m_sNameGrassMassConsumed;

    double m_dMinMass;
    double m_dMaxMass;
    double m_dGrowthRate;

    omp_lock_t* m_aGLocks;

    int setAvailableMass();
    int subtractMassConsumed();

public:
    static const std::string  asNames[];
};

#endif
