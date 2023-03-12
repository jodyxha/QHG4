#include <cstdio>
#include <cstring>

#include "types.h"
#include "strutils.h"
#include "stdstrutilsT.h"
#include "LineReader.h"
#include "qhgXML.h"


// the format of a qhg XML:
//   qhgxml       ::= <line>*
//   line         ::= <tag> <CR>
//   tag          ::= "<" <tag_contents> ">"
//   tag_contents ::= <end_tag> | <start_tag> | <empty_tag>
//   end_tag      ::= "/" <tag_name>
//   start_tag    ::= <tag_name> [<attribute>]*
//   empty_tag    ::= <start_tag> "/"
//   attribute    ::= <att_name> "=" <att_value>
//   tag_name     ::= <word>
//   att_name     ::= <word>
//   attvalue     ::= "\"" <ascii>* "\"" 
//   word         ::= <word_start> [<word_cont>]* 
//   word_start   ::= <alpha> | "_"
//   word_cont    ::= <word_start> | <digit> | "+"  | "-" | "$"
//   
// tag structure
//  tag    ::= <start_tag> <tag>* <end_tag> | <empty_tag>  
//
// qhg XML DTD
//   <class>       contains <module>* and one <priorities>, or <pop_params>*
//                 has  atttributes "name", "species_name", "species_id"
//   <module>      contains <param>* and <module>*
//                 has attribute "name"
//   <param>       empty tag
//                 has attributes "name" and "value"
//   <priorities>  contains <prio>*
//                 has no attributes"
//   <prio>        empty tag
//                 has attributes "name" and "value"
//   <pop_params>  contains <vardef>*
//                 has no attributes"
//   <vardef>      empty tag
//                 has attributes "name" and "value"
// DTD 
//   <!ELEMENT class  ((module+, priorities?, vardefs?) >
//   <!ELEMENT module (param*) (module*) >
//   <!ELEMENT param EMPTY >
//   <!ELEMENT priorities (prio*) >
//   <!ELEMENT prio  EMPTY >
//   <!ELEMENT vardefs (var*) >
//   <!ELEMENT vardef  EMPTY >
//   <!ATTLIST class     name         CDATA #REQUIRED >
//   <!ATTLIST class     species_name CDATA #REQUIRED >
//   <!ATTLIST class     species_id   CDATA #REQUIRED >
//   <!ATTLIST module    name         CDATA #REQUIRED >
//   <!ATTLIST module    id           CDATA #IMPLIED  >
//   <!ATTLIST param     name         CDATA #REQUIRED >
//   <!ATTLIST param     value        CDATA #REQUIRED >
//   <!ATTLIST prio      name         CDATA #REQUIRED >
//   <!ATTLIST prio      value        CDATA #REQUIRED >
//   <!ATTLIST var       name         CDATA #REQUIRED >
//   <!ATTLIST var       value        CDATA #REQUIRED >

// the symbols
#define SYM_NONE            0
#define SYM_WORD            1
#define SYM_OPEN_BRA        2
#define SYM_CLOSE_BRA       3
#define SYM_OPEN_SLASH_BRA  4
#define SYM_CLOSE_SLASH_BRA 5
#define SYM_QUOTE           6
#define SYM_EQUALS          7
#define SYM_SLASH           8

// the symbol names (for debugging)
const char *pSymNames[] = {
    "SYM_NONE",
    "SYM_WORD", 
    "SYM_OPEN_BRA",
    "SYM_CLOSE_BRA",
    "SYM_OPEN_SLASH_BRA",
    "SYM_CLOSE_SLASH_BRA",
    "SYM_QUOTE",
    "SYM_EQUALS",
    "SYM_SLASH",
};

// tag types
#define TYPE_NO_TAG     0
#define TYPE_START_TAG  1
#define TYPE_EMPTY_TAG  2
#define TYPE_END_TAG    3

#define NAME_ROOT "root"

