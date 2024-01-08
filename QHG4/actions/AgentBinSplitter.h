#ifndef __AGENTBINSPLITTER_H__
#define __AGENTBINSPLITTER_H__

#include <hdf5.h>

#include <vector>
#include <string>
#include "Action.h"
#include "ParamProvider2.h"
#include "LBBase.h"
#include "LayerBuf.h"
#include "LBController.h"

const static std::string ATTR_ABS_NAME              = "AgentBinSplitter";
const static std::string ATTR_ABS_BIN_MIN_NAME      = "BinMin";
const static std::string ATTR_ABS_BIN_MAX_NAME      = "BinMax";
const static std::string ATTR_ABS_NUM_BINS_NAME     = "NumBins";
const static std::string ATTR_ABS_VAR_FIELD_NAME    = "VarField";

template<typename T>
class AgentBinSplitter : public Action<T> {
public:
    AgentBinSplitter(SPopulation<T> *pPop, SCellGrid *pCG, std::string sID, LBController *pAgentController);
    virtual ~AgentBinSplitter();
 
    virtual int preLoop();
   
    virtual int preWrite(float fTime);
    virtual int writeAdditionalDataQDF(hid_t hActionGroup);

    virtual int tryGetAttributes(const ModuleComplex *pMC);
    virtual int extractAttributesQDF(hid_t hActionGroup);
    virtual int writeAttributesQDF(hid_t hActionGroup);

protected:

    int getVariableOffset();
    double getDVal(T &ag);

    int m_iNumThreads;
    
    LBController *m_pAgentController;
    
    double m_dBinMin;
    double m_dBinMax;
    int    m_iNumBins;
    std::string m_sVarField;
    int m_iVarOffset;
    hid_t m_hVarType;

    std::vector<LBController *> *m_pvLBCs; // numthread array of LBC* vectors
    std::vector<LayerBuf<T>>    *m_pvLBs;   // numthread array of LB vectors

    bool m_bVerbose;
    static const char *asNames[];
};


#endif
