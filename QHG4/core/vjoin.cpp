#include <cstdio>
#include <vector>
#include <string>

typedef std::vector<std::string> stringvec;
typedef unsigned int             uint;

std::string join(stringvec &v, std::string sSep) {
    std::string sJoin  = "";
    for (uint i = 0; i < v.size(); i++) {
        if (!sJoin.empty()) {
            sJoin = sJoin + sSep;
        }
        sJoin = sJoin + v[i];
    }
    return sJoin;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    stringvec v = {"the", "quick", "brown", "fox"};
    std::string s;
    s = join(v, " + ");
    printf("result: %s\n", s.c_str());
    return iResult;
}