bool s_bVerbose = false;

//----------------------------------------------------------------------------
// createInstance
//
qhgXMLNode *qhgXMLNode::createInstance(char *pLine, LineReader *pLR, int iLevel) {
    qhgXMLNode *pN = new qhgXMLNode(pLR, iLevel);
    //    int iResult = pN->init(pLine);
    int iResult = pN->parseNode(pLine);
    if (iResult != 0) {
        delete pN;
        pN = NULL;
    }
    return pN;
}

//----------------------------------------------------------------------------
// createRoot
//
qhgXMLNode *qhgXMLNode::createRoot() {
    qhgXMLNode *pN = new qhgXMLNode(NULL, 0);
    pN->setName(NAME_ROOT);
    return pN;
}


//----------------------------------------------------------------------------
// constructor
//
qhgXMLNode::qhgXMLNode(LineReader *pLR, int iLevel) :
    m_pLR(pLR),
    m_sName(""),
    m_pNext(NULL),
    m_pChild(NULL),
    m_bClosed(false),
    m_iLevel(iLevel),
    m_iType(TYPE_NO_TAG) {
}



//----------------------------------------------------------------------------
// destructor
//
qhgXMLNode::~qhgXMLNode() {

    if (m_pNext != NULL) {
        delete m_pNext;
    }
    if (m_pChild != NULL) {
        delete m_pChild;
    }
}


//----------------------------------------------------------------------------
// setName
//
void qhgXMLNode::setName(const std::string sName) {
    m_sName = sName;
}


//----------------------------------------------------------------------------
// getNextSym
//   we expect tokens to be separated by white space
//
int qhgXMLNode::getNextSym () {
    int iSym = SYM_NONE;
    skipBlanks();

    if (isalpha(*m_pCur) || (*m_pCur == '_')) {
        readWord();
        iSym = SYM_WORD;

    } else if (*m_pCur == '<') {
        iSym = SYM_OPEN_BRA;
        m_pCur++;
        if (*m_pCur == '/') {
            iSym = SYM_OPEN_SLASH_BRA;
            m_pCur++;
        }

    } else if (*m_pCur == '/') {
        ++m_pCur;
        if (*m_pCur == '>') {
            iSym = SYM_CLOSE_SLASH_BRA;
            m_pCur++;
        }

    } else if (*m_pCur == '>') {
        iSym = SYM_CLOSE_BRA;
        m_pCur++;

    } else if (*m_pCur == '"') {
        readString();
        iSym = SYM_QUOTE;
        m_pCur++;

    } else if (*m_pCur == '=') {
        iSym = SYM_EQUALS;
        m_pCur++;

    } else if (*m_pCur == '/') {
        iSym = SYM_SLASH;
        m_pCur++;

    } else {
        stdprintf("Unknown symbol [%s]\n", m_pCur);
        m_iErr = -1;
    }
    //    stdprintf("end of getnextsym: sym %s [%s]\n", pSymNames[iSym], m_pCur);
    return iSym;
}


//----------------------------------------------------------------------------
// skipBlanks
//
char *qhgXMLNode::skipBlanks () {
    while (isspace(*m_pCur)) {m_pCur++;}
    return m_pCur;
}


//----------------------------------------------------------------------------
// readString
//   first a '"', then any (ascii) character, then a '"'
//
char *qhgXMLNode::readString () {
    char *p0 = NULL;
    // must eatthe leading quote
    m_pCur++;

    bool bInString = true; 
    p0 = m_pCur;
    while (bInString && (*m_pCur != '\0')) {
        
        if (*m_pCur == '"') {
            bInString = false;
        } else {

            if (*m_pCur == '\\') {
                ++m_pCur;
                
                if (*m_pCur == '"') {
                    //Ignore it
                }
            } else {
                ++m_pCur;
            }
        }
    }
    strncpy(m_sCurString, p0, m_pCur - p0);
    m_sCurString[m_pCur-p0] = '\0';
    if (bInString) {
        stdprintf("string not closed with quote [%s][%s]\n", p0, m_pCur);
        m_iErr = -1;

    }
    return m_pCur;
}


