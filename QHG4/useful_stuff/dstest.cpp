#include <cstdio>

#include "xha_strutilsT.h" 
#include "DepScanner.h"

#include <regex>

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    std::string sCalleeDir   = "";
    std::string sCalleeSuffs = "";
    std::string sCallerDir   = "";
    std::string sCallerSuffs = "";

    if (iArgC > 1) {
        sCalleeDir = apArgV[1];
        size_t iPos1 = sCalleeDir.find(":");
        if (iPos1 != std::string::npos) {
            sCalleeSuffs = sCalleeDir.substr(iPos1+1);
            sCalleeDir = sCalleeDir.substr(0, iPos1);
        } else {
            sCalleeSuffs = "";
        }

        if (iArgC > 2) {
            sCallerDir = apArgV[2];
            size_t iPos2 = sCallerDir.find(":");
            if (iPos2 != std::string::npos) {
                sCallerSuffs = sCallerDir.substr(iPos2+1);
                sCallerDir = sCallerDir.substr(0, iPos2);
            } else {
                sCallerSuffs = "";
            }

        } else {
            sCallerDir   = sCalleeDir;
            sCallerSuffs = sCalleeSuffs;
        }

            
        xha_printf("dirsA caller [%s], callee [%s]\n", sCallerDir, sCalleeDir);
        xha_printf("dirsA caller [%s], callee [%s]\n", sCallerDir.c_str(), sCalleeDir.c_str());
    

        DepScanner *pDS = DepScanner::createInstance(sCalleeDir, sCalleeSuffs, sCallerDir, sCallerSuffs);

        if (pDS != NULL) {
            iResult = pDS->find_all_deps();
            if (iResult == 0) {
                pDS->show_deps();
            }
            delete pDS;
        }
    
    } else {
        printf("%s - find dependencies\n", apArgV[0]);
        printf("usage:n");
        printf("  %s <callee-dir>[:<callee-suffs>] [<caller-dir>[:<caller-suffs>]]\n", apArgV[0]);
        printf("where\n");
        printf("  callee-dir    directory containing potentially referenced files\n");
        printf("  callee-suffs  '+'-separated lest of suffices to consider ('cpp','h','py','sh')\n");
        printf("  caller-dir    directory containing file potentially referencing files from <callee-dir>\n");
        printf("  caller-suffs  '+'-separated lest of suffices to consider ('cpp','h','py','sh')\n");
        printf("If <caller-dir> is omitted, <callee-dir> is used\n");
    }

    /*
    const char sPat[]  = ".*hyb_extract .*";
    const char sLine[] = "${QHG_DIR}/useful_stuff/hyb_extract -p ${f} -g ${grid_file} -n ${item_type} -o ${out_body} -l ${loc_file}  ${head}";
  
    if (std::regex_match(sLine, std::regex(sPat))) {
        printf("match!\n");
    } else {
        printf("no match!\n");
    }
    
    */
    return iResult;
}
