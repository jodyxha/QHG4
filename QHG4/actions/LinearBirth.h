#ifndef __LINBIRTH_H__
#define __LINBIRTH_H__

#include "Action.h"
#include "ParamProvider2.h"

#define ATTR_LINBIRTH_NAME "LinearBirth"
#define ATTR_LINBIRTH_B0_NAME "LinearBirth_b0"
#define ATTR_LINBIRTH_TURNOVER_NAME "LinearBirth_theta"
#define ATTR_LINBIRTH_CAPACITY_NAME "LinearBirth_K"

class WELL512;

template<typename T>
class LinearBirth : public Action<T> {
    
 public:
    LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL);
    LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dB0, double dTheta, double dK);
    LinearBirth(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512 **apWELL, double dB0, double dTheta, double* adK, int iStride);

    virtual ~LinearBirth();

    virtual int initialize(float fT);
    virtual int execute(int iA, float fT);

    virtual int extractAttributesQDF(hid_t hSpeciesGroup);
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);  
    virtual int modifyAttributes(const std::string sAttrName, double dValue);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);


 protected:
    WELL512 **m_apWELL;
    double *m_adB;
    double m_dB0;
    double m_dTheta;
    double m_dK;
    double* m_adK;
    int m_iStride;

    static const char * asNames[];
};

#endif
