#include <cstdio>

#include "qhgXML.h"


void showNode(qhgNode *pN, const char *pIndent) {
    printf("%sName %s\n", pIndent, pN->getName());
    printf("%s  Attrs:\n", pIndent);
    attrlist &mA = pN->getAttrs();
    attrlist::const_iterator it;

    for (it = mA.begin(); it != mA.end(); ++it) {
        printf("%s    %s : %s\n", pIndent, it->first.c_str(), it->second.c_str());
    }

    printf("%s  Children:\n", pIndent);
    char sIndent[256];
    sprintf(sIndent, "%s  ", pIndent);
    qhgNode *pC = pN->getChild();
    while (pC != NULL) {
        showNode(pC, sIndent);
        pC = pC->getNext();
    }
}

int main(int iArgC, char *apArgV[]) {

    std::map<std::string, std::string>;
    mAttr["name"]="value";
    
    int iResult = -1;
    if (iArgC > 1) {
        qhgXML *pQX = qhgXML::createInstance(apArgV[1]);
        if (pQX != NULL) {
            printf("qhgXML finished\n");
            showNode(pQX->getRoot(), "");
            delete pQX;
        } else {
            printf("Couldn't create qhgXML\n");
            
        }
    } else {
        printf("usage %s <xmlfile>\n", apArgV[0]);
    }
    return iResult;
}
        
