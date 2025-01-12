#include <string>
#include <vector>
#include <iostream>
#include <cstdarg>

#include "types.h"
#include "colors.h"

//#include "xha_strutils.cpp" 
#include "xha_strutilsT.h" 









template <typename... Ts>
std::string fmt1(const std::string &fmt, Ts... vs)
{
    //char b;
    size_t required = std::snprintf(nullptr, 0, fmt.c_str(), vs...) + 1;
        // See comments: the +1 is necessary, while the first parameter
        //               can also be set to nullptr

    char *bytes=new char[required];
    printf("sizeof buf: %zd\n", sizeof(bytes));
    std::snprintf(bytes, required, fmt.c_str(), vs...);
    std::string sOut = bytes;
    return std::string(sOut);
}

template<typename T>
void stntest(const std::string s, T *t) {
    if (strToNum(s, t)) {
        std::cout << s << " is: [" << *t << "]\n";
    } else {
        std::cout << xha_sprintf("couldn't convert [%s]\n", s);
    }
}


template <typename... Ts>
void pfmt1(const std::string &fmt, Ts... vs)
{
    std::string sOut = fmt1(fmt, vs...);
    std::cout << sOut;
}



void basic_tests() {

    std::string sTests[] ={ "%d apples found in box %s with an averge weight of %03.5fg (about %.2f%%) so what",
                            "%d apples found in box %s with an averge weight of %03.5fg (about %.2f%%)",
                            "there are %d apples found in box %s with an averge weight of %03.5fg (about %.2f%%)",
                            "there are %d apples found in box %s with an averge weight of %03.5fg, not %d"};
    
    std::string sTest =     "there are %d apples found in box %s [%20s] [%-20S] with an averge weight of %03.5fg, this is a char [%c], this i an exp [%e], this is a hex[%08x], not %d%%";
    std::string sInsi =     "pflotsch";


    std::string sRes = "";

    
    std::string sInsert = "ice";
    std::cout << "----the nefmt---\n";
    sRes = fmt1("%d bottles of %s, and nuffing: ", 15, "rum");
    std::cout << "Result fmt [" << sRes << "]\n";
    std::cout << "----the pfmt---\n";
    pfmt1("hohoho, %d bottles of %s, and nuffing more\n", 771, "rum");
    std::cout << "----the new xha_sprintf---\n";
    sRes = xha_sprintf("hohoho, %d bottles of %s, and %s", 771, "rum", sInsert);
    std::cout << "Result xha_sprintf [" << sRes << "]\n";
    sRes = xha_sprintf("%d ho's, %d bottles of %s, and %s", 3, 771, "rum", sInsert);
    std::cout << "Result xha_sprintf [" << sRes << "]\n";
    sRes = xha_sprintf("%d ho's, %d bottles of %s, and %s!!", 3, 771, "rum", sInsert);
    std::cout << "Result xha_sprintf [" << sRes << "]\n";
    sRes = xha_sprintf("%d ho's, %ld bottles of %s, and %s!!", 3, 771l, "rum", sInsert);
    std::cout << "Result xha_sprintf [" << sRes << "]\n";
    sRes = xha_sprintf("%d ho's, %ld bottles of %s, and %s!!", 3, 771l, "rum", sInsert);
    std::cout << "Result xha_sprintf [" << sRes << "]\n";

    std::cout << "\n";
    std::cout << "----the new xha_printf---\n";
    xha_printf("hohoho, %d bottles of %s, and %s", 771, "rum", sInsert);
    std::cout << "\n";
    xha_printf("%d ho's, %d bottles of %s, and %s", 3, 771, "rum", sInsert);
    std::cout << "\n";
    xha_printf("%d ho's, %d bottles of %s, and %s!!", 3, 771, "rum", sInsert);
    std::cout << "\n";
    xha_printf("%d ho's, %ld bottles of %s, and %s!!", 3, 7777727277271l, "rum", sInsert);
    std::cout << "\n";


    std::cout << "\n";
    std::cout << "----the new strtonum---\n";
    
    char c=0;
    std::string sc = "69";
    stntest(sc, &c);
    c = c + 255;
    std::cout << c << "\n";;
    printf("as number: %d\n", c);

    short int si=0;
    std::string ssi = "333";
    stntest(ssi, &si);
    si = si + 65535;
    std::cout << si << "\n";;

    uint ui=0;
    std::string sui = "3";
    stntest(sui, &ui);
    ui = ui - 5; 
    std::cout << ui << "\n";;

    int i=0;
    std::string syi = "666666";
    stntest(syi, &i);
    i = i - 7777;
    std::cout << i << "\n";;

    long long ll=0;
    std::string sll = "31231231231232";
    stntest(sll, &ll);
    std::cout << ll  + 11111111111111<< "\n";;

    long l=0;
    std::string sl = "1231231231";
    stntest(sl, &l);
    std::cout << l  + 1111111111<< "\n";;


    float f = 0;
    std::string sf = "23123.1232";
    stntest(sf, &f);
    f = f * 5;
    std::cout << f << "\n";;

    double d = 0;
    std::string sd = "3123123123.1232";
    stntest(sd, &d);
    d = d * 10;
    std::cout << d << "\n";;



    char *pEnd;
    printf("now c\n");
    char xxsc = 0;
    xxsc = (char) strtol("3", &pEnd, 10);
    if (*pEnd == '\0') {
        std::cout << "ok: sc is[" << xxsc <<"]\n";
    }  else {
        printf("pEnd is [%s]\n", pEnd);
    }

    printf("now uc\n");
    unsigned char xxuc = 0;

    xxuc = (unsigned char) strtol("53", &pEnd, 10);
    
    if (*pEnd == '\0') {
        std::cout << "ok: c is[" << xxuc <<"]\n";
        printf("oook: [%d]\n", xxuc);
    }  else {
        printf("pEnd is [%s]\n", pEnd);
    }
    
    std::cout << printf("done\n");
    int k = 3;
    int *m_pCG = &k;
    std::cout <<"%d\n";
    xha_printf("[setGrid] Grid read successfully: %d\n", *m_pCG);
    std::cout <<"%p\n";
    xha_printf("[setGrid] Grid read successfully: %p\n", m_pCG);
   
}



