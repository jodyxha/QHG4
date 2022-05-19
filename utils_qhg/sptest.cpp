#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <type_traits>

#include "stdstrutils.h"
#include "stdstrutilsT.h"


typedef std::vector<std::string> stringvec;


template<typename T>
std::string calcCircleArea(T value) {
    return std::to_string(3.14159*value*value);
}

template<typename T, typename U>
std::string calcCircleArea(T value1, U value2) {
    return std::to_string(value1*value2);
}

//----------------------------------------------------------------------------
//  recursiveFormat
//    normale recursion level
//
template<typename T, typename... Args>
std::string recursiveArea(stringvec vFlags,  uint i) {
    return "";
}

//----------------------------------------------------------------------------
//  recursiveFormat
//    normale recursion level
//
template<typename T, typename... Args>
std::string recursiveArea(stringvec vFlags,  uint i, T value, Args... args) {
    std::string sRes = "";
    if (i < vFlags.size()) {
        if (vFlags[i] == "circle") {
            sRes += calcCircleArea(value);
        } else if (vFlags[i] == "square") {
            //            auto value2 = fetchNextParam(args...);
            //            sRes += calcSquareArea(value, value2);
            i++; // for a square we needed 2 arguments
        } else {
        }
        // handle the next tasks
        sRes += " "+recursiveArea(vFlags, i+1, args...);
    }
    return sRes;
}




/*
template<typename U, typename... Args>
U fetchNextParam(const U value2, Args... args) {
    return value2;
}

template<typename T, typename U>
std::string starFormat(const std::string sFormat, const T value1, const U value2) {

    size_t required = snprintf(NULL, 0, sFormat.c_str(), value1, value2);
    char sTemp[required+1];
    sprintf(sTemp, sFormat.c_str(), value1, value2);
    return std::string(sTemp);
} 


//----------------------------------------------------------------------------
// recursiveFormat
//   termination version (of stdstrutilsT::recursiveFormat)
//
std::string recursiveFormat(stringvec &vParts, stringvec &vFormats, uint i) {
    return vParts[i];
}

//----------------------------------------------------------------------------
// simpleFormat
//  format a char using sFormat
//
template<typename T>
std::string simpleFormat(const std::string sFormat, const T t) {

    size_t required = snprintf(NULL, 0, sFormat.c_str(), t);
    char sTemp[required+1];
    sprintf(sTemp, sFormat.c_str(), t);
    return std::string(sTemp);
} 

//----------------------------------------------------------------------------
//  recursiveFormat
//    normale recursion level
//
template<typename T, typename... Args>
std::string recursiveFormat(stringvec &vParts, stringvec &vFormats, uint i, T value, Args... args) {

    std::string sRes = "";
    if (i < vFormats.size()) {
        sRes += vParts[i];
        //--- trying to handle star
        
        
        if (vFormats[i].find('*') != std::string::npos) {
            if constexpr (sizeof...(Args) > 0) {
                    auto value2 = fetchNextParam(args...);
                    sRes += starFormat(vFormats[i], value, value2);
                    //    i++; //for a star expression we need 2 arguments
            } else {
                throw std::runtime_error("wrong number of arguments\n");
            }
            
        } else {
        
            sRes += simpleFormat(vFormats[i], value);
        }
        sRes += recursiveFormat(vParts, vFormats, i+1, args...);
    }
    return sRes;
}
*/
/*
template<typename T>
std::string vecToString(const std::string sFormat, const T t) {
    std::string sVec = "";
    if ((sFormat == "%v") || (sFormat == "%bv")) {
        bool bBrack = (sFormat[1] == 'b');

        if (bBrack) {
            sVec += "[";
        }
        for (uint i = 0; i < t.size(); ++i) {
            if (i > 0) {
                if (bBrack) {
                    sVec += ", ";
                } else {
                    sVec += " ";
                }
            }
            std::stringstream ss("");
            ss << t.at(i);
            sVec += ss.str();    
            //            sVec += t.at(i);
        }
        if (bBrack) {
            sVec += "]";
        }
        
    } else {
        throw std::runtime_error("unknown vector format\n");
    }
    return sVec;
}

//----------------------------------------------------------------------------
//  recursiveX
//    normale recursion level
//
template<typename T>
std::string oinkX(std::string sFlags,  uint i, T value) {
    std::string sRes = "";
    if ((sFlags == "%v") || (sFlags == "%bv")) {
        sRes += vecToString(sFlags, value);
    } else {
    }
        // handle the next tasks

    
    return sRes;
}


template<typename T>
std::string vecDummy(const std::string sFormat, const T t) {
    std::string sVec = "";
    if (sFormat == "%v")  {
        std::stringstream ss("");
        ss << t[0];
        sVec += ss.str();    
        
    } else {
        throw std::runtime_error("unknown vector format\n");
    }
    return sVec;
}

template<typename T>
std::string recursiveY(stringvec vFlags,  uint i) {
    return "";
}

//----------------------------------------------------------------------------
//  recursiveY
//    normale recursion level
//
template<typename T, typename... Args>
std::string recursiveY(stringvec vFlags,  uint i, T value, Args... args) {
   
  std::string sRes = "";
  if (i < vFlags.size()) {
      if (vFlags[i] == "%v") {
          sRes += vecDummy(vFlags[i], value);
      }
        // handle the next tasks
       sRes += " "+recursiveY(vFlags, i+1, args...);
    }
    return sRes;
}
*/

