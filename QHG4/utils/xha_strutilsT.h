/*============================================================================
| xha_strutilsT.h
| 
|  Templated functions for std::string-capable printf and other tools
|  
|  Also see xha_strutils.h, xha_strutils.cpp
|
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __XHASTRUTILST_H__
#define __XHASTRUTILST_H__

#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <type_traits>
#include "xha_strutils.h" 

/*
template<typename... Args>
static std::string xha_sprintf(const std::string sFormat, Args... args);

template<typename... Args>
static void xha_fprintf(FILE *fOut, const std::string sFormat, Args... args);

template<typename... Args>
static void xha_printf(const std::string sFormat, Args... args);

template<typename T>
static bool strToNum(const std::string sData, T *t);

template<typename T>
static bool strToHex(const std::string sData, T *t);
*/

//----------------------------------------------------------------------------
// strToNum
//  for all integer types
//
template<typename T>
bool strToNum(const std::string sData, T *t) {
    bool bResult = false;
    char *pEnd;
    if (std::is_arithmetic<T>::value) {
        if (std::is_integral<T>::value) {
            *t = (T) strtol(sData.c_str(), &pEnd, 10);
        } else if (std::is_floating_point<T>::value) {
            *t = (T) strtod(sData.c_str(), &pEnd);
        } 
        bResult = (*pEnd == '\0');
    }
    return bResult;
}


//----------------------------------------------------------------------------
// strToHex
//  for all integer types
//
template<typename T>
bool strToHex(const std::string sData, T *t) {
    char *pEnd;
    *t = (T) strtol(sData.c_str(), &pEnd, 16);
    return (*pEnd == '\0');
}

/******************* xha_printf & co *******************/

struct sp_error:std::runtime_error {
    sp_error(std::string e, std::string _format): runtime_error(e), m_format(_format){};
    const std::string &format() { return m_format;};
    const std::string m_format;
};

//----------------------------------------------------------------------------
// vecFormat
//  catch-all for all types that are not vectors: do nothing 
//
template<typename T>
std::string vecFormat(const std::string sFormat, const T t) {
    return "";
}


//----------------------------------------------------------------------------
// vecFormat
//  format a vector with elements that have a << operator: 
//  loop through vector and  print every element individually. 
//  the 'b' option adds brackets and commas.
//  If a vector element is itself a vector, the 'r' option recursively 
//  prints the vector
//
template<typename T>
std::string vecFormat(const std::string sFormat, const std::vector<T> t) {
    std::string sVec = "";
    if ((sFormat == "%v") || (sFormat == "%bv") || (sFormat == "%rv")) {
        bool bBrackets  = (sFormat[1] == 'b') || (sFormat[1] == 'r');
        bool bRecursion = (sFormat[1] == 'r');
        if (bBrackets) {
            sVec += "[";
        }
        for (uint i = 0; i < t.size(); ++i) {
            if (i > 0) {
                if (bBrackets) {
                    sVec += ", ";
                } else {
                    sVec += " ";
                }
            }
            // elements without <<-operator are displayed as '?'
            std::stringstream ss("");
            if constexpr(requires {ss << t.at(i);}) {
                ss << t.at(i);
                sVec += ss.str();    
            } else {
                if (bRecursion) {
                    sVec += vecFormat(sFormat, t.at(i));
                } else {
                    sVec += "?";
                }
            }
            
        }   
        if (bBrackets) {
            sVec += "]";
        }
            
    } else {
        throw sp_error("unknown vector format", sFormat);
    }
    return sVec;
} 

#ifndef CRAY


//----------------------------------------------------------------------------
// fetchNextParam
//  get next argument from args
//  (fetchNextParams() with no arguments in xha_strutils.cpp)
//
//  CRAY: This function cant be compiled by the current cray compiler in 
//  Busan (8.7.5), because it doesn't like
//    "Non-POD class type passed through ellipsis."
//
template<typename U, typename... Args>
U fetchNextParam(const U value2, Args... args) {
    return value2;
}


//----------------------------------------------------------------------------
// fetchNextTwoParams
//  get the first two arguments from args
//  (fetchNextTwoParams() with no arguments in xha_strutils.cpp)
//
//  CRAY: This function cant be compiled by the current cray compiler in 
//  Busan (8.7.5), because it doesn't like
//    "Non-POD class type passed through ellipsis."
//
template<typename V, typename U, typename... Args>
std::pair<V, U> fetchNextTwoParams(const V value1, const U value2, Args... args) {
    return std::pair<V, U>{value1, value2};
}

template<typename U>
std::pair<U, U> fetchNextTwoParams(const U value2) {
    throw sp_error("Wrong number of arguments", "");
}



//----------------------------------------------------------------------------
// starFormat
//  format number (usually something like "%0*d" with two integers 
//  (width and value), where width replaces the star)
//
//  CRAY: This function cant be compiled by the current cray compiler in 
//  Busan (8.7.5), because it doesn't like
//    "Non-POD class type passed through ellipsis."
//
template<typename T, typename U, typename... Args>
std::string starFormat(const std::string sFormat, const T value1, const U value2) {

    size_t required = snprintf(NULL, 0, sFormat.c_str(), value1, value2);
    char *sTemp = new char[required+1];

    sprintf(sTemp, sFormat.c_str(), value1, value2);
    std::string sOut = sTemp;

    delete[] sTemp;
    return std::string(sOut);
} 


