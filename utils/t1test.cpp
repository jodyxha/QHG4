#include "t1.h"

int main(int iArgC, char *apArgV[]) {
    
    T1 x(1,0,0);
    T1 y(0,1,0);

    T1 z = x *y;
    return 0;
}
