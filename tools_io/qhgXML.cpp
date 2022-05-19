#include <cstdio>
#include <cstring>

#include "types.h"
#include "strutils.h"
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
//   <class>       contains <module>* and one <priorities>
//                 has  atttributes "name", "species_name", "species_id"
//   <module>      contains <param>*
//                 has attribute "name"
//   <param>       empty tag
//                 has attributes "name" and "value"
//   <priorities>  contains <prio>*
//                 has no attributes"
//   <prio>        empty tag
//                 has attributes "name" and "value"
// DTD
//   <!ELEMENT class  (module+, priorities?) >
//   <!ELEMENT module (attribute*) >
//   <!ELEMENT attribute EMPTY >
//   <!ELEMENT priorities (prio*) >
//   <!ELEMENT prio  EMPTY >
//   <!ATTLIST class     name         CDATA #REQUIRED >
//   <!ATTLIST class     species_name CDATA #REQUIRED >
//   <!ATTLIST class     species_id   CDATA #REQUIRED >
//   <!ATTLIST module    name         CDATA #REQUIRED >
//   <!ATTLIST attribute name         CDATA #REQUIRED >
//   <!ATTLIST attribute value        CDATA #REQUIRED >
//   <!ATTLIST prio      name         CDATA #REQUIRED >
//   <!ATTLIST prio      value        CDATA #REQUIRED >

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
qhgNode *qhgNode::createInstance(char *pLine, LineReader *pLR, int iLevel) {
    qhgNode *pN = new qhgNode(pLR, iLevel);
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
qhgNode *qhgNode::createRoot() {
    qhgNode *pN = new qhgNode(NULL, 0);
    pN->setName(NAME_ROOT);
    return pN;
}


//----------------------------------------------------------------------------
// constructor
//
qhgNode::qhgNode(LineReader *pLR, int iLevel) :
    m_pLR(pLR),
    m_pName(NULL),
    m_pNext(NULL),
    m_pChild(NULL),
    m_bClosed(false),
    m_pCurWord(NULL),
    m_pCurString(NULL),
    m_iType(TYPE_NO_TAG),
    m_iLevel(iLevel) {
}



//----------------------------------------------------------------------------
// destructor
//
qhgNode::~qhgNode() {
    delete[] m_pName;
    if (m_pNext != NULL) {
        delete m_pNext;
    }
    if (m_pChild != NULL) {
        delete m_pChild;
    }
    if (m_pCurWord != NULL) {
        delete[] m_pCurWord;
    }
    if (m_pCurString != NULL) {
        delete[] m_pCurString;
    }

}


//----------------------------------------------------------------------------
// setName
//
void qhgNode::setName(const char *pName) {
    if (pName != NULL) {
        m_pName = new char[strlen(pName)+1];
        strcpy(m_pName, pName);
    }
}


//----------------------------------------------------------------------------
// getNextSym
//   we expect tokens to be separated by white space
//
int qhgNode::getNextSym () {
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
        printf("Unknown symbol [%s]\n", m_pCur);
        m_iErr = -1;
    }
    //    printf("end of getnextsym: sym %s [%s]\n", pSymNames[iSym], m_pCur);
    return iSym;
}


//----------------------------------------------------------------------------
// skipBlanks
//
char *qhgNode::skipBlanks () {
    while (isspace(*m_pCur)) {m_pCur++;}
    return m_pCur;
}


//----------------------------------------------------------------------------
// readString
//   first a '"', then any (ascii) character, then a '"'
//
char *qhgNode::readString () {
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
    strncpy(m_pCurString, p0, m_pCur - p0);
    m_pCurString[m_pCur-p0] = '\0';
    if (bInString) {
        printf("string not closed with quote [%s][%s]\n", p0, m_pCur);
        m_iErr = -1;

    }
    return m_pCur;
}


//----------------------------------------------------------------------------
// readWord
//
char *qhgNode::readWord () {
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
     
       strncpy(m_pCurWord, p0, m_pCur - p0);
       m_pCurWord[m_pCur-p0] = '\0';
    } else {
        printf("word must start with letter or '_'\n");
        m_iErr = -1;
    }
    return m_pCur;
}


