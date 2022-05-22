#include <iostream>
#include <sstream>
#include <string>
#include <vector>


#include "stdstrutils.h"
#include "stdstrutilsT.h"

class obloff {
public:
    obloff(int iA, std::string sB): m_iA(iA), m_sB(sB) {};

    friend std::ostream& operator<<(std::ostream& os, const obloff& o);
    friend std::stringstream& operator<<(std::stringstream& os, const obloff& o);

    int m_iA;
    std::string m_sB;
};

std::ostream& operator<<(std::ostream& os, const obloff& o) {
    os  << "(" << o.m_iA << ";" << o.m_sB << ")";
    return os;
}
std::stringstream& operator<<(std::stringstream& os, const obloff& o) {
    os  << "(" << o.m_iA << ";" << o.m_sB << ")";
    return os;
}

typedef std::vector<std::vector<stringvec>> stringvecvecvec;


int main(void) {
    int iResult = 0;
    std::string s1 = "blabla";
    int iW = 5;
    int iV = 471;
    stringvec vStrings = {"eis", "zwei", "drey", "vier"};
    intvec    vNumbers = {2,7,1,8,2,8,1};  
    obloff ob1(3,"three");
    obloff ob2(4,"four");
    std::vector<obloff> vO = {ob1, ob2};
    std::vector<stringvec> bombo = {{"one", "two", "three"},{"eis", "zwei", "drue"},{"uno", "due", "tre"}};
    stringvecvecvec sss = {{{"one","eis"},{"two","zwei","due"}},{{"tre"}},{{"four","vier", "quatro", "quatre"},{"five", "foif","cinque"}}};

    stdprintf("metest\n"); 
    stdprintf("simple string: %s\n", s1); 
    stdprintf("obv: %bv, glonng:%d\n", vO,2); 
    stdprintf("two obs: %O & %O\n", ob1, ob2); 
        
    stdprintf("one: s %s, two: star %0*d, three: vecs %bv, four: vecn %bv end\n", s1, iW, iV, vStrings, vNumbers);
    stdprintf("%s, two: star %0*d, three: vecs %bv, four: vecn %bv end\n", s1, iW, iV, vStrings, vNumbers);
    stdprintf("%s, two: star %0*d, three: vecs %bv, four: vecn %bv\n", s1, iW, iV, vStrings, vNumbers);
    stdprintf("one: vecn %bv, two: s %s, three: star %0*d, four: vecs %bv end\n", vNumbers, s1, iW, iV, vStrings);
    stdprintf("dense %bv%s%0*d%bv esned\n", vNumbers, s1, iW, iV, vStrings);
    std::string s2 = stdsprintf("%s, two: star %0*d, three: vecs %bv, four: vecn %bv", s1, iW, iV, vStrings, vNumbers);
    stdprintf("from stdsprintf:[%s]\n", s2);
    stdprintf("one: vecn %bv, two: s %s, three: star %0*d, four: vecs %bv, five: o: % end\n", vNumbers, s1, iW, iV, vStrings, ob1);
    stdprintf("zero: vo %bv, one: vecn %bv, two: s %s, three: star %0*d, four: vecs %bv, five: o: %O end\n", vO, vNumbers, s1, iW, iV, vStrings, ob1);
    
    stdprintf("doublestar %*.*f\n", 12, 5, 3.1415);
    stdprintf("doublestar2 %*.*f for %d and %*s\n", 12, 5, 3.1415, 12, 13, "voegeli");

    stdprintf("doublestar bad %*.*f\n", 12, 5, "ee");
    stdprintf("two vecs %bv <-> %v\n", vStrings, vStrings);
    //    stdprintf("triplestar bad %*.*_*f\n", 12, 5, "ee");
    stdprintf("bombo pure %v\n", bombo);
    stdprintf("bombo brack %bv\n", bombo);
    stdprintf("bombo rec %rv\n", bombo);
    stdprintf("sss rec %rv\n", sss);

    stringvec vParts;
    uint iNum = splitString("12 4 66", vParts, " ");
    for (uint i = 0; i < iNum; i++) {
        int d;
        if (strToNum(vParts[i], &d)) {
            stdprintf("conv ok: %d\n", d);
        } else {
            stdprintf("conv err: '%s'\n", vParts[i]);
        }
    }

    return iResult;
}