//----------------------------------------------------------------------------
// readWord
//
char *qhgXMLNode::readWord () {
    char *p0 = m_pCur;
    
    if (isalpha(*m_pCur) || (*m_pCur == '_')) {
        m_pCur++;
        while (!isspace(*m_pCur) && 
               (isalnum(*m_pCur) || 
                (*m_pCur == '-') || 
                (*m_pCur == '+') || 
                (*m_pCur == '_') || 
                (*m_pCur == '$'))) {
            m_pCur++;
        }
     
       strncpy(m_sCurWord, p0, m_pCur - p0);
       m_sCurWord[m_pCur-p0] = '\0';
    } else {
        stdprintf("word must start with letter or '_'\n");
        m_iErr = -1;
    }
    return m_pCur;
}


//---------------------------------------------------------------------------
// parseTag
//  analyze the tag and get the attributes if present
//  
int qhgXMLNode::parseTag(char *pLine)  {
    m_iErr = -1;
   
    m_iType   = TYPE_NO_TAG;
    m_bClosed = false;
    m_pCur    = trim(pLine);
   
    int iSym = getNextSym();
   
    if (iSym == SYM_OPEN_SLASH_BRA) {
        // this should be an end tag
        iSym = getNextSym();
        if (iSym == SYM_WORD) {
            setName(getCurWord());

            iSym = getNextSym();
            if (iSym == SYM_CLOSE_BRA) {
                m_bClosed = true;
                if (s_bVerbose) stdprintf("Closing %s on level %d\n", m_sName, m_iLevel); 
                m_iType = TYPE_END_TAG;
                m_iErr = 0;
                if (s_bVerbose) stdprintf("Have close tag [%s]\n", m_sName);
            } else {
                m_iErr = -1;
                stdprintf("Expected '>'\n");
            }
        } else  {
            stdprintf("'</' should be followed by tag name\n");
            m_iErr = -1;
        }

    } else if (iSym == SYM_OPEN_BRA) {
        // start tag or empty tag
        iSym = getNextSym();
        m_bClosed = false;
        //        iSym = getNextSym();
        if (iSym == SYM_WORD) {
            m_sName = getCurWord();
            if (s_bVerbose) stdprintf("Have tag name [%s]\n", m_sName);
            iSym = getNextSym();
            m_iErr = 0;

            while ((m_iErr == 0) && (iSym == SYM_WORD)) {
                const char *pAN = getCurWord();
                std::string sAttrName = pAN;
                iSym = getNextSym();
                if ((m_iErr == 0) && (iSym == SYM_EQUALS)) {
                    iSym = getNextSym();
                    if ((m_iErr == 0) && (iSym == SYM_QUOTE)) {
                        iSym = getNextSym();
                        const char *pAV = getCurString();
                        std::string sAttrValue = pAV;
                        m_mAttrs[sAttrName] = sAttrValue;
                        //m_iErr = 0;
                        if (s_bVerbose) stdprintf("Have attr name [%s] => [%s]\n", sAttrName, sAttrValue);
                    } else {
                        stdprintf("attribute values for [%s:%s] must be quoted\n", m_sName, sAttrName);
                        m_iErr = -1;
                    }
                } else {
                    stdprintf("expected '=' in attribute\n");
                    m_iErr = -1;
                }
                if (s_bVerbose) stdprintf("end of attr loop: sym %s [%s]\n", pSymNames[iSym], m_pCur);
            }
            // after attributes either a '>" or  "/>"
            if ((m_iErr == 0) && (iSym == SYM_CLOSE_BRA)) {
                m_iType = TYPE_START_TAG;
                
                
            } else if ((m_iErr == 0) && (iSym == SYM_CLOSE_SLASH_BRA)) {
                m_iType = TYPE_EMPTY_TAG;
                m_bClosed = true;
                if (s_bVerbose) stdprintf("closing %s on level %d\n", m_sName, m_iLevel);

            } else {
                m_iErr = -1;
                stdprintf("expected '>' or '/>' after last attribute\n");
            }
        } else {
            m_iErr = -1;
            stdprintf("expected word after '>'\n");
        }
               
    } else {
        m_iType = TYPE_NO_TAG;
        m_iErr = -1;
        stdprintf("Tag must start with '<' or '</' [%s]\n", pLine);
    } 

    if (s_bVerbose) stdprintf("---------\n");
    return m_iErr;
}

