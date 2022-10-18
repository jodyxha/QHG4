#ifndef __QHGXML_H__
#define __QHGXML_H

#include <map>
#include <string>

#include "LineReader.h"

class qhgXMLNode {
public:
    static qhgXMLNode *createInstance(char *pLine, LineReader *pLR, int iLevel);
    static qhgXMLNode *createRoot();

    ~qhgXMLNode();
    
    const std::string getName()    {return m_sName;};
    qhgXMLNode  *getNext()    {return m_pNext;};
    qhgXMLNode  *getChild()   {return m_pChild;};
    stringmap &getAttrs()   {return m_mAttrs;}; 
    int       getTagType() {return m_iType;}; 

    void setName(const std::string sName);
    void setNext(qhgXMLNode *pNext)     { m_pNext = pNext;};
    void setChild(qhgXMLNode *pChild)   { m_pChild = pChild;};

    const char *getCurWord()  { return m_sCurWord;};
    const char *getCurString()  { return m_sCurString;};
protected:
    qhgXMLNode(LineReader *pLR, int iLevel);
    int init(char *pLine);
    int parseNode(char *pLine);
    //    int processTag(char *pLine);
    int parseTag(char *pLine);
    char *skipBlanks();
    int   getNextSym();
    char *readWord();
    char *readString();

    LineReader *m_pLR;
    std::string m_sName;
    stringmap m_mAttrs;
    qhgXMLNode *m_pNext;
    qhgXMLNode *m_pChild;
    bool m_bClosed;
    char *m_pCur;
    char m_sCurWord[512];
    char m_sCurString[512];

    int m_iLevel;
    int m_iType;
    int m_iErr;
};

class qhgXMLTree {
public: 
    static qhgXMLTree *createInstance(const std::string sFile);
    ~qhgXMLTree() { delete m_pRoot;};
    qhgXMLNode *getRoot() { return m_pRoot;};
protected:
    qhgXMLTree();
    int init(const std::string sFile);
    qhgXMLNode *m_pRoot;
};

                                                                                                  
#endif
