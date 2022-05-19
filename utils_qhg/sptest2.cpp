#include <iostream>
#include <sstream>
#include <string>
#include <vector>

typedef std::vector<std::string> stringvec;

// dummy function: return string version of first vectorelemenr
template<typename T>
std::string vecDummy(const std::string sFormat, const T t) {
    std::stringstream ss("");
    if (t.size() > 0) {
        ss << t[0];
    }
    return  ss.str();    
}

// recursion termination
//template<typename T>
std::string recursiveY(stringvec &vFlags,  uint i) {
    return "";
}

// walk through arguments
template<typename T, typename... Args>
std::string recursiveY(stringvec &vFlags,  uint i, T value, Args... args) {
   
    std::string sRes = "";
    if (vFlags[i] == "%v") {
        sRes += vecDummy(vFlags[i], value);
    }    
    sRes += " "+recursiveY(vFlags, i+1, args...);
    
    return sRes;
}


int main(void) {
    stringvec vPasta   = {"spagis", "nudle", "penne", "tortellini"};
    stringvec vFormats = {"%v", "%s"};

    std::string st = "";
    st += recursiveY(vFormats, 0, vPasta, "test12");
    

    std::cout << ">>" << st  << "<<" << std::endl;

    /*
    stringvec vFlags = {"circle1", "circle2", "circle3"};
    std::string t = recursiveArea(vFlags, 0, 0.2, 3, 4.1);
    std::cout << t << std::endl;
    
    std::string t1 = stdsprintf("X%%");
    printf("x+:[%s]\n", t1.c_str());
    stdprintf("x+:[%s]\n", t1.c_str());
    std::string t2 = stdsprintf("%%Y");
    printf("+Y:[%s]\n", t2.c_str());
    stdprintf("+Y:[%s]\n", t2.c_str());
    std::string t3 = stdsprintf("X%%Y");
    printf("x+y:[%s]\n", t3.c_str());
    stdprintf("x+y:[%s]\n", t3.c_str());
    std::string t4 = stdsprintf("%%X%%Y");
    printf("+x+y:[%s]\n", t4.c_str());
    stdprintf("+x+y:[%s]\n", t4.c_str());
    std::string t5 = stdsprintf("X%%Y%%");
    printf("x+y+:[%s]\n", t5.c_str());
    stdprintf("x+y+:[%s]\n", t5.c_str());
    std::string t6 = stdsprintf("%%X%%Y%%");
    printf("+x+y+:[%s]\n", t6.c_str());
    stdprintf("+x+y+:[%s]\n", t6.c_str());
    
    std::string t7 = stdsprintf("%%X%%");
    printf("+x+:[%s]\n", t7.c_str());
    stdprintf("+x+:[%s]\n", t7.c_str());

    std::string t8 = stdsprintf("%%%%X");
    printf("++x:[%s]\n", t8.c_str());
    stdprintf("++x:[%s]\n", t8.c_str());

    std::string t9 = stdsprintf("%%%%");
    printf("++:[%s]\n", t9.c_str());
    stdprintf("++:[%s]\n", t9.c_str());

    std::string t10 = stdsprintf("%%%%0");
    printf("++0:[%s]\n", t10.c_str());
    stdprintf("++0:[%s]\n", t10.c_str());
    
    std::string t11 = stdsprintf("%%%%0X");
    printf("++0x:[%s]\n", t11.c_str());
    stdprintf("++0x:[%s]\n", t11.c_str());
    
    std::string t12 = stdsprintf("%%0d");
    printf("+0d:[%s]\n", t12.c_str());
    stdprintf("+0d:[%s]\n", t12.c_str());
    
    std::string t13 = stdsprintf("%%%%0d");
    printf("++0d:[%s]\n", t13.c_str());
    stdprintf("++0d:[%s]\n", t13.c_str());

    std::string t14 = stdsprintf("%%%%0d %s", "argh");
    printf("++0d:[%s]\n", t14.c_str());
    stdprintf("++0d:[%s]\n", t14.c_str());
    
    std::string t15 = stdsprintf("%%q %s", "argh");
    printf("++q %%s:[%s]\n", t15.c_str());
    stdprintf("++q %%s:[%s]\n", t15.c_str());
        
    std::string t16 = stdsprintf("%s_%0*d.dat", "argh", 7, 3);
    printf("%%s_%%0*d.dat:[%s]\n", t16.c_str());
    stdprintf("%%s_%%0*d.dat:[%s]\n", t16.c_str());
    

        
    std::string s2 = stdsprintf("%%0d");
    printf("[%%%%0d]      -> [%s]\n", s2.c_str());
    stdprintf("[%%%%0d]      -> [%s]\n", s2);
    */
    return 0;
}
