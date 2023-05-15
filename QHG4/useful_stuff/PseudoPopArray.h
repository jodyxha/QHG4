#ifndef __PSEUDOPOP_ARRAY_H__
#define __PSEUDOPOP_ARRAY_H__

#include <string>
#include <vector>

#include "types.h"


class PseudoPopArray {
public:
    PseudoPopArray(const std::string sArrayName, const std::string sPathPat);
    
    virtual const stringvec &findMatches(const std::string sPopQDF, const std::string sEnvQDF)=0;
    virtual double *createArray(const std::string sPath, const std::string sPopQDF, const std::string sEnvQDF)=0;
    
    virtual ~PseudoPopArray();
    int getArraySize() { return m_iArrSize;};
    virtual const std::string getArrayName() {return m_sArrayName;}; 
    virtual const std::string getUsedPath()  {return m_sUsedPath;}; 
    
protected:
    std::string m_sArrayName;
    std::string m_sPathPat;
    double     *m_pdData;
    std::string m_sUsedPath;
    int         m_iArrSize;
    stringvec   m_vRequired;
   
};



#endif
