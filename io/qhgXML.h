#ifndef __QHGXML_H__
#define __QHGXML_H

#include <map>
#include <string>

#include "LineReader.h"

class qhgNode {
public:
    static qhgNode *createInstance(char *pLine, LineReader *pLR, int iLevel);
    static qhgNode *createRoot();

    ~qhgNode();
    
    const std::string getName()    {return m_sName;};
    qhgNode  *getNext()    {return m_pNext;};
    qhgNode  *getChild()   {return m_pChild;};
    stringmap &getAttrs()   {return m_mAttrs;}; 
    int       getTagType() {return m_iType;}; 

    void setName(const std::string sName);
    void setNext(qhgNode *pNext)     { m_pNext = pNext;};
    void setChild(qhgNode *pChild)   { m_pChild = pChild;};

    const char *getCurWord()  { return m_sCurWord;};
    const char *getCurString()  { return m_sCurString;};
protected:
    qhgNode(LineReader *pLR, int iLevel);
    int init(char *pLine);
    int parseNode(char *pLine);
    int processTag(char *pLine);
    int parseTag(char *pLine);
    char *skipBlanks();
    int   getNextSym();
    char *readWord();
    char *readString();

    LineReader *m_pLR;
    std::string m_sName;
    stringmap m_mAttrs;
    qhgNode *m_pNext;
    qhgNode *m_pChild;
    bool m_bClosed;
    char *m_pCur;
    char m_sCurWord[512];
    char m_sCurString[512];

    int m_iLevel;
    int m_iType;
    int m_iErr;
};

class qhgXML {
public: 
    static qhgXML *createInstance(const std::string sFile);
    ~qhgXML() { delete m_pRoot;};
    qhgNode *getRoot() { return m_pRoot;};
protected:
    qhgXML();
    int init(const std::string sFile);
    qhgNode *m_pRoot;
};


#endif
