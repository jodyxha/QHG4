/*============================================================================
| utils
| 
|  Various constants as well as infinities and NaNs
|  
|  Author: Jody Weissmann
\===========================================================================*/ 
#ifndef __UTILS_H__
#define __UTILS_H__
 

#include <cmath>
#include <cstdio>
#include <limits>
#include <numbers>

#include "types.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef NUL
#define NUL '\0'
#endif

#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)

//static double(*deg2rad)(double) = [](double x) {return x*M_PI/180.0;};
//static double(*rad2deg)(double) = [](double x) {return x*180.0/M_PI;};

/// string lengths
// real path_max is 4096?
const int MAX_PATH = 2048;
const int MAX_LINE = 1024;
const int SHORT_INPUT = 64;
const int LONG_INPUT = 256;

/// some constants
const double S    = sqrt(3.0);
const double SINV = 1/S;

/// time periods
const float DAY  = 1.0f;
const float WEEK = 7.0f;
const float YEAR = 365.256363f;

const float MONTH = YEAR/12;
const float QUART = YEAR/4;

const float SECS_PER_DAY = 86400;

const double RADIUS_EARTH     = 6371300.0;
const double RADIUS_EARTH_KM  = RADIUS_EARTH/1000;
const double RADIUS_EARTH2 = RADIUS_EARTH * RADIUS_EARTH;

const int REP_ASEXUAL    =  0;
const int REP_SEXUAL     =  1;
const char STR_ASEXUAL[] = "asexual";
const char STR_SEXUAL[]  = "sexual";



const unsigned int ZONE_NONE = 0;
const unsigned int ZONE_CORE = 1;
const unsigned int ZONE_EDGE = 2;
const unsigned int ZONE_HALO = 3;


const double Q_PI = std::numbers::pi;
const float  fNaN = std::nanf("QHG4");
const double dNaN = std::nan("QHG4");
const float  fPosInf = std::numeric_limits<float>::infinity();
const double dPosInf = std::numeric_limits<double>::infinity();
const float  fNegInf = -1.0 * fPosInf;
const double dNegInf = -1.0 * dPosInf;

// arithmetic auxiliaries
#define jsignum(x) (((x)>0)?1:(((x)<0)?-1:0))
//#define jmax(x,y) (((x)>(y))?(x):(y))
//#define jmin(x,y) (((x)<(y))?(x):(y))
#endif
