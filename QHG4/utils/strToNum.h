#ifndef __STRTONUM_H__
#define __STRTONUM_H__

#include <string>

template<typename T>
bool strToNum(const std::string sData, T *t);
bool strToNum(const std::string sData, float *f);
bool strToNum(const std::string sData, double *d);



#endif
