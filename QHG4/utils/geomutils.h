/*============================================================================
| geomutils
| 
|  Some geometric utilities. Amongst others,
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __GEOMUTILS_H__
#define __GEOMUTILS_H__
#include "types.h"

class Vec3D;

typedef struct sphercoord_t {
    double dLon;
    double dLat;
    sphercoord_t(double dLon0, double dLat0):dLon(dLon0),dLat(dLat0){};
} sphercoord;

//----------------------------------------------------------------------------
//  whichSide
//  return value :
//  > 0                    to the left of
//  = 0  point (x0, y0) is     on           the line
//  < 0                    to the right of
//
//  defined by (x1,y1)->(x2, y2),
//          
//
inline double whichSide(double fX0, double fY0, 
                        double fX1, double fY1, 
                        double fX2, double fY2) {
    return ((fX2-fX1)*(fY0-fY1) - (fY2-fY1)*(fX0-fX1));
}

//----------------------------------------------------------------------------
//  isPointInPoly
//  returns value
//    false : point (X0,Y0) is outside of poly
//    true  : point inside poly
//
//    upper (high y) and right (high x) borders are not part of the polygon
//    i.e. for the unit square [0,1]x[0,1] points of the form (1,y) and (x,1)
//    are considered to be on the outside
// 
bool isPointInPoly(double fX0, double fY0, 
                   int iN, 
                   double *afX, double *afY);


int isPointInPoly2(double fX0, double fY0, 
                   int iN, 
                   double *afX, double *afY);

int isPointInPoly2(double fX0, double fY0, VEC_POINTS vp);

//----------------------------------------------------------------------------
//  getPolyBounds
// 
void getPolyBounds(int iN, float *afX, float *afY,
                float *pfXMin, float *pfYMin,
                float *pfXMax, float *pfYMax);

//----------------------------------------------------------------------------
//  isPointInBox
//   returns true if point is in Box (including border)
inline bool isPointBox(float fX0, float fY0,
                     float fXMin, float fYMin,
                     float fXMax, float fYMax) {
    return ((fX0 >= fXMin) && (fX0 <= fXMax) &&
            (fY0 >= fYMin) && (fY0 <= fYMax));
}

//----------------------------------------------------------------------------
//  spherInterpol
//    returns a point which is spherically interpolated between P1 and P2
//    with a factor of T (0 <= T <= 1)
//    T=0  -> P1
//    T=1  -> P2
//    T=0.5 -> a point halfway between P1 and P2 on the sphere
//
sphercoord spherInterpol(const sphercoord &dP1, const sphercoord &dP2, double dT);

Vec3D spherInterpol(const Vec3D &v1, const Vec3D &v2, double dT);

sphercoord cart2PolarD(const Vec3D &v);
Vec3D      polarD2Cart(const sphercoord &p);
sphercoord cart2Polar(const Vec3D &v);
Vec3D      polar2Cart(const sphercoord &p);

double spherdist(double dLon1, double  dLat1, double dLon2, double dLat2, double dR);
double spherdistDeg(double dLon1, double  dLat1, double dLon2, double dLat2, double dR);
double spherdist(Vec3D *v1, Vec3D *v2, double dR);
double spherdist(double dX0, double dY0, double dZ0, double dX1, double dY1, double dZ1, double dR);
double spherdist(double dX0, double dY0, double dZ0, double dLon2, double dLat3, double dR);
 
double cartdist(double dX1, double  dY1, double dX2, double dY2, double dScale);

#endif
