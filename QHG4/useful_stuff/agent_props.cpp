#include <cstdio>

#include <hdf5.h>

#include "qhg_consts.h"
#include "QDFUtils.h"


int main(int iArgC,char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 2) {
        
        hid_t h = qdf_openFile(apArgV[1], "r");
        if (h != H5P_DEFAULT) {
            char sPop[1024];
            sprintf(sPop, "Populations/%s", apArgV[2]);
            if (qdf_link_exists(h, sPop)) {
                hid_t g = qdf_openGroup(h, sPop);
                if (g != H5P_DEFAULT) {
                    hid_t d = qdf_openDataSet(g, "AgentDataSet"); 
                    if (d != H5P_DEFAULT) {
                        hid_t t = H5Dget_type(d);
                        int n = H5Tget_nmembers(t);
                        for (int i = 0; i < n; i++) {
                            hid_t st = H5Tget_member_type(t, i);
                            H5T_class_t c = H5Tget_member_class(t, i);
                            size_t ss = 8*H5Tget_size(st);
                            char s[1024];
                            if (c == H5T_INTEGER)  {
                                sprintf(s,"int_%zd", ss);
                            } else if (c == H5T_FLOAT) {
                                sprintf(s,"float_%zd", ss);
                            } else {
                                sprintf(s, "unsupported class");
                            }
                            printf("%3d: %s (%s)\n", i, H5Tget_member_name(t, i), s);
                        }
                        H5Tclose(t);
                        qdf_closeDataSet(d);

                    } else {
                        printf("Couldn't open dataset [%s] in [%s]\n", "AgentDataSet", sPop);
                    }
                    qdf_closeGroup(g);
                } else {
                    printf("Couldn't open group [%s] in [%s]\n", sPop, apArgV[1]);
                }


            } else {
                printf("No group [%s] found in [%s]\n", sPop, apArgV[1]);
            }
            qdf_closeFile(h);
        } else {
            printf("%s seems no to be a qdf file\n", apArgV[0]);
        }

    } else {
        printf("%s - list agent properties\n", apArgV[0]);
        printf("usage:\n");
        printf("  %s <pop_qdf> <species>\n", apArgV[0]);
    }

    return iResult;
}
