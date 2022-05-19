#ifndef __ICOUTIL_H__
#define __ICOUTIL_H__

#include <cmath>
#include "Vec3D.h"


const int POLY_TYPE_NONE = -1;
const int POLY_TYPE_ICO  = 0;
const int POLY_TYPE_OCT  = 1;
const int POLY_TYPE_TET  = 2;

const int NOTIFY_LOAD          =  1;
const int NOTIFY_SWITCH        =  2;
const int NOTIFY_ICO_MODE      =  3;
const int NOTIFY_FLAT_PROJ     =  4;
const int NOTIFY_FLAT_LINK     =  5;
const int NOTIFY_TILE_SPLITTER =  6;
const int NOTIFY_NEW_GP        =  7;
const int NOTIFY_NEW_H         =  8;
const int NOTIFY_CREATED       =  9;
const int NOTIFY_TILED         = 10;
const int NOTIFY_ROT_MODE      = 11;


static const int  MODE_POINTS = 0;
static const int  MODE_LINES  = 1;
static const int  MODE_PLANES = 2;


const int MODE_ICO_FULL   = 0x00000001;
const int MODE_ICO_RECT   = 0x00000002;
const int MODE_ICO_LAND   = 0x00000003;

const int MODE_RECT_QUAD  = 0x00001000;
const int MODE_RECT_HEX   = 0x00002000;

const int STATE_ICO       = 0x00010000;
const int STATE_FLAT      = 0x00020000;

const int MASK_ICO_MODE   = 0x0000000f;
const int MASK_FLAT_PROJ  = 0x0000000f;
const int MASK_FLAT_LINK  = 0x0000f000;


typedef struct box {
    double dLonMin;
    double dLonMax;
    double dLatMin;
    double dLatMax;
     box(double _dLonMin, double _dLonMax, double _dLatMin, double _dLatMax) : dLonMin(_dLonMin), dLonMax(_dLonMax), dLatMin(_dLatMin), dLatMax(_dLatMax){};
     box() : dLonMin(0), dLonMax(0), dLatMin(0), dLatMax(0){};
} tbox;


inline void cart2Sphere(Vec3D *v, double *pdLon, double *pdLat) {
    double r = v->calcNorm();
    *pdLat = asin(v->m_fZ/r);
    *pdLon = atan2(v->m_fY, v->m_fX);
}



#endif
