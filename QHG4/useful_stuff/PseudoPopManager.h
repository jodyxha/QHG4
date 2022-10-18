#ifndef __PSEUDOPOPMANAGER_H__
#define __PSEUDOPOPMANAGER_H__


#include <vector>
#include <string>
#include "types.h" 
#include "PseudoPopArray.h"

typedef std::vector<PseudoPopArray*> ppopvec;

class PseudoPopManager {

public:
    static PseudoPopManager *createInstance();

    virtual const stringvec &findMatches(const std::string sPopQDF, const std::string sEnvQDF);
    virtual double *createArray(const std::string sPath, const std::string sPopQDF, const std::string sEnvQDF);
    int getArraySize() { return m_iArrSize;};
    const std::string getUsedPath() { return m_sFullPath;};

    virtual ~PseudoPopManager(); 
protected:
    PseudoPopManager();
    int init();
    
    int m_iArrSize;
    std::string m_sFullPath;
    ppopvec m_vPseudos;
    stringvec m_vMatches;
};
#endif
