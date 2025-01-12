#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <ctime>


#include "ParamReader.h"
#include "xha_strutils.h"
#include "xha_strutils.cpp"
#include "xha_strutilsT.h"
#include "colors.h"
#include "MessLoggerT.h"
#include "SimLoader.h"
#include "StatusWriter.h"


#define OPT_LOG        "--log-file="

bool s_bCheck;


//-----------------------------------------------------------------------------
// helpParams
//
void helpParams() {
    xha_printf("  -h,                        show help\n");            
    xha_printf("  --help=<topic>             show help for topic (use name of option or \"all\")\n");            
    xha_printf("  --grid=<grid-file>         set grid file\n");     
    xha_printf("  --geo=<geo-file>           set geography file\n");       
    xha_printf("  --climate=<climate-file>   set climate file\n");    
    xha_printf("  --veg=<veg-file>           set vegetation file\n");    
    xha_printf("  --nav=<nav-file>           set navigation file\n");    
    xha_printf("  --pops=<pop-list>          set population files\n");       
    xha_printf("  --output-name=<name>       set output file na,e\n");
    xha_printf("  --data-dirs=<dirnames>     data directories (default: \"./\")\n");    
    xha_printf("  --info-string              information to be written to root group of output files\n");
    xha_printf("  --select=<desc>            string describing which groups t write\n");


}





//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        std::string sLogFile = DEF_LOG_FILE;
  
        // is a log-file set?
        for (int i = 0; i < iArgC; i++) {
            std::string sCur{apArgV[i]};
            if (sCur.find(OPT_LOG) != std::string::npos) {
                size_t iPos = sCur.find("=");
                if (iPos != std::string::npos) {
                    sLogFile = sCur.substr(iPos+1);
                }
                    
            }

        }
        MessLogger::create(sLogFile);
        time_t t = time(NULL);
        LOG_DISP("-----------------------------------------\n");
        LOG_DISP("Starting Logfile %s\n", trim(ctime(&t)));
        LOG_DISP("-----------------------------------------\n");

        SimLoader *pSSim = new SimLoader();
        iResult = pSSim->readOptions(iArgC, apArgV);

        if (iResult == 0) {
  
            pSSim->showInputs();
            xha_printf("trying to merge---\n");
            int iNumMerged = pSSim->mergePops();
            xha_printf("merged %d pops\n", iNumMerged);
            //iResult = pSim->startLoop();
            if (iResult == 0) {
                
                //iResult = pSSim->writeState(WR_POP);
                iResult = pSSim->write();
                if (iResult == 0) {
                    xha_fprintf(stderr, "%s+++success+++%s\n", colors::GREEN, colors::OFF);
                } else {
                    xha_fprintf(stderr, "%s---error in write---%s\n", colors::RED, colors::OFF);
                }
            } else {
                xha_fprintf(stderr, "%s---error in loops---%s\n", colors::RED, colors::OFF);
            }
            
        } else {
            if (iResult == 3) {
                xha_printf("%susage%s: %d\n", colors::RED, colors::OFF, PARAMREADER_ERR_MANDATORY_MISSING);
                xha_printf("%s", colors::RED);
                SimLoader::helpParams();
                xha_printf("%s\n", colors::OFF);
            } else if (iResult == 2) {
                SimLoader::showTopicHelp(pSSim->getHelpTopic());
            } else {
                xha_fprintf(stderr, "%s---error in readOptions---%s\n", colors::RED, colors::OFF);
            }                
        }
        delete pSSim;
       
    } else {
        xha_printf("%susage%s\n", colors::RED, colors::OFF);
        xha_printf("%s", colors::RED);
        SimLoader::helpParams();
        xha_printf("%s\n", colors::OFF);
        
    }
    return iResult;
}