//----------------------------------------------------------------------------
// parseNode
//
int qhgXMLNode::parseNode(char *pLine)  {
    if (s_bVerbose) stdprintf("entered parseNode level %d (closed %s)\n", m_iLevel, m_bClosed?"yes":"no");
    
    m_iErr = parseTag(pLine);

    if (s_bVerbose) {
        stdprintf("After parse tag on level %d (iErr %d)\n", m_iLevel, m_iErr);
        stdprintf("   name %s\n", m_sName);
        stdprintf("   attrs:");
        stringmap::const_iterator it;
        for (it = m_mAttrs.begin(); it != m_mAttrs.end(); ++it) {
            stdprintf("  %s:%s", it->first, it->second);
        }
        stdprintf("\n");
        stdprintf("  closed: %s\n", m_bClosed?"yes":"no");
    }



    while ((m_iErr == 0)  &&  (!m_pLR->isEoF()) && (!m_bClosed)) {
        pLine = m_pLR->getNextLine();
        if (pLine != NULL) {
            if (s_bVerbose) stdprintf("First line in parseNode level %d: [%s] (eof %s, closed %s)\n", m_iLevel, pLine, m_pLR->isEoF()?"yes":"no", m_bClosed?"yes":"no");
    
            m_bClosed = false;
            qhgXMLNode *pNew = createInstance(pLine, m_pLR, m_iLevel+1);
            if (pNew != NULL) {
                // is it the  end tag for our current tag? 
                int iCurType = pNew->getTagType();
                if (iCurType == TYPE_END_TAG) {
                    if (pNew->getName() == m_sName) {
                        m_bClosed = true;
                        if (s_bVerbose) stdprintf("closing %s on level %d\n", m_sName, m_iLevel);
                    } else {
                        stdprintf("end tag without corresponding start tag [%s]\n", m_sName);
                        m_iErr = -1;
                    }
                    // we don't need a node from the end tag
                    delete pNew;
                } else if ((iCurType == TYPE_START_TAG) ||
                           (iCurType == TYPE_EMPTY_TAG)) {
                    
                    // add it as a child
                    stringmap mAttrNew = pNew->getAttrs();
                    if (m_pChild != NULL) {
                        qhgXMLNode *pLast = m_pChild;
                        while (pLast->getNext() != NULL) {
                            pLast = pLast->getNext();
                        }
                        if (s_bVerbose) stdprintf("making [%s %s] the sibling of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"],  pLast->getName(), pLast->getAttrs()["name"]);
                        pLast->m_pNext = pNew;
                    } else {
                        if (s_bVerbose) stdprintf("making [%s %s] the only child of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"], m_sName, m_mAttrs["name"]);
                        m_pChild = pNew;
                    }
                    
                } else {
                    stdprintf("unknown tag type [%s]\n", pNew->getName());
                    m_iErr = -1;
                }
            } else {
                stdprintf("couldn't create node for [%s]\n", pLine);
                m_iErr = -1;
            }
        }
        
    } 
    if (!m_bClosed) {
        stdprintf("The tag [%s] has no corresponding end tag\n", m_sName);
    }
    if (s_bVerbose) stdprintf("left parseNode level %d (closed %s) (iErr %d)\n", m_iLevel, m_bClosed?"yes":"no", m_iErr);
    return m_iErr;
}


