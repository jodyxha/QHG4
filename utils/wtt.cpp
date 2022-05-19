#include <cstdio>
#include <cstring>

#include <vector>
#include <string>

typedef unsigned int uint;

uint splitString(const std::string sOriginal, const char *pSep, std::vector<std::string> &vParts) {
    char sCopy[sOriginal.size()+1];
    strcpy(sCopy, sOriginal.c_str());
    uint iNum = 0;
    char *p = strtok(sCopy, pSep);
    while (p != NULL) {
        iNum++;
        vParts.push_back(p);
        p = strtok(NULL, pSep);
    }
    return iNum;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    std::string s0{"tratratralllalew"};
    std::vector<std::string> vParts0;
    std::string s1{"der Mann, der im Garten steht, sagt ';'!"};
    std::vector<std::string> vParts1;
    std::string s2{";der Mann, der im Garten steht, sagt ';'!"};
    std::vector<std::string> vParts2;
    std::string s3{"der Mann, der im Garten steht, sagt ';'!;"};
    std::vector<std::string> vParts3;
    std::string s4{";der Mann, der im Garten steht, sagt ';'!;"};
    std::vector<std::string> vParts4;

    printf("%s\n", s0.c_str());
    uint iNum0 = splitString(s0.c_str(), ",; ", vParts0);
    printf("  %d |", iNum0);
    for (uint i = 0; i < vParts0.size(); i++) {
        printf("%s|", vParts0[i].c_str());
    }
    printf("\n");


    printf("%s\n", s1.c_str());
    uint iNum1 = splitString(s1.c_str(), ",; ", vParts1);
    printf("  %d |", iNum1);
    for (uint i = 0; i < vParts1.size(); i++) {
        printf("%s|", vParts1[i].c_str());
    }
    printf("\n");


    printf("%s\n", s2.c_str());
    uint iNum2 = splitString(s2.c_str(), ",; ", vParts2);
    printf("  %d |", iNum2);
    for (uint i = 0; i < vParts2.size(); i++) {
        printf("%s|", vParts2[i].c_str());
    }
    printf("\n");


    printf("%s\n", s3.c_str());
    uint iNum3 = splitString(s3.c_str(), ",; ", vParts3);
    printf("  %d |", iNum3);
    for (uint i = 0; i < vParts3.size(); i++) {
        printf("%s|", vParts3[i].c_str());
    }
    printf("\n");


    printf("%s\n", s4.c_str());
    uint iNum4 = splitString(s4.c_str(), ",; ", vParts4);
    printf("  %d |", iNum4);
    for (uint i = 0; i < vParts4.size(); i++) {
        printf("%s|", vParts4[i].c_str());
    }
    printf("\n");

    return iResult;
}
