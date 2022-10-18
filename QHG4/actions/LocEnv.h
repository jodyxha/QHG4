#ifndef __LOC_ENV_H__
#define __LOC_ENV_H__

#include "types.h"
#include "Observable.h"
#include "Observer.h"
#include "PolyLine.h"
#include "Action.h"
#include "ParamProvider2.h"
#include "LayerArrBuf.h"
#include "WELL512.h"
#include "NPPCalc.h"

class LBController;

const static std::string ATTR_LOCENV_NAME                 = "LocEnv";

const static std::string ATTR_LOCENV_WATERFACTOR_NAME     = "NPPCap_water_factor";
const static std::string ATTR_LOCENV_COASTALFACTOR_NAME   = "NPPCap_coastal_factor";
const static std::string ATTR_LOCENV_COASTAL_MIN_LAT_NAME = "NPPCap_coastal_min_latitude";
const static std::string ATTR_LOCENV_COASTAL_MAX_LAT_NAME = "NPPCap_coastal_max_latitude";
const static std::string ATTR_LOCENV_NPPMIN_NAME          = "NPPCap_NPP_min";
const static std::string ATTR_LOCENV_NPPMAX_NAME          = "NPPCap_NPP_max";
const static std::string ATTR_LOCENV_KMAX_NAME            = "NPPCap_K_max";
const static std::string ATTR_LOCENV_KMIN_NAME            = "NPPCap_K_min";
const static std::string ATTR_LOCENV_ALT_PREF_POLY_NAME   = "NPPCap_alt_pref_poly";


// it is not very nice to derive LocEns from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T>
class LocEnv : public Action<T> , public Observer {
public:

    LocEnv(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, WELL512** apWELL, LBController *pAgentController);
    virtual ~LocEnv();

    // get action parameters from QDF 
    virtual int extractAttributesQDF(hid_t hSpeciesGroup);

    // write action parameters to QDF
    virtual int writeAttributesQDF(hid_t hSpeciesGroup);
    virtual int tryGetAttributes(const ModuleComplex *pMC);

    void notify(Observable *pObs, int iEvent, const void *pData);
    int recalculateGlobalCapacities();
    
    // from Action
    virtual int preLoop();
    virtual int initialize(float fTime);

    virtual bool isEqual(Action<T> *pAction, bool bStrict);

    const double *getArr(int iAgentIndex);
    int init();
protected:

    LBController *m_pAgentController;
    bool          m_bNeedUpdate;

    LayerArrBuf<double> m_aEnvVals; // 7 entries per agent (self + 6 neighbors)

    double   *m_dAlt;
    double   *m_dNPP;



    int m_iNumThreads;
    int m_iLocArrSize;

    double  *m_adCapacities[2];
    double   m_dWaterFactor[2];
    double   m_dCoastalFactor[2];
    double   m_dCoastalMinLatitude[2];
    double   m_dCoastalMaxLatitude[2];
    double   m_dNPPMin[2];
    double   m_dNPPMax[2];
    double   m_dKMin[2];
    double   m_dKMax[2];

    NPPCalc     *m_pNPPCalc;
    PolyLine    *m_pAltPrefPoly[2];           // how to go from env values to weights
    std::string  m_sPLParName[2];

    static const stringvec s_vNames;
    static const std::string asSpecies[];


};


#endif
