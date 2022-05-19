#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

//----------------------------------------------------------------------------
// strToNum
//  for integer and floating point types
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
// main
//
int main(int iARgC, char *apArgV[]) {
    std::vector<std::string> vNumbers {"17","271828182846", "3.141592656359", "0.61803398875"};
    int    i = 0;
    long   l = 0;
    float  f = 0;
    double d = 0;

    for (unsigned int k = 0; k < vNumbers.size(); k++) {
        std::cout << "#" << k << " " << vNumbers[k] << ":\n";
        if (strToNum(vNumbers[k], &i)) {
            std::cout << "as int:    " << i << "\n";
        } else {
            std::cout << "failed as int\n";
        }

        if (strToNum(vNumbers[k], &l)) {
            std::cout << "as long:   " << l << "\n";
        } else {
            std::cout << "failed as long\n";
        }
        
        if (strToNum(vNumbers[k], &f)) {
            std::cout << "as float:  " << f << "\n";
        } else {
            std::cout << "failed as float\n";
        }
        
        if (strToNum(vNumbers[k], &d)) {
            std::cout << "as double: " << d << "\n";
        } else {
            std::cout << "failed as double\n";
        }
    }
    return 0;
}

