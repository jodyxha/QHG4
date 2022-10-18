#ifndef __PSEUDOPOPCOUNTS_H__
#define __PSEUDOPOPCOUNTS_H__

#include <string>
#include <vector>

#include "types.h"
#include "PseudoPopArray.h"

#define PSEUDO_COUNT_NAME "AgentCount"
#define PSEUDO_COUNT_PAT  "(/?Populations/)([0-9A-Za-z_]+)"

class PseudoPopCounts : public PseudoPopArray {
public:
    static PseudoPopArray *createInstance();
    virtual const stringvec &findMatches(const std::string sPopQDF, const std::string sEnvQDF);
    virtual double *createArray(const std::string sPath, const std::string sPopQDF, const std::string sEnvQDF);
    
    virtual ~PseudoPopCounts() {};
    virtual const std::string &getFullPath() { return m_sUsedPath;};
protected:
    PseudoPopCounts();
    int init();
    int checkRequired(std::string sSpecies, const std::string sPopQDF, const std::string sEnvQDF);

    stringvec m_vAvailablePop;
    const stringvec m_vAvailableEnv;
};



#endif
