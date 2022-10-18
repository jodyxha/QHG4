#ifndef __VEC3DX_H__
#define __VEC3DX_H__

#define EPSC 1e-5

#include <initializer_list>

class Vec3DX {
public:
    // constructors
    inline Vec3DX(): m_fX(0), m_fY(0),m_fZ(0){};
    inline Vec3DX(double fX, double fY, double fZ): m_fX(fX), m_fY(fY),m_fZ(fZ){};
    inline Vec3DX(Vec3DX &v): m_fX(v.m_fX), m_fY(v.m_fY),m_fZ(v.m_fZ){};
    inline Vec3DX(Vec3DX &&v): m_fX(v.m_fX), m_fY(v.m_fY),m_fZ(v.m_fZ){};
    Vec3DX(std::initializer_list<double> args);
    inline Vec3DX(const Vec3DX *pv): m_fX(pv->m_fX), m_fY(pv->m_fY),m_fZ(pv->m_fZ){};

    // assignment 
    inline void set(double fX, double fY, double fZ) { m_fX = fX;m_fY = fY; m_fZ = fZ;};
    inline void set(const Vec3DX *pv) { m_fX = pv->m_fX;m_fY = pv->m_fY; m_fZ = pv->m_fZ;};
    inline void set(const Vec3DX &v) { m_fX = v.m_fX;m_fY = v.m_fY; m_fZ = v.m_fZ;};

    // arithmetics
    // add (+=)
    inline void add(const Vec3DX *pv) { m_fX+=pv->m_fX; m_fY+=pv->m_fY; m_fZ+=pv->m_fZ;};
    inline void add(const Vec3DX &v) { m_fX+=v.m_fX; m_fY+=v.m_fY; m_fZ+=v.m_fZ;};
    // subtract (-=)
    inline void subtract(const Vec3DX *pv) { m_fX-=pv->m_fX; m_fY-=pv->m_fY; m_fZ-=pv->m_fZ;};
    inline void subtract(const Vec3DX &v) { m_fX-=v.m_fX; m_fY-=v.m_fY; m_fZ-=v.m_fZ;};
    // scale (*=)
    inline void scale(double f) { m_fX *= f; m_fY *= f; m_fZ *= f;};

    // norms and dists
    inline double calcNorm() const { return sqrt(m_fX*m_fX+m_fY*m_fY+m_fZ*m_fZ);};
    double dist(const Vec3DX *pv);
    double dist(const Vec3DX &v);
    inline void normalize() { scale(1/calcNorm());};

    // products and angles
    inline double dotProduct(const Vec3DX *pv) const {return m_fX*pv->m_fX + m_fY*pv->m_fY + m_fZ*pv->m_fZ;};
    inline double dotProduct(const Vec3DX &v) const {return m_fX*v.m_fX + m_fY*v.m_fY + m_fZ*v.m_fZ;};
    Vec3DX *crossProduct(const Vec3DX *pv) const;
    double getCrossSize(const Vec3DX *pv) const;
    double getCrossSize(const Vec3DX &v) const;
    double getAngle(const Vec3DX *pv) const;
    double getAngle(const Vec3DX &v) const;
    
    inline bool operator==(const Vec3DX &v) const {return (fabs((float)(m_fX+EPSC)-(float)(v.m_fX+EPSC)) < EPSC) && 
            (fabs((float)(m_fY+EPSC)-(float)(v.m_fY+EPSC)) < EPSC) && 
            (fabs((float)(m_fZ+EPSC)-(float)(v.m_fZ+EPSC)) < EPSC);};
    
    inline bool operator<(const Vec3DX &v) const { return (((float)(m_fX+EPSC) <   (float)(v.m_fX+EPSC)) || 
                                                           ((!((float)(v.m_fX+EPSC) < (float)(m_fX+EPSC))) && ((float)(m_fY+EPSC) <  (float)(v.m_fY+EPSC))) || 
                                                           ((!((float)(v.m_fX+EPSC) < (float)(m_fX+EPSC))) && (!((float)(v.m_fY+EPSC) < (float)(m_fY+EPSC)))  && ((float)(m_fZ+EPSC) < (float)(v.m_fZ+EPSC))));};
    
    void trunc();

    // add; changes this
    inline Vec3DX &operator+=(const Vec3DX &v) { m_fX += v.m_fX; m_fY += v.m_fY; m_fZ += v.m_fZ; return *this;};
    // subtract; changes this
    inline Vec3DX &operator-=(const Vec3DX &v) { m_fX -= v.m_fX; m_fY -= v.m_fY; m_fZ -= v.m_fZ; return *this;};
    // scale; changes this
    inline Vec3DX &operator*=(const double f) { m_fX *= f; m_fY *= f; m_fZ *= f; return *this;};
    // cross; changes this
    Vec3DX &operator*=(const Vec3DX &v1);
    // dot product
    inline double  operator|(const Vec3DX &v) const {return m_fX*v.m_fX + m_fY*v.m_fY + m_fZ*v.m_fZ;};
    // assign
    inline Vec3DX &operator=(const Vec3DX &v1) { m_fX = v1.m_fX; m_fY = v1.m_fY; m_fZ = v1.m_fZ; return *this;};

    
    // the components of the vector
    double m_fX;
    double m_fY;
    double m_fZ;

};
// arithmetics (unchanging arguments)
// add
inline Vec3DX operator+(const Vec3DX &v1,  const Vec3DX &v2){
    return Vec3DX(v1.m_fX+v2.m_fX, v1.m_fY+v2.m_fY, v1.m_fZ+v2.m_fZ);
}

// subtract
inline Vec3DX operator-(const Vec3DX &v1,  const Vec3DX &v2) {
    return Vec3DX(v1.m_fX-v2.m_fX, v1.m_fY-v2.m_fY, v1.m_fZ-v2.m_fZ);
}

// cross
inline Vec3DX operator*(const Vec3DX &v1,  const Vec3DX &v2) {
    return Vec3DX(v1.m_fY*v2.m_fZ - v1.m_fZ*v2.m_fY, 
                  v1.m_fZ*v2.m_fX - v1.m_fX*v2.m_fZ,
                  v1.m_fX*v2.m_fY - v1.m_fY*v2.m_fX);
}

// scale
inline Vec3DX operator*(const Vec3DX &v1,  double f){
    return Vec3DX(v1.m_fX*f,v1.m_fY*f,v1.m_fZ*f);
}

// scale
inline Vec3DX operator*(double f, const Vec3DX &v1) {
    return Vec3DX(v1.m_fX*f,v1.m_fY*f,v1.m_fZ*f);
}

// dot
inline double dot(const Vec3DX &v1,  const Vec3DX &v2) { return v1.dotProduct(v2);};



#endif
