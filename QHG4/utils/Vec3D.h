/*============================================================================
| Vec3D
| 
|  A simple class for 3d vectors.
|  Note: all operations defined below DO NOT change the this vector, but
|  create new instances.
|  
|  It's the caller's responability to clean them  up.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __VEC3D_H__
#define __VEC3D_H__

const double EPSC = 1e-5;

class Vec3D {
public:
    Vec3D();
    Vec3D(double fX, double fY, double fZ);
    Vec3D(const Vec3D *pv);
    Vec3D(const Vec3D &v);
    void set(double fX, double fY, double fZ) { m_fX = fX;m_fY = fY; m_fZ = fZ;};
    void set(const Vec3D *pv) { m_fX = pv->m_fX;m_fY = pv->m_fY; m_fZ = pv->m_fZ;};
    void set(const Vec3D &v) { m_fX = v.m_fX;m_fY = v.m_fY; m_fZ = v.m_fZ;};
    void add(const Vec3D *pv);
    void add(const Vec3D &v);
    void subtract(const Vec3D *pv);
    void subtract(const Vec3D &v);
    void scale(double f);
    double calcNorm() const;
    double dist(const Vec3D *pv);
    double dist(const Vec3D &v);
    void normalize();
    inline double dotProduct(const Vec3D *pv) const {return m_fX*pv->m_fX + m_fY*pv->m_fY + m_fZ*pv->m_fZ;};
    inline double dotProduct(const Vec3D &v) const {return m_fX*v.m_fX + m_fY*v.m_fY + m_fZ*v.m_fZ;};
    Vec3D *crossProduct(const Vec3D *pv) const;
    double getCrossSize(const Vec3D *pv) const;
    double getCrossSize(const Vec3D &v) const;
    double getAngle(const Vec3D *pv) const;
    double getAngle(const Vec3D &v) const;
    bool operator==(const Vec3D &v) const;
    
    void trunc();

    // oder relation first x, then y, then z
    //    inline bool operator<(const Vec3D &v) const { printf("%20.18f,%20.18f,%29.18f) < (%20.18f,%20.18f,%20.18f): (%e,%e,%e): %s\n", m_fX, m_fY, m_fZ, v.m_fX, v.m_fY, v.m_fZ, m_fX-v.m_fX, m_fY-v.m_fY, m_fZ-v.m_fZ,((m_fX < v.m_fX) || ((m_fX == v.m_fX)&&(m_fY < v.m_fY)) || ((m_fX == v.m_fX)&&(m_fY == v.m_fY)&&(m_fZ < v.m_fZ)))?"yes":"no");return (m_fX < v.m_fX) || ((m_fX == v.m_fX)&&(m_fY < v.m_fY)) || ((m_fX == v.m_fX)&&(m_fY == v.m_fY)&&(m_fZ < v.m_fZ));};
    // cast it to float to reduce precision (avoiding "epsilontics")
    /*
    inline bool operator<(const Vec3D &v) const { return (((float)(m_fX+EPSC) <   (float)(v.m_fX+EPSC)) || 
                                                          (((float)(m_fX+EPSC) == (float)(v.m_fX+EPSC)) && ((float)(m_fY+EPSC) <  (float)(v.m_fY+EPSC))) || 
                                                          (((float)(m_fX+EPSC) == (float)(v.m_fX+EPSC)) && ((float)(m_fY+EPSC) == (float)(v.m_fY+EPSC))  && ((float)(m_fZ+EPSC) < (float)(v.m_fZ+EPSC))));};
    */
    inline bool operator<(const Vec3D &v) const { return (((float)(m_fX+EPSC) <   (float)(v.m_fX+EPSC)) || 
                                                          ((!((float)(v.m_fX+EPSC) < (float)(m_fX+EPSC))) && ((float)(m_fY+EPSC) <  (float)(v.m_fY+EPSC))) || 
                                                          ((!((float)(v.m_fX+EPSC) < (float)(m_fX+EPSC))) && (!((float)(v.m_fY+EPSC) < (float)(m_fY+EPSC)))  && ((float)(m_fZ+EPSC) < (float)(v.m_fZ+EPSC))));};
    /*
    bool operator<(const Vec3D &v) const;
    */
    /*
    static Vec3D vX;
    static Vec3D vY;
    static Vec3D vZ;
    */

    // add; changes this
    inline Vec3D &operator+=(const Vec3D &v) { m_fX += v.m_fX; m_fY += v.m_fY; m_fZ += v.m_fZ; return *this;};
    // subtract; changes this
    inline Vec3D &operator-=(const Vec3D &v) { m_fX -= v.m_fX; m_fY -= v.m_fY; m_fZ -= v.m_fZ; return *this;};
    // scale; changes this
    inline Vec3D &operator*=(const double f) { m_fX *= f; m_fY *= f; m_fZ *= f; return *this;};
    // cross; changes this
    Vec3D &operator*=(const Vec3D &v1);
    // dot product
    inline double  operator|(const Vec3D &v) const {return m_fX*v.m_fX + m_fY*v.m_fY + m_fZ*v.m_fZ;};
    // assign
    inline Vec3D &operator=(const Vec3D &v1) { m_fX = v1.m_fX; m_fY = v1.m_fY; m_fZ = v1.m_fZ; return *this;};


    // the components of the vector
    double m_fX;
    double m_fY;
    double m_fZ;

};

// arithmetics (unchanging arguments)
// add
inline Vec3D operator+(const Vec3D &v1,  const Vec3D &v2){
    return Vec3D(v1.m_fX+v2.m_fX, v1.m_fY+v2.m_fY, v1.m_fZ+v2.m_fZ);
}

// subtract
inline Vec3D operator-(const Vec3D &v1,  const Vec3D &v2) {
    return Vec3D(v1.m_fX-v2.m_fX, v1.m_fY-v2.m_fY, v1.m_fZ-v2.m_fZ);
}

// cross
inline Vec3D operator*(const Vec3D &v1,  const Vec3D &v2) {
    return Vec3D(v1.m_fY*v2.m_fZ - v1.m_fZ*v2.m_fY, 
                  v1.m_fZ*v2.m_fX - v1.m_fX*v2.m_fZ,
                  v1.m_fX*v2.m_fY - v1.m_fY*v2.m_fX);
}

// scale
inline Vec3D operator*(const Vec3D &v1,  double f){
    return Vec3D(v1.m_fX*f,v1.m_fY*f,v1.m_fZ*f);
}

// scale
inline Vec3D operator*(double f, const Vec3D &v1) {
    return Vec3D(v1.m_fX*f,v1.m_fY*f,v1.m_fZ*f);
}

// dot
inline double dot(const Vec3D &v1,  const Vec3D &v2) { return v1.dotProduct(v2);};

#endif
