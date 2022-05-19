#include <cstdio>
#include "Vec3D.h"

void showVec(Vec3D *pv, const char *pCaption) {
    printf("%s: (%f, %f, %f)\n", pCaption, pv->m_fX, pv->m_fY, pv->m_fZ);
}

void showVec(const Vec3D &v, const char *pCaption) {
    printf("%s: (%f, %f, %f)\n", pCaption, v.m_fX, v.m_fY, v.m_fZ);
}

int main(int iArgC, char *apArgV[])  {
    
    Vec3D v1(1,3,6);
    showVec(v1, "v1");
    Vec3D v2{2,5,8};
    showVec(v2,"v2");

    Vec3D v3 = v1 + v2; 
    showVec(v3, "v3");

    printf("length(vr): %f\n", v3.calcNorm());
    v3.normalize();
    showVec(v3, "v3 normalized");

    Vec3D x(1, 0, 0);
    Vec3D y(0, 1, 0);
    Vec3D z(0, 0, 1);
    showVec(x,"x");
    showVec(y,"y");
    showVec(z,"z");
    Vec3D xy = x * y;
    showVec(xy, "x * y");
    if (z == xy) {
        printf("x * y is indeed z\n");
    } else {
        printf("something is wrong...\n");
    }

    Vec3D v = 0.3*(v2-v1)*(v3+x);
    showVec(v, "0.3*(v2-v1)*(v3+x)");

    showVec(v3.calcNorm()*(v1+v2-v3)*(x+y), "v3.calcNorm()*(v1+v2-v3)*(x+y) (direct)");

    return 0;
}



