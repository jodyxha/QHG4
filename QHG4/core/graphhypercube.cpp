#include <cstdio>
#include <cstdlib>

#include "GraphDesc.h"


uint jpow(uint x, uint y) {
    uint y0 = y;
    uint p = x;
    if (y == 0) {
        p = 1;
    } else {
        if (y > 1) {
            uint q = jpow(x, y/2);
            p = q*q;
            if (y%2 != 0) {
                p *= x;
            }
        }
    }
    return p;
}

uint count_bits(uint n) {
    uint count = 0;

    while (n > 0) {
        count += (n & 1);
        n >>= 1;
    }
    return count;
}

void flip_bits(uint n, uint w, std::vector<uint> &vconns) {
    uint p = 1;
    while (w > 0) {
        vconns.push_back(n ^ p);
        p <<= 1;
        w--;
    }
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    int dim = atoi(apArgV[1]);
    if (dim > 0) {
        printf("%s\n", HEADER_LINE.c_str());
        uint n = jpow(2,dim);
        //printf("num cells %d\n", n);
        printf("%s%d\n", NUMNODE_PREFIX.c_str(), n);
        uint iMax = 0;
        for (uint i = 0; i < n; i++) {
            std::vector<uint> v;
            flip_bits(i, dim, v);
            if (iMax < v.size()) {
                iMax = v.size();
            }
        }
        printf("%s%d\n", MAXLINKS_PREFIX.c_str(), iMax);

        printf("%s\n", NODES_BEGIN.c_str());
        uint i  = 0;
        for (i = 0; i < n; i++) {
            printf(" %d", i);
            if ((i+1)%8 == 0) {
                printf("\n");
            }
        }
        if ((i+1)%8 != 0) {
            printf("\n");
        }
        printf("%s\n", NODES_END.c_str());



        printf("%s\n", LINKS_BEGIN.c_str());
        for (uint i = 0; i < n; i++) {
            std::vector<uint> v;
            flip_bits(i, dim, v);
            printf(" %d: ", i);
            for (uint j = 0; j < v.size(); j++) {
                printf(" %d", v[j]);
            }
            printf("\n");
        }
        printf("%s\n", LINKS_END.c_str());

    }
    return iResult;
}

