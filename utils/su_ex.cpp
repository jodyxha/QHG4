#include <vector>
#include <string>
#include <iostream>
#include "stdstrutilsT.h"


typedef unsigned int uint;
typedef std::vector<std::string> stringvec;

typedef std::vector<std::vector<stringvec>> stringvecvecvec;

// an object with operator <<
class testobj {
public:
    testobj(int iA, std::string sB): m_iA(iA), m_sB(sB) {};

    friend std::stringstream& operator<<(std::stringstream& os, const testobj& o);

    int m_iA;
    std::string m_sB;
};

std::stringstream& operator<<(std::stringstream& os, const testobj& o) {
    os  << "(" << o.m_iA << ";" << o.m_sB << ")";
    return os;
}


int main(int iArgC, char *apArgV[]) {

    // initialize some paraeters
    stringvec vStrings = {"eis", "zwei", "drey", "vier"};
    intvec    vNumbers = {2,7,1,8,2,8,1};
    testobj ob1(3,"three");
    testobj ob2(4,"four");
    std::vector<testobj> vecobj = {ob1, ob2};
    stringvecvecvec varlen = {{{"one","eis"},{"two","zwei","due"}},{{"tre"}},{{"four","vier", "quatro", "quatre"},{"five", "foif","cinque"}}};

    // print them
    stdprintf("numbers (\"%%v\"):  %v\n",  vNumbers);
    stdprintf("numbers (\"%%bv\"): %bv\n", vNumbers);
    stdprintf("varlen  (\"%%v\"):  %v\n",  varlen);
    stdprintf("varlen  (\"%%rv\"): %rv\n", varlen);
    stdprintf("ob1     (\"%%O\"):  %O\n",  ob1);
    stdprintf("vecobj: (\"%%v\");  %v\n",  vecobj);
    stdprintf("vecobj: (\"%%bv\"); %bv\n", vecobj);
    return 0;
}
