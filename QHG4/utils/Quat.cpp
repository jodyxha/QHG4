/*============================================================================
| Quat
| 
|  A simple class for quaternions.
|  Note: all operations defined below DO NOT change the this quaternion, but
|  create new instances.
|  
|  It's the caller's responability to clean them  up.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <cmath>
#include "Vec3D.h"
#include "Quat.h"

//------------------------------------------------------------------------
// constructor
//   definition of all components
//
Quat::Quat(double fR, double fI, double fJ, double fK) {
    m_fR = fR;      
    m_fI = fI;      
    m_fJ = fJ;      
    m_fK = fK;      
}
    
//------------------------------------------------------------------------
// constructor
//   definition of a pure quaternion by components
//
Quat::Quat(double fI, double fJ, double fK) {
    m_fR = 0;       
    m_fI = fI;      
    m_fJ = fJ;      
    m_fK = fK;      
}
    
//------------------------------------------------------------------------
// constructor
//   definition of a pure quaternion by a vector
//
Quat::Quat(const Vec3D *v) {
    m_fR = 0;       
    m_fI = v->m_fX;      
    m_fJ = v->m_fY;      
    m_fK = v->m_fZ;      
}
    
//------------------------------------------------------------------------
// constructor
//   definition of a pure quaternion by a vector
//
Quat::Quat(const Vec3D &v) {
    m_fR = 0;       
    m_fI = v.m_fX;      
    m_fJ = v.m_fY;      
    m_fK = v.m_fZ;      
}
    
//------------------------------------------------------------------------
// constructor
//   definition of a quaternion having only a real part
//
Quat::Quat(double fR) {
    m_fR = fR;      
    m_fI = 0;       
    m_fJ = 0;       
    m_fK = 0;       
}
    
//------------------------------------------------------------------------
// constructor
//   definition of the unit quaternion
//
Quat::Quat() {
    m_fR = 1;       
    m_fI = 0;       
    m_fJ = 0;       
    m_fK = 0;       
}
    
//------------------------------------------------------------------------
// constructor
//   copy constructor
//
Quat::Quat(const Quat *q){
    m_fR = q->m_fR;
    m_fI = q->m_fI;
    m_fJ = q->m_fJ;
    m_fK = q->m_fK;
}
    
//------------------------------------------------------------------------
// constructor
//   copy constructor
//
Quat::Quat(const Quat &q){
    m_fR = q.m_fR;
    m_fI = q.m_fI;
    m_fJ = q.m_fJ;
    m_fK = q.m_fK;
}
    
//------------------------------------------------------------------------
// createRotation
//   create a quaternion defined by a rotation axis and an angle
//
Quat *Quat::createRotation(double fAngle, double fX, double fY, double fZ){
    double fS = sin(fAngle/2);
    double fC = cos(fAngle/2);
    return new Quat(fC, fX*fS, fY*fS, fZ*fS);
}

//------------------------------------------------------------------------
// makeRotation
//   create a quaternion defined by a rotation axis and an angle
//
Quat Quat::makeRotation(double fAngle, double fX, double fY, double fZ){
    double fS = sin(fAngle/2);
    double fC = cos(fAngle/2);
    return Quat(fC, fX*fS, fY*fS, fZ*fS);
}

//------------------------------------------------------------------------
// createRotation
//   create a quaternion defined by a rotation axis and an angle
//
Quat *Quat::createRotation(double fAngle, const Vec3D &v){
    return createRotation(fAngle, v.m_fX, v.m_fY, v.m_fZ);
}

//------------------------------------------------------------------------
// makeRotation
//   create a quaternion defined by a rotation axis and an angle
//
Quat Quat::makeRotation(double fAngle, const Vec3D &v){
    return makeRotation(fAngle, v.m_fX, v.m_fY, v.m_fZ);
}

//------------------------------------------------------------------------
// createRotation
//   create a quaternion which rotates pvFrom to pvTo
//
Quat *Quat::createRotation(const  Vec3D *pvFrom, const Vec3D *pvTo) {
    // calculate axis
    Vec3D *pvAxis = pvFrom->crossProduct(pvTo);
    // get angle
    double dAngle = pvFrom->getAngle(pvTo);
    // create rotation
    pvAxis->normalize();
    Quat *rot1 = Quat::createRotation(dAngle, pvAxis);

    // clean up
    delete pvAxis;

    return rot1;
}

//------------------------------------------------------------------------
// makeRotation
//   make a quaternion which rotates pvFrom to pvTo
//
Quat Quat::makeRotation(const Vec3D &vFrom, const Vec3D &vTo) {
    // calculate axis
    Vec3D vAxis = vFrom * vTo;
    // get angle
    double dAngle = vFrom.getAngle(vTo);
    // create rotation
    vAxis.normalize();
 
    return Quat::makeRotation(dAngle, vAxis);
}


    
//------------------------------------------------------------------------
// add
//   add q  to this quaternion
//
void Quat::add(const Quat *q) {
    m_fR += q->m_fR;
    m_fI += q->m_fI;
    m_fJ += q->m_fJ;   
    m_fK += q->m_fK;
}

//------------------------------------------------------------------------
// sub
//   subtract q from this quaternion
//
void Quat::sub(const Quat *q) {
    m_fR -= q->m_fR;
    m_fI -= q->m_fI;
    m_fJ -= q->m_fJ;
    m_fK -= q->m_fK;
}

//------------------------------------------------------------------------
// conjugate 
//   conjugates this quaternion
//
void Quat::conjugate() {
    m_fI *= -1;
    m_fJ *= -1;   
    m_fK *= -1;
}
    
//------------------------------------------------------------------------
// mult
//   multiplies this quaternion with q 
//   result: first q, then this
//
void Quat::mult(const Quat *q) {
    double dR = m_fR*q->m_fR - m_fI*q->m_fI - m_fJ*q->m_fJ - m_fK*q->m_fK;
    double dI = m_fR*q->m_fI + m_fI*q->m_fR + m_fJ*q->m_fK - m_fK*q->m_fJ;
    double dJ = m_fR*q->m_fJ + m_fJ*q->m_fR + m_fK*q->m_fI - m_fI*q->m_fK;
    double dK = m_fR*q->m_fK + m_fK*q->m_fR + m_fI*q->m_fJ - m_fJ*q->m_fI;
    
    m_fR = dR;
    m_fI = dI;
    m_fJ = dJ;
    m_fK = dK;
}

//------------------------------------------------------------------------
// mult
//   multiplies this quaternion with q and places result in qRes
//   result: first q, then this
//
void Quat::mult(Quat *q, Quat *qRes) {
    double dR = m_fR*q->m_fR - m_fI*q->m_fI - m_fJ*q->m_fJ - m_fK*q->m_fK;
    double dI = m_fR*q->m_fI + m_fI*q->m_fR + m_fJ*q->m_fK - m_fK*q->m_fJ;
    double dJ = m_fR*q->m_fJ + m_fJ*q->m_fR + m_fK*q->m_fI - m_fI*q->m_fK;
    double dK = m_fR*q->m_fK + m_fK*q->m_fR + m_fI*q->m_fJ - m_fJ*q->m_fI;
    
    qRes->m_fR = dR;
    qRes->m_fI = dI;
    qRes->m_fJ = dJ;
    qRes->m_fK = dK;
}

//------------------------------------------------------------------------
// calcNorm
//   return the length of this
//
double Quat::calcNorm() const{
    double f =  sqrt(m_fR*m_fR + m_fI*m_fI + m_fJ*m_fJ + m_fK*m_fK);
    return f;
}
    
//------------------------------------------------------------------------
// normalize 
//   normalizes this quaternion
//
void Quat::normalize() {
    double f = calcNorm();
    scale(1/f);
}
    
//------------------------------------------------------------------------
// invert 
//   inverts this quaternion
//
void Quat::invert() {
    double f = calcNorm();
    conjugate();
    scale(1/f*f);
}
    
//------------------------------------------------------------------------
// scale
//   returns a new quaternion which is a scaled version of this
//
void Quat::scale(double f) {
    m_fR *= f;
    m_fI *= f;
    m_fJ *= f;
    m_fK *= f;
}
    
//------------------------------------------------------------------------
// apply
//   applying this to q
//   For a given quaternion p 
//   p.apply(q) = p*q*p^{-1}
//   returns a new quaternion which is the result of
//   conjugating q with this.
//   (this is not changed)
//
Quat *Quat::apply(const Quat *q) const {
    // inverse quaternion for this
    Quat *qRes = new Quat(this);
    Quat *qInv = new Quat(this);
    qInv->invert();
    // multiply with argument
    qRes->mult(q);
    //multiply result with this
    qRes->mult(qInv);
    delete qInv;
    return qRes;
}
    
//------------------------------------------------------------------------
// apply
//   applying this quaternion to the vector v
//   For a given quaternion p 
//   p.apply(v) = p*v'*p^{-1}
//   where v' is the pure quaternion (0, v.x, v.y, v.z)
//   returns a new vector which is the result of
//   conjugating v (interpreted as a pure quaternion) with this
//   (this is not changed)
//
Vec3D *Quat::apply(const Vec3D *v) const {
    Quat *qTemp = new Quat(v);
    Quat *q = apply(qTemp);
    Vec3D *pVRes = new Vec3D(q->m_fI, q->m_fJ, q->m_fK);
    delete q;
    delete qTemp;
    return pVRes;
}


//------------------------------------------------------------------------
// operator^
//   (same as apply, but for refs instead of pointers)
//   applying this quaternion to the vector v
//   For a given quaternion p 
//   p.apply(v) = p*v'*p^{-1}
//   where v' is the pure quaternion (0, v.x, v.y, v.z)
//   returns a new vector which is the result of
//   conjugating v (interpreted as a pure quaternion) with this
//   (this is not changed)
//
Quat Quat::operator^(const Quat &q) const {
    return Quat((*this) * q * !(*this));
}

//------------------------------------------------------------------------
// operator^
//   (same as apply, but for refs instead of pointers)
//   applying this quaternion to the vector v
//   For a given quaternion p 
//   p.apply(v) = p*v'*p^{-1}
//   where v' is the pure quaternion (0, v.x, v.y, v.z)
//   returns a new vector which is the result of
//   conjugating v (interpreted as a pure quaternion) with this
//   (this is not changed)
//
Vec3D Quat::operator^(const Vec3D &v) const {
    Quat qTemp = (*this) * Quat(v) * !(*this);
    return Vec3D(qTemp.m_fI, qTemp.m_fJ, qTemp.m_fK);
}


//----------------------------------------------------------------------------
// operator+
//  adds q2 to q1
//
Quat operator+(const Quat &q1, const Quat &q2) {
    return Quat(q1.m_fR+q2.m_fR, q1.m_fI+q2.m_fI,q1.m_fJ+q2.m_fJ,q1.m_fK+q2.m_fK);
}

//----------------------------------------------------------------------------
// operator~
//  subtracts q2 from q1return conjugate quaernion
//
Quat operator-(const Quat &q1, const Quat &q2) {
    return Quat(q1.m_fR-q2.m_fR, q1.m_fI-q2.m_fI,q1.m_fJ-q2.m_fJ,q1.m_fK-q2.m_fK);
}


//----------------------------------------------------------------------------
// operator*
//  multiplies q1 by q2
//
Quat operator*(const Quat &p, const Quat &q) {
    double dR = p.m_fR*q.m_fR - p.m_fI*q.m_fI - p.m_fJ*q.m_fJ - p.m_fK*q.m_fK;
    double dI = p.m_fR*q.m_fI + p.m_fI*q.m_fR + p.m_fJ*q.m_fK - p.m_fK*q.m_fJ;
    double dJ = p.m_fR*q.m_fJ + p.m_fJ*q.m_fR + p.m_fK*q.m_fI - p.m_fI*q.m_fK;
    double dK = p.m_fR*q.m_fK + p.m_fK*q.m_fR + p.m_fI*q.m_fJ - p.m_fJ*q.m_fI;

    return Quat(dR, dI, dJ, dK);
}

//----------------------------------------------------------------------------
// operator/
//  divides q1 by q2return conjugate quaernion
//
Quat operator/(const Quat &p, const Quat &q) {
    return p*!q;
}

//----------------------------------------------------------------------------
// operator~
//  return conjugate quaernion
//
Quat operator~(const Quat &q1) {
    return Quat(q1.m_fR, -q1.m_fI, -q1.m_fJ, -q1.m_fK);
}

//----------------------------------------------------------------------------
// operator!
//  returns the inverse of q1
//
Quat operator!(const Quat &q1) {
    double f = q1.calcNorm();
    return (1/f)*Quat(q1.m_fR, -q1.m_fI, -q1.m_fJ, -q1.m_fK);
}


//----------------------------------------------------------------------------
// operator*=
//  multiplies this with q1
//
Quat Quat::operator*=(const Quat &q) {
    double dR = m_fR*q.m_fR - m_fI*q.m_fI - m_fJ*q.m_fJ - m_fK*q.m_fK;
    double dI = m_fR*q.m_fI + m_fI*q.m_fR + m_fJ*q.m_fK - m_fK*q.m_fJ;
    double dJ = m_fR*q.m_fJ + m_fJ*q.m_fR + m_fK*q.m_fI - m_fI*q.m_fK;
    double dK = m_fR*q.m_fK + m_fK*q.m_fR + m_fI*q.m_fJ - m_fJ*q.m_fI;
    
    m_fR = dR;
    m_fI = dI;
    m_fJ = dJ;
    m_fK = dK;

    return *this;
}

//----------------------------------------------------------------------------
// operator/=
//  divides this by q1
//
Quat Quat::operator/=(const Quat &q) {
    Quat qr = this / q;
    m_fR = qr.m_fR;
    m_fI = qr.m_fI;
    m_fJ = qr.m_fJ;
    m_fK = qr.m_fK;

    return *this;
}
