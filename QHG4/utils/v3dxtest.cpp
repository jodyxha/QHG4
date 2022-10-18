#include <cstdio>
#include <complex>

#include "Vec3DX.h"

// compile: g++  -g -Wall  Vec3DX.cpp v3dxtest.cpp

void showVec(const char* pCaption, const Vec3DX &v) {
    printf("%s: (%f, %f, %f)\n", pCaption, v.m_fX, v.m_fY, v.m_fZ);
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    Vec3DX a;
    Vec3DX b(3, 2, 1);
    Vec3DX c{2, 4, 6};
    Vec3DX d = {1, 3, 5};
    showVec("a", a);
    showVec("b", b);
    showVec("c", c);
    showVec("d", d);

    printf("a=b\n");
    a = b;
    showVec("a", a);
 
    printf("c+=b\n");
    c += b;
    showVec("a", a);
    showVec("b", b);
    showVec("c", c);

    printf("c.b = %f\n", c | b);

    printf("u(a); u += d\n");
    Vec3DX u(a);
    showVec("a", a);
    showVec("d", d);
    u += d;
    showVec("u", u);


    printf("before\n");
    showVec("d", d);
    showVec("c", c);
    printf("v = d + c\n");
    Vec3DX v(d + c); 
    showVec("d", d);
    showVec("c", c);
    showVec("v", v);

    showVec("direct", d+Vec3DX(3.2,2.4,1.6));

    showVec("0.1*(d+c)*a", 0.1*(d+c)*a);
    Vec3DX xxx = {1, 0, 0};
    Vec3DX yyy = {0, 1, 0};

    Vec3DX *zzz = xxx.crossProduct(&yyy);
    showVec("X.cross(Y)", zzz);

    Vec3DX www = xxx * yyy;
    showVec("X*Y", www);
    showVec("X*=Y", xxx*=yyy);
    showVec("xxx", xxx);
    showVec("yyy", yyy);
    delete zzz;
    showVec("3*xxx", 3*xxx);
    showVec("yyy*3", yyy*3);

    printf("dist((%f,%f,%f),(%f,&%f,%f)): %f\n", xxx.m_fX, xxx.m_fY, xxx.m_fZ, yyy.m_fX, yyy.m_fY, yyy.m_fZ, xxx.dist(yyy));
    printf("angle((%f,%f,%f),(%f,&%f,%f)): %f\n", xxx.m_fX, xxx.m_fY, xxx.m_fZ, yyy.m_fX, yyy.m_fY, yyy.m_fZ, xxx.getAngle(yyy));
    /*
    std::complex<double> a1(3,4);
    std::complex<double> a2(7,11);
    std::complex<double> a3 = a1 +a2;
    printf("%f+i%f + %f+i%f = %f+i%f\n", real(a1), imag(a1), real(a2), imag(a2), real(a3), imag(a3)); 


    std::complex<double> a4 = std::complex<double>(1,9) +std::complex<double>(2,2);
    printf("%f+i%f + %f+i%f = %f+i%f\n", real(a1), imag(a1), real(a2), imag(a2), real(a4), imag(a4)); 
    */
    return iResult;    

}
