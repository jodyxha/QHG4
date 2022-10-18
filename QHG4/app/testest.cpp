#include <cstdio>
#include <cstring>
#include <string>

#include "stdstrutilsT.h"

#define PW_NONE            0
#define PW_AGENTS_ONLY     1
#define PW_STATS_ONLY      2
#define PW_ADDITIONAL_ONLY 4
#define PW_ALL             7

int strp_old(std::string sSub, std::string &sD) {
    int iWS = PW_NONE;

    const char *pSpecial = strpbrk(sSub.c_str(), "#%~*");
    sD = "_";
                    
    if (pSpecial != NULL) {
        const char *p = strchr(pSpecial, '*');
        if (p != NULL) {
            iWS = PW_ALL;
            sD += "PMA";
        } else {  
            p = strpbrk(sSub.c_str(), "#%~");
            while (p == strpbrk(p, "#%~")) {
                switch(*p) {
                case '#':
                    sD += "P";
                    iWS |= PW_AGENTS_ONLY;
                    break;
                case '%':
                    sD += "M";
                    iWS |= PW_STATS_ONLY;
                    break;
                case '~':
                    sD += "A";
                    iWS |= PW_ADDITIONAL_ONLY;
                    break;
                }
                p++;
            }
        }
        
    } else {
        iWS = PW_ALL;
        sD = "";
    }
    return iWS;
}


int strp_new(std::string sSub, std::string &sD) {
    int iWS = PW_NONE;

    size_t iPosSpecial = sSub.find_first_of("#%~*");
    sD = "_";
                    
    if (iPosSpecial != std::string::npos) {
        size_t iStar = sSub.find_first_of('*', iPosSpecial);
        if (iStar != std::string::npos) {
            iWS = PW_ALL;
            sD += "PMA";
        } else {  

            size_t iPos = sSub.find_first_of("#%~", iPosSpecial);
            while (iPos != std::string::npos) {
                char c = sSub.at(iPos);
                switch(c) {
                case '#':
                    sD += "P";
                    iWS |= PW_AGENTS_ONLY;
                    break;
                case '%':
                    sD += "M";
                    iWS |= PW_STATS_ONLY;
                    break;
                case '~':
                    sD += "A";
                    iWS |= PW_ADDITIONAL_ONLY;
                    break;
                }
                iPos = sSub.find_first_of("#%~", iPos+1);
            }
        }
        
    } else {
        iWS = PW_ALL;
        sD = "";
    }
    return iWS;
}

int main(int iArgC, char *apArgV[]) {
    int iResult;
    
    stringvec vSubs = {"sapiens", "sapiens%", "sapiens#", "sapiens~",
                       "sapiens%#", "sapiens%~", "sapiens#~", 
                       "sapiens#%", "sapiens~%", "sapiens~#", 
                       "sapiens#%~", "sapiens~%#", "sapiens~#%",
                       "sapiens*"};
    std::string sD1;
    std::string sD2;
    for (uint i = 0; i < vSubs.size(); i++) {
        int iWS1 = strp_old(vSubs[i], sD1);
        int iWS2 = strp_old(vSubs[i], sD2);
        stdprintf("result for [%12s]: [%4s], %d  <--> [%4s], %d  %s\n", vSubs[i], sD1, iWS1, sD2, iWS2, ((iWS1==iWS2)&&(sD1==sD2))?"ok":"diff!"); 
    }

    return iResult;
}
