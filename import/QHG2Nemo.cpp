#include <cstdio>
#include <cstring>
#include <vector>
#include <map>

#include "utils.h"
#include "strutils.h"
#include "types.h"
#include "LineReader.h"

typedef std::map<int,int> lookup;
typedef std::vector<int> nodelist;

int readIDX(char *pIDXFile, lookup &mQDF2IDX) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(pIDXFile, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *p = pLR->getNextLine();
            if (p != NULL) {
                char *p1 = strtok(p, " \t");
                if (p1 != NULL) {
                    char *p2 = strtok(NULL, " \t");
                    if (p2 != NULL) {
                        int iIDX;
                        int iQDF;
                        if (strToNum(p1, &iIDX)) {
                            if (strToNum(p2, &iQDF)) {
                                mQDF2IDX[iQDF] = iIDX;
                            } else {
                                printf("not a number [%s]\n", p2);
                                iResult = -1;
                            }
                        } else {
                            printf("not a number [%s]\n", p1);
                            iResult = -1;
                        }
                    } else {
                        printf("expected two numbers\n");
                        iResult = -1;
                    }
                } else {
                    printf("empty line?");
                    iResult = -1;
                }
            }
                }
    } else {
        printf ("Couldn't open [%s] for reading\n", pIDXFile);
    }
    return  iResult;
}

int readNodes(char *pQDF, nodelist &vNodes) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(pQDF, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *p = pLR->getNextLine();
            if (p != NULL) {
                int iNode;
                if (strToNum(p, &iNode)) {
                    vNodes.push_back(iNode);
                } else {
                    printf("not a number [%s]\n", p);
                    iResult = -1;
                }
            }
        }
    } else {
        printf ("Couldn't open [%s] for reading\n", pQDF);
    }
    return  iResult;
}

int translate(nodelist vNodes, lookup mQDF2IDX, FILE *fOut) {
    int iResult = 0;

    for (uint i = 0; (iResult == 0) && (i < vNodes.size()); i++) {
        lookup::const_iterator it = mQDF2IDX.find(vNodes[i]);
        if (it != mQDF2IDX.end()) {
            fprintf(fOut, "%d\n", it->second);
        } else {
            printf("Node ID [%d] not found in table\n", vNodes[i]);
            iResult = -1;
        }
    }
    return iResult;
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    if (iArgC > 2) {
        char *pIDXFile  = apArgV[1];
        char *pQDFNodes = apArgV[2];
        FILE *fOut = NULL;
        if  (iArgC > 3) {
            char *pOutputFile = apArgV[3];
            fOut = fopen(pOutputFile, "wt");
            if (fOut == NULL) {
                printf ("Couldn't open [%s] for writing\n", pOutputFile);
                iResult = -1;
            }
        } else {
            fOut = stdout;
        }

        if (iResult == 0) {
            lookup mQDF2IDX;
            iResult = readIDX(pIDXFile, mQDF2IDX);

            if (iResult == 0) {
                nodelist vNodes;
                iResult = readNodes(pQDFNodes, vNodes);

                if (iResult == 0) {
                    iResult = translate(vNodes, mQDF2IDX, fOut);
                    if (iResult == 0) {
                        printf("+++ success +++\n");
                    } else {
                        printf("--- failure ---\n");
                    }
                }
            }
        }
        if (iArgC >3) {
            fclose(fOut);
        }
    } else {
        printf(" %s <idxfile> <qdffile> [<outputfile>]\n", apArgV[0]);
    }
    return iResult;
}
