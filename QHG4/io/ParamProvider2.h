#ifndef __PARAMPROVIDER2_H__
#define __PARAMPROVIDER2_H__

#include <string>
#include <map>
#include <vector>

#include "types.h"
#include "qhgXML.h"




class ModuleComplex;

typedef std::map<std::string, ModuleComplex*> modulemap;

class ModuleComplex {
public:
    ModuleComplex() {};
    ModuleComplex(const stringmap smParams, modulemap &mSubModules);
    ~ModuleComplex();
    int addParam(const std::string sKey, const std::string &sVal);
    int addSubModule(const std::string sKey, ModuleComplex *pSubModule);
    
    const std::string getParam(const std::string sKey);
    ModuleComplex *getModule(const std::string sModuleName);

    const stringmap &getAttributes() const { return m_mParams;};
    const modulemap &getSubModules() const { return m_mSubModules;};
protected:
    
    stringmap   m_mParams;
    modulemap   m_mSubModules;
};

typedef struct classinfo_t {
    stringmap   cattr;
    modulemap   mods;
    stringmap   prios;
    stringmap   vardefs;
} classinfo;



typedef std::map<std::string, classinfo> classes;


class ParamProvider2 {
public:
    static ParamProvider2 *createInstance(const std::string sXMLFile);
    ~ParamProvider2();

    void showTree();
    classes &getClasses() {return m_mClasses;};
    const modulemap *getClass(const std::string sClassName);
    int selectClass(const std::string sClassName);
    const classinfo *getClassInfo() { return m_pCurClassInfo;};
    const stringmap *getParams(const std::string ModuleName);
    const std::string getSelected() { return m_sCurClassName;};
    const std::string getSpeciesName() { return m_sSpeciesName;};
    /*
    const stringmap *getModule(const strstrmap *pM, const char *pModuleName);
    const char *getParamValue(const char *pModuleName, const char *pParamName);
    */
    const std::vector<std::string> getClassNames() { return m_vClassNames;};

protected:
    ParamProvider2();
    int init(const std::string sXML);
    
    int processParam(qhgXMLNode *pParam, const std::string subtag, stringmap &att_param);
    int processModule(qhgXMLNode *pModule, modulemap &m_Modules);
    int processPriorities(qhgXMLNode *pPrios, stringmap &pa);
    int processVarDefs(qhgXMLNode *pPrios, stringmap &vd);
    int processClass(qhgXMLNode *pClass);

    classes  m_mClasses;
    const classinfo *m_pCurClassInfo;
    std::string m_sCurClassName;
    std::string m_sSpeciesName;
    std::vector<std::string>  m_vClassNames;
    modulemap   m_mModules;

};



#endif
