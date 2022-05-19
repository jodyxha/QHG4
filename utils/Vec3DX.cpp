/*============================================================================
| Vec3DX
| 
|  A simple class for 3d vectors.
|  Note: all operations defined below DO NOT change the this vector, but
|  create new instances.
|  
|  It's the caller's responability to clean them  up.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cstdio>
#include <cmath>
#include "Vec3DX.h"


    
//------------------------------------------------------------------------
// constructor
//
Vec3DX::Vec3DX(std::initializer_list<double> args) {
    auto p = args.begin();
    m_fX = *p;
    p++;
    m_fY = *p;
    p++;
    m_fZ = *p;
}


//------------------------------------------------------------------------
// dist
//
double Vec3DX::dist(const Vec3DX *pv) { 
    return (*this - *pv).calcNorm();
    //    Vec3DX A(*this - *pv); 
    //    return A.calcNorm();
}


//------------------------------------------------------------------------
// dist
//
double Vec3DX::dist(const Vec3DX &v) { 
    Vec3DX A(*this - v); 
    return A.calcNorm();
}


//------------------------------------------------------------------------
// crossProduct
//   returns a new vector which is the result of 
//   the vector product of this and v
//
Vec3DX *Vec3DX::crossProduct(const Vec3DX *pv) const {
        return new Vec3DX(m_fY*pv->m_fZ - m_fZ*pv->m_fY,
                          m_fZ*pv->m_fX - m_fX*pv->m_fZ,
                          m_fX*pv->m_fY - m_fY*pv->m_fX);
}
    
//------------------------------------------------------------------------
// getCrossSize
//   the length of the cross product of this and v
//   useful to calculate triangle and parallelogram areas
//
double Vec3DX::getCrossSize(const Vec3DX *pv) const {
    Vec3DX v(m_fY*pv->m_fZ - m_fZ*pv->m_fY,
            m_fZ*pv->m_fX - m_fX*pv->m_fZ,
            m_fX*pv->m_fY - m_fY*pv->m_fX);
    return v.calcNorm();
}

//------------------------------------------------------------------------
// getCrossSize
//   the length of the cross product of this and v
//   useful to calculate triangle and parallelogram areas
//
double Vec3DX::getCrossSize(const Vec3DX &v) const {
    Vec3DX v1(m_fY*v.m_fZ - m_fZ*v.m_fY,
            m_fZ*v.m_fX - m_fX*v.m_fZ,
            m_fX*v.m_fY - m_fY*v.m_fX);
    return v1.calcNorm();
}

//------------------------------------------------------------------------
// getAngle
//   returns the angle between this and v
//
double Vec3DX::getAngle(const Vec3DX *pv) const {
    double dAngle = 0;
    double nn = calcNorm()*pv->calcNorm();
    if (nn > 0) {
        double v = dotProduct(pv)/nn;
        if (v > 1) {
            v = 1;
        } else if (v < -1) {
            v = -1;
        }
        dAngle = acos(v); 
    }
    return dAngle;
}
    
//------------------------------------------------------------------------
// getAngle
//   returns the angle between this and v
//
double Vec3DX::getAngle(const Vec3DX &v) const {
    double dAngle = 0;
    double nn = calcNorm()*v.calcNorm();
    if (nn > 0) {
        double d = dotProduct(v)/nn;
        if (d > 1) {
            d = 1;
        } else if (d < -1) {
            d = -1;
        }
        dAngle = acos(d); 
    }
    return dAngle;
}
    
 
//------------------------------------------------------------------------
// trunc
//   truncates the values at float precision
//
void Vec3DX::trunc() {
    m_fX = float(m_fX);
    m_fY = float(m_fY);
    m_fZ = float(m_fZ);
}



//------------------------------------------------------------------------
// operator*=
//   cross product
//
// cross
Vec3DX &Vec3DX::operator*=(const Vec3DX &v1) {
    float fX = m_fY*v1.m_fZ - m_fZ*v1.m_fY;
    float fY = m_fZ*v1.m_fX - m_fX*v1.m_fZ;
    float fZ = m_fX*v1.m_fY - m_fY*v1.m_fX;
    m_fX = fX;
    m_fY = fY;
    m_fZ = fZ;
    return *this;
}


/*

// add
Vec3DX operator+(const Vec3DX &v1, const Vec3DX &v2) {
    return Vec3DX(v1.m_fX+v2.m_fX, v1.m_fY+v2.m_fY, v1.m_fZ+v2.m_fZ);
}


// sub
Vec3DX operator-(const Vec3DX &v1, const Vec3DX &v2) {
    return Vec3DX(v1.m_fX-v2.m_fX, v1.m_fY-v2.m_fY, v1.m_fZ-v2.m_fZ);
}

// cross
Vec3DX operator*(const Vec3DX &v1, const Vec3DX &v2) {
    return Vec3DX(v1.m_fY*v2.m_fZ - v1.m_fZ*v2.m_fY, 
                  v1.m_fZ*v2.m_fX - v1.m_fX*v2.m_fZ,
                  v1.m_fX*v2.m_fY - v1.m_fY*v2.m_fX);
}

// scale
Vec3DX operator*(const Vec3DX &v1, double f) {
    return Vec3DX(v1.m_fX*f,v1.m_fY*f,v1.m_fZ*f);
}
// scale
Vec3DX operator*(double f, const Vec3DX &v1) {
    return Vec3DX(v1.m_fX*f,v1.m_fY*f,v1.m_fZ*f);
}

*/
