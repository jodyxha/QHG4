#include <cstdio>
#include <cstdlib>
/*
if (id < 12) {
    type = V;
    j = id;
} else if (id < 12+30*subdiv) {
    type = E;
    j1 = id -12;
    e = j1/subdiv; // int division
    j = id -subdiv*e - 12;
} else if (id < 2+10*(subdiv+1)*(subdiv+1)) {
    j1 = id -12-30*subdiv;
    f = 2*j1/(subdiv*(subdiv-1));
    j = id -12 -f*subdiv*(subdiv-1)/2;  // int division
}

vmax = 12     
emax = 30*subdiv;
fmax = subdiv*(subdiv-1)/2;

if (id < vmax) {
    type = V;
    j = id;
} else {
    id -= vmax;
    if (id < emax) {
        type = E;
        e = id/subdiv; // int division
        j = id -subdiv*e;
    } else {
        id -= emax;
        if (id < 20*svmax) {
            type = F;
            f =  id/s2; // int division
            j = id - f*fmax;
        } else {
            //err
        }
    }
}
*/

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 2) {
        int subdiv = atoi(apArgV[1]);
        int id     = atoi(apArgV[2]);

        int vmax = 12;
        int emax = 30*subdiv;
        int fmax = subdiv*(subdiv-1)/2;

        char type = '\0';
        int j = -1;
        int n = 0;
        if (id < vmax) {
            type = 'V';
            j = id;
        } else {
            id -= vmax;
            if (id < emax) {
                type = 'E';
                n = id/subdiv; // int division
                j = id -subdiv*n;
            } else {
                id -= emax;
                if (id < 20*vmax) {
                    type = 'F';
                    n =  id/fmax; // int division
                    j = id - n*fmax;
                } else {
                    //err
                    iResult = -1;
                }
            }
        }
        if (iResult == 0) {
            printf("type %c; n:%d, j %d\n", type, n, j);
        } else {
            printf("id to high (%d >= %d)\n", id, vmax+emax+20*fmax);
        }
        
    } else {
        printf("%s <subidiv> <id>\n", apArgV[0]);
    }
    return iResult;
}