//----------------------------------------------------------------------------
// starFormat2
//  format a float (usually something like "%*.*f" with two integers 
//  (width and prec), where width and  replace the stars)
//  In order two avoid compiler errors, we make sure that that width and prec
//  are integers and the value is a number.
//
//  CRAY: This function cant be compiled by the current cray compiler in 
//  Busan (8.7.5), because it doesn't like
//    "Non-POD class type passed through ellipsis."
//
template<typename T, typename U, typename V, typename... Args>
std::string starFormat2(const std::string sFormat, const T value1, const U value2, const V value3) {
    std::string sOut;
    if (std::is_integral<T>::value && std::is_integral<U>::value && (std::is_integral<V>::value || std::is_floating_point<V>::value)) {
   
        size_t required = snprintf(NULL, 0, sFormat.c_str(), value1, value2, value3);
        char *sTemp = new char[required+1];

        sprintf(sTemp, sFormat.c_str(), value1, value2, value3);

        sOut = sTemp;
        delete[] sTemp;
    } else {
        throw sp_error("non-integral width or precision for doublestar format", sFormat);
    }
    return sOut;

} 

#endif


//----------------------------------------------------------------------------
// simpleFormat
//  catch for vectors (should not be sent to simpleFormat, but the compiler
//  doesn't know)
//
template<typename T>
std::string simpleFormat(const std::string sFormat, const std::vector<T> t) {
    return "";
}


//----------------------------------------------------------------------------
// simpleFormat
//  format a POD or an object with <<-operator
//  The format string must have the form recognized by sprintf,
//  or have the form "%O" (big Oh). 
//  The 'O' option allows the printing of any object with an 
//  associated <<-operator. To make this compilable, we use the c++20
//  type function 'requires'. 
//
//  CRAY: The current Cray compiler used in Busan (8.7.5) can't do std=c++20, 
//  that's why the else-branch is excluded for CRAY.
//
template<typename T>
std::string simpleFormat(const std::string sFormat, const T t) {
    std::string sRes = "";
    
    if (sFormat.size() > 0) {
        if (sFormat != "%O") {
            size_t required = snprintf(NULL, 0, sFormat.c_str(), t);
            char *sTemp = new char[required+1];

            sprintf(sTemp, sFormat.c_str(), t);

            sRes = std::string(sTemp);
            delete[] sTemp;
        } else {
#ifndef CRAY
            std::stringstream ss("");
            if constexpr(requires {ss << t;}) {
               
                ss << t;
                sRes += ss.str();    
            }
#endif

        }
    } 
    
    return sRes;
} 


//----------------------------------------------------------------------------
//  recursiveFormat
//    normal recursion level
//    (the termination function is defined in xha_strutils.cpp)
//    Assumptions
//    - vParts has one more element than vFormats
//    - the number of arguments in Args is equal to the number
//      of values required by the format strings
//      
//    CRAY: fetchNextParam() and statFormat() pass non-POD types through the 
//    ellipsis, which the current Cray compiler in Busan (8.7.5) doesn't like.
//    
template<typename T, typename... Args>
std::string recursiveFormat(stringvec &vParts, stringvec &vFormats, uint i, T value, Args... args) {

    std::string sRes = "";
    if (i < vFormats.size()) {
        sRes += vParts[i];
        //--- trying to handle star

        if (vFormats[i].find('*') != std::string::npos) {
#ifndef CRAY
            size_t iNumStars = std::count(vFormats[i].begin(), vFormats[i].end(), '*');
            //std::cout << ">Found " << iNumStars << " stars: " << (iNumStars < 2) << std::endl;
            auto value2 = fetchNextParam(args...);
            if (iNumStars < 2) {
                sRes += starFormat(vFormats[i], value, value2);
                                                
            } else if (iNumStars == 2) {
                if (sizeof...(args)  > 1) {
                    //                    auto value2 = fetchNextParam(args...);
                    //                    auto value3 = fetchNextParam2(args...);
                    //                    sRes += starFormat2(vFormats[i], value, value2, value3);
                    auto valuepair = fetchNextTwoParams(args...);
                    sRes += starFormat2(vFormats[i], value, valuepair.first, valuepair.second);
                } else {

                    throw sp_error("Need 3 args for doublestar expression", vFormats[i]);
                }
            } else {
                char sMess[1024];
                sprintf(sMess, "Can't handle %zd stars", iNumStars);
                throw sp_error(sMess, vFormats[i]);
            }
#endif            
        } else if (vFormats[i].back() == 'v') {
            sRes += vecFormat(vFormats[i], value);
        } else {
        
            sRes += simpleFormat(vFormats[i], value);
        }
        sRes += recursiveFormat(vParts, vFormats, i+1, args...);
    }
    return sRes;
}


