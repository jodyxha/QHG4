#ifndef __CSVXMLCHECHKER_H__
#define __CSVXMLCHECHKER_H__

#include <string>
#include <map>
#include <vector>

#include "types.h"

typedef std::vector<stringmap> stringmapvec;


class CSVXMLChecker {
public:
    static CSVXMLChecker *createInstance(std::string sCSVFile, stringvec vCSVIgnoreKeys);
    virtual ~CSVXMLChecker();    

    int setCSVIgnores(stringvec vCSVIgnoreKeys);

    int setXML(std::string sXMLFile, stringvec vXMLIgnoreKeys);
    int setXMLIgnores(stringvec vXMLIgnoreKeys);

    const std::string  &getCSVName()           {return m_sCSVFile;};
    const stringmapvec &getCSVContents()       {return m_smvCSVContents;};
    const stringvec    &getCSVHeaders()        {return m_vCSVHeaders;};
    const stringvec    &getCSVHeadersReduced() {return m_vCSVHeadersReduced;};
    const stringvec    &getCSVIgnoredKeys()    {return m_vCSVIgnored;};

    const std::string  &getXMLName()        {return m_sXMLFile;};
    const stringmap    &getXMLContents()    {return m_smXMLContents;};
    const stringvec    &getXMLKeys()        {return m_vXMLKeys;};
    const stringvec    &getXMLKeysReduced() {return m_vXMLKeysReduced;};
    const stringvec    &getXMLIgnoredKeys() {return m_vXMLIgnored;};

    const stringvec    &getCSVnotXML()      {return m_vCSVnotXML;};
    const stringvec    &getXMLnotCSV()      {return m_vXMLnotCSV;};
    const stringvec    &getCSVandXML()      {return m_vCSVandXML;};

    const intvec       &getMatchIndexes()   {return m_vMatches;};

    void getDifferences();
    int compareValues(stringvec vCommonKeys);

protected:
    CSVXMLChecker();
    int init(std::string sCSVFile, stringvec vCSVIgnoreKeys);
    int extractCSVContents(std::string sCSVFile,  stringvec vCSVIgnoreKeys);
    int extractXMLContents(std::string sXMLFile,  stringvec vXMLIgnoreKeys);
 
    std::string  m_sCSVFile;
    std::string  m_sXMLFile;
    stringvec    m_vCSVIgnoreKeys;
    stringmapvec m_smvCSVContents;
    stringvec    m_vCSVHeaders;
    stringvec    m_vCSVIgnored;
    stringvec    m_vCSVHeadersReduced;

    stringvec    m_vXMLIgnoreKeys;
    stringmap    m_smXMLContents;
    stringvec    m_vXMLKeys;
    stringvec    m_vXMLKeysReduced;
    stringvec    m_vXMLIgnored;

  
    stringvec    m_vCSVnotXML;
    stringvec    m_vXMLnotCSV;
    stringvec    m_vCSVandXML;

    intvec       m_vMatches;
};

#endif
