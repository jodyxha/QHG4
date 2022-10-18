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

#ifndef __QUAT_H__
#define __QUAT_H__

class Vec3D;

class Quat {

public:
    
    Quat(double fR, double fI, double fJ, double fK);
    Quat(const Quat *pq);
    Quat(const Quat &q);
    Quat(double fI, double fJ, double fK);
    Quat(const Vec3D *pv);
    Quat(const Vec3D &v);
    Quat(double fR);
    Quat();
    static Quat makeRotation(double fAngle, double fX, double fY, double fZ);
    static Quat makeRotation(double fAngle, const Vec3D &v);
    static Quat makeRotation(const  Vec3D &vFrom, const Vec3D &vTo);
    static Quat *createRotation(double fAngle, double fX, double fY, double fZ);
    static Quat *createRotation(double fAngle, const Vec3D &v);
    static Quat *createRotation(const  Vec3D *pvFrom, const Vec3D *pvTo);
    void add(const Quat *q);
    void sub(const Quat *q);
    void conjugate();
    void mult(const Quat *q);
    double calcNorm() const;
    void normalize();
    void invert();
    void scale(double f);
    Quat *apply(const Quat *q) const;
    Vec3D *apply(const Vec3D *v) const;
    //void apply(Vec3D *v, Vec3D *vR);


    void mult(Quat *q, Quat *qRes);

    // the quaternions components
    double m_fR;
    double m_fI;
    double m_fJ;
    double m_fK;
    

    //@@TODO
    // apply
    Vec3D operator^(const Vec3D &v) const; 
    Quat  operator^(const Quat &q) const; 
    //  operator~  for conjugate (return new quat or change self?)
    //  operator+, operator-, operator*, operator/
    //  operator+=, operator-=, operator*=, operator/=
    inline Quat operator+=(const Quat &q) { m_fR+=q.m_fR; m_fI+=q.m_fI; m_fJ+=q.m_fJ; m_fK+=q.m_fK; return *this;};
    inline Quat operator-=(const Quat &q) { m_fR-=q.m_fR; m_fI-=q.m_fI; m_fJ-=q.m_fJ; m_fK-=q.m_fK; return *this;};
    Quat operator*=(const Quat &q);
    Quat operator/=(const Quat &q);
    Quat operator*=(double d) { return Quat(m_fR*d, m_fI*d, m_fJ*d, m_fK*d);};
      
};
Quat operator*(const Quat &q1, const Quat &q2);
Quat operator/(const Quat &q1, const Quat &q2);
inline Quat operator+(const Quat &q1, const Quat &q2);
inline Quat operator-(const Quat &q1, const Quat &q2);
// conjugate
Quat operator~(const Quat &q1);
// inverse
Quat operator!(const Quat &q1);
inline Quat operator*(const Quat &q, double d) {  return Quat(q.m_fR*d, q.m_fI*d, q.m_fJ*d, q.m_fK*d);};
inline Quat operator*(double d, const Quat &q) {  return Quat(q.m_fR*d, q.m_fI*d, q.m_fJ*d, q.m_fK*d);};


#endif