//----------------------------------------------------------------------------
// xha_sprintf
//   Using sprintf-style formats and an arbitrary number of arguments to
//   create formatted output.
//   in addition to the ordinary format characters there are
//   "%v"   print the elements of a std::vector
//   "%bv"  print the elements of a std::vector with brackets and commas
//   If the elements of the vector have a <<-operator they are printed,
//   otherwise a '?' is shown.
//   "%O"   print any object with a <<-.operator
//
//   First we split the format string into a vector of pure format strings,
//   and a vector containing the string portions between two consecutive 
//   format strings. The vector of string portions shold have one more element
//   than the vector of pure format strings:
// 
//   sFormat = vParts[0] + vPureFormats[0] + ... + vPureFormats[N] + vParts[N+1]
//
template<typename... Args>
std::string xha_sprintf(const std::string sFormat, Args... args) {
    // the format codes 
    std::string sConvs {"csdiouxXaAeEfFgGpvO"};
    //std::string sConvs {"csdiouxXaAeEfFgGp"};
    std::vector<std::string> vPureFormats;
    std::vector<std::string> vParts;

    // find all '%'
    bool bGoOn = true;
    size_t iPrevPos = 0;
    std::string sLeftOver = "";
    while (bGoOn) {
        size_t iPos1 = sFormat.find('%', iPrevPos);

        if (iPos1 != std::string::npos) {
            if (iPos1 < sFormat.length()-1) {
                if (sFormat[iPos1+1] != '%') {

                    if (iPos1 > 0) {
                        vParts.push_back(sLeftOver + sFormat.substr(iPrevPos, iPos1-iPrevPos)); 

                    } else {
                        vParts.push_back(sLeftOver);

                    }
                    sLeftOver="";
                    uint iPos2 = sFormat.find_first_of(sConvs, iPos1+1);
                    if (iPos2 != std::string::npos) {
                        // handle normal format:
                        // save format in vPureFormats
                        std::string sCurFormat = sFormat.substr(iPos1, iPos2-iPos1+1);
                        vPureFormats.push_back(sCurFormat);
                        iPrevPos = iPos2+1;
                    } else {
                        // no terminating conversion character
                        // the case where the conversion character follows long after 
                        // the '%' is not caught here; sprintf will handle this 
                        throw std::runtime_error("no terminating conversion character");
                    }
                } else {
                    // handle '%%' and what came before it
                    sLeftOver = sLeftOver + sFormat.substr(iPrevPos, iPos1-iPrevPos) + "%";
                    iPrevPos = iPos1 + 2;
                }
            } else {
                // no terminating conversion ('%' is last character)
                // this should not happen: a single % without conversion character
                throw std::runtime_error("Found single '%' at end of format string\n");

            }
        } else {
            // no more '%' found
            // add all ordinary string stuff we haven't used yet to vParts 
            vParts.push_back(sLeftOver+sFormat.substr(iPrevPos));
            bGoOn = false;
        }
    }

    std::string sResult = "";
    
    // handle "*": for each star we find, we must insert an empty string to vPureFormats
    // immediately following the format string containing the star(s), and an empty string 
    // to vParts at the appropriate spots
    // we expect at most 2 stars in a format
    uint iTotStars = 0;
    for (uint i = 0; i < vPureFormats.size(); ++i) {
        uint iNumStars = 0;
        bool bInside = (i < vPureFormats.size() - 1);
        size_t iPosX = vPureFormats[i].find_first_of('*', 0);
        while (iPosX != std::string::npos) {
            //printf("we have a star at %zd; i=%d, formats: %zd\n", iPosX, i, vPureFormats.size());
            // we have a star
            iNumStars++;
            // insert empty format after iPosX ...
            //            if (i < vPureFormats.size() - 1) {
            if (bInside) {
                //printf("inserting dummy format at %u\n", i+1);
                vPureFormats.insert(vPureFormats.begin()+i+1, "");
                vParts.insert(vParts.begin()+i+1, "");
            } else {
                // ... or at the end
                //printf("inserting dummy format at the end\n");
                vPureFormats.push_back("");
                vParts.push_back("");
            }
            iPosX = vPureFormats[i].find_first_of('*', iPosX+1);
        }
        iTotStars += iNumStars;
        if (iNumStars > 2) {
            throw std::runtime_error("Can't handle more than 2 '*' in a single format expression\n");
        } else {
            // probably not possible to check:
            // one '*' -> int, {numeric or string}
            // two '*' -> int, int, float
        }
    }
        
    try {
        sResult += recursiveFormat(vParts, vPureFormats, 0, args...);
    } catch (sp_error& e) {
        sResult = "Error: "+std::string(e.what()) + "\n";
    }
    return sResult;
}


//----------------------------------------------------------------------------
// xha_printf
//
template<typename... Args>
void xha_printf(const std::string sFormat, Args... args) {
    std::string sRes = xha_sprintf(sFormat, args...);
    std::cout << sRes;
}


//----------------------------------------------------------------------------
// xha_fprintf
//
template<typename... Args>
void xha_fprintf(FILE *fOut, const std::string sFormat, Args... args) {
    std::string sRes = xha_sprintf(sFormat, args...);
    fprintf(fOut, "%s", sRes.c_str());
}

#endif
