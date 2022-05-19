#include <cstdio>
#include <cmath>
#include "Vec3D.h"
#include "Quat.h"
void showVec(Vec3D *pv, const char *pCaption) {
    printf("%s: (%f, %f, %f)\n", pCaption, pv->m_fX, pv->m_fY, pv->m_fZ);
}

void showVec(const Vec3D &v, const char *pCaption) {
    printf("%s: (%f, %f, %f)\n", pCaption, v.m_fX, v.m_fY, v.m_fZ);
}

void showQuat(Quat *pq, const char *pCaption) {
    printf("%s: (%f, %f, %f, %f)\n", pCaption, pq->m_fR, pq->m_fI, pq->m_fJ, pq->m_fK);
}

void showQuat(Quat q, const char *pCaption) {
    printf("%s: (%f, %f, %f, %f)\n", pCaption, q.m_fR, q.m_fI, q.m_fJ, q.m_fK);
}


int main(int iArgC, char *apArgV[])  {

    Quat q1(1,2,3,4);
    q1.normalize();
    Quat q2=!q1;
    // q2.invert();
    showQuat(q1, "q1");
    showQuat(q2, "q2");

    q1.mult(&q2);
    showQuat(q1, "q1*q2");
    Quat e = q1/q1;
    showQuat(e, "q1/q1");

    Vec3D v1(1,3,6);

    Vec3D v2 = q2^v1;

    showVec(v1, "v1");
    showVec(v2,"v2");


    Quat qrot = Quat::makeRotation(2*M_PI/3, (1.0/sqrt(3.0))*Vec3D(1, 1, 1));

    Vec3D x(1,0,0);
    Vec3D y(0,1,0);
    Vec3D z(0,0,1);


    Vec3D xr = qrot^x;
    Vec3D yr = qrot^y;
    Vec3D zr = qrot^z;
    showVec(x,  "x ");
    showVec(xr, "xr ");
    showVec(y,  "y ");
    showVec(yr, "yr ");
    showVec(z,  "z ");
    showVec(zr, "zr ");

    q2 *= qrot;
    showQuat(q2, "q2*qrot");
    q2 /= qrot;
    showQuat(q2, "(q2*qrot)/qrot");
    showQuat(3*q2, "(3*q2*qrot)/qrot");
 
    return 0;
}

 