//---------------------------------------------------------------------------
// parseTag
//  analyze the tag and get the attributes if present
//  
int qhgNode::parseTag(char *pLine)  {
    m_iErr = -1;
   
    char sTagName[512];
    m_iType   = TYPE_NO_TAG;
    m_bClosed = false;
    m_pCur    = trim(pLine);
   
    int iSym = getNextSym();
   
    if (iSym == SYM_OPEN_SLASH_BRA) {
        // this should be an end tag
        iSym = getNextSym();
        if (iSym == SYM_WORD) {
            char sTemp[512];
            setName(getCurWord());

            iSym = getNextSym();
            if (iSym == SYM_CLOSE_BRA) {
                m_bClosed = true;
                if (s_bVerbose) printf("Closing %s on level %d\n", m_pName, m_iLevel); 
                m_iType = TYPE_END_TAG;
                m_iErr = 0;
                if (s_bVerbose) printf("Have close tag [%s]\n", m_pName);
            } else {
                m_iErr = -1;
                printf("Expected '>'\n");
            }
        } else  {
            printf("'</' should be followed by tag name\n");
            m_iErr = -1;
        }

    } else if (iSym == SYM_OPEN_BRA) {
        // start tag or empty tag
        iSym = getNextSym();
        m_bClosed = false;
        //        iSym = getNextSym();
        if (iSym == SYM_WORD) {
            const char *pCW = getCurWord();
            m_pName = new char[strlen(pCW)+1];
            strcpy(m_pName, pCW);
            if (s_bVerbose) printf("Have tag name [%s]\n", m_pName);
            iSym = getNextSym();
            char sAttrName[512];
            char sAttrValue[512];
            m_iErr = 0;

            while ((m_iErr == 0) && (iSym == SYM_WORD)) {
                const char *pAN = getCurWord();
                strcpy(sAttrName, pAN);
                iSym = getNextSym();
                if ((m_iErr == 0) && (iSym == SYM_EQUALS)) {
                    iSym = getNextSym();
                    if ((m_iErr == 0) && (iSym == SYM_QUOTE)) {
                        iSym = getNextSym();
                        const char *pAV = getCurString();
                        strcpy(sAttrValue, pAV);
                        m_mAttrs[sAttrName] = sAttrValue;
                        //m_iErr = 0;
                        if (s_bVerbose) printf("Have attr name [%s] => [%s]\n", sAttrName, sAttrValue);
                    } else {
                        printf("attribute values must be quoted\n");
                        m_iErr = -1;
                    }
                } else {
                    printf("expected '=' in attribute\n");
                    m_iErr = -1;
                }
                if (s_bVerbose) printf("end of attr loop: sym %s [%s]\n", pSymNames[iSym], m_pCur);
            }
            // after attributes either a '>" or  "/>"
            if ((m_iErr == 0) && (iSym == SYM_CLOSE_BRA)) {
                m_iType = TYPE_START_TAG;
                
                
            } else if ((m_iErr == 0) && (iSym == SYM_CLOSE_SLASH_BRA)) {
                m_iType = TYPE_EMPTY_TAG;
                m_bClosed = true;
                if (s_bVerbose) printf("closing %s on level %d\n", m_pName, m_iLevel);

            } else {
                m_iErr = -1;
                printf("expected '>' or '/>' after last attribute\n");
            }
        } else {
            m_iErr = -1;
            printf("expected word after '>'\n");
        }
               
    } else {
        m_iType = TYPE_NO_TAG;
        m_iErr = -1;
        printf("Tag must start with '<' or '</' [%s]\n", pLine);
    } 

    if (s_bVerbose) printf("---------\n");
    return m_iErr;
}

//----------------------------------------------------------------------------
// parseNode
//
int qhgNode::parseNode(char *pLine)  {
    if (s_bVerbose) printf("entered parseNode level %d (closed %s)\n", m_iLevel, m_bClosed?"yes":"no");
    if (m_pCurWord != NULL) {
        delete[] m_pCurWord;
    }
    m_pCurWord = new char[strlen(pLine)+1];
    if (m_pCurString != NULL) {
        delete[] m_pCurString;
    }
    m_pCurString = new char[strlen(pLine)+1];
    


    m_iErr = parseTag(pLine);

    if (s_bVerbose) {
        printf("After parse tag on level %d\n", m_iLevel);
        printf("   name %s\n", m_pName);
        printf("   attrs:");
        stringmap::const_iterator it;
        for (it = m_mAttrs.begin(); it != m_mAttrs.end(); ++it) {
            printf("  %s:%s", it->first.c_str(), it->second.c_str());
        }
        printf("\n");
        printf("  closed: %s\n", m_bClosed?"yes":"no");
    }



    while ((m_iErr == 0)  &&  (!m_pLR->isEoF()) && (!m_bClosed)) {
        pLine = m_pLR->getNextLine();
        if (pLine != NULL) {
            if (s_bVerbose) printf("First line in parseNode level %d: [%s] (eof %s, closed %s)\n", m_iLevel, pLine, m_pLR->isEoF()?"yes":"no", m_bClosed?"yes":"no");
    
            m_bClosed = false;
            qhgNode *pNew = createInstance(pLine, m_pLR, m_iLevel+1);
            if (pNew != NULL) {
                // is it the  end tag for our current tag? 
                int iCurType = pNew->getTagType();
                if (iCurType == TYPE_END_TAG) {
                    if (strcmp(pNew->getName(), m_pName) == 0) {
                        m_bClosed = true;
                        if (s_bVerbose) printf("closing %s on level %d\n", m_pName, m_iLevel);
                    } else {
                        printf("end tag without corresponding start tag [%s]\n", m_pName);
                        m_iErr = -1;
                    }
                    // we don't need a node from the end tag
                    delete pNew;
                } else if ((iCurType == TYPE_START_TAG) ||
                           (iCurType == TYPE_EMPTY_TAG)) {
                    
                    // add it as a child
                    stringmap mAttrNew = pNew->getAttrs();
                    if (m_pChild != NULL) {
                        qhgNode *pLast = m_pChild;
                        while (pLast->getNext() != NULL) {
                            pLast = pLast->getNext();
                        }
                        if (s_bVerbose) printf("making [%s %s] the sibling of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"].c_str(),  pLast->getName(), pLast->getAttrs()["name"].c_str());
                        pLast->m_pNext = pNew;
                    } else {
                        if (s_bVerbose) printf("making [%s %s] the only child of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"].c_str(), m_pName, m_mAttrs["name"].c_str());
                        m_pChild = pNew;
                    }
                    
                } else {
                    printf("unknown tag type [%s]\n", pNew->getName());
                    m_iErr = -1;
                }
            } else {
                printf("couldn't create node for [%s]\n", pLine);
                m_iErr = -1;
            }
        }
        
    }
    if (!m_bClosed) {
        printf("The tag [%s] has no corresponding end tag\n", m_pName);
    }
    if (s_bVerbose) printf("left parseNode level %d (closed %s)\n", m_iLevel, m_bClosed?"yes":"no");
    return m_iErr;
}


