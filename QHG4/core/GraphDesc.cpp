
#include <string>
#include <map>
#include <vector>

#include <string.h>

#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "LineReader.h"
#include "surfconsts.h"
#include "GraphDesc.h"



//----------------------------------------------------------------------------
// createInstance   
//
GraphDesc *GraphDesc::createInstance(std::string sFileName) {
    GraphDesc *pGD = new GraphDesc();
    int iResult = pGD->init(sFileName);
    if (iResult != 0) {
        delete pGD;
        pGD = NULL;
    }
    return pGD;
}


//----------------------------------------------------------------------------
// constructor   
//
GraphDesc::GraphDesc()
    : m_iNumNodes(0),
      m_iMaxLinks(0) {

    m_mNodeLinks.clear();
    m_sData[SURF_TYPE] = SURF_GRAPH;
    m_sData[SURF_GRAPH_LINKS] = std::to_string(m_iMaxLinks);

}

//----------------------------------------------------------------------------
// destructor   
//
GraphDesc::~GraphDesc() {
}


//----------------------------------------------------------------------------
// init   
//
int GraphDesc::init(std::string sFileName) {
    int iResult = -1;
    LineReader *pLR = LineReader_std::createInstance(sFileName, "rt");
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine();
        if (HEADER_LINE == pLine) {

            pLine = pLR->getNextLine();
            if (strstr(pLine, NUMNODE_PREFIX.c_str()) == pLine) {
                char *px = strchr(pLine, '=');
                if (px != NULL) {
                    *px = '\0';
                    px++;
                    uint iN = 0;
                    if (strToNum(px, &iN)) {
                        m_iNumNodes = iN;
                        pLine = pLR->getNextLine();
                        if (strstr(pLine, MAXLINKS_PREFIX.c_str()) == pLine) {
                            char *py = strchr(pLine, '=');
                            uint iL = 0;
                            if (py != NULL) {
                                *py = '\0';
                                py++;
                                if (strToNum(py, &iL)) {
                                    m_iMaxLinks = iL;

                                    
                                    iResult = readNodes(pLR);
                                    if (iResult == 0) {
                                        iResult = readLinks(pLR);
                                        if (iResult == 0) {
                                            stdprintf("Successfully read file [%s]\n", sFileName);
                                        }
                                    }
                                } else {
                                    iResult = -1;
                                    stdprintf("Expected number instead of [%s]\n", px);
                                }        
                                
                            } else {
                                iResult = -1;
                                stdprintf("Expected '=' in 'MAXLINKS=' line instead of [%s]\n", pLine);
                            }        

                        } else {
                            iResult = -1;
                            stdprintf("Expected 'NUMNODES=' line instead of [%s]\n", pLine);
                        }        
                    } else {
                        iResult = -1;
                        stdprintf("Expected number instead of [%s]\n", px);
                    }        
                    
                } else {
                    iResult = -1;
                    stdprintf("Expected '=' in 'NUMNODE=' line instead of [%s]\n", pLine);
                }        

                } else {
                    iResult = -1;
                    stdprintf("Expected 'NUMNODES=' line instead of [%s]\n", pLine);
                }        
        } else {
            iResult = -1;
            stdprintf("Expected header line instead of [%s]\n", pLine);
        }

        
        delete pLR;
    } else {
        iResult = -1;
        stdprintf("Couldn't open [%s] for reading\n", sFileName);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readNodes   
//
int GraphDesc::readNodes(LineReader *pLR) {
    int iResult = 0;
    char *pLine = pLR->getNextLine();
    
    if (strcmp(pLine, NODES_BEGIN.c_str()) == 0) {
        pLine = pLR->getNextLine();
        while (/*pLR->isEoF() &&*/ (pLine != NULL) && (strcmp(NODES_END.c_str(), pLine) != 0)) {
            //stdprintf("node line: [%s]\n", pLine);
            stringvec vParts;
            uint iNum = splitString(pLine, vParts, " \t", false);
            
            for (uint i = 0; (i < iNum) && (iResult == 0); i++) {
                uint iNode;
                if (strToNum(vParts[i], &iNode)) {
                    intvecmap::const_iterator it = m_mNodeLinks.find(iNode);
                    if (it == m_mNodeLinks.end()) {
                        m_mNodeLinks[iNode].clear();
                    } else {  
                        iResult = -1;
                        stdprintf("The node number [%u] is already registered\n", iNode);
                    }
                } else {
                    iResult = -1;
                    stdprintf("Expected a node number, not [%s]\n", vParts[i]);
                }

            }

            pLine = pLR->getNextLine();
        }
    } else {
        iResult = -1;
        stdprintf("Expected 'NODES_BEGIN' line instead of [%s]\n", pLine);
    }
    if (NODES_END == pLine) {
        if (m_mNodeLinks.size() != m_iNumNodes) {
            iResult = -1;
            stdprintf("Found %u nodes in NODES section intead of %u\n", m_mNodeLinks.size(), m_iNumNodes);
        }
    } else {
        iResult = -1;
        stdprintf("Expected 'NODES_END' line instead of [%s]\n", pLine);
    }
    return iResult;

}

//----------------------------------------------------------------------------
// readLinks   
//
int GraphDesc::readLinks(LineReader *pLR) {
    int iResult = 0;
    char *pLine = pLR->getNextLine();
    if (LINKS_BEGIN == pLine) {
        pLine = pLR->getNextLine();
        while (/*pLR->isEoF() &&*/ (pLine != NULL) && (strcmp(LINKS_END.c_str(), pLine) != 0)) {
            //stdprintf("link line: [%s]\n", pLine);
            stringvec vParts;
            uint iNum = splitString(pLine, vParts, " \t:", false);
            
            uint iCurNode = 0;
            if (strToNum(vParts[0], &iCurNode)) {
                    intvecmap::iterator it = m_mNodeLinks.find(iCurNode);
                    if (it != m_mNodeLinks.end()) {
                        if (iNum <= m_iMaxLinks) {
                            iResult = -1;
                            stdprintf("Node [%d] has %u links (max links:%u)\n", vParts[0], iNum-1, m_iMaxLinks);
                        } 
                        if (iResult == 0) {
                            std::vector<uint> &vLinks = it->second;

                            for (uint i = 1; (i < iNum) && (iResult == 0); i++) {
                                
                                uint iNode;
                                if (strToNum(vParts[i], &iNode)) {
                                    intvecmap::const_iterator it = m_mNodeLinks.find(iNode);
                                    if (it != m_mNodeLinks.end()) {
                                        // @@ check if link already in link list
                                        std::vector<uint>::const_iterator it2 = std::find(vLinks.begin(), vLinks.end(), iNode);
                                        if (it2 == vLinks.end()) {
                                            vLinks.push_back(iNode);
                                        } else {
                                            iResult = -1;
                                            stdprintf("The the link %u for node [%u] is a duplicate\n", iNode, iCurNode);
                                        }
                                    } else {  
                                        iResult = -1;
                                        stdprintf("The node number [%u] is not registered\n", iNode);
                                    }
                                
                                } else {
                                    iResult = -1;
                                    stdprintf("Expected a node number, not [%s]\n", vParts[i]);
                                }
                            }
                        }
                    } else {
                        iResult = -1;
                        stdprintf("Node [%d] is not registered\n", iCurNode);
                    }

            }   else {
                iResult = -1;
                stdprintf("Expected a node number, not [%s]\n", vParts[0]);
            }
 
            pLine = pLR->getNextLine();

        }
    } else {
        iResult = -1;
        stdprintf("Expected 'LINKS_BEGIN' line instead of [%s]\n", pLine);
    }
    if (LINKS_END != pLine) {
        iResult = -1;
        stdprintf("Expected 'LINKS_END' line instead of [%s]\n", pLine);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readLinks   
//
int GraphDesc::write(std::string sFileName) {
    int iResult = -1;

    // determine number of digits for output ints
    uint p = 1;
    uint k = m_iNumNodes;
    while (k > 0) {
        k /= 10;
        p++;
    }
    //printf("N%d -> %d\n", m_iNumNodes, p);

    FILE *fOut = fopen(sFileName.c_str(), "wt");
    if (fOut != NULL) {

        // header
        stdfprintf(fOut, "%s\n", HEADER_LINE);
        stdfprintf(fOut, "%s%d\n", NUMNODE_PREFIX, m_iNumNodes);
        stdfprintf(fOut, "%s%d\n", MAXLINKS_PREFIX, m_iMaxLinks);

        // nodes
        stdfprintf(fOut, "%s\n", NODES_BEGIN);
        intvecmap::const_iterator it;
        int iC = 0;
        for (it=m_mNodeLinks.begin(); it != m_mNodeLinks.end(); ++it) {
            stdfprintf(fOut, "%*d", p, it->first);
            iC++;
            if ((iC % 16) == 0) {
                stdfprintf(fOut, "\n");
            }
        }
        if ((iC % 16) != 0) {
            stdfprintf(fOut, "\n");
        }
        stdfprintf(fOut, "%s\n", NODES_END);

        // links
        stdfprintf(fOut, "%s\n", LINKS_BEGIN);
        for (it=m_mNodeLinks.begin(); it != m_mNodeLinks.end(); ++it) {
            stdfprintf(fOut, " %*d: ", p, it->first);
            for (uint i = 0; i < it->second.size(); i++) {
                stdfprintf(fOut, "%*d", p, it->second[i]);
            }
            stdfprintf(fOut, "\n");
        }
        stdfprintf(fOut, "%s\n", LINKS_END);
        

        fclose(fOut);
    } else {
        stdprintf("Couldn't open [%s] for writing\n", sFileName);
    }
    return iResult;
}