// types for advanced tests
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



void advanced_tests() {
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

    xha_printf("metest\n"); 
    xha_printf("simple string: %s\n", s1); 
    xha_printf("obv: %bv, glonng:%d\n", vO,2); 
    xha_printf("two obs: %O & %O\n", ob1, ob2); 
        
    xha_printf("one: s %s, two: star %0*d, three: vecs %bv, four: vecn %bv end\n", s1, iW, iV, vStrings, vNumbers);
    xha_printf("%s, two: star %0*d, three: vecs %bv, four: vecn %bv end\n", s1, iW, iV, vStrings, vNumbers);
    xha_printf("%s, two: star %0*d, three: vecs %bv, four: vecn %bv\n", s1, iW, iV, vStrings, vNumbers);
    xha_printf("one: vecn %bv, two: s %s, three: star %0*d, four: vecs %bv end\n", vNumbers, s1, iW, iV, vStrings);
    xha_printf("dense %bv%s%0*d%bv esned\n", vNumbers, s1, iW, iV, vStrings);
    std::string s2 = xha_sprintf("%s, two: star %0*d, three: vecs %bv, four: vecn %bv", s1, iW, iV, vStrings, vNumbers);
    xha_printf("from xha_sprintf:[%s]\n", s2);
    xha_printf("one: vecn %bv, two: s %s, three: star %0*d, four: vecs %bv, five: o: % end\n", vNumbers, s1, iW, iV, vStrings, ob1);
    xha_printf("zero: vo %bv, one: vecn %bv, two: s %s, three: star %0*d, four: vecs %bv, five: o: %O end\n", vO, vNumbers, s1, iW, iV, vStrings, ob1);
    
    xha_printf("doublestar %*.*f\n", 12, 5, 3.1415);
    xha_printf("doublestar2 %*.*f for %d and %*s\n", 12, 5, 3.1415, 12, 13, "voegeli");

    xha_printf("doublestar bad %*.*f\n", 12, 5, "ee");
    xha_printf("two vecs %bv <-> %v\n", vStrings, vStrings);
    //    xha_printf("triplestar bad %*.*_*f\n", 12, 5, "ee");
    xha_printf("bombo pure %v\n", bombo);
    xha_printf("bombo brack %bv\n", bombo);
    xha_printf("bombo rec %rv\n", bombo);
    xha_printf("sss rec %rv\n", sss);

    stringvec vParts;
    uint iNum = splitString("12 4 66", vParts, " ");
    for (uint i = 0; i < iNum; i++) {
        int d;
        if (strToNum(vParts[i], &d)) {
            xha_printf("conv ok: %d\n", d);
        } else {
            xha_printf("conv err: '%s'\n", vParts[i]);
        }
    }
    xha_printf("and some %scolor%s!%s!%s!\n", colors::RED, colors::OFF, colors::BLUE, colors::OFF);
}



int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    //basic_tests();
    advanced_tests();

    return iResult;
}