int main(void) {
    /*
    stringvec vParts = {"part 1:", " part2:", " part3:", " part4:", ""};
    stringvec vFormats = {"%03d", "%d", "%s", "%0*d"};

    std::string s = recursiveFormat(vParts, vFormats, 0, 1, 3, "flarz", 5, 7);
    std::cout << s << std::endl;

    stdprintf("bane is %s_%d%s.dat\n", "zomblorg", 99, "C1");
    
    */

    //    stdprintf("bane is %s_%d%s.dat\n", "zomblorg", 99, "C1");
    //    stdprintf("line is %s and %*.d of %s uff\n", "morlock", 4, 99, "C3");
    
    stringvec vPasta = {"spagis", "nudle", "penne", "tortellini"};
    intvec vNumbers = {3,1,4, 1, 5, 9, 2, 6, 5};
    //stdprintf(" the vector %s has [%d]\n\n", "oinksly", 13);
    /*
    stdprintf(" the vector %s has %0*d elements[0: %s]g\n", "vPasta", 3, vPasta.size(), vPasta[0]);
    stdprintf("%d sez the vector %s has %0*d elements[0: %s]g\n", 1, "vPasta", 3, vPasta.size(), vPasta[0]);
    stdprintf(" the vector %s has %0*d elements[0: %s] with %dg\n", "vPasta", 3, vPasta.size(), vPasta[0], 22);
    */
    stdprintf(" the vector %s has %0*d elements[0: %s] with %0*dpf\n", "vPasta", 3, vPasta.size(), vPasta[0], 4, 22);
    
    //stdprintf("oinks with %0*d \n", 3, 22);
    
    stdprintf("The vector %s has size %0*d with contents [%v]\n", "vPasta", 3, vPasta.size(), vPasta);
    stdprintf("The vector %s has size %0*d with contents [%v]\n", "vNumbers", 4, vNumbers.size(), vNumbers);

    std::string s3 = vecFormat("%bv", vPasta);
    stdprintf("%s\n", s3);
    /*
    std::string sV = vecToString("%bv", vPasta);
    stdprintf("pasta vecToSring:[%s]\n", sV);
    std::string sZ = vecToString("%bv", vZ);
    stdprintf("vZ vecToSring:[%s]\n", sZ);
    stdprintf("%s%s [%d]:\n%s  ", "  ", "sCaption", 3, "  ");
    
    
    stringvec vF={"%v", "%d", "%s", "%v"};
    std::string sr = "";
    sr += oinkX("%v", 0, vPasta);
    sr += oinkX("%bv", 0, vZ);

    std::cout << "trulllal ---" << sr  << " ---llalllurt" << std::endl;


    std::string st = "";
    st += recursiveY(vF, 0, vPasta,13, "schn", vZ);
    

    std::cout << "trulllal ---" << st  << " ---llalllurt" << std::endl;
        */
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