//----------------------------------------------------------------------------
// init
//   first process the tag
//   if it is closed
int qhgNode::init(char *pLine)  {
 
    //    int iResult = processTag(pLine);
    int iResult = parseTag(pLine);
    if (s_bVerbose) printf("[%s] has %zd attributes (res %d)\n", m_pName, m_mAttrs.size(), iResult);
    
    if (m_pLR != NULL) {
        if (s_bVerbose) printf("res:%d, eof:%s closed %s\n", iResult, m_pLR->isEoF()?"yes":"no", m_bClosed?"yes":"no");
        while ((iResult >= 0) &&  (!m_pLR->isEoF()) && (!m_bClosed)) {
            pLine = m_pLR->getNextLine();
            if (pLine != NULL) {
                if (s_bVerbose) printf("new line [%s]\n", pLine);
                qhgNode *pNew = createInstance(pLine, m_pLR, m_iLevel+1);
                if (pNew != NULL) {
                    if (strstr(pNew->getName(), m_pName) == pNew->getName()) {
                        m_bClosed = true;
                    } else {
                        stringmap mAttrNew = pNew->getAttrs();
                        if (m_pChild != NULL) {
                            qhgNode *pLast = m_pChild;
                            while (pLast->getNext() != NULL) {
                                pLast = pLast->getNext();
                            }
                            if (s_bVerbose) printf("making [%s %s] the sibling of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"].c_str(),  pLast->getName(), pLast->getAttrs()["name"].c_str());
                            pLast->m_pNext = pNew;
                        } else {
                            if (s_bVerbose) printf("making [%s %s] the only child of [%s %s]\n", pNew->getName(), pNew->getAttrs()["name"].c_str(), m_pName, m_mAttrs["name"].c_str());
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
//  create qhgXML inbstance and process the file
//
qhgXML *qhgXML::createInstance(const char *pFile) {
    qhgXML *pS = new qhgXML();
    int iResult = pS->init(pFile);
    if (iResult != 0) {
        delete pS;
        pS = NULL;
    }
    return pS;
}


//----------------------------------------------------------------------------
// constructor
//
qhgXML::qhgXML() 
    :m_pRoot(NULL) {
}


//----------------------------------------------------------------------------
// init
//   here we collerct top-level tags and dd the as children to the 
//   artficial "root" node
//
int qhgXML::init(const char *pFile) {
    int iResult = -1;
    char s[256];
    strcpy(s, "<root>");

    LineReader *pLR = LineReader_std::createInstance(pFile, "r");
    if (pLR != NULL) {
        // build the root node
        m_pRoot = qhgNode::createRoot();
        iResult = 0;
        // process all top-level tags 
        while ((iResult == 0) && (!pLR->isEoF())) {
            char *pLine = pLR->getNextLine();
            if (pLine != NULL) {
                qhgNode *pNew = qhgNode::createInstance(pLine, pLR, 1);
                if (pNew != NULL) {
                    qhgNode *pC = m_pRoot->getChild();
                    if (pC != NULL) {
                        pC->setNext(pNew);
                    } else {
                        m_pRoot->setChild(pNew);
                    }
                } else {
                    printf("Couldn't create Node for [%s]\n", pLine);
                    // failed to create Node
                }
            }
        }

        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pFile);
    }
    return iResult;
}