//----------------------------------------------------------------------------
// init
//   first process the tag
//   if it is closed
int qhgXMLNode::init(char *pLine)  {
    //    int iResult = processTag(pLine);
    int iResult = parseTag(pLine);
    if (s_bVerbose) stdprintf("[%s] has %zd attributes (res %d)\n", m_sName, m_mAttrs.size(), iResult);
    
    if (m_pLR != NULL) {
        if (s_bVerbose) stdprintf("res:%d, eof:%s closed %s\n", iResult, m_pLR->isEoF()?"yes":"no", m_bClosed?"yes":"no");
        while ((iResult >= 0) &&  (!m_pLR->isEoF()) && (!m_bClosed)) {
            pLine = m_pLR->getNextLine();
            if (pLine != NULL) {
                if (s_bVerbose) stdprintf("new line [%s]\n", pLine);
                qhgXMLNode *pNew = createInstance(pLine, m_pLR, m_iLevel+1);
                if (pNew != NULL) {
                    //if (strstr(pNew->getName(), m_pName) == pNew->getName()) {
                    if (pNew->getName().find(m_sName) == 0) {
                        m_bClosed = true;
                    } else {
                        stringmap mAttrNew = pNew->getAttrs();
                        if (m_pChild != NULL) {
                            qhgXMLNode *pLast = m_pChild;
                            while (pLast->getNext() != NULL) {
                                pLast = pLast->getNext();
                            }
                            if (s_bVerbose) stdprintf("making [%s %s] the sibling of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"],  pLast->getName(), pLast->getAttrs()["name"]);
                            pLast->m_pNext = pNew;
                        } else {
                            if (s_bVerbose) stdprintf("making [%s %s] the only child of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"], m_sName, m_mAttrs["name"]);
                            m_pChild = pNew;
                        }
                    }
                } else {
                    iResult = -1;
                }
            }
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createInstance
//  create qhgXMLTree inbstance and process the file
//
qhgXMLTree *qhgXMLTree::createInstance(const std::string sFile) {
    qhgXMLTree *pS = new qhgXMLTree();
    int iResult = pS->init(sFile);
    if (iResult != 0) {
        delete pS;
        pS = NULL;
    }
    return pS;
}


//----------------------------------------------------------------------------
// constructor
//
qhgXMLTree::qhgXMLTree() 
    : m_pRoot(NULL) {
}


//----------------------------------------------------------------------------
// init
//   here we collerct top-level tags and dd the as children to the 
//   artficial "root" node
//
int qhgXMLTree::init(const std::string sFile) {
    int iResult = -1;

    LineReader *pLR = NULL;
    if (sFile.ends_with(".gz")) {
        pLR = LineReader_gz::createInstance(sFile, "r");
    } else {
        pLR = LineReader_std::createInstance(sFile, "r");
    }
    if (pLR != NULL) {
        // build the root node
        m_pRoot = qhgXMLNode::createRoot();
        iResult = 0;
        // process all top-level tags 
        while ((iResult == 0) && (!pLR->isEoF())) {
            char *pLine = pLR->getNextLine();
            if (pLine != NULL) {
                qhgXMLNode *pNew = qhgXMLNode::createInstance(pLine, pLR, 1);
                if (pNew != NULL) {
                    qhgXMLNode *pC = m_pRoot->getChild();
                    if (pC != NULL) {
                        pC->setNext(pNew);
                    } else {
                        m_pRoot->setChild(pNew);
                    }
                } else {
                    stdprintf("Couldn't create Node for [%s]\n", pLine);
                    iResult = -1;
                    // failed to create Node
                }
            }
        }

        delete pLR;
    } else {
        stdprintf("Couldn't open [%s] for reading\n", sFile);
    }
    return iResult;
}
